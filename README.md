# Fingerprint

![Schema](circuit_image.svg)

![Built Prototype](built_image.jpg)

## Bill of Materials

* 1 [Arduino Mega](https://store.arduino.cc/products/arduino-mega-2560-rev3) - ca 53 EUR
* 1 JM-101 Fingerprint Sensor (AliExpress or sth) - ca 10 EUR
* 1 SH1106 1.3" 128 x 64 Pixel OLED Display (AliExpress or sth) - ca 4 EUR
* 1 Traffic light module (or Red, Yellow, and Green LEDs with appropriate resistors) - ca 0.50 EUR
* 1 Servo - ca 3 EUR
* 1 Breadboard - ca 2 EUR
* 1 Set of Jumper wires - ca 3 EUR

Total cost: ca 75 EUR, depending on what you already have.

## How to Use

1. Install Visual Studio Code and PlatformIO.
2. Clone this repository, and open it in Visual Studio Code.
3. Connect the components according to the wiring diagram below.
4. Move the tools/multitool.cpp to the src folder to build and upload the multitool firmware to the Arduino Mega.
5. Open the Serial Monitor at 9600 baud and enroll your fingerprints using the multitool firmware.
6. Remove the multitool.cpp from the src folder and build and upload the main firmware to the Arduino Mega.
7. You can now open/close the lock (servo, you can hook this up to anything) using your enrolled fingerprints.

## Wiring

Edit this project interactively in [Cirkit Designer](https://app.cirkitdesigner.com/project/f00a5960-7620-446b-900b-7fb7076e3451).

| Pin on Arduino Mega | Pin on Fingerprint Sensor |
|---------------------|---------------------------|
| 18 (TX1)            | RX                        |
| 19 (RX1)            | TX                        |
| GND                 | GND                       |
| 3.3V                | VCC                       |

| Pin on Arduino Mega | Pin on OLED Display       |
|---------------------|---------------------------|
| 5V                  | VCC                       |
| GND                 | GND                       |
| 21 (SCL)            | SCL                       |
| 20 (SDA)            | SDA                       |

| Pin on Arduino Mega | Pin on LED               |
|---------------------|--------------------------|
| 2                   | Red LED (Anode)          |
| 3                   | Yellow LED (Anode)       |
| 4                   | Green LED (Anode)        |
| GND                 | LED Cathodes             |

| Pin on Arduino Mega | Pin on Servo             |
|---------------------|--------------------------|
| 5                   | Signal (Yellow Wire)     |
| GND                 | GND                      |
| 5V                  | VCC                      |

## Lapse URLs

* [Circuit Design](https://lapse.hackclub.com/timelapse/v32__6YKCWld)

## AI use

* Code help (line completions)
* Re-Structuring the multitool firmware
