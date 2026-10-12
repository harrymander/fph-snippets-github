A non-complete list of all the things I did wrong when I built this board...

Known issues (v3)
=================

- Too many through-hole components - make it quite difficult to solder.

- Component value silkscreen was not printed, so the testpoints are not
  labelled. Not sure if this is a KiCAD issue or an error by the manufacturer.

- Mic. pre-amp feedback capacitors (C25, C26) form a low-pass filter which is
  unnecessary if using the anti-aliasing filters.

Known issues (v2)
=================

- Op-amps used in mic amplifier circuits (MAX4466EXK) use a SC70-5 package (aka
  SOT353), but the footprint on the PCB is for a SOT23-5, which is too large.
  Take care placing part: hand soldering may be required to bridge the pins to
  the pads after reflow. *Using a different op-amp in v3*

- The ADC currently uses the internal voltage reference of 4.096 V. The ADCREF
  pin should be tied to 5 V (perhaps with the addition of a zero-ohm resistor
  or jumper so the internal reference can be used). *Added jumper to 5V in v3*

- Having header for external reset button could be useful. *Added header in v3*

- Microphone amplifier gains still may not be right. Could be a good idea to
  have programmable gains either via programmable gain amplifiers or
  programmable resistors with existing op-amp circuits. *For v3: added jumpers
  to bypass resistors, allows selecting a few different gains; simpler than a
  PGA.*

- SFM3000 flow rate meter range is too small for coughs. *Use SFM3003, which has
  range of -150 - 300 L/min and uses 3.3 V supply, for v3*

- Mounting holes for pressure and flow rate sensors are too small. *Enlarged
  flow meter mounting holes for v3*

- Should add some sort of anti-aliasing filter on ADC inputs. *Added for v3*

- The cutoff frequency for the high-pass filter on the ear microphone pre-amp is
  about 80 Hz, which is way too high. Should be set to same frequency as that of
  the mask amp (8 Hz). *Fixed in v3*

Known Issues (v1)
=================

- STDC connector: I added this so I could use my ST-Link v3 but forgot to finish
  wiring it up. *Removed in v2*

- Serial connector: intended as another way to get data from the instrumentation
  PCB, but one of the signal lines goes right through a capacitor leg. *Removed
  in v2*

- Needs a power button/switch

- Needs headers for external buttons and LEDs. *Added in v2*

- USART SPI chip select is on the wrong USART module, but I don't think we can
  use HW CS anyway so probably not a big loss. *Fixed in v2*

- Power supply for the SPEC H2S sensor. This sensor needs constant power to
  maintain its calibration settings. Some kind of rechargable battery solution
  is needed on board to maintain this. This might not be that important if the
  signal processing only compares the change in H2S level from before and after
  breathing. *No constant power supply for v2 board, but calibration procedure
  has been added to firmware.*

- External power supply connection. Currently the only way to power the
  instrumentation is through the USB connector. This isn't suitable for proper
  battery powered situations.

- Better power supply filtering would be a good idea for ADC accuracy.

- Wiring for the audio jacks is wrong. Connected the signal to the sheath
  instead of the tip. This should be fixed in future versions. Or at least in
  the boxed up version using the two pin header. *Switched to TRRS in v2*

- Possibly limited in sample rate due to SD card transfer rate. Testing suggests
  this should be faster than it currently is, perhaps there are issues in the
  FAT layer... *Unknown - switched to FatFs in software refactor*

- H2S sensor is connected with a 9 pin connector on the PCB while the device
  actually uses 8 pins... *Fixed in v2*

- Trace for IO_SYNC runs through a via connecting R15 to inverting input of U5
  (requires rework to use IO_SYNC) *IO_SYNC removed in v2*
