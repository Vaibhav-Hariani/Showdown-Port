A port of the pokemon showdown simulator to C. Performance focused for applications in reinforcement learning.

Currently building with a target of Gen1OU. Depending on time and training constraints, will scale for additional generations.

## Generating files

Some files are generated programmatically because they are fancy lookup tables.
To generate them, run
```sh
python3 data/write_move_array.py
```

## Building

For convenience, all files are in one directory.
We can make the build system fancier in the future.
Until then:

```sh
cd sim
clang sim.c -Wall -Wextra -O2 -std=c11
```

You can also build with debug logs:
```sh
cd sim
clang sim.c -Wall -Wextra -O2 -std=c11 -DDEBUG=1
```
