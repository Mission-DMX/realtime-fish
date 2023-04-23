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
2. Perform a recursive clone of this repository.
3. Call `gmake` on the root directory of this repo.

## USB DMX adapter support
Interfacing with Usb DMX dongles is implemented for FTDI based devices. The corresponding
universe type is called `ftdi_universe`.

##### Warning
USB dmx adapters are bad. The better ones out there (for example the entec ones) use FTDI chips.
Even those have bad timing properties and only support a small sub set of the DMX512 specification
and should be avoided if a proper Art-Net node (albeit the entec ones are still better than some
crap Art-Net nodes out there) can be used.

##### Warning
Interfacing with USB devices is always a synchronous operation. If your USB adapter is using
a low speed interface (1.0, 1.1, 2.0, 3.0) this will significantly slow down the event loop!

