#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int max_iterations = 400; 
int capacity = 1000;      
int dimensions = 0;
double epsilon = 0.001;   
int K;
int number_of_vectors;

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

Cluster *clusters = NULL;
Vector *vectors = NULL;

int count_dimensions(char *line) {
    int i = 0;
    dimensions = 1;
    
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
    char *current_position = line;
    char *end_position;
    int i;

    vector.coordinates = malloc(dimensions * sizeof(double)); //alocate memory for coordinates
    vector.cluster_assignment = -1;

    for (i=0; i < dimensions; i++) {
        vector.coordinates[i] = strtod(current_position, &end_position);
        current_position = end_position;
        if (*current_position == ',') {
            current_position++; // Move past the comma
        }
    }
    return vector;
}


int is_empty_line(char *line) {
    return line[0] == '\n' || line[0] == '\0' || line[0] == '\r';
}


int allocate_memory_for_vectors() {
    Vector *temp;
    capacity *= 2; // Double the capacity
        temp = realloc(vectors, capacity * sizeof(Vector));  
        if (temp == NULL || vectors == NULL) {
            printf("An Error Has Occurred\n");
            free(vectors);
            free(temp);
            return 0; // Indicate failure
        }
        vectors = temp;
        return 1; // Indicate success
    }  

    
Vector *read_lines_from_text() {
    char *line = NULL;
    Vector vector;
    size_t len = 0;
    int allocation_result;

    vectors = malloc(capacity * sizeof(Vector)); // Allocate memory for vectors
    number_of_vectors = 0;
    
    
    while (getline(&line, &len, stdin) != -1) {
        if (is_empty_line(line)) continue; // Skip empty lines
        if (dimensions == 0) {
            dimensions = count_dimensions(line);
        }
        vector = create_vector_from_line(line, dimensions);
        if (number_of_vectors >= capacity) {
            allocation_result = allocate_memory_for_vectors();
            if (allocation_result==0) {
                free(line);
                return NULL; // Exit if memory allocation fails
            }
        }
        vectors[number_of_vectors] = vector;
        number_of_vectors++;
    }
    free(line);
    return vectors;
}

double calculate_distance(Vector *v1, Vector *v2) { //* in def is for declaring type.
    double sum = 0.0;
    double diff;
    int i;

    for (i = 0; i < dimensions; i++) {
        diff = v1->coordinates[i] - v2->coordinates[i]; // Use -> to access coordinates since v1 and v2 are pointers
        sum += diff * diff;
    }
    return sqrt(sum);
}

void initialize_clusters() {
    int i;
    clusters = malloc(K * sizeof(Cluster));
    for (i = 0; i < K; i++) {
        clusters[i].id_number = i;
        clusters[i].number_of_assigned_vectors = 0;
        clusters[i].current_centroid.coordinates = malloc(dimensions * sizeof(double)); 
        clusters[i].previous_centroid.coordinates = malloc(dimensions * sizeof(double));
    }
}

void copy_vector(Vector *from, Vector *to) {
    int i;
    for (i = 0; i < dimensions; i++) {
        to->coordinates[i] = from->coordinates[i];
    }
}


void initialize_centroids() {
    int i;
    for (i = 0; i < K; i++) {
        copy_vector(&vectors[i], &clusters[i].current_centroid);
        vectors[i].cluster_assignment = i; 
    }
}

int find_closest_cluster(Vector *vector) {
    int closest_cluster_index = 0;
    double min_distance;
    double distance;
    int i;

    min_distance = calculate_distance(vector, &clusters[0].current_centroid);
    for (i = 1; i < K; i++) {
        distance = calculate_distance(vector, &clusters[i].current_centroid);
        if (distance < min_distance) {
            min_distance = distance;
            closest_cluster_index = i;
        }
    }
    return closest_cluster_index;
}

void reset_clusters(Cluster *clusters) {
    int i;
    for (i = 0; i < K; i++) {
        clusters[i].number_of_assigned_vectors = 0;
    }
}

void assign_vectors_to_clusters(Cluster *clusters, Vector *vectors) {
    
    int i;
    int closest_cluster_index;
    reset_clusters(clusters);

    for (i = 0; i < number_of_vectors; i++) {
        closest_cluster_index = find_closest_cluster(&vectors[i]);
        clusters[closest_cluster_index].number_of_assigned_vectors++;
        vectors[i].cluster_assignment = closest_cluster_index;
    }
}

void update_centroids(Cluster *clusters, Vector *vectors) {
    int i, j;
    int cluster_index;

    for (i = 0; i < K; i++) {
        for (j = 0; j < dimensions; j++) {
            clusters[i].previous_centroid.coordinates[j] = clusters[i].current_centroid.coordinates[j];
            clusters[i].current_centroid.coordinates[j] = 0.0;
        }
    }

    for (i = 0; i < number_of_vectors; i++) {
        cluster_index = vectors[i].cluster_assignment;
        for (j = 0; j < dimensions; j++) {
            clusters[cluster_index].current_centroid.coordinates[j] += vectors[i].coordinates[j];
        }
    }

    for (i = 0; i < K; i++) {
        if (clusters[i].number_of_assigned_vectors > 0) {
            for (j = 0; j < dimensions; j++) {
                clusters[i].current_centroid.coordinates[j] /= clusters[i].number_of_assigned_vectors;
            }
        }
    }
}

int check_convergence(Cluster *clusters) {
    int i;
    for (i = 0; i < K; i++) {
        if (calculate_distance(&clusters[i].current_centroid, &clusters[i].previous_centroid) > epsilon) {
            return 0; // Not converged
        }
    }
    return 1; // Converged
}


void clustering() {
    int iterations = 0;
    initialize_clusters();
    initialize_centroids();

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
    int i, j;
for (i = 0; i < K; i++) {
        for (j = 0; j < dimensions; j++) {
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

void free_vectors() {
    int i;
    for (i = 0; i < number_of_vectors; i++) {
        free(vectors[i].coordinates);
    }   
    free(vectors);       
}

void free_clusters() {
    int i;
    for (i = 0; i < K; i++) {
        free(clusters[i].current_centroid.coordinates);
        free(clusters[i].previous_centroid.coordinates);
    }
    free(clusters);
}
int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        printf("An Error Has Occurred\n"); // 
        return 1; // 
    }
    K = atoi(argv[1]);
    if (argc == 3) {
        max_iterations = atoi(argv[2]);
    }

    if (max_iterations <= 1 || max_iterations >= 800) {
        printf("Incorrect maximum iteration!\n"); // [cite: 18]
        return 1; // 
    }

    read_lines_from_text();
    if (vectors == NULL) {
        free_vectors();
        printf("An Error Has Occurred\n");
        return 1; // Exit if reading vectors failed
    }

    if (K <= 1 || K >= number_of_vectors) {
        printf("Incorrect number of clusters!\n"); // [cite: 18]
        free_vectors();
        return 1; // 
    }

    clustering();
    print_centroids();

    free_clusters(); 
    free_vectors();           
    return 0;
}
