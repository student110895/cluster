#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int max_iterations;
max_iterations = 400; // Default value for maximum iterations
int K;
Vector *vectors;
int number_of_vectors;
int capacity;
capacity = 1000; // To keep track of allocated memory for vectors
int dimensions;
dimensions = 0;
double epsilon = 0.001; // Convergence threshold


typedef struct Vector {
    double *coordinates;
    int cluster_assignment; 
} Vector;


typedef struct {
    int id_number;
    int number_of_assigned_vectors;
    Vector current_centroid;
    Vector previous_centroid;
} Cluster;

Cluster *clusters;


int count_dimensions(char *line) {
    dimensions = 1;
    int i;
    i = 0;
    
    while (line[i] != '\0' && line[i] != '\n') {
        if (line[i] == ',') {
            dimensions++;
        }
        i++;
    }
    return dimensions;
}

Vector create_vector_from_line(char *line, int dimensions) {
    Vector vector;
    vector.coordinates = malloc(dimensions * sizeof(double)); //alocate memory for coordinates
    vector.cluster_assignment = -1;

    char *current_position;
    current_position = line;
    char *end_position;

    for (int i = 0; i < dimensions; i++) {
        vector.coordinates[i] = strtod(current_position, &end_position);
        current_position = end_position;
        if (*current_position == ',') {
            current_position++; // Move past the comma
        }
    }
    return vector;
}


int is_empty_line(char *line) {
    return line[0] == '\n' || line[0] == '\0';
}


void allocate_memory_for_vectors() {
    Vector *temp;
    capacity *= 2; // Double the capacity
        temp = realloc(vectors, capacity * sizeof(Vector));  
        if (temp == NULL || vectors == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            free(vectors);
            free(temp);
            exit(EXIT_FAILURE);
        }
        vectors = temp;
    }  

    
Vector *read_lines_from_text() {
    int dimensions = 0;
    char *line = NULL;
    Vector vector;
    vectors = malloc(capacity * sizeof(Vector)); // Allocate memory for vectors
    number_of_vectors = 0;
    size_t len = 0;
    
    while (getline(&line, &len, stdin) != -1) {
        if (is_empty_line(line)) continue; // Skip empty lines
        if (dimensions == 0) {
            dimensions = count_dimensions(line);
        }
        vector = create_vector_from_line(line, dimensions);
        if (number_of_vectors >= capacity) {
            allocate_memory_for_vectors();
        }
        vectors[number_of_vectors] = vector;
        number_of_vectors++;
    }
    free(line);
    return vectors;
}


Cluster* initialize_clusters() {
    Cluster *clusters = malloc(K * sizeof(Cluster));
    for (int i = 0; i < K; i++) {
        clusters[i].id_number = i;
        clusters[i].number_of_assigned_vectors = 0;
        clusters[i].current_centroid.coordinates = malloc(dimensions * sizeof(double)); 
        clusters[i].previous_centroid.coordinates = malloc(dimensions * sizeof(double));
    }
    return clusters;
}

void copy_vector(Vector *from, Vector *to) {
    for (int i = 0; i < dimensions; i++) {
        to->coordinates[i] = from->coordinates[i];
    }
}


int initialize_centroids() {
    for (int i = 0; i < K; i++) {
        for (int j = 0; j < dimensions; j++) {
            copy_vector(&vectors[i], &clusters[i].current_centroid);
            vectors[i].cluster_assignment = i; 
        }
    }
}

int find_closest_cluster(Vector *vector) {
    int closest_cluster_index = 0;
    double min_distance = calculate_distance(vector, &clusters[0].current_centroid);
    
    for (int i = 1; i < K; i++) {
        double distance = calculate_distance(vector, &clusters[i].current_centroid);
        if (distance < min_distance) {
            min_distance = distance;
            closest_cluster_index = i;
        }
    }
    return closest_cluster_index;
}

void reset_clusters(Cluster *clusters) {
    for (int i = 0; i < K; i++) {
        clusters[i].number_of_assigned_vectors = 0;
    }
}

void assign_vectors_to_clusters(Cluster *clusters, Vector *vectors) {
    reset_clusters(clusters);
    for (int i = 0; i < number_of_vectors; i++) {
        int closest_cluster_index = find_closest_cluster(&vectors[i]);
        clusters[closest_cluster_index].number_of_assigned_vectors++;
        vectors[i].cluster_assignment = closest_cluster_index;
    }
}

void update_centroids(Cluster *clusters, Vector *vectors) {
    for (int i = 0; i < K; i++) {
        for (int j = 0; j < dimensions; j++) {
            clusters[i].previous_centroid.coordinates[j] = clusters[i].current_centroid.coordinates[j];
            clusters[i].current_centroid.coordinates[j] = 0.0;
        }
    }

    for (int i = 0; i < number_of_vectors; i++) {
        int cluster_index = vectors[i].cluster_assignment;
        for (int j = 0; j < dimensions; j++) {
            clusters[cluster_index].current_centroid.coordinates[j] += vectors[i].coordinates[j];
        }
    }

    for (int i = 0; i < K; i++) {
        if (clusters[i].number_of_assigned_vectors > 0) {
            for (int j = 0; j < dimensions; j++) {
                clusters[i].current_centroid.coordinates[j] /= clusters[i].number_of_assigned_vectors;
            }
        }
    }
}

int check_convergence(Cluster *clusters) {
    for (int i = 0; i < K; i++) {
        if (calculate_distance(&clusters[i].current_centroid, &clusters[i].previous_centroid) > epsilon) {
            return 0; // Not converged
        }
    }
    return 1; // Converged
}




double calculate_distance(Vector *v1, Vector *v2) { //* in def is for declaring type.
    double sum = 0.0;
    for (int i = 0; i < dimensions; i++) {
        double diff = v1->coordinates[i] - v2->coordinates[i]; // Use -> to access coordinates since v1 and v2 are pointers
        sum += diff * diff;
    }
    return sqrt(sum);
}

void clustering() {
    initialize_clusters();
    initialize_centroids();
    int iterations = 0;
    while (iterations < max_iterations) {
        assign_vectors_to_clusters(clusters, vectors);
        update_centroids(clusters, vectors);
        if (check_convergence(clusters)) {
            break; // Converged
        }
        iterations++;
    }
}

void print_centroids() {
for (int i = 0; i < K; i++) {
        for (int j = 0; j < dimensions; j++) {
            printf("%.4f", clusters[i].current_centroid.coordinates[j]);
            
            // If it is NOT the last dimension, print a comma
            if (j < dimensions - 1) {
                printf(",");
            }
        }
        // After printing all dimensions for one centroid, print a newline
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    int K = atoi(argv[1]);
    if (argc == 3) {
        max_iterations = atoi(argv[2]);
    }
    if (K <= 0 || max_iterations <= 0) {
        fprintf(stderr, "Error: K and max_iterations must be positive integers.\n");
        return EXIT_FAILURE;
    }
    read_lines_from_text();
    clustering();
    print_centroids();


    
    


    // Initialize clusters and vectors
    // Call clustering function
    return 0;
}
