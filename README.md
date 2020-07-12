Laboratory Power Manager
========================

Laboratory Power Manager based on ESP32 chip.

## Main Features

* QR Code Login
* One-click ON/OFF
* Timeout Reminder (Audio)
* LCD Display (QR Code / User Info / Time Remaining)

## Preparing

### Obtain the source

```
git clone --recursive https://github.com/redchenjs/laboratory_power_manager_esp32.git
```

### Update an existing repository

```
git pull
git submodule update --init --recursive
```

### Setup the tools

```
./esp-idf/install.sh
```

## Building

### Setup the environment variables

```
export IDF_PATH=$PWD/esp-idf
source ./esp-idf/export.sh
```

### Configure

```
idf.py menuconfig
```

* All project configurations are under the `Laboratory Power Manager` menu.

### Flash & Monitor

```
idf.py flash monitor
```
