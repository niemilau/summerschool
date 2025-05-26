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

[HDF5 file structure. "Groups" are analogous to directories on a Unix-like filesystem, and datasets then correspond to files.](../img/hdf5_structure.png).

The HDF5 API provides functions for creating and manipulating HDF5 files and datasets within them. The API is very flexible, giving the programmer full control over how datasets should be created or accessed. The price to pay for this flexibility is that the API is rather verbose and abstract. For example,

[{caption=caption test}](../img/hdf5_dataset.png)