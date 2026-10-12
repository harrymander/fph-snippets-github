# Code samples

1. `recap-instrumentation`: Research data collection device.

   C project for Atmel (Microchip) ATSAM4S16B using Freertos. Modules of
   interest:

   * `recap-instrumentation/sw/src/sensors.c`: sensor reading in timer ISR.
   * `recap-instrumentation/sw/drivers/twi-async/twi-async.c`: interrupt-driven
     'asynchronous' I2C read.

2. `recap-usb-reader`: Host application for the device above.

   Cross-platform C++ project for realtime data acquisition and visualisation.
   Uses Dear ImGui immediate mode GUI framework. GUI and application logic run
   from the event loop of graphics framework (DirectX on Windows, GLFW on
   Linux).

   Modules of interest:

   * `recap-usb-reader/gui/src/device.hpp`: boundary between the device library
     and the GUI. State pattern.
   * `recap-usb-reader/gui/src/observable.hpp`: observer pattern used to
     decouple the device layer from the GUI views.
   * `recap-usb-reader/lib/usb-reader/include/recap/usb-reader/reader.hpp`:
     device library interface.

3. `swallow-labeller`: Tool for annotating recordings from the device.
   (`complete-swallow-labelling` branch).

   C++, Dear ImGui. Modules of interest:

   * `swallow-labeller/src/gui/task-filter.cpp`: defining filters for different
     items in GUI list. Example of runtime polymorphism.
   * `swallow-labeller/src/gui/task-view.cpp`, lines 203-346: undo/redo using
     the command pattern.
