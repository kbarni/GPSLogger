# GPSLogger

GPS logger application for PPK (post process kinematics) for u-blox *Neo M8P* and *Zed F9P* GNSS units.

PPK provides centimeter-precision positioning using logs acquired from a GNSS unit and data form a nearby base station. The correction is not real time, but it doesn't need an internet connection, like RTK. For applications, like precise photo georeferencing this can be an optimal solution.

Typical application would be to connect a Raspberry Pi Zero W or similar to the GNSS unit for data acquisition.

## Features

- Logging of RAW GPS data
- Web server for live monitoring 

Planned features:

- integrate RAW to RINEX conversion using RTKLIB.

## Prerequisites

- cmake
- QT5 (with QSerialPort and QWebSockets modules)

## Building and installation

    mkdir build
    cd build
    cmake ..
    make
    sudo make install
    
For full functionality build the webserver and enable the provided systemd service.

## PPK correction using RTKLIB command line utilities

**RAW to RINEX conversion:**

    convbin [inputfile.raw] -r ubx -o [outfile.obs]

**PPK correction:**

Use the `rtkpost` (or `rtkpost_qt`) utility from RTKLib, and use the settings from the provided `rtkpost_gpslogger.conf` file.

Download the base station data from [IGN France](rgp.ign.fr/DONNEES/diffusion/) site, load the previously created rover `obs`, the base station `.21o` and `.21n` files and run *Execute*
