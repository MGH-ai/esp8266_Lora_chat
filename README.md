# ESP8266 Chat Server with LoRa Communication (Wi-Fi Hotspot Only)

## Overview

This project implements a chat server environment using the **ESP8266** microcontroller, which operates as a **Wi-Fi hotspot** (without internet access). Communication between devices is facilitated via **LoRa technology** (Ra-02 or Ra-01 modules). Users can chat with each other by connecting to the Wi-Fi hotspot created by the ESP8266.

## Features

- **Wi-Fi Hotspot:** Uses the ESP8266 to create a Wi-Fi hotspot for users to connect, without requiring internet access.
- **LoRa Communication:** Utilizes Ra-02 or Ra-01 LoRa modules for long-range, low-power communication between ESP8266 devices.
- **Encryption (Work-in-Progress):** A placeholder for encryption functionality, which is not yet operational.
- **File Transfer (Work-in-Progress):** Option for file transfer in the chat HTML interface, but functionality is incomplete.

## Hardware Requirements

- **ESP8266 Modules:** Compatible with various ESP8266 boards.
- **LoRa Ra-02 or Ra-01 Modules:** For communication between ESP8266 devices.

## Software Requirements

- **PlatformIO for CLion** (or **PlatformIO for Visual Studio Code** for Windows users): PlatformIO is used to build and upload the code to the ESP8266.
- **Libraries:**
  - `Arduino.h`
  - `ESP8266WiFi.h`
  - `DNSServer.h`
  - `ESP8266WebServer.h`
  - `FS.h`
  - `stdlib.h`
  - Libraries for encryption (currently not functional):
    - `Crypto.h`
    - `AES.h`
    - `GCM.h`
  - **LoRa Communication:**
    - `SPI.h`
    - `LoRa.h`

## Installation

1. **Set Up PlatformIO:**
   - Install [PlatformIO](https://platformio.org/) as an IDE plugin for Visual Studio Code or CLion.
   - Open the project folder in your preferred IDE (Visual Studio Code or CLion).

2. **Install Dependencies:**
   - The required libraries will automatically be installed based on the `platformio.ini` configuration file.

3. **Upload Code:**
   - Connect the ESP8266 to your computer and select the appropriate board in PlatformIO.
   - Click the "Upload" button to upload the code to the ESP8266.
   - Click the "Upload File System Image" button to upload the HTML, CSS, JavaScript, and config file to the ESP8266.

## Usage

1. **Configure LoRa Settings:**
   - Adjust the frequency in the code to match your region’s LoRa regulations.

2. **Deploy Devices:**
   - Connect the LoRa modules to the ESP8266 boards.
   - Power on the devices to establish communication over LoRa.

3. **Connect via Wi-Fi Hotspot:**
   - Users can connect to the ESP8266's Wi-Fi network to participate in the chat.
   - You can change the WiFi hotspot SSID and password in the code.
   - Change the spreading factor and other parameters in the chat app to match each other.

4. **File Transfer (Optional):**
   - The chat interface includes an option for file transfer, although it is not yet functional.

## Contributing

Contributions are welcome! Please fork the repository, make your changes, and submit a pull request. If you're working on encryption or file transfer functionality, feel free to submit improvements or bug fixes!

## License

This project is licensed under the **GNU Affero General Public License v3**.
