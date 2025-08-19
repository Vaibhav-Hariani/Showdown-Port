# Copilot Instructions for Showdown-Port

This guide enables AI coding agents to be productive in the Showdown-Port codebase, a C port of the Pokémon Showdown simulator focused on performance and reinforcement learning applications.

## Big Picture Architecture
- **C Simulator (`Showdown-Port/`)**: Core battle logic, data structures, and simulation engine in C. Targeting Gen1OU, with plans for more generations.
- **Data Files**: Pokémon, moves, and type data are in `data_sim/` and `data_labels/`. These are generated or scraped from external sources (see `generators/`).
- **Python Generators**: Scripts in `generators/` automate creation of C header files from raw CSVs in `raw_data/`.
- **Node.js Reference (`sim/` in pokemon-showdown)**: TypeScript implementation for reference and cross-validation. Protocols and data formats are documented in `SIMULATOR.md` and `SIM-PROTOCOL.md`.

## Developer Workflows
- **Build**: Use standard C build tools (e.g., `gcc`). No custom build scripts detected; check for Makefiles if present.
- **Data Generation**: Run Python scripts in `generators/` to update C headers when raw data changes. Example:
  ```bash
  python3 generators/scrape_serebii.py
  python3 generators/write_learnsets.py
  python3 generators/write_move_array.py
  ```
- **Testing**: No explicit test suite found. Validate by running the simulator and comparing outputs to the Node.js reference implementation.
- **Debugging**: Use standard C debugging tools (e.g., `gdb`). For protocol debugging, compare against logs and documentation in `SIMULATOR.md` and `SIM-PROTOCOL.md`.

## Project-Specific Conventions
- **Data Flow**: Raw CSVs → Python generators → C header files → Simulator logic.
- **Battle Protocol**: Follows Pokémon Showdown's text-based protocol. See `SIMULATOR.md` and `SIM-PROTOCOL.md` for message formats and examples.
- **File Naming**: Generated files are prefixed with `generated_` in `data_sim/`. Labels and enums are separated for clarity.
- **Comments**: Use C-style comments in C files. Python scripts use standard docstrings and inline comments.

## Integration Points
- **External Data**: Pokémon/move data scraped from Serebii and other sources via Python scripts.
- **Cross-Component Communication**: Simulator logic in C mirrors the Node.js reference, enabling validation and protocol compatibility.
- **Reinforcement Learning**: Performance optimizations are prioritized for RL use cases; see comments in core C files for details.

## Key Files & Directories
- `sim/` (C): Core simulator logic (`battle.h`, `pokemon.h`, `sim.c`, etc.)
- `data_sim/`: Generated C headers for moves, Pokémon, types
- `generators/`: Python scripts for data generation
- `raw_data/`: Source CSVs for moves and learnsets
- `sim/` (Node.js): Reference implementation and protocol docs (`SIMULATOR.md`, `SIM-PROTOCOL.md`)

## Example Patterns
- **Adding a New Generation**: Update raw CSVs, run generators, update C logic to handle new data.
- **Validating Protocol**: Compare C simulator output to Node.js reference using documented message formats.

---

If any section is unclear or missing details, please provide feedback or specify which workflows, conventions, or integration points need further documentation.
