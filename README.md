# nRF5_SDK_MIDI

This project adds USB MIDI support for the Nordic Semiconductor nRF5 SDK 17. It contains usb midi class support and an example of its use. The files should be placed according to their paths in the nRF5 SDK.

The code is not extensively tested and should not be regarded as stable as of yet. The USB MIDI function is also not configurable. For now the MIDI descriptor is set in midi_usbd_descriptors.h similar to example given in USB Device Class Definition for MIDI Devices(https://www.usb.org/document-library/usb-midi-devices-10). This will be further improved upon.
