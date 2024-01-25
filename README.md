# Introduction
For using the Drongo software, it's possible to configure certain parts of the software through arguments. These arguments can specify the number of geophones and the output folder for the data. Additionally, there are commands that ensure the program automatically starts when the Raspberry PI is powered on.

This manual provides a description of how to install the software, how certain parts of the software can be configured, and how automatic startup works.

## Installing the Program
To install the program, the Raspberry PI must already have a Linux operating system on the SD card.
1. To install the software, 3 software packages are required: cmake, gpiod, and git. Cmake is needed for compiling the program, gpiod contains the libraries required for controlling the GPIO pins, and Git is a code project manager that can be used to fetch code from an online repository. These can be installed on the Raspberry PI with the following command:
   `"sudo apt update && sudo apt install gpiod libgpiod-dev cmake"`
2. Once these packages are installed, the software code can be fetched with the following command:
   `"git clone https://github.com/TiniTinyTerminator/Drongo_software.git"`
   This retrieves the code from the internet to a local folder on your Raspberry PI.
3. Navigate to this folder using `"cd {folder}"`
4. Create a build folder with `"mkdir build"`
5. Enter this folder using `"cd build"`
6. Then execute the following command to compile the software:
   `"cmake ../ && make -j4"`
7. The installation of the program can then be done with the following command:
   `"sudo make install"`

### Installation of Compilation Programs
It may be necessary to install additional software packages for generating the program. For compiling the C++ code of the program, downloading the build-essentials software package may be required:
   `"sudo apt install build-essentials"`

## Starting the Program
To start the program, the command `"drongo_software"` can be invoked via the Linux console. This automatically starts the data acquisition of the measurement computer system. The program will then save data for 4 geophones (the maximum number) to a folder named `"drongo_data"`.

The program can be turned off after data acquisition is complete by pressing CTR-C in the console.

### Startup Arguments
During the startup of the program, arguments can be called to adjust the number of reading channels and the output directory. These can be called as follows:
   `"drongo_software -arg1 -arg2"`
   Where arg1 and arg2 are different arguments.

#### Adjusting the Output Directory
To adjust the output directory, the -o argument with the desired folder can be added to the command as follows:
   `"drongo_software -o {folder}"`
   This also makes it possible to save the data to external storage. Then specify the location where the external storage is attached for the folder.

#### Adjusting the Number of Connected Geophones
The number of geophones that the measurement system should read can be adjusted with the following function:
   `"drongo_software -n {number of geophones}"`
   Here, a number from 1 to 4 can be given for the number of geophones from which data should be read. These are numbered as shown in the figure on the right.

### Automatic Startup of Software When Measurement System is Powered On
It is possible to automatically start the program when it is connected to power. This can be done with systemd, a program for Linux that automates the startup, shutdown, and logging of programs.

To enable automatic startup, the following command can be used:
   `"systemctl enable drongo.service"`
   The program will henceforth be started immediately after Linux boots up.

This can be disabled with the following command:
   `"systemctl disable drongo.service"`
