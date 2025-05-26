## Self study module on HDF5

HDF5 is a file format that has become popular in scientific and industrial computing due to its flexibility, I/O performance and portability.

These notes give an introduction to the HDF5 file format and its C/Fortran API.
We will cover the following:
    - Creating and writing HDF5 datasets into a file
    - Writing metadata for the datasets
    - Investigating HDF5 file contents with command line tools
    - Writing to HDF5 file from multiple MPI processes
    - Using HDF5 hyperslabs to selectively operate on parts of a dataset

You can find the official HDF5 documentation [here](https://support.hdfgroup.org/documentation/hdf5/latest/index.html). Especially useful are the User Guide and Reference Manual tabs.

### Code examples and exercises

The HDF5 API provides functions for creating and manipulating HDF5 files and datasets within them. The API is very flexible, giving the programmer full control over how datasets should be created or accessed. The price to pay for this flexibility is that the programming interface is rather verbose and abstract. For example, many API calls allow the programmer to configure their behavior by passing **HDF5 Property List** objects as function argument, but in many cases the default behavior is sufficient in which case we instead pass `H5P_DEFAULT`.

Most API functions that create HDF5 state (eg. file or dataset creation) return an integer identifier of type `hid_t` to the created resource, instead of returning a direct pointer to it. Likewise, functions that operate on HDF5 objects take in these IDs, or **handles**, as arguments. This is a somewhat common way of hiding implementation details of library objects or structs from the programmer.

There are example programs and code exercises associated with these notes.

TODO:
    - mention here that we use C-style API
    - C++ users may find the C++ bindings useful
    - Mention simplified HDF5 API
    - mention h5py

### HDF5 file structure

HDF5 files are binary files intended for storing arbitrary N-dimensional datasets, where each element in the dataset may itself be a complex object ("heterogeneous data").
There is no limit on how big the datasets can be; HDF5 can hold arbitrarily large amounts of data.
HDF5 has a complex, filesystem-like structure that allows one file to hold many datasets in an organized fashion.

[HDF5 file structure. "Groups" are analogous to directories on a Unix-like filesystem, and datasets then correspond to files.](./img/hdf5_structure.png)



The HDF5 data model separates the **shape** of data from the dataset itself. Data shape (number of rows, columns etc.) in HDF5 is called a **dataspace**. Dataspaces and datasets must be managed separately by the programmer, and creation of a dataset requires a valid dataspace.
    - Analogy from Python: `numpy` arrays and their `numpy.shape` objects.

[Nice image{caption=caption test}](./img/hdf5_dataset.png)

The minimal steps for creating an HDF5 file and writing a dataset to it are as follows:
    1. Create the file using [`H5Fcreate()`](https://docs.hdfgroup.org/archive/support/HDF5/doc/RM/RM_H5F.html#File-Create), with appropriate creation flags and configuration options.
    2. Create a [**dataspace**](https://support.hdfgroup.org/documentation/hdf5/latest/group___h5_s.html#ga8e35eea5738b4805856eac7d595254ae) to represent shape of the data. Usually we are interested in writing N-dimensional arrays; dataspaces corresponding to these are called "simple" in HDF5. A simple dataspace can be created with [`H5Screate_simple()`](https://support.hdfgroup.org/documentation/hdf5/latest/group___h5_s.html#ga8e35eea5738b4805856eac7d595254ae) in which we specify the dimensionality and number of elements along each dimension.
    3. Create a [**dataset**](https://support.hdfgroup.org/documentation/hdf5/latest/_h5_d__u_g.html) by calling [`H5Dcreate()`](https://docs.hdfgroup.org/archive/support/HDF5/doc/RM/RM_H5D.html#Dataset-Create). In this function call we specify which file this dataset is to be created in, type of data that we are storing (eg. integers of floats), and a valid dataspace for defining dataspace shape.
    4. Call [`H5Dwrite()`](https://docs.hdfgroup.org/archive/support/HDF5/doc/RM/RM_H5D.html#Dataset-Write) to write data to into the dataset. We have to specify the target dataspace, type of data to be written and a valid pointer to memory address where the source data resides. We must also pass _two_ dataspaces for specifying shapes of source and target data:
        - A "memory space" or **memspace**, which defines how the source data is aligned in memory. If the source data is contiguous and has same logical shape as the dataspace used by the dataset, we may use the special keyword `H5S_ALL`; otherwise a valid memspace must be manually created (via `H5Screate_simple()`) and passed to `H5Dwrite()`.
        - A "file space", which is an another dataspace object specifying where in the dataset the data should be written to. Passing `H5S_ALL` means we write the full dataset.

This is a lot of programming overhead just for outputting data to a file! For simple writes most of this machinery is indeed unnecessary, but becomes very useful when working with complex or parallel data.

#### Case study: `hdf5-write-matrix`

Have a look at the example program (C or Fortran) in (`hdf5-write-matrix`)[hdf5-write-matrix/]. This program creates a contiguous 1D array and writes it to an HDF5 file as a 2D dataset (a common way of implementing multidimensional arrays is to use a large 1D array and simply interpret it as N-dimensional).