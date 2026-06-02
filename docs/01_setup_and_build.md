# ChampSim Setup and Build Notes

## Repository Setup

The official ChampSim repository is kept as `upstream`.

My own GitHub repository is kept as `origin`.

## Build Fix: CLI11 Link Flag

On this WSL setup, the build failed at the final linking stage with:

```text
/usr/bin/ld: cannot find -lCLI11
