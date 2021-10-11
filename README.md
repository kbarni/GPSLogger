# GPSLogger

GPS logger application for PPK (post process kinematics) for u-blox *Neo M8P* and *Zed F9P* GNSS units.

PPK provides centimeter-precision positioning using logs acquired from a GNSS unit and data form a nearby base station. The correction is not real time, but it doesn't need an internet connection, like RTK. For applications, like precise photo georeferencing this can be an optimal solution.

Typical application would be to connect a Raspberry Pi Zero W or similar to the GNSS unit for data acquisition.

## Features

- Logging of RAW GPS data
- Web server for live monitoring 
- Converting the recorded raw data to csv file

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

The correction is made automatically using the provided `correct.sh`/`correct.ps1` utility.

It needs the `convbin` and `rnx2rtkp` utilities from RTKLib in the current folder.

Useage (Linux/Mac):

    ./correct.sh [inputfile.ubx] [base.21o]

Useage (Windows PowerShell):

    ./correct.ps1 [inputfile.ubx] [base.21o]

The base station data (`[base.21o]` and `[base.21n]`) can be downloaded in France from the [IGN France](rgp.ign.fr/DONNEES/diffusion/) site.

### Manual PPK correction

**RAW to RINEX conversion:**

    convbin -r ubx -o [outfile.obs] [inputfile.ubx]

**PPK correction:**

    rnx2rtk -k rtkpost.conf -o [outfile.pos] [rover.obs] [base.21o] [base.21n]
    
where `rover.obs` is the rover observation file obtained in the previous step; `[base.21o]` and `[base21n]` are the base station data (see above).
