#include <vector>

#include "hdf5.h"
#include <mpi.h>


int main(int argc, char **argv) {

    // Initialize MPI
    int rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // This exercise is serial, ie. we let only rank 0 do stuff
    if (rank == 0) {

        // Create an array of integers. This is 1D, but we will interpret and write it as a 2D matrix by specifying row/column counts.
        // This is a common way of implementing multidimensional arrays of arbitrary shapes.
        const size_t rows = 4;
        const size_t columns = 5;

        std::vector<int> matrix(rows * columns);
        for (int i = 0; i < rows*columns; i++)
        {
            matrix[i] = i;
        }

        // Now create a new HDF5 file. For comparison, recall standard C-style file creation:
        // FILE* filePointer = fopen("my_file", 'w');
        // Here the second argument specifies Access Mode ('w' = write, overriding any existing contents)

        // In HDF5 the file creation function is `H5Fcreate` (https://docs.hdfgroup.org/archive/support/HDF5/doc/RM/RM_H5F.html#File-Create).
        // It takes additional arguments that can be used for advanced configurations; here we just use the defaults (OK for most applications).
        // Note that instead of getting a direct pointer to the created HDF5 file object,
        // the return type `hid_t` is an integer ID for the object ("handle to object" approach).
        hid_t fileId = H5Fcreate(
            "matrix.h5",   // file name
            H5F_ACC_TRUNC,    // "truncate mode", ie. overwrite existing file. Read-write access is always implied
            H5P_DEFAULT,    // Default file creation options
            H5P_DEFAULT     // Default file access options (we explore this in a follow-up exercise)
        );

        // Proceed to write the data as a 2D array. HDF5 separates data shape info from the actual data:
        //  - HDF5 Dataspace: Defines shape and dimensionality of the data, ie. the layout. In our case, shape of our matrix.
        //  - HDF5 Dataset: The actual data. In our case the 1D array.
        // We are required to create the dataspace description first before a dataset can be written.

        // Shape of the dataspace
        hsize_t dims[2] = {rows, columns};
        // Create a dataspace ("simple dataspace" in HDF5 means multidimensional array). We again get an object ID as the return value
        hid_t dataspaceId = H5Screate_simple(
            2,  // 2D dataspace
            dims,    // Shape, ie. how many rows and columns
            NULL // Could set maximum row/column count here, NULL means unspecified
        );

        // Create the dataset, no actual I/O here yet. The last 3 arguments to this function are configuration options for different features of HDF5.
        // The default options are OK for our case, so we just pass H5P_DEFAULT.
        hid_t datasetId = H5Dcreate(
            fileId,          // Which file this dataset will reside in
            "IntegerMatrix", // Name of the dataset
            H5T_NATIVE_INT,  // Specify that the data consists of 'int' types. This way HDF5 knows how to interpret the data on any platform (portability!)
            dataspaceId,     // Dataspace to use for this dataset, ie. data shape.
            H5P_DEFAULT,     // Default link creation options. Advanced feature: "links" in HDF5 behave like symlinks in UNIX
            H5P_DEFAULT,     // Default creation options
            H5P_DEFAULT      // Default access options
        );

        // Perform the actual write. Return value is an error code and will be < 0 if there was an error.
        // We skip error checking here for simplicity.
        herr_t status = H5Dwrite(
            datasetId,      // Dataset to write to
            H5T_NATIVE_INT, // Type of the data, should match the type used when defining the dataset
            H5S_ALL,        // Dataspace describing layout of the memory buffer.
            H5S_ALL,
            H5P_DEFAULT,
            matrix.data()
        );

        // Write some metadata. In HDF5 this is done via Attributes which we "attach" to a dataset.
        // We must first create an Attribute Space, defining the layout of the attribute (much like Dataspace).
        // Here we just write a dummy integer to act as metadata.
        // In a real program this integer could represent eg. the program version used when generating the data.
        int dummyMetadata = 42;

        hid_t attributeSpaceId = H5Screate(H5S_SCALAR); // metadata is of scalar type
        // Create the attribute and associate it with our dataset
        hid_t attributeId = H5Acreate(
            datasetId,                // Handle to the dataset
            "DummyMetadataInteger", // Name of the attribute
            H5T_NATIVE_INT,           // Type of the attribute
            attributeSpaceId,         // Handle to the Attribute Space
            H5P_DEFAULT,              // Default creation options
            H5P_DEFAULT               // Default access options
        );
        // Write the attribute
        status = H5Awrite(attributeId, H5T_NATIVE_INT, &dummyMetadata);

        // Cleanup by closing (deallocating) all HDF5 objects that we created, in reverse order.
        H5Aclose(attributeId);
        H5Sclose(attributeSpaceId);
        H5Dclose(datasetId);
        H5Sclose(dataspaceId);
        H5Fclose(fileId);
    }

    MPI_Finalize();
    return 0;
}
