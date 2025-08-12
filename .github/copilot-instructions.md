# Copilot Instructions for Showdown-Port

Welcome to the `Showdown-Port` project! This document provides guidance for AI coding agents to be productive in this codebase. The project is a performance-focused port of the Pokémon Showdown simulator to C, targeting applications in reinforcement learning. The current focus is on Gen1OU, with potential scaling for additional generations.

## Project Overview

### Key Components
- **`data_readable/`**: Contains header files with readable labels for moves and Pokémon data.
  - Example: `move_labels.h`, `pokedex_labels.h`
- **`data_sim/`**: Includes core simulation data structures and enums.
  - Example: `move_enum.h`, `pokedex.h`, `pokemon_base.h`
- **`sim/`**: Implements the simulation logic.
  - Example: `sim.c`, `battle.h`, `pokemon.h`

### Data Flow
1. **Input**: Data is read from `data_readable/` and `data_sim/`.
2. **Processing**: Simulation logic in `sim/` processes the data.
3. **Output**: Results are generated for reinforcement learning applications.

## Developer Workflows

### Building the Project
- Use a C compiler (e.g., `gcc`) to build the project.
- Example command:
  ```bash
  gcc -o sim sim/sim.c
  ```

### Testing
- No explicit test framework is set up. Add test cases manually in `sim.c` or other files.

### Debugging
- Use `gdb` for debugging.
- Example command:
  ```bash
  gdb ./sim
  ```

## Project-Specific Conventions

- **Header Files**: Organized by purpose (`data_readable/` for labels, `data_sim/` for enums and base data).
- **Simulation Logic**: Centralized in `sim/`.
- **Performance Focus**: Avoid unnecessary abstractions to maintain high performance.

## Integration Points

- **Reinforcement Learning**: Outputs are designed to integrate with RL frameworks.
- **Pokémon Showdown**: Data and logic are inspired by the original simulator.

## Examples

### Adding a New Move
1. Update `move_enum.h` in `data_sim/`.
2. Add logic in `sim.c` to handle the move.

### Modifying Pokémon Data
1. Update `pokedex.h` in `data_sim/`.
2. Ensure changes are reflected in `pokemon_base.h`.

---

Feel free to iterate on this document as the project evolves. Feedback is welcome!
