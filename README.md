# Cherry Tomato Picking System - Low-Level Control (STM32)

[English](./README.md) | [中文](./README_zh.md)

This project is the low-level control program for an automated cherry tomato picking system, based on the STM32F103 (Cortex-M3) microcontroller.
The low-level controller operates the vertical movement of the **lifting platform** and the opening/closing of the **end-effector gripper**, interacting with the upper-level controller (e.g., Raspberry Pi, PC) via simple serial commands.

## 📖 Project Overview

Designed for fruit and vegetable harvesting scenarios, this STM32 firmware acts as the actuator controller. It receives ASCII string commands from the upper-level system and executes the corresponding motion controls.

*   **Lifting Platform Control**: Supports manual (jog) and automatic (PID position closed-loop) control strategies. It drives the motor direction via relays and uses an optical encoder for precise positioning.
*   **Gripper Control**: Controls the opening and closing angle of the end-effector via the CAN bus.
*   **State Management**: Built-in Hierarchical Finite State Machine (HFSM) ensures safe transitions between different system modes (Idle, Moving, Error).

## 🛠️ Environment & Configuration

*   **Hardware Platform**: STM32F103 (Standard Peripheral Library)
*   **IDE**: Keil MDK-ARM v5 / VS Code (Embedded IDE extension)
*   **Compiler**: ARMCC (AC5)
*   **Language**: C (C99 Standard)

## 📂 Code Structure

The project follows a layered architecture with low coupling:

```text
src/
├── hal/                    # Hardware Abstraction Layer
│   ├── can.c               # CAN bus interface
│   ├── dwt.c               # DWT timer interface
│   ├── systick.c           # SysTick timer interface
│   ├── timer.c             # General timer interface
│   └── usart.c             # USART communication interface
├── driver/                 # Driver Layer
│   ├── d_relay.c           # Relay driver (controls lift motor direction)
│   ├── d_gripper.c         # Gripper driver (CAN communication control)
│   └── d_encoder.c         # Encoder interface (position feedback)
├── service/                # Service Layer
│   ├── s_delay.c           # Blocking/non-blocking delay services
│   ├── s_wireless_comms.c  # Wireless/serial communication protocol parsing
│   ├── s_pid.c             # PID position control algorithm
│   └── s_log.c             # Logging and debugging
├── app/                    # Application Layer
│   ├── a_fsm.c/.h          # Finite State Machine (main business logic)
│   └── a_board.c/.h        # Board-level initialization (hardware resource configuration)
└── main.c                  # Program entry point
```

## ⚙️ Functional Modules

### 1. Communication Protocol
The controller communicates with the upper-level system via USART1 (Baud rate: 115200). Commands are ASCII strings starting with `$` and ending with `#`.

**Command List:**

| Module | Function | Command Format | Description |
| :--- | :--- | :--- | :--- |
| **Lift** | Up | `$LIFT_UP#` | Relay active, platform moves up |
| | Down | `$LIFT_DOWN#` | Relay active, platform moves down |
| | Stop | `$LIFT_STOP#` | Stop motor |
| | Set Height | `$LIFT_SET:<float>#` | E.g., `$LIFT_SET:150.5#` (Unit: mm), triggers automatic PID movement |
| **Gripper** | Open | `$GRIP_OPEN#` | Open gripper to preset angle |
| | Close | `$GRIP_CLOSE#` | Close gripper to preset angle |
| | Set Angle | `$GRIP_SET:<float>#` | E.g., `$GRIP_SET:1.57#` (Unit: rad) |

### 2. Finite State Machine (FSM)
System states are managed by `a_fsm.c` using a hierarchical design:

*   **Normal Mode**
    *   **Idle**: System ready, waiting for commands.
    *   **LiftMoving**: Entered upon receiving `$LIFT_SET`, PID algorithm takes over relay control until the target position is reached.
*   **Error Mode**: Entered upon hardware failure or anomaly, system halts for protection.

### 3. Hardware Connections

*   **Relay (Lift Motor)**:
    *   GPIOB Pin 0 (Direction A)
    *   GPIOB Pin 1 (Direction B)
*   **Gripper**: CAN1 Bus
*   **Serial (Wireless)**: USART1 (TX/RX)
