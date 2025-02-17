# Intel8085-Simulator

## Overview
The **Intel8085-Simulator** is a under work (⚠️) project, aimed at creating a simulator for the **Intel 8085 microprocessor**. It allows users to run binary files containing Intel 8085 opcodes and observe their execution.

## How It Works
1. **Input Binary File**: Users or developers provide a binary file containing Intel 8085 opcodes. This file represents the program they want to simulate.
2. **Simulation Process**:
    - The program is loaded starting from 0x0, so all address should be with respect to 0x0
    - The Stack is the last 1kB of memory
    - The simulator reads the binary file and interprets the opcodes.
    - It emulates the execution of instructions step by step.
    - Registers, memory, and other components of the 8085 microprocessor are updated accordingly.
4. **Output and Debugging**:
    - Users can observe the state of registers, memory, and other relevant components.
    - **Note**: Some opcodes may not be fully implemented yet.

## Features:
- **Instruction Execution**: Simulates the execution of Intel 8085 instructions.
- **Register State**: Displays the current state of registers (accumulator, flags, etc.).

## Supported Opcodes:
- RED = <span style='color: red;'>NOT IMPLEMENTED</span>.
- YELLOW = <span style='color: yellow;'>IMPLEMENTED but NOT TESTED</span>.
- GREEN = <span style='color: green;'>IMPLEMENTED & TESTED</span>.
![opcodes-table-of-intel-8085 (1)](https://github.com/user-attachments/assets/6fb450fa-254b-4f3e-94f8-40f85f0aa92c)
 


## Usage
1. **Clone the Repository**:
    ```
    git clone https://github.com/akshatd120/Intel8085-Simulator.git
    ```
2. **Compile and Run**:
  Open the Intel-8085-Sim.sln in Visual Studio 22 and build!

## Screenshot of execution
![image](https://github.com/akshatd120/Intel8085-Simulator/assets/98334833/f24e0036-d365-4806-9bd4-5c4d01b16189)

## Contributing
Contributions are welcome! If you'd like to add missing opcodes or improve existing features, feel free to submit a pull request.

## License
This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.

---

Please note that this is a simplified example, and you should tailor it to your project's specific details. Feel free to enhance and customize the README as needed! 🚀
