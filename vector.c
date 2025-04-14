#include <chrono>
#include <mpi.h>
#include <cstdlib>
#include <iostream>
#include <time.h>

using namespace std;

void randomVector(int vector[], int size) 
{
    for (int i = 0; i < size; i++) 
    {
        vector[i] = rand() % 100;
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    const int total_size = 1000;
    const int chunk_size = total_size / num_procs;
    
    // Allocate memory for local chunks
    int* chunk_v1 = new int[chunk_size];
    int* chunk_v2 = new int[chunk_size];
    int* chunk_result = new int[chunk_size];

    // Root process generates full vectors
    int* v1 = nullptr;
    int* v2 = nullptr;
    int* result = nullptr;
    
    if (rank == 0) {
        srand(time(0));
        v1 = new int[total_size];
        v2 = new int[total_size];
        result = new int[total_size];
        randomVector(v1, total_size);
        randomVector(v2, total_size);
    }

    auto start = chrono::high_resolution_clock::now();

    // Distribute data by scattering v1 and v2 
    MPI_Scatter(v1, chunk_size, MPI_INT, chunk_v1, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(v2, chunk_size, MPI_INT, chunk_v2, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Parallel computation
    for (int i = 0; i < chunk_size; i++) {
        chunk_result[i] = chunk_v1[i] + chunk_v2[i];
    }

    // Collect results
    MPI_Gather(chunk_result, chunk_size, MPI_INT, result, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    auto stop = chrono::high_resolution_clock::now();

    if (rank == 0) {
        auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
        cout << "Parallel Execution time: " << duration.count() << " microseconds" << endl;
        delete[] v1;
        delete[] v2;
        delete[] result;
    }

    delete[] chunk_v1;
    delete[] chunk_v2;
    delete[] chunk_result;

    MPI_Finalize();
    return 0;
}
