# Door Sensor System

## Overview
This project is a door monitoring system that utilizes two sensors to track movement through two different doors. The system consists of:
- **Two motion sensors** to detect activity near each door.
- - **PIR sesnor** to detect movement.
- **A Python-based GUI** to visualize and manage door status in real-time.

The project is designed for security and monitoring purposes, providing insights into movement trends.

## Features
- Real-time detection of movement using motion sensors
- Separate monitoring for two doors
- Interactive GUI for visualization
- Data logging for movement events
- Easy setup and configuration

## Technologies Used
- **Python** for the main logic and GUI
- **Tkinter** for the graphical interface
- **Raspberry Pi and Arduino**for sensor integration

## Installation
### Prerequisites
Ensure you have the following installed:
- Python 3.x
- Required Python libraries (install using `pip`):
  ```sh
  pip install tkinter py-serial
  ```
- Hardware setup (if using external sensors)

### Running the Application
1. Clone this repository:
   ```sh
   git clone https://github.com/Igoro8064/door-count.git
   cd door-count
   ```
2. Run the Python script:
   ```sh
   python GUI.py
   ```
3. Open the GUI and monitor door movement in real-time.

## Usage
- **Start the application**: Run `GUI.py` to launch the GUI.
- **View real-time movement updates**: The GUI displays movement detected by each sensor.
  


## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact
For any questions or contributions, feel free to reach out by creating an issue on GitHub.

