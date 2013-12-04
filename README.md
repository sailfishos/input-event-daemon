Input Event Daemon
==================
Input-event-daemon is a program that streams input events from network to a
input device.

The daemon listens for tcp connections at port 7856 by default. The daemon also
creates a new input device to /dev/input. When a new tcp connection is
establised, the daemon streams any input events it receives via that connection
to the input device.

The daemon is used by SDK's Emulator Control which uses it to emulate device's
hardware keys; power, volume up and volume down keys.
