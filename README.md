<<<<<<< HEAD
# MQTT_Ethernet
A project using the MQTT protocol with the ESP32 MCU.

The work was developed on two parts:

1. Ethernet interface

The Ethernet interface was implemented using the LAN8720 transceiver. The ESP32 uses its internal MAC address, a socket is opened, a fixed IP is configured, and the device starts operating in listening mode, thus forming the TCP/IP stack.

2. MQTT Communication

 After configuring the network interface, communication was established with an MQTT broker, in this case Mosquitto.

 Two topics were created:

- A publish topic, used to signal that the ESP32 is properly connected to the broker.

- A subscribe topic, through which the broker sends messages and receives responses from the device

This is an application still in development, with the intention of reusing and expanding it to other projects.

The board with the LAN8720 was easy to implement. The main point of attention was the REF_CLK signal, through which the transceiver provides a 50 MHz clock to the ESP32. As this clock is applied to a pin that also influences the MCU boot process, in some situations the board would enter bootloader mode. Applying a pull-up resistor to the line proved effective in solving the problem.
=======
ESP-IDF template app
====================

This is a template application to be used with [Espressif IoT Development Framework](https://github.com/espressif/esp-idf).

Please check [ESP-IDF docs](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for getting started instructions.

*Code in this repository is in the Public Domain (or CC0 licensed, at your option.)
Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.*
>>>>>>> f0a1194 (First Commit)
