Lab Equipment Manager
=====================

Lab Equipment Manager based on ESP32 chip.

## Main Features

* QR Code login
* One-Click ON / OFF
* Timeout reminder (Audio prompt)
* LCD display (QR Code / User info / Time remaining)

## Preparing

### Obtain the source

```
git clone --recursive https://github.com/redchenjs/lab_equipment_manager_esp32.git
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

* All project configurations are under the `Lab Equipment Manager` menu.

### Flash & Monitor

```
idf.py flash monitor
```
