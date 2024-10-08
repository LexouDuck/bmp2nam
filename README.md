# bmp2nam
---

This is a tool which can color-reduce bitmap images to make them tilemap-friendly.

NOTE: this tool is still quite experimental and incomplete, and has several quirky behaviors which need to be fixed.

---

Here is an example of how it works, taking in a very restrictive set of limitations such as the [graphical limitations of the NES](https://nesrocks.com/blog/nes-graphics/):

- using as input the following image:

![](ref/example-input.bmp)

- it spits out the following image:

![](ref/example-output.bmp)

- and here is what the commandline output log looks like:

![](ref/cli-log.png)

---

this tool was originally made for NES tile graphics in particular.

Here is a quick but pretty detailed [summary of graphical limitations on the NES](https://nesrocks.com/blog/nes-graphics/):

- 2 types of graphics: "sprites" (which can be freely placed anywhere onscreen or offscreen) and "tiles" (which must be positioned aligned to the background tile grid)
- tiles are 8x8 pixels in size and are stored in tilesets holding 256 tiles each.
- groups of tiles in a 2x2 tile layout form a 16x16 pixel "meta-tile".
- 4 colors per meta-tile (i.e. a 16x16 square of background)
- there are 8 palettes (of 4 colors each): 4 palettes for sprites, and 4 palettes for tiles
- only 64 sprites (not meta-sprites) can be drawn on the screen (can be "solved" using flickering)
- only the 8 first sprites of any horizontal scanline will be drawn (can be "solved" using flickering)
- sprites can be 8x8 pixels in size (or 8x16 when a certain mode is enabled)
- 4 colors per sprite (first of which is the transparent color, so 3 usable colors really)
- groups of 8x8 sprites area often assembled into larger "meta-sprites"
