# RECAP instrumentation program

## Building

On Ubuntu/Debian, install dependencies by running:

```
sudo apt update
sudo apt install gcc-arm-none-eabi gdb-multiarch libnewlib-arm-none-eabi openocd
```

The GCC version *must* be 10.3.1, the version in Ubuntu 22.04. If your distro's
package manager doesn't install this version, you can use the supplied Docker
image as a build container. First build the image:

```
docker build -t recap-build:latest .
```

Then to build the firmware, run:

```
./devcontainer.sh make
```

Or just run `devcontainer.sh` without any arguments to open an interactive shell
in the container.

For an alternative to the Docker image, download the older version from
https://developer.arm.com/downloads/-/gnu-rm and add to `PATH`. However, this
version may be slightly different to the Ubuntu package, so builds may not be
reproducible between the different compilers. The Docker image seems to give
identical images to the CI build, which runs on Ubuntu 22.04.

GDB is used to program and debug the microcontroller via OpenOCD. Run `make
program` to flash the application onto the MCU. The MCU will then reset and run
the new program. GDB can be launched using `make debug` which will automatically
connect to the target and halt the program.

If `make program` fails, it is possibly because the *magic bit* (a boot security
bit) isn't set for the MCU. This can be done with `make bootflash`.

The full list of supported commands are:

* `make` build the app for the currnet board target.
* `make program` load the app onto the connected board.
* `make debug` launch GDB and connect to the board.
* `make bootflash` must be run once to allow the MCU to run code from flash.

### Build metadata

A firmware version string is generated from the last tag name and the current
Git commit by the `scripts/version.sh` script. See the *Version string* section
below for more detail.

The GNU build ID (see `ld(1)` or [this
article](https://interrupt.memfault.com/blog/gnu-build-id-for-firmware) for more
info) is embedded into the built binary at the `.gnu_build_id` section. This is
a 20-byte SHA1 checksum that will be the same in all identical binaries - builds
on the same Git revision should produce identical binaries regardless of when
and where they are built. The build ID can be read with `readelf`:

```
arm-none-eabi-readelf -n build/recap-instrumentation.elf
```

On the device, sending a `b` character over the USB CDC interface will cause the
build ID to be printed as a 40-character hexadecimal digest followed by a
newline (LF).

### Testing

See the [README in the `test` subdirectory](./test/README.md).

## Configuration

The build can be configured via environment variables which are written to an
auto-generated `appconfig.h` header file during the build. Changing any of these
options should trigger a rebuild. The most important options with their default
values are listed below:

```bash
# If non-zero, toggle TIMING_TEST_PIO (set in target.h) at the sampling rate.
# Used for testing the accuracy of the sample loop timing (e.g. by measuring the
# pin timing with an oscilloscope).
TEST_TIMING=0

# If non-zero, enables USB/button control for starting/stopping recording.
USB_CONTROL=1
BTN_CONTROL=1

# ADC voltage reference in millivolts. Supported values are 5000 and 4096. 5 V
# reference requires jumping the external AREF pin to 5 V, otherwise the ADC
# uses an internal 4.096 V reference. The voltage reference is only used by the
# firmware to set the correct ADC reference byte in the output data record (see
# below for data record format).
ADC_VREF=5000
```

There are more options that can be configured for testing; see the `Makefile`
for more info.

## Normal operation

On startup, the `STATUS` LED will be solid while the device initialises. When
`STATUS` starts blinking slowly, the device is ready to record. When recording,
the `STATUS` LED will blink rapidly.

### USB data stream

The device will register as a USB CDC device with a vendor ID of `0x03eb` and a
product/model ID of `0x6202`.

When recording starts, the recorded data is output to the USB interface as
described in the *Output format specification* section, with the file size and
offset fields set to 0. The host application is responsible for setting these
fields to their correct values after all data has been read if saving the
measurements to a file using the specified format.

A host application for reading the data stream is available at
https://github.com/harrymander/recap-usb-reader.

### Button control

Enabled by setting `BTN_CONTROL=1`. Press the `RECORD` button to start and stop
recording.

### USB control

Enabled by setting `USB_CONTROL=1`. Write a single `s` (`0x73`) character to
start recording and a `x` (`0x78`) to stop. Note that there may be a delay
between sending `x` and the recording stopping: if in USB output mode the reader
should continue reading until it receives the final sentinel sample.

### H2S sensor calibration

During recording, and if USB control is enabled, sending a `z` (0x7a) character
will zero the H2S sensor. Sensor should sit in a 'clean air' environment for at
least 60 minutes to calibrate. See sensor datasheet for more information.

## Error reporting

If the device is in an error state, the `ERROR` LED will blink rapidly while the
`STATUS` LED will be off.

## Output format specification (version 5)

The output data structure is summarised below. All integers are little-endian.
Signed integers use two's complement encoding.

| Field                    | Description                   |
| ------------------------ | ----------------------------- |
| File signature           | 7 byte ASCII string `-RECAP-` |
| Major version number     | `U8`                          |
| Minor version number     | `U8`                          |
| File size                | `U64`                         |
| Offset to samples        | `U64`                         |
| Header                   | Nul-terminated UTF-8 string   |
| Samples                  | See below                     |
| Sentinel sample          | See below                     |

Individual fields are described in the following subsections.

### File signature

The first 7 bytes of every record file contains the following (decimal) values:

```
45 82 69 67 65 80 45
```

That is, the ASCII-encoded string `-RECAP-`.

### Major and minor version numbers

The major and minor version numbers are unsigned 8-bit integers. The major
version number is incremented when a change is made that is not backwards
compatible. The minor version number is incremented when a new header field
(before the samples) is added, for example. A application that supports reading
a file format of the same major version number will be able to read files with a
higher minor version number by simply ignoring the additional fields.

### File size

The file size field is an unsigned 64-bit integer specifying the size of the
entire file in bytes.

### Offset to first sample

The sample offset field is an unsigned 64-bit integer specifying the offset
relative to the beginning of the file of the first data sample.

### Header

The record header is a nul-terminated UTF-8 string consisting of
newline-separated fields of the format:

```
Key: Value
```

The `Key` and `Value` may not contain any newline characters. Any spaces between
the colon and `Value` are ignored. Spaces following the `Value` are included in
the value, as are any spaces before or after the `Key`. `Key` and `Value` cannot
be empty.

The header must contain the following fields:

| Field name    | Description                                              | Notes |
| ------------- | -------------------------------------------------------- | ----- |
| `Firmware`    | Firmware version string                                  | (1)   |
| `Build`       | SHA1 checksum hexadecimal digest                         | (2)   |
| `Sample-Rate` | Audio sample rate in kilohertz (integer)                 |       |
| `VREF`        | Microphone ADC voltage reference in millivolts (integer) |       |
| `Board`       | Board name                                               |       |
| `Flow`        | Flow meter model                                         |       |
| `Flow-Offset` | Flow meter L/min conversion offset (integer)             | (3)   |
| `Flow-Scale`  | Flow meter L/min conversion scale factor (integer)       | (3)   |
| `Flow-Sign`   | Flow meter integer sign (1 or 0)                         | (4)   |
| `H2S-Barcode` | H2S barcode string                                       |       |

Integer fields must be in decimal notation. The header may contain additional
fields not listed above whose values are undefined.

**Notes**

1. The firmware version string must be in the following format:

   ```
   {major}.{minor}.{patch}[-{extra}]
   ```

   The `major`, `minor`, `patch` numbers must be non-negative integers, The
   optional `extra` version info string at the end must be separated from the
   patch number by a hyphen (`-`) and, if present, must contain at least one
   character and cannot contain whitespace.

2. 40 character hexadecimal digest of the GNU build ID (see above).

3. The flow meter's raw value is converted into litres per minute using the
   following formula:

   ```
   L/min = (raw - offset) / scale
   ```

4. `1` if the flow meter sample field is a signed 16-bit integer, `0` if
   unsigned. (See *Samples* section.)

### Samples

Because the microphones are sampled at a higher sample rate compared to the
other sensors, there are two types of samples sent: *audio samples* and
*complete samples*. There will always be `n - 1` audio samples between complete
samples, where `n` is the audio sample rate in kilohertz (as given by the
`Sample-Rate` header field), that is, the other sensors are read-out at a rate
of 1 kHz, apart from the H2S sensor readings which are sampled at approximately
1 Hz. The first sample and sentinel sample are  are always complete samples.

For example, given an audio sample rate of 5 kHz, a 3 ms record will look like:

```
[file signature, size and offset fields, and header]
[complete sample, millis = 1]
[audio sample]
[audio sample]
[audio sample]
[audio sample]
[complete sample, millis = 2]
[audio sample]
[audio sample]
[audio sample]
[audio sample]
[complete sample, millis = 3]
[audio sample]
[audio sample]
[audio sample]
[audio sample]
[sentinel sample]
```

**Audio sample:**

| Field          | Type  | Unit | Missing value | Note |
| -------------- | ----- | -----| ------------- | ---- |
| `ear-audio`    | `U16` | ADC  | N/A           | (1)  |
| `mask-audio`   | `U16` | ADC  | N/A           | (1)  |

Total size = 4 bytes

**Complete sample:**

| Field          | Type  | Unit                        | Missing value | Note |
| -------------- | ----- | --------------------------- | ------------- | ---- |
| `sample_time`  | `U32` | milliseconds                | N/A           | (2)  |
| `flow`         | `?16` | (See note)                  | N/A           | (3)  |
| `pressure`     | `I16` | 60/pascals                  | N/A           | (4)  |
| `ear-audio`    | `U16` | ADC                         | N/A           | (1)  |
| `mask-audio`   | `U16` | ADC                         | N/A           | (1)  |
| `event`        | `U8`  | bitmask                     | N/A           | (5)  |
| `h2s_ppb`      | `I32` | parts per billion           | `INT32_MIN`   | (6)  |
| `h2s_temp`     | `I8`  | degrees Celsius             | (See note)    | (7)  |
| `h2s_humidity` | `U8`  | percent (relative humidity) | (See note)    | (8)  |

Total size = 19 bytes

**Notes:**

1. The `ear` and `mask` audio readings are 16-bit ADC values referenced to the
   voltage specified by the ADC reference byte in the record header. These
   fields cannot be missing.

2. The first `sample_time` must be 1 millisecond and all times must be in
   consecutive order. Pressure, flow rate, mask audio, and ear audio readings
   are output every sample (1 millisecond). The H2S reading is output
   approximately every second. This field cannot be missing.

3. May be signed or unsigned depending on the value of `Flow-Sign`. The raw
   `flow` readings can be converted to litres per minute (L/min) as follows:

   ```
   L/min = (raw - offset) / scale
   ```

   Where `offset` and `scale` are determined by the `Flow-Offset` and
   `Flow-Scale` header fields. This field cannot be missing.

4. The raw `pressure` readings can be converted in pascals as follows:

   ```
   Pa = raw / 60
   ```

   This field cannot be missing.

5. The least-significant bit (bit 0) of the `event` field is set if the event
   indicator button on the device is being pressed. The remaining 7 bits of this
   field may be set by another application (e.g. the host application) to
   indicate other events of interest. This field cannot be missing.

6. The `h2s_ppb` reading is in parts per billion. If there is no reading for
   this sample, the field has a value of `-0x80000000`.

7. The `h2s_temp` reading is in degrees Celsius. If the `h2s_ppb` field is
   missing, this field is also considered missing and its value is undefined.

8. The `h2s_humidity` reading is in percent relative humidity. If the `h2s_ppb`
    field is missing, this field is also considered missing and its value is
    undefined.

### Sentinel sample

After the last data sample, a *complete sample* with a `sample_time` of 0 is
required. The values of the remaining fields are undefined.

### Old output formats

Version 5 of the record format was a significant change from previous versions.
See the README in the source tree at a tagged version for more detail on a
specific deprecated format.
