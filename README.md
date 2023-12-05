# vtop

## Overview
vtop is a system monitoring tool using GTK to visualize CPU, disk, and RAM/swap usage and processes. It provides a real-time graphical representation of system performance.

## Project Structure
The project tree looks like this:

## Project Tree Structure

```
.
|-- Makefile
|-- README.md
|-- src
|   |-- cpu.c
|   |-- cpu.h
|   |-- disk.c
|   |-- disk.h
|   |-- main.c
|   |-- process.c
|   |-- process.h
|   |-- ram_swap.c
|   |-- ram_swap.h
```


## Installation (GTK for Debian-based OS)

To set up the required GTK development libraries for building and running vtop on Debian-based systems (such as Ubuntu), follow these steps:

1. Open a terminal window.

2. Update the package lists for upgrades and new installations:

    ```bash
    sudo apt update
    ```

3. Install GTK development libraries:

    ```bash
    sudo apt install libgtk-3-dev
    ```

This installs the necessary GTK development libraries needed to compile and run vtop on your Debian-based system.


## Usage
To compile the project, use the provided Makefile. Ensure GTK development libraries are installed.

Run `make` in the root directory to compile the project.

Execute the compiled `vtop` binary to launch the monitoring dashboard.

## Features
- Divides the main window into four sections:
  - CPU graph
  - Disk graph
  - RAM/Swap graph
  - Process view 
- Updates graphs periodically to display real-time system statistics.
