# ESP32 Network Controlled Car

A WiFi-controlled robot car with obstacle detection, radio communication, and real-time telemetry.

## Features

- **WiFi Control**: Web-based interface for remote control
- **Obstacle Detection**: Ultrasonic sensor prevents collisions
- **Radio Communication**: APC220 module for long-range messaging
- **GPS Tracking**: NEO-M8N module for position tracking
- **Real-time Telemetry**: Live distance, speed, signal, and location monitoring
- **PWM Speed Control**: Variable motor speed (0-100%)

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32 CONTROL SYSTEM                    │
├─────────────────────────────────────────────────────────────┤
│  WiFi Module  │  Web Server  │  Motor Control  │  Sensors   │
│      ↓        │      ↓       │       ↓         │     ↓      │
│   Router ←────┼──→ HTTP ←────┼──→ PWM Signals ─┼──→ I/O     │
│              │              │                 │            │
│              │              │                 │            │
│   ┌──────────┼──────────────┼─────────────────┼────────┐   │
│   │          │              │                 │        │   │
│   │    ┌─────▼─────┐   ┌────▼────┐      ┌────▼───┐    │   │
│   │    │Web Client │   │Motors   │      │Sensors │    │   │
│   │    │Dashboard  │   │L298N    │      │HC-SR04 │    │   │
│   │    └───────────┘   │Driver   │      │APC220  │    │   │
│   │                    └─────────┘      └────────┘    │   │
│   └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## Hardware Components

### ESP32 DevKit V1
- **Microcontroller**: Dual-core 240MHz
- **WiFi**: 802.11 b/g/n
- **GPIO Pins**: 30 digital I/O
- **PWM Channels**: 16
- **Operating Voltage**: 3.3V
- **Input Voltage**: 5V (via USB/VIN)

### L298N Motor Driver
- **Motor Channels**: 2 (dual H-bridge)
- **Motor Voltage**: 5V-35V
- **Logic Voltage**: 5V
- **Max Current**: 2A per channel
- **PWM Frequency**: Up to 40kHz

### HC-SR04 Ultrasonic Sensor
- **Range**: 2cm - 400cm
- **Accuracy**: ±3mm
- **Frequency**: 40kHz
- **Operating Voltage**: 5V
- **Trigger Pulse**: 10µs

### APC220 Radio Module
- **Frequency**: 418MHz-455MHz
- **Range**: Up to 1000m (open area)
- **Baud Rate**: 1200-19200 bps
- **Interface**: UART (TTL)
- **Power**: 3.3V-5V

### NEO-M8N GPS Module
- **Channels**: 72 acquisition, 22 tracking
- **Accuracy**: 2.5m CEP (50%)
- **Cold Start**: 26s
- **Hot Start**: 1s
- **Update Rate**: 1-10Hz
- **Interface**: UART (TTL)
- **Power**: 3.3V-5V

### DC Gear Motors (2x)
- **Voltage**: 3V-6V
- **RPM**: 200 (at 6V)
- **Torque**: High torque geared
- **Current**: ~150mA (no load)

## Pin Configuration

```
ESP32 DevKit V1 Pin Layout:
                     ┌─────────────┐
                     │    ESP32    │
                     │   DevKit    │
               3V3 ──┤1          30├── GND
               GND ──┤2          29├── GPIO23
              GPIO15──┤3          28├── GPIO22
              GPIO2 ──┤4          27├── GPIO1/TX
              GPIO0 ──┤5          26├── GPIO3/RX
              GPIO4 ──┤6          25├── GPIO21
              GPIO16──┤7          24├── GND
              GPIO17──┤8          23├── GPIO19
              GPIO5 ──┤9          22├── GPIO18 (Echo)
              GPIO18──┤10         21├── GPIO5  (Trig)
              GPIO19──┤11         20├── GPIO17 (APC TX)
              GPIO21──┤12         19├── GPIO16 (APC RX)
               GND ──┤13         18├── GPIO4  (GPS RX)
              GPIO22──┤14         17├── GPIO2  (GPS TX)
               GND ──┤13         18├── GPIO4
              GPIO22──┤14         17├── GPIO0
              GPIO23──┤15         16├── GPIO2
                     └─────────────┘
```

## Wiring Diagram

```
┌─────────────┐    ┌──────────────┐    ┌─────────────┐
│   ESP32     │    │    L298N     │    │   Motors    │
│             │    │              │    │             │
│ GPIO27 ─────┼────┤IN1      OUT1├────┤ Motor1 (+)  │
│ GPIO26 ─────┼────┤IN2      OUT2├────┤ Motor1 (-)  │
│ GPIO14 ─────┼────┤ENA           │    │             │
│             │    │              │    │             │
│ GPIO33 ─────┼────┤IN3      OUT3├────┤ Motor2 (+)  │
│ GPIO25 ─────┼────┤IN4      OUT4├────┤ Motor2 (-)  │
│ GPIO32 ─────┼────┤ENB           │    │             │
│             │    │              │    │             │
│ 5V ─────────┼────┤VCC           │    └─────────────┘
│ GND ────────┼────┤GND           │
└─────────────┘    └──────────────┘

┌─────────────┐    ┌──────────────┐
│   ESP32     │    │   HC-SR04    │
│             │    │              │
│ GPIO5 ──────┼────┤Trig          │
│ GPIO18 ─────┼────┤Echo          │
│ 5V ─────────┼────┤VCC           │
│ GND ────────┼────┤GND           │
└─────────────┘    └──────────────┘

┌─────────────┐    ┌──────────────┐
│   ESP32     │    │   APC220     │
│             │    │              │
│ GPIO17 ─────┼────┤RXD           │
│ GPIO16 ─────┼────┤TXD           │
│ 5V ─────────┼────┤VCC           │
│ GND ────────┼────┤GND           │
└─────────────┘    └──────────────┘

┌─────────────┐    ┌──────────────┐
│   ESP32     │    │   NEO-M8N    │
│             │    │              │
│ GPIO2 ──────┼────┤RXD           │
│ GPIO4 ──────┼────┤TXD           │
│ 3.3V ───────┼────┤VCC           │
│ GND ────────┼────┤GND           │
└─────────────┘    └──────────────┘
```

## Software Flow

```
┌─────────────┐
│   START     │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│ Initialize  │
│ - WiFi      │
│ - Sensors   │
│ - Motors    │
│ - Radio     │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│ Start Web   │
│ Server      │
└──────┬──────┘
       │
       ▼
┌─────────────┐    ┌─────────────┐
│ Main Loop   │───▶│Handle Web   │
│             │    │Requests     │
└──────┬──────┘    └─────────────┘
       │
       ▼
┌─────────────┐    ┌─────────────┐
│Handle Radio │───▶│Process      │
│Data         │    │Commands     │
└──────┬──────┘    └─────────────┘
       │
       ▼
┌─────────────┐    ┌─────────────┐
│Send         │───▶│Movement     │
│Telemetry    │    │Control      │
└──────┬──────┘    └──────┬──────┘
       │                  │
       ▼                  ▼
┌─────────────┐    ┌─────────────┐
│Check        │───▶│Obstacle     │
│Obstacles    │    │Detection    │
└──────┬──────┘    └──────┬──────┘
       │                  │
       └──────────────────┘
```

## Movement Logic

```
Movement Request
       │
       ▼
┌─────────────┐
│Get Distance │
│from Sensor  │
└──────┬──────┘
       │
       ▼
┌─────────────┐     YES    ┌─────────────┐
│Distance >   │───────────▶│Execute      │
│10cm?        │            │Movement     │
└──────┬──────┘            └─────────────┘
       │ NO
       ▼
┌─────────────┐
│Stop Motors  │
│Send Alert   │
└─────────────┘

Movement Types:
┌─────────────┐    ┌─────────────┐
│  FORWARD    │    │  REVERSE    │
│ M1: Forward │    │ M1: Reverse │
│ M2: Forward │    │ M2: Reverse │
└─────────────┘    └─────────────┘

┌─────────────┐    ┌─────────────┐
│    LEFT     │    │   RIGHT     │
│ M1: Stop    │    │ M1: Forward │
│ M2: Forward │    │ M2: Stop    │
└─────────────┘    └─────────────┘
```

## Web Interface Features

### Control Panel
- **Directional Buttons**: Forward, Reverse, Left, Right, Stop
- **Speed Slider**: 0-100% motor speed control
- **Real-time Response**: Instant command execution

### Radio Communication
- **Message Input**: Send custom text messages
- **Status Display**: Show last received message
- **Range**: Up to 1km in open areas

### Telemetry Dashboard
- **Distance**: Real-time obstacle detection (cm)
- **WiFi Signal**: Connection strength (dBm)
- **Radio Status**: Last received radio message
- **GPS Location**: Live latitude/longitude coordinates
- **Satellite Count**: Number of GPS satellites in view
- **Auto-refresh**: Updates every 1 second

## Installation

1. **Hardware Assembly**:
   - Connect components according to wiring diagram
   - Ensure proper power supply (5V for motors, 3.3V for ESP32)
   - Mount ultrasonic sensor facing forward

2. **Software Setup**:
   ```bash
   # Install Arduino IDE
   # Add ESP32 board support
   # Install required libraries:
   # - WiFi (built-in)
   # - WebServer (built-in)
   # - HardwareSerial (built-in)
   ```

3. **Configuration**:
   - Update WiFi credentials in code
   - Upload sketch to ESP32
   - Connect to car's IP address via web browser

## Usage

1. **Power On**: Connect battery/power supply
2. **Connect**: Join car's WiFi network or connect to same network
3. **Access**: Open web browser to ESP32's IP address
4. **Control**: Use web interface buttons and sliders
5. **Monitor**: View real-time telemetry data

## Troubleshooting

### Common Issues

**WiFi Connection Failed**:
- Check SSID/password
- Verify network availability
- Reset ESP32 and retry

**Motors Not Responding**:
- Check L298N connections
- Verify power supply voltage
- Test motor driver enable pins

**Obstacle Detection Issues**:
- Clean ultrasonic sensor
- Check trigger/echo connections
- Verify 5V power supply

**Radio Communication Problems**:
- Check APC220 wiring
- Verify baud rate (9600)
- Test with serial monitor

**GPS Issues**:
- Ensure clear sky view for satellite reception
- Check NEO-M8N wiring (RX/TX)
- Wait for GPS fix (may take 30+ seconds outdoors)
- Verify 3.3V power supply

## Technical Specifications

| Component | Specification |
|-----------|---------------|
| **Microcontroller** | ESP32 DevKit V1 |
| **WiFi Range** | 50-100m (indoor) |
| **Radio Range** | Up to 1000m (APC220) |
| **GPS Accuracy** | 2.5m CEP (NEO-M8N) |
| **Detection Range** | 2-400cm (HC-SR04) |
| **Motor Voltage** | 3-6V DC |
| **Operating Current** | ~500mA (total) |
| **Control Interface** | Web Browser |
| **Update Rate** | 1Hz (telemetry) |

## Future Enhancements

- **Camera Integration**: Live video streaming
- **Navigation System**: Waypoint-based autonomous driving
- **Battery Monitor**: Voltage and charge level display
- **Mobile App**: Dedicated smartphone application
- **Autonomous Mode**: AI-powered obstacle avoidance
- **Geofencing**: Virtual boundary alerts

## License

This project is based on Random Nerd Tutorials ESP32 car example with additional features for radio communication and enhanced obstacle detection.