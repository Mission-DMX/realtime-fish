# realtime-fish
The part of the software that translates the users wishes to DMX in real time.

## Compiling
1. Make sure that the following libraries are installed:
	* libnl-dev
	* libev-dev
	* libprotobuf-dev
	* libspdlog-dev
	* protobuf-compiler
	* xsdcxx
	* libxerces-c-dev
	* libusb-dev
	* libftdi-dev
	* pkg-config
	* liblua5.4-dev

   If you would also like to run unit tests, you'll also need libboost-test-dev.
2. Perform a recursive clone of this repository.
3. Call `gmake` on the root directory of this repo. Tip: issue `make -j<ncores> BUILD_MODE=Release`
if you're building a release.

## Using the Behringer X-Touch (Extender) as Input
Please attach the devices using USB and boot them up in `CtrlRel` Mode with `USB` as their
interface protocol.

## USB DMX adapter support
Interfacing with Usb DMX dongles is implemented for FTDI based devices. The corresponding
universe type is called `ftdi_universe`.

A corresponding entry in the `udev` rules should be made, allowing the user associated with
running fish to communicate with the dongle without superuser permissions.

##### Warning
USB dmx adapters are bad. The better ones out there (for example the entec ones) use FTDI chips.
Even those have bad timing properties and only support a small sub set of the DMX512 specification
and should be avoided if a proper Art-Net node (albeit the entec ones are still better than some
crap Art-Net nodes out there) can be used.

##### Warning
Interfacing with USB devices is always a synchronous operation. If your USB adapter is using
a low speed interface (1.0, 1.1, 2.0, 3.0) this will significantly slow down the event loop!

## IO Board support
Communication with the io board is done using an FTDI600Q chip as regular USB2.0 speeds are
nowhere near enough. In order for this to work, the [FT60X](https://github.com/lambdaconcept/ft60x_driver/tree/master)
driver needs to be installed and loaded. A corresponding entry to `/etc/modules` should do the trick.
