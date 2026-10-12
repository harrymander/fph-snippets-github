RECAP Instrumentation
=====================

Data collection device for the RECAP project. This is a reimplementation of the
original data collection prototype that was implemented using a Raspberry Pi.
This implementation is based around a Atmel SAM4S microcontroller connected to
the following sensors:

 - An ADC for audio recording
 - Air flow meter
 - Differential pressure sensor
 - H2S gas sensor

The data is continuously measured by the device and streamed over a USB CDC
interface using a custom data format. See [`sw/README.md`](sw/README.md) for
more information.
