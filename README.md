

# Smart Helmet (ESP32 + Blynk IoT)

A safety-focused smart helmet using ESP32 that monitors temperature (DHT11), air quality (MQ135), obstacle distance (ultrasonic), and vibration (impact). It shows live status on a 16x2 I2C LCD, triggers buzzer/LED/laser on alerts, and streams telemetry to Blynk IoT.

## Features
- Real-time sensors: DHT11, MQ135 (analog+digital), HC-SR04 ultrasonic, vibration switch
- LCD I2C status + WiFi icon
- Buzzer, LED, Laser on alert
- Blynk IoT dashboard (V0..V5)
- Robust loop: BlynkTimer (no `delay`), WiFi auto-reconnect

## Pin Map (ESP32)
