# IV Drips Monitoring System using ESP32, HX711 & Telegram Alerts

An IoT-based real-time IV fluid monitoring system that uses an **ESP32**, **HX711 load cell**, **LCD display**, and **Telegram Bot API** to notify medical staff when the IV bottle reaches a critical low level.

## 📌 Table of Contents
- [Project Overview](#-project-overview)
- [Features](#-features)
- [System Architecture](#-system-architecture)
- [Hardware Requirements](#-hardware-requirements)
- [Software Requirements](#-software-requirements)
- [Local Environment Setup](#-local-environment-setup)
- [Installing Required Libraries](#-installing-required-libraries)
- [Circuit Diagram & Pin Mapping](#-circuit-diagram--pin-mapping)
- [How the System Works](#-how-the-system-works)
- [Troubleshooting](#-troubleshooting)
- [License](#-license)

## 📘 Project Overview

This project monitors the weight of an IV bottle using a **load cell + HX711 module** and sends **Telegram alerts** to medical staff when the fluid level drops below a configured threshold.

The system includes the following capabilities:

- Displays real-time IV fluid weight on an LCD  
- Allows threshold setting using three push buttons  
- Supports multi-stage alert levels (25%, 50%, 75%)  
- Automatically detects if the IV bottle is removed  
- Runs entirely on an ESP32 microcontroller  

## ⭐ Features

- Real-time IV fluid monitoring  
- Three-stage alert system (25%, 50%, 75%)  
- Automatic Telegram bot notifications  
- LCD display interface for live status  
- Custom IV threshold configuration using buttons  
- Built-in safety detection for bottle removal  
- Low-cost and easily scalable hardware design  

## 🧱 System Architecture
```
IV Bottle → Load Cell → HX711 → ESP32 → LCD Display
                                      ↓
                                    Wi-Fi
                                      ↓
                              Telegram Notifications 

```

## 🛠️ Hardware Requirements

- ESP32 Dev Board  
- HX711 Load Cell Amplifier  
- Load Cell (5kg / 10kg)  
- 16x2 LCD with I²C module  
- Push Buttons × 3 (Hundreds, Tens, Units)  
- Buzzer (active/passive)  
- LED + Resistor  
- Jumper Wires  
- Breadboard or PCB  

## 💻 Software Requirements

### Arduino IDE
Download and install the Arduino IDE:  
- [Arduino IDE Download](https://www.arduino.cc/en/software)


### ESP32 Board Package
Install the ESP32 board support package via **Boards Manager**:

## 🔧 Local Environment Setup

### 1. Clone the Repository

```bash
git clone https://github.com/jayaprasadp133/IV-drips-Monitoring.git
cd IV-drips-Monitoring
```
## 📂 Project Structure & Workflow

After cloning the repository, your project folder will contain:


### 📁 Folder Overview

| Folder | Purpose |
|---|---|
| **weight_sensor_calibration_code** | Contains Arduino code used to obtain the load cell calibration factor (HX711). Run this *first* when setting up the system. |
| **actual_code_for_weight_with_cali** | Uses your calibration value to measure weight accurately. This is the main weight-reading program. |
| **full_project_drips_alret** | Final IoT + Telegram alert system. Includes threshold logic, LCD, buzzer, and WiFi connectivity. |
| **output** | Contains output logs/files if generated. |
| **README.md** | Project documentation. |



## ✅ Step-1: Calibrate the Load Cell (FIRST TIME ONLY)

*You already calibrated your sensor, but this guide helps new users repeat it.*

Go to
**weight_sensor_calibration_code/weight_sensor_calibration_code.ino**

1. Open the calibration sketch:


2. Install the required libraries:
   - `HX711`
   - `ArduinoJson` (optional depending on code)

3. Upload the sketch to your board.

4. Open **Serial Monitor**  
   - Set baud rate to **9600** (or what the sketch specifies)

5. Place **known weights** (e.g., 100g, 200g, 500g, etc.) on the load cell.

6. Adjust the calibration factor printed in Serial Monitor until the reading matches the actual weight.

7. Save this final value — you will use it in the main code.

---

## ✅ Step-2: Insert Calibration Value into Main Code

1. Open: (actual_code_for_weight_with_cali/) folder for checks the weight can calculated correctly


2. Inside the `actual_code_for_weight_with_cali.ino` file, find:

```cpp
float calibration_factor = -7050; // example
```
3. Replace this with the calibration factor you obtained.

## ✅ Step-3: Upload the Main Measurement Code 


1. Connect your ESP32 / Arduino board

2. Install required libraries:

    `HX711` check pins 

| PINS | Number |
|---|---|
| DOUT | 4 |
| SCK | 5 |



3. Verify:

- Serial Monitor prints correct weight

- IV bottle weight decreases consistently as fluid drips

## ✅ Step-4: Integrate the Full IV Drip Alert System

After confirming that the main weight-reading program works correctly, you can proceed to integrate the complete IoT-based IV Drip Monitoring and Alert System.

### 1. Open the Full Alert System Code 

Navigate to:


Open the `full_project_drips_alret` Folder and find `full_project_drips_alret.ino`  file inside this folder.

---

### 2. Required Libraries

Make sure the following libraries are installed in **Arduino IDE**:

- WiFi.h  *(comes with ESP32 board package)*
- HTTPClient.h
- HX711  *(install from Library Manager: "HX711")*
- LiquidCrystal_I2C  *(install: "LiquidCrystal_I2C")*

You can install these via:

`Arduino IDE → Tools → Manage Libraries`

Search and install:
- **HX711**
- **LiquidCrystal_I2C**
- **ArduinoJson** (if needed in other parts)


### 3. Pin Definitions (ESP32 → Components)

```cpp
#define DOUT 4          // HX711 Data pin
#define SCK 5           // HX711 Clock pin

#define LED_PIN 25      // Alert LED
#define BUZZER_PIN 26   // Alert Buzzer

#define BTN_HUNDREDS 32 // Button for 100s place
#define BTN_TENS 33     // Button for 10s place
#define BTN_UNITS 27    // Button for units place
```



### 4. Update the Required Parameters

In the code, update the following:

#### 🔹 Calibration Factor
Replace the existing calibration value with the value you obtained during Step-1.

Example:
```cpp
float calibration_factor = -7050; // replace with your value
```

---

## 🌐 **WiFi & Telegram Configuration**


```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <HX711.h>
#include <LiquidCrystal_I2C.h>

// --- WiFi Credentials ---
const char* ssid = "WIFI_NAME";
const char* password = "WIFI_PASS";

// --- Telegram Bot Info ---
String botToken = "YOUR_BOT_TOKEN_HERE";  
String chatID[] = { "2060211608", "1619368608" };   // send to multiple users
```
## 🤖 How to Get Telegram Bot Token & Chat ID

To enable Telegram notifications, you must create a bot and obtain:

- ✅ **Bot Token**
- ✅ **Chat ID** (your Telegram user ID)

Follow the steps below.

---

### 🟦 Step 1 — Create a Telegram Bot (Get the Bot Token)

1. Open **Telegram**.
2. Search for **@BotFather**.
3. Start a chat and send the command:
`start`
4. Then create a new bot: `newbot`

5. BotFather will ask for:
- A **name** for your bot (any name)
- A **username** for your bot (must end with `_bot`)

6. After creating it, BotFather will give you a message like:
 ```ccp
Done! Congratulations on your new bot.
Use this token to access the HTTP API:
1234567890:AdsdesdedJKlmNopQR_swdeSxYZ
```

7. Copy this value:

`BOT TOKEN = 1234567890:AdsdesdedJKlmNopQR_swdeSxYZ`

## 🟩 Step 2 — Get Your Telegram Chat ID

To send alerts, the ESP32 must know the **user's chat ID**.

Follow these steps:

1. Open **Telegram**.
2. In the search bar, type: `@userinfobot`
or alternative:
3. Start the bot by sending:`/start`
4. The bot will reply with your user information, including:
`Your user ID: 2060211608`

5. Copy this **user ID** and paste it into your code: 

```cpp
String chatID[] = { "2060211608" };
```
## 🛠️ Step 3 — Insert Bot Token & Chat ID into the Program

Once you have successfully obtained:

- ✅ **Bot Token** (from @BotFather)  
- ✅ **Chat ID** (from @userinfobot or @RawDataBot)

You must add them to your ESP32 program so it can send messages through Telegram.

Follow the steps below.

---

### 📝 1. Open Your Main Program File

Go to:
`full_project_drips_alret/`

and open the `full_project_drips_alret.ino` file inside.

---

### 🔐 2. Insert Your Bot Token

Find the line in your code:

```cpp
String botToken = "YOUR_TELEGRAM_BOT_TOKEN";
```
Replace "YOUR_TELEGRAM_BOT_TOKEN" with the actual token given by BotFather.

---

### 👤 3. Insert Your Chat ID

Find the line:

```cpp
String chatID[] = { "CHAT_ID_HERE" };
```


Replace "CHAT_ID_HERE" with the Chat ID you obtained.

---

### 📤 4. Upload the Program

After updating the following in your code:

- ✅ Bot Token  
- ✅ Chat IDs  
- ✅ WiFi SSID / Password  
- ✅ Calibration Factor  

Upload the code to your ESP32 using the Arduino IDE:

`Verify → Upload → Open Serial Monitor`


---

### 📲 5. Test the Telegram Alerts

After uploading the program:

1. Open your bot in **Telegram**
2. Press **Start** (to activate the bot)
3. Wait for the ESP32 to **connect to WiFi**
4. Trigger any alert:
   - Threshold alert  
   - Weight drop alert  
   - Bottle removal  
5. Check Telegram — you should receive instant alerts

### Example Messages

`Patient Name: Joan.
IV Drip Level Alert: IV bottle reached 50%`
