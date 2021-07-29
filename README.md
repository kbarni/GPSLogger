# GPSLogger

GPS logger application for PPK (post process kinamatics) for u-blox Neo M8P or Zed F9P GNSS units.

Typical application would be to connect a Raspberry Pi Zero W to the GPS for data acquisition.

## Features

- Logging of RAW GPS data
- Web server for live monitoring 

Planned features:

- integrate RAW to RINEX conversion using RTKLIB.

## Prerequisites

- cmake
- QT5 (with QSerialPort and QWebSockets)

## Building and installation

    mkdir build
    cd build
    cmake ..
    make
    sudo make install
    
For full functionality build the webserver and enable the provided systemd service.
