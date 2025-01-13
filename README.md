# STM32 Cortex-M4 Real-Time Operating System

This project is a simple real-time operating system (RTOS) designed for the ARM Cortex-M4 processor, specifically tailored for the STM32 microcontroller series. It provides basic multitasking capabilities, allowing the execution of multiple threads in a cooperative and preemptive manner.

## Features

- **Multithreading Support**: Efficient management of multiple threads including creation, execution, and scheduling.
- **Cooperative Scheduling**: Threads can voluntarily yield control to allow other threads to execute.
- **Preemptive Scheduling**: Supports time-slicing to ensure that high-priority threads receive adequate CPU time.
- **Stack Management**: Includes a stack allocator that handles memory allocation for thread stacks within a predefined pool.
- **Priority-based Interrupt Handling**: Configurable interrupt priorities for system handler and service calls.
- **UART Debugging**: Integrated UART support for debugging and logging thread states and messages.

## Hardware Requirements

- STM32 microcontroller with a Cortex-M4 processor (tested on STM32F4 series)
- USART2 peripheral for UART communication
- General-purpose I/O pins for basic signaling and user input

## Software Requirements

- STM32CubeIDE for project compilation and flashing
- HAL (Hardware Abstraction Layer) drivers for STM32
- ARM Cortex-M4 toolchain

## Getting Started

### Setting Up Your Development Environment

1. Install STM32CubeIDE from the [STMicroelectronics website](https://www.st.com/en/development-tools/stm32cubeide.html).
2. Clone this repository to your local machine.
3. Open the project in STM32CubeIDE.
4. Build the project and flash it to your STM32 device.

### Configuration

Modify the `kernel.h` file to adjust memory management settings, including stack size and pool size, according to your application's needs.

## Usage

After flashing the RTOS to your STM32 device, the system starts with two sample threads:

- **Cooperative Thread**: Demonstrates voluntary yielding.
- **Preemptive Thread**: Alters a shared global variable and simulates workload.

You can monitor the output and behavior through a serial terminal connected to USART2 at 115200 bps.

## Contributing

Contributions to this project are welcome. Please fork the repository and submit a pull request with your proposed changes.

## Authors

- Noah Yacowar

Feel free to contact me for any feedback or questions.
