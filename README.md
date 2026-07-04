# ZVM (Z Virtual Machine) - v1.3

A lightweight, robust Virtual Machine written in C, featuring a custom dynamic memory architecture. The project decouples the CPU execution cycle from static memory constraints by utilizing a specialized dynamic block manager (`Blobify`) for its core segments.

---

## Architecture & Features

The ZVM is designed to mirror real hardware behavior through abstract, managed blocks of raw bytes:

* **Dynamic Memory Blobs (`Blobify`):** Replaced legacy static arrays with elastic, bounds-checked `blb_blob_t` allocations for critical memory zones.
* **Separated Memory Segments:** * **Data Segment (RAM):** Dynamically allocated virtual RAM supporting secure read/write boundaries.
  * **Stack Segment:** Implements a dynamic, inverted hardware stack that grows downward with integrated overflow and underflow protection.
* **Streamlined Pipeline:** Complete 4-byte fetch, decode, and execute cycle operating directly over opcodes.

---

## Project Structure

The project layout is modular and clean, utilizing header-based routing to allow single-file compilation:

* `main.c` - Entry point containing the compiled bytecode array (assembly instructions).
* `zvm.h` / `zvm.c` - Core VM loop management (Fetch, Decode, Execute).
* `zvm_types.h` - Virtual hardware definition and registers configuration.
* `zvm_instruction.c` - Implementation of bytecode instruction handlers (e.g., LDI, PUSH, POP, STR).
* `zvm_io.c` - Abstracted hardware device handlers (Screen, Keyboard).
* `blobify.c` / `blobify.h` - Custom dynamic block allocation and memory protection unit.

---

## Building and Running

Thanks to the automated branching inside `zvm.h`, you do not need to link multiple C files in your command. You can compile the entire architecture by targeting just the `main.c` file.

### Prerequisites
Make sure you have `gcc` (or any standard C compiler) installed on your system.

### Compilation
Open your terminal inside the project directory and run:

```bash
gcc main.c -o zvm_app
