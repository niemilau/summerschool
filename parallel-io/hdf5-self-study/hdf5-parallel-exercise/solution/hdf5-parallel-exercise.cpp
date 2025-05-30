#include <mpi.h>
#include <hdf5.h>

#include <vector>
#include <cassert>

int main(int argc, char** argv) {


    // Initialize MPI
    int rank, ntasks;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);

    // Rank r will write r + 1 integers
    std::vector<int> data(rank + 1, rank);

    hid_t plist = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_mpio(plist, MPI_COMM_WORLD, MPI_INFO_NULL);

    hid_t file = H5Fcreate(
        "stuff.h5",        // file name
        H5F_ACC_TRUNC,        // Truncate mode, read/write access is implied
        H5P_DEFAULT,        // Default creation behavior
        plist               // Non-default File Access behavior to allow MPI-IO
    );

    // Calculate dataspace 1D size
    size_t dataspaceSize = 0;
    for (size_t i = 0; i < ntasks; i++) {
        dataspaceSize += (i + 1);
    }

    hsize_t dims[] = { (hsize_t) dataspaceSize };
    hid_t dataspace = H5Screate_simple(1, dims, NULL);
    hid_t dataset = H5Dcreate(file, "ranks", H5T_NATIVE_INT, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // Calculate hyperslab start for this rank
    hsize_t slabStart = 0;
    for (size_t i = 0; i < rank; i++) {
        slabStart += i + 1;
    }

    const hsize_t start[] = { slabStart };
    const hsize_t count[] = { (hsize_t) (rank + 1) };

    herr_t status = H5Sselect_hyperslab(
        dataspace,    // Dataspace the hyperslab is part of
        H5S_SELECT_SET,     // Selection operation, can be used to modify existing selections. H5S_SELECT_SET just overrides any existing selection
        start,                  // Hyperslab start
        NULL,           // Stride
        count,
        NULL                    // default blocking
    );
    assert(status >= 0 && "Hyperslab selection error");

    hid_t memspace = H5Screate_simple(1, count, NULL);
    H5Dwrite(dataset, H5T_NATIVE_INT, memspace, dataspace, H5P_DEFAULT, data.data());

    H5Sclose(memspace);
    H5Dclose(dataset);
    H5Sclose(dataspace);
    H5Fclose(file);
    H5Pclose(plist);
    MPI_Finalize();

    return 0;
}