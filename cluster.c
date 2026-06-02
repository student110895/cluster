#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>





int max_iterations = 400; // Default value for maximum iterations
int K;




typedef struct {
    double *coordinates;
    Cluster *cluster_assignment;
} Vector;



typedef struct {
    int id_number;
    Vector *assigned_vectors;
    int number_of_assigned_vectors;
    Vector current_centroid;
    Vector previous_centroid;
} Cluster;

Cluster *clusters;


int count_dimensions(char *line) {
    int dimensions = 1;
    int i = 0;
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
    vector.coordinates = malloc(dimensions * sizeof(double));
    vector.cluster_assignment = NULL;

    char *current_position = line;
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

void read_lines_from_text() {
    int dimensions = 0;
    char *line = NULL;
    Vector vector;

    size_t len = 0;
    while (getline(&line, &len, stdin) != -1) {
        if (is_empty_line(line)) continue; // Skip empty lines
        if (dimensions == 0) {
            dimensions = count_dimensions(line);
        }
        vector = create_vector_from_line(line, dimensions);

      // Implementation to read data points and create vectors
    }
    free(line);
}



int main(int argc, char *argv[]) {
    int K = atoi(argv[1]);
    if (argc == 3) {
        max_iterations = atoi(argv[2]);
    }
    
    
    while (getline(&)


    // Initialize clusters and vectors
    // Call clustering function
    return 0;
}
