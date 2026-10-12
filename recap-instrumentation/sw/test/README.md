# Testing

Unit tests are run on the build host and are built and run with CMake/CTest.

To configure the build with CMake for the first time:

```
mkdir build
cd build
cmake ..
```

Then inside the build directory, build tests with `make` and run with `ctest`,
e.g.

```
ctest --output-on-failure
```

Note you have to re-run `make` before running `ctest` if you change any files.
