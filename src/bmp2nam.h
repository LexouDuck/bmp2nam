/* ************************************************************************** */
/*                                                                            */
/*                                   BMP2NAM                                  */
/*                                                                            */
/* ************************************************************************** */

#ifndef __BMP2NAM_H
#define __BMP2NAM_H

/*!
**	NB: This code uses some abbreviations to refer to certain encodings.
**		Here is a simple index/glossary of these abbreviations:
**	- BMP: (BitMaP file)
**		This is the most basic kind of image file: refers to the input given by the user.
**	- PAL: (PALette file)
**		A small, simple binary file which holds colors
**	- CHR: (CHaR file)
**		The Nintendo-given name for a file which holds only raw pixel data, in 8x8 tiles (no palette).
**	- NAM: (NAMetable file)
**		The Nintendo-given name for a file which holds metatile info (4 8x8 tiles + a palette = one 16x16 metatile)
*/

/*
** ************************************************************************** *|
**                                   Includes                                 *|
** ************************************************************************** *|
*/

#include <libccc.h>
#include <libccc/color.h>
#include <libccc/sys/logger.h>
#include <libccc/math/sort.h>

#include "SDL.h"



/*
** ************************************************************************** *|
**                             Configuration Macros                           *|
** ************************************************************************** *|
*/

// The arbitrary threshold used for color merging
#define THRESHOLD	8686



/*!	@defgroup BMP
**	@{
**		BMP2NAM input BMP file
*/

//! The amount of bits-per-pixel in the input BMP file (usually written BPP)
#define BMP_BPP		(8)

//! The maximum amount of colors in the input BMP file
#define BMP_MAXCOLORS	(256)

//! @}



/*!	@defgroup REFPAL
**	@{
**		BMP2NAM reference palette file
**	All the possible colors for the output files (PAL/CHR/NAM)
*/

//! The size (in bytes) of one color in the reference palette file
#define REFPAL_COLORSIZE	(3)
//! The amount of colors in the reference palette file
#define REFPAL_COLORS		(64)
//! The size (in bytes) of the reference palette file
#define REFPAL_SIZE		(t_size)(REFPAL_COLORS * REFPAL_COLORSIZE)

//! The (relative) file path of the reference palette file
#define REFPAL_FILEPATH		"pal/nes.pal"

//! @}



/*!	@defgroup PAL
**	@{
**		NES Palette Format
**	Stores 4 distinct palettes of 4 colors each.
**	These colors are stored as indices to the standard NES palette.
**	The first color each of these four palettes must be the same.
*/

//! The amount of colors contained in one of the output color palettes
#define PAL_SUB_COLORS	(4)
//! The amount of palettes contained in the output PAL file
#define PAL_SUB_AMOUNT	(4)

//! The size (in bytes) of one color contained in one of the output color palettes
#define PAL_COLORSIZE	(1)
//! The amount of colors contained in the output PAL file, in total
#define PAL_COLORS		(PAL_SUB_COLORS * PAL_SUB_AMOUNT)
//! The size (in bytes) of the output PAL file
#define PAL_SIZE	(t_size)(PAL_COLORSIZE * PAL_COLORS)

//! the filepath prefix/suffix for the output PAL file
#define PAL_FILE(X)		X".pal"

//! @}



/*!	@defgroup CHR
**	@{
**		NES 8x8 Tiled Pixel Data Format
**	Stores 256 distinct 'tiles' (or 'chars', hence chr) of 8x8 pixel data.
**	The colors for these pixels are 2-bits (can be 0-3) to go with a PAL.
*/

//! The amount of bits-per-pixel for the output CHR file (usually written BPP)
#define CHR_BPP		(2)
//! The dimensions of a CHR tile (in pixels, used for both width and height)
#define CHR_TILE	(8)

//! The maximum amount of colors for the output CHR file
#define CHR_MAXCOLORS	(4)

//! The width (in CHR tiles) of the output CHR file
#define CHR_W_TILES		(16)
//! The height (in CHR tiles) of the output CHR file
#define CHR_H_TILES		(16)

//! The width (in pixels) of the output NAM file
#define CHR_W	(CHR_W_TILES * CHR_TILE)
//! The height (in pixels) of the output NAM file
#define CHR_H	(CHR_H_TILES * CHR_TILE)

//! The size (in bytes) of a single CHR tile
#define CHR_SIZE_TILE	(t_size)((CHR_BPP * CHR_TILE * CHR_TILE) / 8)
//! The size (in bytes) for the output CHR file
#define CHR_SIZE		(t_size)(CHR_SIZE_TILE * CHR_W_TILES * CHR_H_TILES)

//! the filepath prefix/suffix for the output CHR file
#define CHR_FILE(X)		X".chr"

//! @}



/*!	@defgroup NAM
**	@{
**		NES Nametable & Attributes Format
**	Stores 960 bytes of metatile data and 64 bytes of palette data (1kb total).
**	The Nametable data describes how four 8x8 CHR tiles are grouped together
**	into 16x16 'metatiles' to make up a tileset of 16x16-size tiles (where
**	the data has a row-width of 32 bytes).
**	The Attributes data stores 64 bytes of palette info for the 16x16 metatiles.
*/

//! The dimensions of a NAM metatile (in pixels, used for both width and height)
#define NAM_TILE		(16)

//! The maximum amount of colors for the output NAM file
#define NAM_MAXCOLORS	(16)

//! The width (in NAM metatiles) of the output NAM file
#define NAM_W_TILES		(16)
//! The height (in NAM metatiles) of the output NAM file
#define NAM_H_TILES		(15)

//! The total amount of NAM metatiles in the output NAM file
#define NAM_TILES	(NAM_W_TILES * NAM_H_TILES)

//! The width (in pixels) of the output NAM file
#define NAM_W	(NAM_W_TILES * NAM_TILE)
//! The height (in pixels) of the output NAM file
#define NAM_H	(NAM_H_TILES * NAM_TILE)

//! The size (in bytes) of a single NAM metatile combo
#define NAM_SIZE_TILE	(t_size)(1)
//! The size (in bytes) of the NAM attributes section (with the palette association data)
#define NAM_SIZE_ATTR	(t_size)(64)
//! The size (in bytes) for the output NAM file
#define NAM_SIZE		(t_size)(NAM_SIZE_TILE * NAM_W_TILES * NAM_H_TILES + NAM_SIZE_ATTR)

//! the filepath prefix/suffix for the output NAM file
#define NAM_FILE(X)		X".nam"

//! @}



/*
** ************************************************************************** *|
**                         Main Program Types & Globals                       *|
** ************************************************************************** *|
*/

//! Stores information about one color palette, and whether is it unique or not
typedef struct	s_palette_
{
	t_sint		duplicate;				//!< If negative, this palette is unique; otherwise index to the first identical palette
	t_bool		identical;				//!< If TRUE, then palette is idential (equal size) to its parent `duplicate`
	t_u32		popularity;				//!< The popularity of this palette (ie: how many NAM tiles use this palette ?)
	t_u8		length;					//!< The amount of colors in this palette (can be any number between `0` and `PAL_SUB_COLORS`)
	t_u8		colors[PAL_SUB_COLORS];	//!< A single output palette, storing the `length` most used colors
}				s_palette;

typedef struct	s_colordiff_
{
	t_s64		value;
	t_u8		index;
}				s_colordiff;

//! Stores information about one color, and how many pixels use it (ie: how popular is the color ?)
typedef struct	s_color_use_
{
	t_argb32	color;		//!< The actual ARGB value for the color in question
	t_u8		index;		//!< The index of the color in the loaded bitmap/tile's palette
	t_u32		occurences;	//!< The amount of occurences/uses of this color in the bitmap/tile
}				s_color_use;

//! Stores information about the colors used in one NAM tile
typedef struct	s_tiles_use_
{
	t_u8		total;		//!< The total amount of different unique colors used in this tile
	s_palette	palette;	//!< The palette for this tile
	s_color_use	colors[BMP_MAXCOLORS];	//! The list of colors (sorted by most-to-least frequently used)
}				s_tiles_use;

//! Stores all of this program's internal state
typedef struct	s_program_
{
	t_char const*	called;	//!< The name of the program, as it was called by the commandline (typically full path)
	s_logger		logger;	//!< The logger, holds internal state for logging to terminal output
	t_char const*	file_input;		//!< (user-specified) The input filepath (with .bmp file extension)
	t_char const*	file_output;	//!< (user-specified) The output filepath (without the file extension)
	t_uint			expected_w;		//!< (user-specified) The expected width (in pixels) for the bitmap file
	t_uint			expected_h;		//!< (user-specified) The expected width (in pixels) for the bitmap file
	s_color_use		colorkey;		//!< (user-specified) The colorkey value provided by the user - if none is specified via argv, then `.colorkey.occurences` will be 0
	t_argb32		ref_palette[REFPAL_COLORS];		//!< (user-specified) The reference palette to use for outputting, and comparing nearest colors from the BMP
	s_palette		output_palettes[PAL_SUB_AMOUNT];//!< (user-specified, or generated) The output palette(s) to use
/*
	t_float*	certainty;		//!< The array of tile/palette association certainty values
	t_float		threshold_lo;	//!< The uncertainty threshold, below which a palette must be thrown out
	t_float		threshold_hi;	//!< The certainty threshold, above which a palette must be forcibly kept
*/
	SDL_Surface*bitmap;							//!< The input file (loaded .bmp file as an SDL_Surface)
	t_u32		bitmap_colors_total;			//!< Whether or not there are to many different unique colors in this bitmap/tile
	s_color_use	bitmap_colors[BMP_MAXCOLORS];	//!< The total amounts of colors used in the bitmap
	s_color_use occur_colors[PAL_COLORS];		//!< The 16 "most used" colors (used to assert the final tileset palettes)
	s_tiles_use	tiles_colors[NAM_TILES];		//!< The total amounts of colors used, per CHR tile
	s_palette	tiles_palettes[NAM_TILES];		//!< The minimum necessary amount of palettes for all tiles (assuming lossless)
	t_u32		tiles_palettes_amount;			//!< The total amount of unique palettes necessary for the bitmap
}				s_program;



//! The total amount of possible unique program option flags
typedef struct	s_program_arg_
{
	t_bool			(*handle_arg)(t_char const*);
	t_char			arg_char;		//!< The character for the short form of this argument: for example, "o" for `-o`
	t_char const*	arg_long;		//!< The string name for the long form of this argument: for example, "output" for `--output`
	t_bool			has_value;		//!< If TRUE, the program option expects a value after an equal char: `--output=./path/to/file.txt`
	t_char const*	description;	//!< The description for this argument, as shown in the `--help`
}				s_program_arg;



typedef enum	e_program_arg_
{
	PROGRAM_ARG_HELP = 0,
	PROGRAM_ARG_VERBOSE,
	PROGRAM_ARG_BITMAP_W,
	PROGRAM_ARG_BITMAP_H,
	PROGRAM_ARG_PALETTE,
	PROGRAM_ARG_COLORKEY,
PROGRAM_ARGS_AMOUNT
}				e_program_arg;



//! This is global variable which holds all internal state for the program
extern s_program	program;



/*
** ************************************************************************** *|
**                           Core Utility Functions                           *|
** ************************************************************************** *|
*/

t_u32	Color_Sum(t_argb32 color);

//! sort indexed colors of the `ref_palette`, by brightness
int Compare_ColorDiffs(s_colordiff c1, s_colordiff c2);
DEFINEFUNC_H_QUICKSORT(s_colordiff, Compare_ColorDiffs)

//! sort colors by popularity
int Compare_ColorUse(s_color_use c1, s_color_use c2);
DEFINEFUNC_H_QUICKSORT(s_color_use, Compare_ColorUse)

//! sort palettes by popularity
int Compare_Palette(s_palette c1, s_palette c2);
DEFINEFUNC_H_QUICKSORT(s_palette, Compare_Palette)

//! sort indexed colors of the `ref_palette`, by brightness
int Compare_Color(t_u8 c1, t_u8 c2);
DEFINEFUNC_H_QUICKSORT(t_u8, Compare_Color)



//! 
t_sint	Palette_Find(s_palette const* palette, t_u8 color);
//! 
t_bool	Palette_Contains(s_palette const* palette, t_u8 color);
//! 
t_bool	Palette_ContainsAll(s_palette const* palette, s_palette const* target);
//! 
s_palette	Palette_GetMostUsedColors(s_color_use const* colors, t_u8 maxlength);
//! 
s_palette const*	Palette_GetNearest(s_palette target, s_palette const* palettes, t_uint length);



//! Returns a new string which displays a colored square in the commandline output
t_char*	ANSI_GetColor(t_argb32 color);
//! Returns a new string which displays a set of colored squares in the commandline output
t_char*	ANSI_GetPalette(s_palette* palette);



int		PrintColorStats(s_color_use const* array, t_size length);



/*
** ************************************************************************** *|
**                           Core Program Functions                           *|
** ************************************************************************** *|
*/

int		CheckBitmap_LoadReferencePalette(void);
int		CheckBitmap_LoadColors(void);
int		CheckBitmap_PixelFormat(void);
int 	CheckBitmap_Dimensions(void);
int		CheckBitmap_TotalColors(void);
int		CheckBitmap_TilesColors(void);
int		CheckBitmap_DuplicatePalettes(void);

int		ConvertBitmap_ApplyRefPalette(void);
int		ConvertBitmap_TotalColorReduction(void);
int		ConvertBitmap_TilesColorReduction(void);
int		ConvertBitmap_AssertOutputPalettes(void);
int		ConvertBitmap_ApplyOutputPalettes(t_bool user_palette);



#endif
