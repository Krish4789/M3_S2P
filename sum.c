#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <ctime>

using namespace std;

void randomVector(int vector[], int size) {
    for (int i = 0; i < size; i++) {
        vector[i] = rand() % 100;
    }
}

int main(int argc, char** argv) {
    int rank, size;
    const int totalSize = 10000; // Total vector size

    MPI_Init(&argc, &argv);                    // Start MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);      // Get process rank
    MPI_Comm_size(MPI_COMM_WORLD, &size);      // Get total processes

    int chunkSize = totalSize / size;          // Chunk for each process

    int* v1 = nullptr;
    int* v2 = nullptr;
    int* v3 = nullptr;

    // Only the root process allocates and fills the full vectors
    if (rank == 0) {
        v1 = new int[totalSize];
        v2 = new int[totalSize];
        v3 = new int[totalSize];

        srand(time(0));
        randomVector(v1, totalSize);
        randomVector(v2, totalSize);
    }

    // Allocate memory for chunks on all processes
    int* local_v1 = new int[chunkSize];
    int* local_v2 = new int[chunkSize];
    int* local_v3 = new int[chunkSize];

    MPI_Barrier(MPI_COMM_WORLD);  // Ensure all processes start together
    double start_time = MPI_Wtime();

    // Scatter v1 and v2 to all processes
    MPI_Scatter(v1, chunkSize, MPI_INT, local_v1, chunkSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(v2, chunkSize, MPI_INT, local_v2, chunkSize, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform local addition
    for (int i = 0; i < chunkSize; i++) {
        local_v3[i] = local_v1[i] + local_v2[i];
    }

    // Use MPI_Reduce to calculate the total sum of all elements in v3
    int local_sum = 0;
    for (int i = 0; i < chunkSize; i++) {
        local_sum += local_v3[i];
    }

    int total_sum = 0;
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();

    if (rank == 0) {
        cout << fixed << setprecision(0);
        cout << "Time taken with MPI: " << (end_time - start_time) * 1e6 << " microseconds" << endl;
        cout << "Total sum of elements in v3: " << total_sum << endl;
    }

    // Free memory
    delete[] local_v1;
    delete[] local_v2;
    delete[] local_v3;
    if (rank == 0) {
        delete[] v1;
        delete[] v2;
        delete[] v3;
    }

    MPI_Finalize();  // End MPI
    return 0;
}
