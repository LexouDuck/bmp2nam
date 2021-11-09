# PAL file format specification
-----

The PAL/PALETTE file format is part of the STDIMG standard formats spec.

These file formats are meant to be lightweight, ambivalent, comprehensive interchange formats for image data.
Specifically, these are geared towards video game image data, where it is common to store color palettes, pixel data, and tilemapping data separately.

There are two equivalent file formats:
- Text format: `.palette` (which is based on the [TOML specification](https://toml.io/en/v1.0.0#spec))
- Binary format: `.pal`, which is comprised of:
	- a fixed-size "**FILE_HEADER**" section (16 bytes)
	- a variable-size "**FILE_CONTENT**" section (size can be known by reading the header)



Here is the legend for how to read each section of the specification:
- "SIZE" is the size (in bytes) of that section
- "DATA" explains the data type of that section
- "TEXT" details the TOML field name of section



-----

## FILE_HEADER

- **DATASIZE**: 16 bytes
- **CONTENTS**: `[magic][color_amount][color_size][encoding][amount]`
Here is the binary octet equivalent notation:
```
offset	0	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15
bytes	'P'	'A'	'L'	[V]	[AA	AA]	[CC	CC]	[EE	EE	EE	EE]	[BB	BB	BB	BB]

- The first three bytes must be the ASCII string "PAL": this is the magic identifier
- [V] is "version": version number of the .pal file encoding used
- [A] is "color_amount": amount of colors in total
- [C] is "color_size": size of one color in the palette
- [E] is "encoding": characters which define
- [B] is "encoding_bits": characters which define
```



### Amount of colors

- **DATASIZE**: 2 bytes
- **DATATYPE**: 16-bit unsigned integer (`u16`)
- **TEXTNAME**: `color_amount`
- **TEXTTYPE**: integer (in UInt16 range)

Expresses the amount of colors contained in this palette.



### Color size

- **DATASIZE**: 2 bytes
- **DATATYPE**: 16-bit unsigned integer (`u16`)
- **TEXTNAME**: `color_size`
- **TEXTTYPE**: integer (in UInt16 range)

Expresses the total size (in bytes) of one color in the palette.



### Color encoding type

- **DATASIZE**: 4 bytes
- **DATATYPE**: ASCII string (`char[4]`)
- **TEXTNAME**: `encoding`
- **TEXTTYPE**: string (maximum 4 characters)

This field is comprised of four ASCII characters, in any order:
- ` `: The space character is a stand-in for nothing, ie: no data
- `A`: An alpha channel value (expresses color transparency, 0 being opaque)
- `R`,`G`,`B`: An additive-colorspace value channel (for "red", "green", "blue" respectively)
- `H`,`S`,`L`: An hue/sat/lum color model value channel (for "hue", "saturation", "luminance" respectively)
- `C`,`M`,`Y`,`K`: A substractive-colorspace value channel (for "cyan", "magenta", "yellow", "black" respectively)



### Color encoding bits

- **DATASIZE**: 4 bytes
- **DATATYPE**: fixed-size array of 4 bytes (`u8[4]`)
- **TEXTNAME**: `encoding_bits`
- **TEXTTYPE**: array of 4 integers (in UInt8 range)

Four unsigned numbers, which indicate the amount of bits used by each color channel, respectively.



-----

## FILE_CONTENT

- **DATASIZE**: `(color_amount * color_size)` bytes
- **CONTENTS**: `[palette]`



### Palette colors

- **DATASIZE**: `(color_amount * color_size)` bytes
- **DATATYPE**: binary data with variable interpretation (`u8[]`)
- **TEXTNAME**: `palette`
- **TEXTTYPE**: can be one of several types:
	- array of integers (size should be equal to `color_amount`, so each integer is one color)
	- array of arrays of integers (each sub-array should have the same size, each integer is one color channel value)

What follows is the actual binary data of the palette, which should be `(color_amount * color_size)` bytes long.
