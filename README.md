# sh

Miniature shell for Linux written in C with `PATH` lookup, `cd` support

## Running shell

```bash
make run
```

## Running tests

```bash
mkdir build
cd build
cmake ..
cmake --build .
ctest --output-on-failure
```
