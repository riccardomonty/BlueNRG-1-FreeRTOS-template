# BlueNRG-1-FreeRTOS Template

personal notes on STEVAL-IDB007V2
- development kit recceived as old sample
- tried with Keil ARM toolchain
- requirement install "BlueNRG1-DK" in advance

## 1 ARM toolchain

IDE: "uVision v5.39", from MDK539 with "Arm Compiler 5.06 update 7 (build 960)"
(see install guide for choose right folder path)

## 2 Project Template

This is a project template for develop FreeRTOS app with BLE stack.
Project structure from scratch is obtained with copy and paste from an example of BlueNRG-DK, e fix some library reference.
<br>
Twice time, I've found a linker error on "HAL_VTIMER_Callback()", solved by removing "radio.c" file from source folder.

**an error in project defines from example: check HS_SPEED_XTAL=HS_SPEED_XTAL_16MHZ**

## 2 Debug

Debugger is not available, any IC debbuger is present on development kit, just MCU with UART-USB bridge capability<br>

## 3 Flash-Program the board

To flash thre project can be used "RF-Flasher Utility" (STSW- BNRGFLASHER)<br>

!! Not needed activate bootloader by hand, it's done by flashing utility
**if needed to activate bootloader: pull-up DIO7 pin and reset MCU**<br>
(see "um2406-the-rfflasher-utility-sw-package--stmicroelectronics.pdf")<br>

---
---

last update 28/01/2024