#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>


int max_iterations = 400; 
int capacity = 1000;      
int dimensions = 0;
double epsilon = 0.001 * 0.001;  /*allows us not to calculate sqrt */ 
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

    vector.coordinates = malloc(dimensions * sizeof(double)); 
    vector.cluster_assignment = -1;

     if (vector.coordinates == NULL) {
        return vector;
    }
    
    for (i=0; i < dimensions; i++) {
        vector.coordinates[i] = strtod(current_position, &end_position);

        /* If strtod couldn't read a number, end_position doesn't move */
        if (current_position == end_position) {
            free(vector.coordinates);
            vector.coordinates = NULL; /* Signal failure */
            return vector;
        }

        current_position = end_position;
        if (*current_position == ',') {
            current_position++; 
        }
    }
    return vector;
}


int is_empty_line(char *line) {
    return line[0] == '\n' || line[0] == '\0' || line[0] == '\r';
}



int allocate_memory_for_vectors() {
    Vector *temp;
    capacity *= 2; 
    temp = realloc(vectors, capacity * sizeof(Vector));  
    if (temp == NULL) { 
        /* Just return failure, let main handle the printing and freeing! */
        return 0; 
    }
    vectors = temp;
    return 1; 
}

int read_lines_from_text() {
    char *line = NULL;
    Vector vector;
    size_t len = 0;
    int allocation_result;

    vectors = malloc(capacity * sizeof(Vector)); 
    if (vectors == NULL) {
        return 0; /* Instantly fail if the first allocation fails */
    }
    
    number_of_vectors = 0;
    
    while (getline(&line, &len, stdin) != -1) {
        if (is_empty_line(line)) continue; /* Skip empty lines */   
        
        if (dimensions == 0) {
            dimensions = count_dimensions(line);
        }
        
        vector = create_vector_from_line(line, dimensions);

        
        /* Catch malformed data and fail out safely */
        if (vector.coordinates == NULL) {
            free(line);
            return 0; /* Return 0 on failure so main can print the error */
        }
        
        if (number_of_vectors >= capacity) {
            allocation_result = allocate_memory_for_vectors();
            if (allocation_result == 0) {
                free(line);
                return 0; /* Return 0 on failure so main can handle it */
            }
        }
        
        vectors[number_of_vectors] = vector;
        number_of_vectors++;
    }
    free(line);
    return 1; /* Return 1 on success */
}   


double calculate_distance(Vector *v1, Vector *v2) { /* in def is for declaring type.*/
    double sum = 0.0;
    double diff;
    int i;

    for (i = 0; i < dimensions; i++) {
        diff = v1->coordinates[i] - v2->coordinates[i]; /* Use -> to access coordinates since v1 and v2 are pointers */
        sum += diff * diff;
    }

    return sum; /* Return squared distance to avoid unnecessary sqrt calculations */
}


void free_clusters() {
    int i;
    for (i = 0; i < K; i++) {
        free(clusters[i].current_centroid.coordinates);
        free(clusters[i].previous_centroid.coordinates);
    }
    free(clusters);
}

int initialize_clusters() {
    int i;
    clusters = calloc(K, sizeof(Cluster));
    
    if (clusters == NULL) {
        return 0; /* Failure */
    }

    for (i = 0; i < K; i++) {
        clusters[i].id_number = i;
        clusters[i].number_of_assigned_vectors = 0;
        clusters[i].current_centroid.coordinates = malloc(dimensions * sizeof(double)); 
        clusters[i].previous_centroid.coordinates = malloc(dimensions * sizeof(double));
        
        if (clusters[i].current_centroid.coordinates == NULL || 
            clusters[i].previous_centroid.coordinates == NULL) {
            free_clusters(); /* Free any previously allocated memory */
            return 0; /* Failure */
        }
    }
    return 1; /* Success */
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
            clusters[i].previous_centroid.coordinates[j] =
                clusters[i].current_centroid.coordinates[j];

            if (clusters[i].number_of_assigned_vectors > 0) {
                clusters[i].current_centroid.coordinates[j] = 0.0;
            }
        }
    }

    for (i = 0; i < number_of_vectors; i++) {
        cluster_index = vectors[i].cluster_assignment;
        for (j = 0; j < dimensions; j++) {
            clusters[cluster_index].current_centroid.coordinates[j] +=
                vectors[i].coordinates[j];
        }
    }

    for (i = 0; i < K; i++) {
        if (clusters[i].number_of_assigned_vectors > 0) {
            for (j = 0; j < dimensions; j++) {
                clusters[i].current_centroid.coordinates[j] /=
                    clusters[i].number_of_assigned_vectors;
            }
        }
    }
}
int check_convergence(Cluster *clusters) {
    int i;
    for (i = 0; i < K; i++) {
        if (calculate_distance(&clusters[i].current_centroid, &clusters[i].previous_centroid) > epsilon) {
            return 0; /* Not converged */
        }
    }
    return 1; /* Converged */
}


int clustering() {
    int iterations = 0;
    
    /* If initialization fails, bubble the 0 back to main! */
    if (initialize_clusters() == 0) {
        return 0; 
    }
    
    initialize_centroids();

    while (iterations < max_iterations) {
        assign_vectors_to_clusters(clusters, vectors);
        update_centroids(clusters, vectors);
        if (check_convergence(clusters)) {
            break; 
        }
        iterations++;
    }
    return 1; /* Success */
}

void print_centroids() {
    int i, j;
for (i = 0; i < K; i++) {
        for (j = 0; j < dimensions; j++) {
            printf("%.4f", clusters[i].current_centroid.coordinates[j]);
            
            /* If it is NOT the last dimension, print a comma   */
            if (j < dimensions - 1) {
                printf(",");
            }
        }
        /* After printing all dimensions for one centroid, print a newline  */
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




int is_valid_whole_number(char *str) {
    char *endptr;
    double val;
    long truncated_val;

    if (str == NULL || str[0] == '\0') {
        return 0;
    }

    /* Convert string to a double */
    val = strtod(str, &endptr);

    /* If strtod couldn't read a number, or it hit letters/garbage at the end */
    if (str == endptr || *endptr != '\0') {
        return 0;
    }

    /* Truncate the decimal by casting it to a long integer */
    truncated_val = (long)val;

    /* Check if the original double matches the truncated version */
    if (val != truncated_val) {
        return 0;
    }

    return 1;
}


int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        printf("An Error Has Occurred\n"); 
        return 1; 
    }
    
    if (!is_valid_whole_number(argv[1])) {
        printf("Incorrect number of clusters!\n");
        return 1;
    }
    K = atoi(argv[1]); 
    
    /* Safely validate and parse max_iterations if provided */
    if (argc == 3) {
        if (!is_valid_whole_number(argv[2])) {
            printf("Incorrect maximum iteration!\n");
            return 1;
        }
        max_iterations = atoi(argv[2]);

        if (max_iterations <= 1 || max_iterations >= 800) {
            printf("Incorrect maximum iteration!\n");
            return 1; 
        }
    }

    if (read_lines_from_text() == 0 || vectors == NULL) {
        printf("An Error Has Occurred\n");
        free_vectors();
        return 1; 
    }

    /* Validate K now that we know number_of_vectors */
    if (K <= 1 || K >= number_of_vectors) {
        printf("Incorrect number of clusters!\n");
        free_vectors();
        return 1; 
    }

    /* Catch clustering memory failures */
    if (clustering() == 0) {
        printf("An Error Has Occurred\n");
        free_vectors();
        /* clusters are already freed inside initialize_clusters on failure */
        return 1;
    }

    print_centroids();

    /* Final clean shutdown */
    free_clusters(); 
    free_vectors();           
    return 0;
}
