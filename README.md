
# FOTA Project with ESP32 and STM32F103

This project enables remote firmware updates (FOTA) using ESP32, STM32F103, and Google Firebase. The ESP32 connects to WiFi, downloads the bootloader and firmware code, and communicates with the STM32.

## Table of Contents

- [1. Introduction](#1-introduction)
- [2. Functions](#2-functions)
- [3. Critical Parts](#3-critical-parts)
- [4. How to Use](#4-how-to-use)

---

## 1. Introduction

This project facilitates firmware updates over-the-air (FOTA) for an STM32F103 microcontroller using an ESP32 and Google Firebase. Key components include WiFi connectivity, Firebase authentication, and MQTT communication.

## 2. Functions

### `Read64byte(char *Host)`

- **Purpose**: Reads 64 bytes from a file and prepares a packet for sending to the STM32.
- **Hint**: Utilizes a file object to read and process data for firmware updates.

### `callback(char* topic, byte* payload, unsigned int length)`

- **Purpose**: Handles incoming MQTT messages.
- **Hint**: Parses MQTT topic and payload to trigger actions based on received messages.

### `fcsDownloadCallback(FCS_DownloadStatusInfo info)`

- **Purpose**: Callback function for Firebase Storage downloads.
- **Hint**: Provides status updates during file downloads from Firebase Storage.

---

## 3. Critical Parts

### Firebase Integration

- **Code**: 
    ```cpp
    Firebase.begin(&config, &auth);
    ```
- **Explanation**: Initializes Firebase with authentication credentials and configuration.

### MQTT Subscription

- **Code**: 
    ```cpp
    client.subscribe("/mahmoud/servo");
    ```
- **Explanation**: Subscribes to the MQTT topic for receiving commands.

### Firmware Update Process

- **Code**: 
    ```cpp
    if (!Firebase.Storage.download(&fbdo, STORAGE_BUCKET_ID, "TestBootloader.bin", "/updat.bin", mem_storage_type_flash, fcsDownloadCallback))
    ```
- **Explanation**: Initiates the firmware download process from Firebase Storage.

---

## 4. How to Use

1. Set up your WiFi credentials, API Key, and Firebase user credentials.
2. Ensure the STM32 is ready to receive firmware updates via the provided bootloader.
3. Flash the ESP32 with this code.
4. Run the code and monitor the serial output for progress updates.

For detailed information on each function, refer to the code comments.

