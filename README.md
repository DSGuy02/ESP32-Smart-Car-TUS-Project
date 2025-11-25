# ESP32 Network Controlled Car

A WiFi-controlled robot car with obstacle detection, radio communication, and real-time telemetry.

## Features

- **Modern Web Interface**: Professional filesystem-based UI with dark theme
- **WiFi Control**: Responsive web-based interface for remote control
- **Keyboard Controls**: WASD/Arrow keys + Spacebar for quick control
- **Obstacle Detection**: Ultrasonic sensor prevents collisions
- **IR Obstacle Detection**: Additional IR sensor for enhanced obstacle avoidance
- **GPS Tracking**: NEO-M8N module for position tracking
- **Real-time Telemetry**: Live distance, speed, signal, and location monitoring
- **PWM Speed Control**: Variable motor speed (0-100%)
- **Buzzer Control**: Independent buzzer button with press-and-hold operation
- **LED Indicators**: Two controllable LEDs with toggle buttons
- **LittleFS Storage**: Web files stored in ESP32 flash memory

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

### HW-870 IR Obstacle Sensor
- **Detection Range**: 3cm - 80cm (adjustable)
- **Response Time**: <1ms
- **Operating Voltage**: 3.3V-5V
- **Interface**: Digital GPIO (HIGH = obstacle detected)
- **Detection Angle**: 35°
- **Sensitivity**: Adjustable via potentiometer

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

### Passive Buzzer
- **Operating Voltage**: 3.3V-5V
- **Frequency Range**: 100Hz-10kHz
- **Sound Level**: 85dB (at 10cm)
- **Interface**: Digital PWM
- **Current**: ~30mA

### LED Indicators (2x)
- **Operating Voltage**: 3.3V-5V
- **Forward Current**: ~20mA
- **Colors**: Red (LED1), Blue (LED2)
- **Interface**: Digital GPIO
- **Brightness**: Standard 5mm LEDs

## Pin Configuration

```
ESP32 DevKit V1 Pin Layout:
                     ┌─────────────┐
                     │    ESP32    │
                     │   DevKit    │
               3V3 ──┤1          30├── GND
               GND ──┤2          29├── GPIO23
              GPIO15──┤3          28├── GPIO22 (LED2)
              GPIO2 ──┤4          27├── GPIO1/TX
              GPIO0 ──┤5          26├── GPIO3/RX
              GPIO4 ──┤6          25├── GPIO21 (LED1)
              GPIO16──┤7          24├── GND
              GPIO17──┤8          23├── GPIO19 (Buzzer)
              GPIO5 ──┤9          22├── GPIO18 (Echo)
              GPIO18──┤10         21├── GPIO5  (Trig)
              GPIO19──┤11         20├── GPIO17 (HW-870 IR)
              GPIO21──┤12         19├── GPIO16
               GND ──┤13         18├── GPIO4  (GPS RX)
              GPIO22──┤14         17├── GPIO2  (GPS TX)
              GPIO23──┤15         16├── GPIO15
                     └─────────────┘

Motor Control Pins (not shown in layout above):
- GPIO27: L298N IN1 (Motor1 Direction)
- GPIO26: L298N IN2 (Motor1 Direction) 
- GPIO14: L298N ENA (Motor1 Speed PWM)
- GPIO33: L298N IN3 (Motor2 Direction)
- GPIO25: L298N IN4 (Motor2 Direction)
- GPIO32: L298N ENB (Motor2 Speed PWM)
```

## Power Distribution

```
┌─────────────┐    ┌──────────────┐    ┌─────────────┐
│  External   │    │    L298N     │    │ Power Rails │
│ Power Supply│    │ Motor Driver │    │             │
│             │    │              │    │             │
│ 7-12V ──────┼────┤ VIN    +5V ──┼────┤ 5V (ESP32)  │
│ GND ────────┼────┤ GND    GND ──┼────┤ GND (Common)│
│             │    │              │    │ 5V (Sensors)│
└─────────────┘    └──────────────┘    └─────────────┘
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
│ 5V ←────────┼────┤+5V           │    └─────────────┘
│ GND ────────┼────┤GND           │
└─────────────┘    │              │
                   │ VCC ←── 7-12V │ (External Power)
                   └──────────────┘

┌─────────────┐    ┌──────────────┐
│   ESP32     │    │   HC-SR04    │
│             │    │              │
│ GPIO5 ──────┼────┤Trig          │
│ GPIO18 ─────┼────┤Echo          │
│ 5V ─────────┼────┤VCC           │
│ GND ────────┼────┤GND           │
└─────────────┘    └──────────────┘

┌─────────────┐    ┌──────────────┐
│   ESP32     │    │   HW-870     │
│             │    │ IR Obstacle  │
│ GPIO17 ─────┼────┤OUT           │
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

┌─────────────┐    ┌──────────────┐
│   ESP32     │    │   Buzzer     │
│             │    │              │
│ GPIO19 ─────┼────┤Signal        │
│ GND ────────┼────┤GND           │
└─────────────┘    └──────────────┘

┌─────────────┐    ┌──────────────┐
│   ESP32     │    │ LED1 (Red)   │
│             │    │              │
│ GPIO21 ─────┼────┤Anode         │
│ GND ────────┼────┤Cathode       │
└─────────────┘    └──────────────┘

┌─────────────┐    ┌──────────────┐
│   ESP32     │    │ LED2 (Blue)  │
│             │    │              │
│ GPIO22 ─────┼────┤Anode         │
│ GND ────────┼────┤Cathode       │
└─────────────┘    └──────────────┘
```

## Project Structure

```
ESP32_Network_Controlled_Car/
├── ESP32_Network_Controlled_Car.ino  # Main Arduino sketch
├── data/                             # Web interface files
│   ├── index.html                    # Modern UI layout
│   ├── style.css                     # Professional styling
│   └── script.js                     # Interactive controls
├── upload_fs.sh                      # Filesystem upload script
└── README.md                         # Project documentation
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
│ - LittleFS  │
│ - Sensors   │
│ - Motors    │
│ - Radio/GPS │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│ Start Web   │
│ Server      │
│ Serve Files │
└──────┬──────┘
       │
       ▼
┌─────────────┐    ┌─────────────┐
│ Main Loop   │───▶│Handle Web   │
│             │    │API Requests │
└──────┬──────┘    └─────────────┘
       │
       ▼
┌─────────────┐    ┌─────────────┐
│Handle Radio │───▶│Process      │
│& GPS Data   │    │Commands     │
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
│Check Master │
│Toggle       │
└──────┬──────┘
       │
       ▼
┌─────────────┐     DISABLED   ┌─────────────┐
│Obstacle     │───────────────▶│Execute      │
│Detection    │                │Movement     │
│Enabled?     │                └─────────────┘
└──────┬──────┘
       │ ENABLED
       ▼
┌─────────────┐
│Check Sensor │
│for Direction│
└──────┬──────┘
       │
       ▼
┌─────────────┐     CLEAR     ┌─────────────┐
│Forward:     │──────────────▶│Execute      │
│Ultrasonic   │               │Movement     │
│Backward:    │               └─────────────┘
│IR Sensor    │
└──────┬──────┘
       │ BLOCKED
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
│ Sensor: US  │    │ Sensor: HW-870 │
└─────────────┘    └─────────────┘

┌─────────────┐    ┌─────────────┐
│    LEFT     │    │   RIGHT     │
│ M1: Stop    │    │ M1: Forward │
│ M2: Forward │    │ M2: Stop    │
│ No Blocking │    │ No Blocking │
└─────────────┘    └─────────────┘
```

## Web Interface Features

### Modern UI Design
- **Glassmorphism**: Translucent cards with backdrop blur effects
- **Dynamic Gradients**: Animated background with floating elements
- **Professional Typography**: Inter font with proper visual hierarchy
- **Responsive Grid**: Adaptive layout for all screen sizes
- **Smooth Animations**: 3D hover effects, transitions, and micro-interactions

### Enhanced Control Methods
- **Interactive Buttons**: Clean text-based movement controls without emojis
- **Buzzer Button**: Dedicated press-and-hold buzzer control with visual feedback
- **LED Controls**: Toggle buttons for two independent LED indicators
- **Obstacle Detection Toggle**: Master control to enable/disable all obstacle detection
- **Keyboard Shortcuts**: 
  - W/↑ = Forward (blocked by ultrasonic sensor)
  - S/↓ = Reverse (blocked by IR sensor)
  - A/← = Left (no blocking)
  - D/→ = Right (no blocking)
  - Spacebar = Stop
- **Custom Speed Slider**: Animated slider with visual markers
- **Visual Feedback**: Button highlighting and active state indicators
- **Loading Screen**: Animated startup with connection status
- **Help System**: Floating keyboard shortcuts modal

### System Status Logging
- **Status Messages**: Real-time system status and alerts
- **Event History**: Timestamped log of system events
- **Connection Status**: Live online/offline status with pulse animation
- **Error Reporting**: Automatic error detection and logging

### Interactive Telemetry Dashboard
- **Animated Values**: Smooth value transitions with color feedback
- **Forward Block Status**: Ultrasonic sensor monitoring (>10cm threshold)
- **Backward Block Status**: HW-870 IR sensor monitoring (digital detection)
- **Obstacle Detection Status**: Master toggle status indicator
- **WiFi Signal Bars**: Animated strength indicators
- **GPS Visualization**: Satellite count with visual indicators
- **Connection Monitoring**: Network status with automatic reconnection
- **Performance Optimized**: Debounced updates and smooth animations
- **Mobile Optimized**: Touch interactions and haptic feedback
- **Auto-refresh**: Real-time updates every 1 second

## Installation

1. **Hardware Assembly**:
   - Connect components according to wiring diagram
   - Use external power supply (7-12V for motors)
   - ESP32 powered via L298N 5V regulator output
   - Mount ultrasonic sensor facing forward

2. **Software Setup**:
   ```bash
   # Install Arduino IDE
   # Add ESP32 board support
   # Install required libraries:
   # - WiFi (built-in)
   # - WebServer (built-in)
   # - LittleFS (built-in)
   # - HardwareSerial (built-in)
   ```

3. **Upload Process**:
   ```bash
   # 1. Upload Arduino sketch normally via Arduino IDE
   
   # 2. Upload web interface files to ESP32 filesystem
   cd ESP32_Network_Controlled_Car/
   ./upload_fs.sh
   
   # Or specify port manually
   ./upload_fs.sh --port /dev/ttyUSB0
   
   # With cleanup after upload
   ./upload_fs.sh --cleanup
   ```

4. **Configuration**:
   - Update WiFi credentials in Arduino code
   - Upload sketch and filesystem
   - Connect to car's IP address via web browser

## Usage

1. **Power On**: Connect battery/power supply
2. **Connect**: Join car's WiFi network or connect to same network
3. **Access**: Open web browser to ESP32's IP address
4. **Control Options**:
   - **Interactive Buttons**: Click 3D styled movement controls
   - **Keyboard**: Use WASD or arrow keys with visual feedback
   - **Speed Control**: Drag animated slider with markers
   - **Buzzer**: Press and hold buzzer button for audio feedback
   - **LEDs**: Click LED buttons to toggle red and blue indicators
   - **Radio Chat**: Type messages in chat interface
   - **Help**: Click keyboard icon for shortcuts guide
5. **Monitor**: View animated real-time telemetry dashboard
6. **Mobile**: Optimized touch interface with haptic feedback

## Troubleshooting

### Common Issues

**WiFi Connection Failed**:
- Check SSID/password
- Verify network availability
- Reset ESP32 and retry

**Motors Not Responding**:
- Check L298N connections
- Verify external power supply (7-12V)
- Ensure L298N VCC connected to external power
- Check ESP32 5V connection from L298N +5V output
- Test motor driver enable pins

**Obstacle Detection Issues**:
- Clean ultrasonic sensor
- Check trigger/echo connections
- Verify 5V power supply from ESP32 to HC-SR04
- Ensure L298N is providing stable 5V output

**HW-870 IR Sensor Issues**:
- Check HW-870 wiring to GPIO17 (digital input)
- Verify 5V power supply to HW-870
- Adjust sensitivity potentiometer on HW-870 module
- Test detection range (3-80cm adjustable)
- Clean IR sensor lens
- Ensure proper mounting angle

**Web Interface Issues**:
- Ensure filesystem uploaded with `./upload_fs.sh`
- Check LittleFS mount in serial monitor
- Verify all files in data/ directory
- Try uploading filesystem again

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
| **Ultrasonic Range** | 2-400cm (HC-SR04) |
| **IR Range** | 3-80cm (HW-870) |
| **Motor Voltage** | 6-12V DC |
| **Logic Voltage** | 5V (from L298N) |
| **Operating Current** | ~1A (total) |
| **Control Interface** | Modern Web Browser |
| **UI Framework** | LittleFS + Modern CSS3/ES6 |
| **Design System** | Glassmorphism with animations |
| **Input Methods** | Mouse/Touch/Keyboard + Haptic |
| **Performance** | Hardware-accelerated animations |
| **Update Rate** | 1Hz (telemetry) |
| **Filesystem** | LittleFS (150KB partition) |

## Future Enhancements

- **Camera Integration**: Live video streaming
- **Navigation System**: Waypoint-based autonomous driving
- **Battery Monitor**: Voltage and charge level display
- **Mobile App**: Dedicated smartphone application
- **Autonomous Mode**: AI-powered obstacle avoidance
- **Geofencing**: Virtual boundary alerts

## Acknowledgments

This project was inspired by the ESP32 WiFi Car tutorial from Random Nerd Tutorials:
https://RandomNerdTutorials.com/esp32-wi-fi-car-robot-arduino/

The project has been significantly expanded with additional features including GPS tracking, radio communication, modern web interface, and comprehensive sensor integration.

## License

This project is open source and available under the MIT License.