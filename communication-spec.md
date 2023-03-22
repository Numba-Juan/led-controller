# Communication specifications for controlling RGB LEDs with the microcontroller

## Communication data setup and structure
The data is sent in variable size packets.
Packets contain both commands and data, including colors to be displayed.

Here is the general layout of a packet:

* Header
    * Contains the command to executed
    * Size: 1 byte (static)
* Size
    * Determines the length of data
    * Is omitted for commands with fixed data size (i.e. change to a static single color)
    * Size: 0 bytes to 1 byte (header dependent)
* Data
    * Contains raw data to be sent
    * Divisions in data are usually implied (i.e. RGB vals are fixed 3 bytes long)
    * Omitted depending upon the header
    * Size: dependent on header or size byte

## Commands

### (Re)Initialize LEDs
* Header: `0x00`

No other data is required.

### Solid single color
* Header: `0x01`
* Data: `<RED><GREEN><BLUE>`

Where `<RED>`, `<GREEN>`, and `<BLUE>` are single bytes that correspond to their color code values.

This internally disables any color cycling effects as well.

### New list of effect colors
* Header: `0x02`
* Size: `<NUMBER OF COLORS>`
* Data: `<RED><GREEN><BLUE>...`

Where `<NUMBER OF COLORS>` is the number of `<RED><GREEN><BLUE>` patterns in the data.
The byte size of the data will be this value multiplied by three.

Where `<RED>`, `<GREEN>`, and `<BLUE>` are single bytes that correspond to their color code values, being a continued pattern with no explicit terminator byte.

### Fade effect
* Header: 0x03

This causes all colors to fade to the next color all at once.

No other data is required.

### Cycle effect
* Header 0x04

This causes colors to apply one LED at a time with notable delay.

No other data is required.
