
#include <libccc.h>
#include <libccc/color.h>
#include <libccc/memory.h>
#include <libccc/string.h>
#include <libccc/sys/logger.h>
#include <libccc/math/math.h>
#include <libccc/math/sort.h>

#include "SDL.h"



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
#define REFPAL_FILEPATH		"gen/nes.pal"

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
	char const*	called;	//!< The name of the program, as it was called by the commandline (typically full path)
	s_logger	logger;	//!< The logger, holds internal state for logging to terminal output
	char const*	file_input;		//!< (user-specified) The input filepath (with .bmp file extension)
	char const*	file_output;	//!< (user-specified) The output filepath (without the file extension)
	t_uint		expected_w;		//!< (user-specified) The expected width (in pixels) for the bitmap file
	t_uint		expected_h;		//!< (user-specified) The expected width (in pixels) for the bitmap file
	s_color_use	colorkey;		//!< (user-specified) The colorkey value provided by the user - if none is specified via argv, then `.colorkey.occurences` will be 0
	t_argb32	ref_palette[REFPAL_COLORS];		//!< (user-specified) The reference palette to use for outputting, and comparing nearest colors from the BMP
	s_palette	output_palettes[PAL_SUB_AMOUNT];//!< (user-specified, or generated) The output palette(s) to use
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



//! This is global variable which holds all internal state for the program
s_program	program;



//! The total amount of possible unique program option flags
typedef struct	s_program_arg_
{
	t_bool		(*handle_arg)(char const*);
	char		arg_char;		//!< The character for the short form of this argument: for example, "o" for `-o`
	char const*	arg_long;		//!< The string name for the long form of this argument: for example, "output" for `--output`
	t_bool		has_value;		//!< If TRUE, the program option expects a value after an equal char: `--output=./path/to/file.txt`
	char const* description;	//!< The description for this argument, as shown in the `--help`
}				s_program_arg;

static t_bool	HandleArg_Verbose(char const* arg)
{
	if (arg == NULL)	return (ERROR);
	program.logger.verbose = TRUE;
	return (OK);
}

static t_bool	HandleArg_BitmapWidth(char const* arg)
{
	if (arg == NULL)	return (ERROR);
	program.expected_w = U32_FromString(arg);
	if (program.expected_w == 0)
		return (ERROR);
	return (OK);
}

static t_bool	HandleArg_BitmapHeight(char const* arg)
{
	if (arg == NULL)	return (ERROR);
	program.expected_h = U32_FromString(arg);
	if (program.expected_h == 0)
		return (ERROR);
	return (OK);
}

static t_bool	HandleArg_Palette(char const* arg)
{
	if (arg == NULL)	return (ERROR);
	t_fd fd = IO_Open(arg, OPEN_READONLY, 0);
	if (fd < 0)
	{
		Log_Error_IO(&program.logger, 0, "Could not open user-specified palette file: %s", arg);
		return (ERROR);
	}
	t_u8* file = NULL;
	t_sintmax size = IO_Read_File(fd, (void**)&file, 0);
	if (size < 0)
	{
		Log_Error_IO(&program.logger, 0, "Could not read user-specified palette file: %s", arg);
		return (ERROR);
	}
	if ((t_size)size != PAL_SIZE)
	{
		Log_Warning(&program.logger, "Palette file specified has incorrect size: %s (was %zu bytes, but should be %zu bytes)",
			arg, (t_size)size, PAL_SIZE);
		return (ERROR);
	}
	t_size index = 0;
	for (int i = 0; i < PAL_SUB_AMOUNT; ++i)
	{
		program.output_palettes[i].length = PAL_SUB_COLORS;
		for (int j = 0; j < PAL_SUB_COLORS; ++j)
		{
			program.output_palettes[i].colors[j] = file[index++];
		}
	}
	Memory_Delete((void**)&file);
	return (OK);
}

static t_bool	HandleArg_ColorKey(char const* arg)
{
	if (arg == NULL)	return (ERROR);
	t_argb32 color = U32_FromString_Hex(arg);
	t_argb32 const* match = Color_ARGB32_GetNearest(color, program.ref_palette, REFPAL_COLORS);
	if (match == NULL)
	{
		Log_Error(&program.logger, 0, "Error while processing color key argument given");
		return (ERROR);
	}
	t_u8 index = (match - program.ref_palette);
	program.colorkey = (s_color_use)
	{
		.color = match[0],
		.index = index,
		.occurences = TRUE,
	};
	return (OK);
}



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

//! This is the list of accepted program arguments
static s_program_arg const	program_args[PROGRAM_ARGS_AMOUNT] =
{
	(s_program_arg){ NULL,					'h', "help",	 FALSE,	"If provided, display only the program usage help and exit." },
	(s_program_arg){ HandleArg_Verbose,		'v', "verbose",	 FALSE,	"If provided, displays additional information while processing the BMP." },
	(s_program_arg){ HandleArg_BitmapWidth,	'w', "bitmap_w", FALSE,	"(expects value, integer: `-w=256`) If provided, sets the expected bitmap width dimension." },
	(s_program_arg){ HandleArg_BitmapHeight,'h', "bitmap_h", FALSE,	"(expects value, integer: `-h=240`) If provided, sets the expected bitmap height dimension." },
	(s_program_arg){ HandleArg_Palette,		'p', "palette",	 TRUE,	"(expects value, filepath: `-p=./path/to/file.pal`) If provided, forces the output to use the given palette (must be a binary .pal file, containing at most 64 different 32-bit colors)." },
	(s_program_arg){ HandleArg_ColorKey,	'c', "colorkey", TRUE,	"(expects value, color: `-c=FF00FF`) If provided, the given color value will be present as the first color for all palettes."}
};
//! A special return value to signal when a help argument has been provided by the user
#define MATCHED_HELP	((int)-1)



/*
** ************************************************************************** *|
**                           Core Utility Functions                           *|
** ************************************************************************** *|
*/

static
t_u32	Color_Sum(t_argb32 color)
{
	return (
		(t_u32)Color_ARGB32_Get_R(color) +
		(t_u32)Color_ARGB32_Get_G(color) +
		(t_u32)Color_ARGB32_Get_B(color));
}



// sort indexed colors of the `ref_palette`, by brightness
static
int Compare_ColorDiffs(s_colordiff c1, s_colordiff c2)
{
	return (c2.value - c1.value);
}
DEFINE_QUICKSORT(s_colordiff, Compare_ColorDiffs)

// sort colors by popularity
static
int Compare_ColorUse(s_color_use c1, s_color_use c2)
{
	return (c1.occurences - c2.occurences);
}
DEFINE_QUICKSORT(s_color_use, Compare_ColorUse)

// sort palettes by popularity
static
int Compare_Palette(s_palette c1, s_palette c2)
{
	return (c1.popularity - c2.popularity);
}
DEFINE_QUICKSORT(s_palette, Compare_Palette)

// sort indexed colors of the `ref_palette`, by brightness
static
int Compare_Color(t_u8 c1, t_u8 c2)
{
	return (Color_Sum(program.ref_palette[c2]) - Color_Sum(program.ref_palette[c1]));
}
DEFINE_QUICKSORT(t_u8, Compare_Color)



static
char*	GetColor(t_argb32 color)
{
	return (String_Format(IO_COLOR_BG("%i")"  "IO_RESET, IO_GetColor(color)));
}

static
char*	GetPalette(s_palette* palette)
{
	char* tmp = NULL;
	char* result = String_New(0);
	if (result == NULL)
	{
		Log_Error(&program.logger, 0, "Could not allocate palette logging output string");
		return (NULL);
	}
	for (int j = 0; j < PAL_SUB_COLORS; ++j)
	{
		if (j < palette->length)
			tmp = GetColor(program.ref_palette[palette->colors[j]]);
		else tmp = String_Duplicate("[]");
		String_Merge(&result, &tmp);
	}
	String_Append(&result, " =");
	for (int j = 0; j < PAL_SUB_COLORS; ++j)
	{
		if (j < palette->length)
			tmp = String_Format(" %.2X", palette->colors[j]);
		else tmp = String_Duplicate(" __");
		String_Merge(&result, &tmp);
	}
	return (result);
}



static
t_sint	Palette_Find(s_palette const* palette, t_u8 color)
{
	for (int i = 0; i < palette->length; ++i)
	{
		if (palette->colors[i] == color)
			return (i);
	}
	return (-1);
}

static
t_bool	Palette_Contains(s_palette const* palette, t_u8 color)
{
	for (int i = 0; i < palette->length; ++i)
	{
		if (palette->colors[i] == color)
			return (TRUE);
	}
	return (FALSE);
}

static
t_bool	Palette_ContainsAll(s_palette const* palette, s_palette const* target)
{
	for (int i = 0; i < target->length; ++i)
	{
		if (!Palette_Contains(palette, target->colors[i]))
			return (FALSE);
	}
	return (TRUE);
}



static
s_palette	Palette_GetMostUsedColors(s_color_use const* colors, t_u8 maxlength)
{
	s_palette result = (s_palette)
	{
		.duplicate = -1,
		.identical = FALSE,
		.length = 0,
		.colors = {0},
	};
	for (int i = 0; i < maxlength; ++i)
	{
		if (colors[i].occurences == 0)
		{
			result.length = i;
			return (result);
		}
		result.colors[i] = colors[i].index;
	}
	result.length = maxlength;
	return (result);
}

static
t_s64	GetSmallestColorDifference(t_u8 target, s_palette const* palette)
{
	t_s64 result = S64_MAX;
	t_s64 diff;
	for (int i = 0; i < palette->length; ++i)
	{
		diff = (t_s64)Color_ARGB32_Difference(
			program.ref_palette[target],
			program.ref_palette[palette->colors[i]]);
		if (result > diff)
		{
			result = diff;
		}
	}
	return (result);
}

static
s_palette const*	Palette_GetNearest(s_palette target, s_palette const* palettes, t_uint length)
{
	s_colordiff	diffs[BMP_MAXCOLORS] = {0};
	// check all identical colors
	for (t_uint i = 0; i < length; ++i)
	{
		diffs[i].index = i;
		for (t_uint j = 0; j < target.length; ++j)
		{
			if (!Palette_Contains(&palettes[i], target.colors[j]))
			{
				diffs[i].value += GetSmallestColorDifference(target.colors[j], &palettes[i]);
			}
		}
	}
	QuickSort_Compare_ColorDiffs(diffs, length);
	return (&palettes[diffs[0].index]);
}



/*
** ************************************************************************** *|
**                      Output Printing Utility Functions                     *|
** ************************************************************************** *|
*/

static
void	PrintUsage(void)
{
	printf("\n"IO_TEXT_BOLD"USAGE"IO_RESET":");
	printf("\n\tbmp2nam [OPTIONS] INPUTFILE [OUTPUTFILE]");
	printf("\n");
	printf("\n"IO_TEXT_BOLD"INPUTFILE"IO_RESET": (necessary)");
	printf("\n\tThe filepath of the BMP file to read (it must be in 8BPP indexed palette format, and must have fewer than 16 colors total).");
	printf("\n");
	printf("\n"IO_TEXT_BOLD"OUTPUTFILE"IO_RESET":");
	printf("\n\tThe filepath of the NAM file to create.");
	printf("\n\tIf not provided, this program will output a file with the same name as the given BMP `INPUTFILE`,");
	printf("\n\tbut will add a CHR/NAM file extension to the output filepath (only if needed).");
	printf("\n");
	printf("\n"IO_TEXT_BOLD"OPTIONS"IO_RESET":");
	printf("\n\tHere is the list of accepted options, both in `-c` short char format, and `--string` long string format:");
	for (int j = 0; j < PROGRAM_ARGS_AMOUNT; ++j)
	{
		printf("\n\t-%c, --%s\t%s",
			program_args[j].arg_char,
			program_args[j].arg_long,
			program_args[j].description);
	}
	printf("\n\n");
}



static
int		PrintColorStats(s_color_use const* array, t_size length)
{
	Log_Message(&program.logger,
		"The bitmap's colors which will be considered (the %u most used colors):",
		PAL_COLORS);
	s_color_use* sorted = QuickSort_New_Compare_ColorUse(array, length);
	if (sorted == NULL)
	{
		Log_Error(&program.logger, 0, "Could not sort bitmap colors by amount of occurences");
		return (ERROR);
	}
	t_float total = (NAM_W * NAM_H);
	for (t_u32 i = 0; i < PAL_COLORS; ++i)
	{
		if (sorted[i].occurences == 0)
			continue;
		program.occur_colors[i] = sorted[i];
		Log_Message(&program.logger,
			"%2i | %s %3i(#%.2X = 0x%.6X), occurences: %u\tie: %.1f%%", i + 1,
			GetColor(sorted[i].color),
			sorted[i].index,
			sorted[i].index,
			sorted[i].color,
			sorted[i].occurences,
			sorted[i].occurences / total * 100.);

	}
	return (OK);
}



/*
** ************************************************************************** *|
**                     Program Argument Handling Functions                    *|
** ************************************************************************** *|
*/

static
t_bool	HandleArgs_Option_Char(char const* arg)
{
	for (t_u32 i = 0; i < PROGRAM_ARGS_AMOUNT; ++i)
	{
		if (arg[0] == program_args[i].arg_char)
		{
			if (program_args[i].has_value)
			{
				arg += 1;
				if (arg[0] != '=')
				{
					Log_Error(&program.logger, 0,
						"Invalid argument syntax: expected '=' symbol immediately after \'%c\' option flag char",
						program_args[i].arg_char);
					return (FALSE);
				}
				arg += 1;
			}
			if (program_args[i].handle_arg)
			{
				if (program_args[i].handle_arg(arg))
					return (FALSE);
			}
			else return (MATCHED_HELP);
			return (TRUE);
		}
	}
	return (FALSE);
}

static
t_bool	HandleArgs_Option_String(char const* arg)
{
	t_size length;

	for (t_u32 i = 0; i < PROGRAM_ARGS_AMOUNT; ++i)
	{
		length = String_Length(program_args[i].arg_long);
		if (String_Equals(arg, program_args[i].arg_long) || (program_args[i].has_value &&
			String_Equals_N(arg, program_args[i].arg_long, length)))
		{
			if (program_args[i].has_value)
			{
				arg += length;
				if (arg[0] != '=')
				{
					Log_Error(&program.logger, 0,
						"Invalid argument syntax: expected '=' symbol immediately after \"%s\" option flag string",
						program_args[i].arg_long);
					return (FALSE);
				}
				arg += 1;
			}
			if (program_args[i].handle_arg)
			{
				if (program_args[i].handle_arg(arg))
					return (FALSE);
			}
			else return (MATCHED_HELP);
			return (TRUE);
		}
	}
	return (FALSE);
}

static
int		HandleArgs_FilePath_Input(char const* arg)
{
	program.file_input = arg;
	Log_Message(&program.logger, "Processing file: %s...", program.file_input);
	program.bitmap = SDL_LoadBMP(program.file_input);
	if (program.bitmap == NULL)
	{
		Log_Error(&program.logger, 0, "Could not load BMP file => %s\n", SDL_GetError());
		return (ERROR);
	}
	return (OK);
}

static
int		HandleArgs_FilePath_Output(char const* arg)
{
	program.file_output = arg;
	return (OK);
}

static
int	HandleArgs(int argc, char** argv)
{
	if (argc < 1 || argv == NULL)
	{
		Log_Error(&program.logger, 0, "Invalid platform, no 'argv' program argument list received.");
		return (ERROR);
	}
	if (argc < 2 || argv[1] == NULL || argv[1][0] == '\0')
	{
		Log_Error(&program.logger, 0, "Expected at least an `INPUTFILE` argument.");
		PrintUsage();
		return (ERROR);
	}
	int	match;
	int tmp;
	for (int i = 1; i < argc; ++i)
	{
		match = FALSE;
		if (argv[i][0] == '-')
		{
			if (argv[i][1] == '\0')
			{
				Log_Error(&program.logger, 0, "Unexpected end of argument after '-'.");
				continue;
			}
			else if (argv[i][1] == '-')
			{
				match = HandleArgs_Option_String(argv[i] + 2);
			}
			else for (size_t j = 1; j < String_Length(argv[i]); ++j)
			{
				tmp = HandleArgs_Option_Char(argv[i] + j);
				if (match)
					match = (tmp == MATCHED_HELP ? tmp : match);
				else match = tmp;
			}
		}
		else
		{
			if (program.bitmap == NULL)
			{
				if (HandleArgs_FilePath_Input(argv[i]) != OK)
					return (ERROR);
			}
			else
			{
				if (HandleArgs_FilePath_Output(argv[i]) != OK)
					return (ERROR);
			}
			match = TRUE;
		}

		if (match == FALSE)
		{
			Log_Error(&program.logger, 0, "Argument not recognized (%s)\n", argv[i]);
			PrintUsage();
			return (ERROR);
		}
		else if (match == MATCHED_HELP)
		{
			PrintUsage();
			return (OK);
		}
	}
	return (OK);
}



/*
** ************************************************************************** *|
**                           Core Program Functions                           *|
** ************************************************************************** *|
*/

static
int		CheckBitmap_LoadReferencePalette(void)
{
	// open and load reference palette file
	t_fd fd = IO_Open(REFPAL_FILEPATH, OPEN_READONLY, 0);
	if (fd < 0)
	{
		Log_Error_IO(&program.logger, 0, "Could not open reference palette file: %s", REFPAL_FILEPATH);
		return (ERROR);
	}
	t_u8* file = NULL;
	t_sintmax size = IO_Read_File(fd, (void**)&file, 0);
	if (size < 0)
	{
		Log_Error_IO(&program.logger, 0, "Could not read reference palette file: %s", REFPAL_FILEPATH);
		return (ERROR);
	}
	if ((t_size)size != REFPAL_SIZE)
	{
		Log_Warning(&program.logger, "Reference palette file has incorrect size: %s (was %zu bytes, but should be %zu bytes)",
			REFPAL_FILEPATH, (t_size)size, REFPAL_SIZE);
	}
	t_u8 r;
	t_u8 g;
	t_u8 b;
	t_size index = 0;
	for (t_u32 i = 0; i < REFPAL_COLORS; ++i)
	{
		r = file[index++];
		g = file[index++];
		b = file[index++];
		program.ref_palette[i] = Color_ARGB32_Set(0, r, g, b);
	}
	Memory_Delete((void**)&file);

	Log_Message(&program.logger,
		"Here is the loaded reference palette (%s), using ANSI terminal color codes:",
		REFPAL_FILEPATH);
	char* str;
	char* tmp;
	index = 0;
	for (int y = 0; y < 16; ++y)
	{
		str = String_New(0);
		if (str == NULL)
		{
			Log_Error(&program.logger, 0, "Could not allocate palette logging output string");
			continue;
		}
		for (int x = 0; x < 16; ++x)
		{
			if (index >= REFPAL_COLORS)
			{
				Log_Message(&program.logger, "\t%s", str);
				String_Delete(&str);
				return (OK);
			}
			tmp = GetColor(program.ref_palette[index]);
			String_Merge(&str, &tmp);
			++index;
		}
		Log_Message(&program.logger, "\t%s", str);
		String_Delete(&str);
	}
	return (OK);
}



static
int		CheckBitmap_LoadColors(void)
{
	if (program.bitmap == NULL ||
		program.bitmap->format == NULL ||
		program.bitmap->format->palette == NULL ||
		program.bitmap->format->palette->colors == NULL)
		return (ERROR);
	SDL_Color* colors = program.bitmap->format->palette->colors;
	int n = program.bitmap->format->palette->ncolors;
	for (int i = 0; i < n; ++i)
	{
		program.bitmap_colors[i].index = i;
		program.bitmap_colors[i].color = Color_ARGB32_Set(
			colors[i].a,
			colors[i].r,
			colors[i].g,
			colors[i].b);
	}
	return (OK);
}



static
int		CheckBitmap_PixelFormat(void)
{
	if (program.bitmap->format == NULL)
	{
		Log_Error(&program.logger, 0, "bitmap.format is NULL");
		return (ERROR);
	}

	Log_Verbose(&program.logger, "Loaded BMP has pixel format:"
		"\n\t- format: %s"
		"\n\t- palette: %s (%i)"
		"\n\t- bits/pixel: %i"
		"\n\t- bytes/pixel: %i",
		program.bitmap->format->format == SDL_PIXELFORMAT_INDEX8 ? "OK" : "DIFFERENT",
		program.bitmap->format->palette == NULL ? "NULL" : "OK",
		program.bitmap->format->palette->ncolors,
		program.bitmap->format->BitsPerPixel,
		program.bitmap->format->BytesPerPixel);

	if (program.bitmap->format->palette == NULL ||
		program.bitmap->format->BitsPerPixel != BMP_BPP ||
		program.bitmap->format->BytesPerPixel != (BMP_BPP / 8) || // TODO more robust rounding here ?
		program.bitmap->format->format != SDL_PIXELFORMAT_INDEX8)
	{
		Log_Message(&program.logger, "BMP file has improper pixel format (must be 8BPP indexed): attempting to convert...");

		SDL_Surface* bitmap = SDL_ConvertSurfaceFormat(program.bitmap, SDL_PIXELFORMAT_INDEX8, 0);
		if (bitmap == NULL)
		{
			Log_Error(&program.logger, 0, "Could not convert BMP to 8BPP indexed format => %s\n", SDL_GetError());
			return (ERROR);
		}
		SDL_FreeSurface(program.bitmap);
		program.bitmap = bitmap;
		if (program.bitmap->format->palette == NULL)
		{
			Log_Error(&program.logger, 0, "bitmap.format.palette is NULL");
			return (ERROR);
		}
		Log_Success(&program.logger, "Converted bitmap to the proper pixel format (8BPP indexed)");
	}
	else Log_Success(&program.logger, "BMP file given has correct pixel format");

	return (OK);
}



static
int 	CheckBitmap_Dimensions(void)
{
	if (program.bitmap->w == NAM_W &&
		program.bitmap->h == NAM_H)
	{
		Log_Success(&program.logger,
			"BMP file has the correct dimensions (%ix%i)",
			program.bitmap->w,
			program.bitmap->h);
		return (OK);
	}
/*
	if (program.bitmap->w > NAM_W)
		program.bitmap->w = NAM_W;
	if (program.bitmap->h > NAM_H)
		program.bitmap->h = NAM_H;
*/
	Log_Warning(&program.logger,
		"BMP file has improper dimensions (%ix%i), only the top-left-most %ix%i pixels will be considered",
		program.bitmap->w,
		program.bitmap->h,
		NAM_W, NAM_H);

	SDL_Surface* bitmap = SDL_CreateRGBSurfaceWithFormat(0,
		NAM_W,
		NAM_H,
		BMP_BPP,
		SDL_PIXELFORMAT_INDEX8);
	if (bitmap == NULL)
	{
		Log_Error(&program.logger, 0,
			"Could not create cropped bitmap which is %ix%i pixels => %s\n",
			NAM_W, NAM_H,
			SDL_GetError());
		return (ERROR);
	}
	if (SDL_SetSurfacePalette(bitmap, program.bitmap->format->palette))
	{
		Log_Error(&program.logger, 0,
			"Could not copy palette for cropped bitmap => %s\n",
			SDL_GetError());
		return (ERROR);
	}
	SDL_Rect rect = { .x=0, .y=0, .w=NAM_W, .h=NAM_H };
	if (SDL_BlitSurface(program.bitmap, &rect, bitmap, NULL))
	{
		Log_Error(&program.logger, 0,
			"Could not crop bitmap image to be %ix%i pixels => %s\n",
			NAM_W, NAM_H,
			SDL_GetError());
		return (ERROR);
	}
	SDL_FreeSurface(program.bitmap);
	program.bitmap = bitmap;
	return (OK);
}



static
int		CheckBitmap_TotalColors(void)
{
	t_u8*	pixels = (t_u8*)program.bitmap->pixels;
	t_u8	pixel;
	t_u8	colors_present = 0;
	for (int y = 0; y < program.bitmap->h; ++y)
	for (int x = 0; x < program.bitmap->w; ++x)
	{
		pixel = pixels[y * program.bitmap->w + x];
		if (program.bitmap_colors[pixel].occurences == 0)
			++colors_present;
		program.bitmap_colors[pixel].occurences += 1;
	}
	program.bitmap_colors_total = colors_present;
	if (colors_present > PAL_COLORS)
	{
		Log_Warning(&program.logger,
			"BMP file given has too many colors: %u (should be %u or fewer)",
			colors_present,
			PAL_COLORS);
	}
	else Log_Success(&program.logger,
		"BMP successfully loaded (uses %u unique colors).",
		colors_present);

	PrintColorStats(program.bitmap_colors, BMP_MAXCOLORS);
	return (OK);
}



static
int		GetColorOccurIndex(t_u8 bmp_color)
{
	for (int i = 0; i < BMP_MAXCOLORS; ++i)
	{
		if (program.occur_colors[i].index == bmp_color)
		{
			return (i);
		}
	}
	return (ERROR);
}

static
int		CheckBitmap_TilesColors()
{
	t_u32	index;
	t_u8*	pixels = (t_u8*)program.bitmap->pixels;
	t_u8	pixel;
	t_u8	colors_present = 0;
	int		color;
	SDL_Point tile;
	for (tile.y = 0; tile.y < NAM_H_TILES; ++tile.y)
	for (tile.x = 0; tile.x < NAM_W_TILES; ++tile.x)
	{
		index = (tile.y * NAM_W_TILES) + tile.x;
		for (int i = 0; i < BMP_MAXCOLORS; ++i)
		{
			program.tiles_colors[index].colors[i] = program.occur_colors[i];
			program.tiles_colors[index].colors[i].occurences = 0;
		}
		colors_present = 0;
		for (int y = 0; y < NAM_TILE; ++y)
		for (int x = 0; x < NAM_TILE; ++x)
		{
			pixel = pixels[(tile.y * NAM_TILE + y) * NAM_W + (tile.x * NAM_TILE + x)];
			color = GetColorOccurIndex(pixel);
			if (color >= 0)
			{
				if (program.tiles_colors[index].colors[color].occurences == 0)
					++colors_present;
				program.tiles_colors[index].colors[color].occurences += 1;
			}
		}
		// sort the tile colors by popularity
		QuickSort_Compare_ColorUse(program.tiles_colors[index].colors, BMP_MAXCOLORS);
		program.tiles_colors[index].total = colors_present;

		if (colors_present > PAL_SUB_COLORS)
		{
			Log_Warning(&program.logger,
				"Tile has too many different colors (%i), in BMP at (x:%i, y:%i)",
				colors_present,
				(tile.x * NAM_TILE),
				(tile.y * NAM_TILE));
			Log_Verbose(&program.logger,
				"Here is the list of color occurences for this %ix%i NAM tile: ",
				NAM_TILE, NAM_TILE);
			for (int i = 0; i < BMP_MAXCOLORS; ++i)
			{
				if (program.tiles_colors[index].colors[i].occurences == 0)
					continue;
				Log_Verbose(&program.logger,
					" - %s %2i(#%.2X = 0x%.6X) => occurences: %i\tie: %.1f%%",
					GetColor(program.tiles_colors[index].colors[i].color),
					program.tiles_colors[index].colors[i].index,
					program.tiles_colors[index].colors[i].index,
					program.tiles_colors[index].colors[i].color,
					program.tiles_colors[index].colors[i].occurences,
					program.tiles_colors[index].colors[i].occurences / (NAM_TILE * NAM_TILE / 100.));
			}
		}
	}
	return (OK);
}



static
int		CheckBitmap_DuplicatePalettes(void)
{
	s_palette*	palette;
	s_palette*	other;

	for (int i = 0; i < NAM_TILES; ++i)
	{
		program.tiles_colors[i].palette = Palette_GetMostUsedColors(program.tiles_colors[i].colors, PAL_SUB_COLORS);
	}
	for (int i = 0; i < NAM_TILES; ++i)
	{
		palette = &program.tiles_colors[i].palette;
		for (int j = i - 1; j >= 0; --j)
		{
			other = &program.tiles_colors[j].palette;
			if (Palette_ContainsAll(other, palette))
			{
//Log_Message(&program.logger, "DEBUG_1A: p:%i contains p:%i", j, i);
				if (other->duplicate > 0)
					palette->duplicate = other->duplicate;
				else
					palette->duplicate = j;
				if (Palette_ContainsAll(palette, &program.tiles_colors[palette->duplicate].palette))
				{
//Log_Message(&program.logger, "DEBUG_1B: p:%i contains p:%i", i, j);
					palette->identical = TRUE;
				}
			}
		}
	}
	for (int i = 0; i < NAM_TILES; ++i)
	{
		palette = &program.tiles_colors[i].palette;
		if (palette->duplicate < 0)
		{
			for (int j = i + 1; j < NAM_TILES; ++j)
			{
				other = &program.tiles_colors[j].palette;
				if (other->duplicate < 0)
				{
					if (Palette_ContainsAll(other, palette))
					{
//Log_Message(&program.logger, "DEBUG_2A: p:%i contains p:%i", j, i);
						palette->duplicate = j;
						if (Palette_ContainsAll(palette, &program.tiles_colors[palette->duplicate].palette))
						{
//Log_Message(&program.logger, "DEBUG_2B: p:%i contains p:%i", i, j);
							palette->identical = TRUE;
						}
					}
				}
			}
		}
	}
	return (OK);
}



static
int		ConvertBitmap_ApplyRefPalette(void)
{
	Log_Verbose(&program.logger, "Finding nearest colors in the reference palette...");
	s_color_use* c = NULL;
	t_u8	nearest[BMP_MAXCOLORS];
	for (t_u32 i = 0; i < BMP_MAXCOLORS; ++i)
	{
		c = &program.bitmap_colors[i];
		if (c == NULL)
			continue;
		t_argb32 const* match = Color_ARGB32_GetNearest(c->color, program.ref_palette, REFPAL_COLORS);
		if (match == NULL)
		{
			Log_Warning(&program.logger,
				"Could not find nearest color to 0x%.6X",
				c->color);
			continue;
		}
		nearest[i] = (match - program.ref_palette);
	}

	Log_Message(&program.logger, "Applying reference palette colors to the bitmap...");
	t_u8*	pixels = (t_u8*)program.bitmap->pixels;
	t_u8	pixel;
	t_u32	index;
	for (int y = 0; y < program.bitmap->h; ++y)
	for (int x = 0; x < program.bitmap->w; ++x)
	{
		index = (y * program.bitmap->w + x);
		pixel = pixels[index];
		pixels[index] = nearest[pixel];
	}
	SDL_Palette* palette = program.bitmap->format->palette;
	Memory_Clear(palette->colors, palette->ncolors * sizeof(SDL_Color));
	for (int i = 0; i < REFPAL_COLORS; ++i)
	{
		palette->colors[i] = (SDL_Color)
		{
			.r = Color_ARGB32_Get_R(program.ref_palette[i]),
			.g = Color_ARGB32_Get_G(program.ref_palette[i]),
			.b = Color_ARGB32_Get_B(program.ref_palette[i]),
			.a = 0
		};
	}
	return (OK);
}



static
int		ConvertBitmap_TotalColorReduction(void)
{
	Log_Message(&program.logger, "Fusing together colors which are perceptually similar...");
	t_u8*		pixels = (t_u8*)program.bitmap->pixels;
	t_u8		pixel;
	t_u32		pixel_index;
	t_u8		total;
	t_argb32	color1;
	t_argb32	color2;
	t_u8 old;
	t_u8 new;
	total = program.bitmap_colors_total;
	if (total <= PAL_COLORS)
		return (OK);
	// find colors (among the most popular) which are very similar, and fuse them
	for (int i = 0; i < total; ++i)
	{
		color1 = program.ref_palette[program.bitmap_colors[i].index];
		for (int j = i + 1; j < total; ++j)
		{
			color2 = program.ref_palette[program.bitmap_colors[j].index];
			if (Color_ARGB32_Difference(color1, color2) <= THRESHOLD)
			{
Log_Verbose(&program.logger, "DEBUG TOTAL | i:%2i, color=%.2X(#%.6X) | j:%2i, color=%.2X(#%.6X)",
	i, program.bitmap_colors[i].index, color1,
	j, program.bitmap_colors[i].index, color2);
				old = program.bitmap_colors[j].index;
				new = program.bitmap_colors[i].index;
				for (int y = 0; y < NAM_H; ++y)
				for (int x = 0; x < NAM_W; ++x)
				{
					pixel_index = (y * NAM_W) + (x);
					pixel = pixels[pixel_index];
					if (pixel == old)
					{
						pixels[pixel_index] = new;
					}
				}
				program.bitmap_colors[j].occurences = 0;
				total -= 1;
			}
			if (total <= PAL_COLORS)
				break;
		}
		if (total <= PAL_COLORS)
			break;
	}
	return (OK);
}



static
int		ConvertBitmap_TilesColorReduction(void)
{
	Log_Message(&program.logger, "Removing superfluous colors for each tile in the bitmap...");
	t_u8*		pixels = (t_u8*)program.bitmap->pixels;
	t_u8		pixel;
	t_u32		pixel_index;
	t_u32		index;
	t_u8		total;
	t_u8		length;
	s_color_use*color;
	t_argb32	color1;
	t_argb32	color2;
	t_argb32	palette[PAL_SUB_COLORS];
	t_u8 old;
	t_u8 new;
	SDL_Point tile;
	for (tile.y = 0; tile.y < NAM_H_TILES; ++tile.y)
	for (tile.x = 0; tile.x < NAM_W_TILES; ++tile.x)
	{
		index = (tile.y * NAM_W_TILES + tile.x);
		total = program.tiles_colors[index].total;
		if (total <= PAL_SUB_COLORS)
			continue;
		length = program.tiles_colors[index].palette.length;
		// find colors (among the most popular) which are very similar, and fuse them
		for (int i = 0; i < length; ++i)
		{
			color1 = program.ref_palette[program.tiles_colors[index].palette.colors[i]];
			for (int j = i + 1; j < length; ++j)
			{
				color2 = program.ref_palette[program.tiles_colors[index].palette.colors[j]];
				if (Color_ARGB32_Difference(color1, color2) <= THRESHOLD)
				{
Log_Verbose(&program.logger, "DEBUG TILES %3i | i:%2i, color=%.2X(#%.6X) | j:%2i, color=%.2X(#%.6X)", index,
	i, program.tiles_colors[index].palette.colors[i], color1,
	j, program.tiles_colors[index].palette.colors[j], color2);
//					Palette_Requantize(&program.tiles_colors[index].palette);
					old = program.tiles_colors[index].palette.colors[j];
					new = program.tiles_colors[index].palette.colors[i];
					for (int y = 0; y < NAM_TILE; ++y)
					for (int x = 0; x < NAM_TILE; ++x)
					{
						pixel_index = (tile.y * NAM_TILE + y) * NAM_W + (tile.x * NAM_TILE + x);
						pixel = pixels[pixel_index];
						if (pixel == old)
						{
							pixels[pixel_index] = new;
						}
					}
					program.tiles_colors[index].colors[j].occurences = 0;
					total -= 1;
				}
				if (total <= PAL_SUB_COLORS)
					break;
			}
			if (total <= PAL_SUB_COLORS)
				break;
		}
		// for each color which isn't among the most popular, replace it with the nearest one that is
		for (t_u8 i = 0; i < length; ++i)
		{
			palette[i] = program.ref_palette[program.tiles_colors[index].palette.colors[i]];
		}
		for (t_u8 i = length; i < total; ++i)
		{
			color = &program.tiles_colors[index].colors[i];
			t_argb32 const* nearest = Color_ARGB32_GetNearest(color->color, palette, length);
			if (nearest == NULL)
				continue;
			old = color->index;
			new = program.tiles_colors[index].palette.colors[nearest - palette];
			for (int y = 0; y < NAM_TILE; ++y)
			for (int x = 0; x < NAM_TILE; ++x)
			{
				pixel_index = (tile.y * NAM_TILE + y) * NAM_W + (tile.x * NAM_TILE + x);
				pixel = pixels[pixel_index];
				if (pixel == old)
				{
					pixels[pixel_index] = new;
				}
			}
Log_Verbose(&program.logger, "DEBUG => i:%2i | old:%.2X(#%.6X), new:%.2X(#%.6X)", i,
	old, program.ref_palette[old],
	new, program.ref_palette[new]);
			total -= 1;
		}
	}
	return (OK);
}



static
int		ConvertBitmap_AssertOutputPalettes(void)
{
	s_palette*	palette;

	// check the popularity of each of the unique palettes
	for (t_uint i = 0; i < NAM_TILES; ++i)
	{
		palette = &program.tiles_colors[i].palette;
		while (palette->duplicate >= 0)
		{
			palette = &program.tiles_colors[palette->duplicate].palette;
		}
		palette->popularity += 1;
	}
	// get total amount of unique palettes
	for (t_uint i = 0; i < NAM_TILES; ++i)
	{
		palette = &program.tiles_colors[i].palette;
		if (palette->duplicate < 0)
		{
			program.tiles_palettes[program.tiles_palettes_amount] = *palette;
			program.tiles_palettes[program.tiles_palettes_amount].duplicate = i; // set `duplicate` field to the original tile index
			++program.tiles_palettes_amount;
		}
	}
	// sort the unique palettes by popularity
	QuickSort_Compare_Palette(program.tiles_palettes, program.tiles_palettes_amount);

	// sort the output palettes by brightness
	for (t_uint i = 0; i < program.tiles_palettes_amount; ++i)
	{
		QuickSort_Compare_Color(program.tiles_palettes[i].colors, PAL_SUB_COLORS);
	}

	// TODO check if there are free color slots in a given palette, and potentially merge palettes this way

	Log_Message(&program.logger,
		"The given BMP file, when broken up into %ix%i-pixel NAM tiles, uses, at minimum, %i palettes:",
		NAM_TILE, NAM_TILE,
		program.tiles_palettes_amount);
	char* str;
	for (t_uint i = 0; i < program.tiles_palettes_amount; ++i)
	{
		palette = &program.tiles_palettes[i];
		str = GetPalette(palette);
		Log_Message(&program.logger,
			"%3i | palette: %s\toccurences: %i\tie: %.1f%%", i, str,
			palette->popularity,
			palette->popularity / (NAM_TILES / 100.));
		String_Delete(&str);
	}

	// insert the user-specified colorkey as the first color for any palette which doesn't have it
	if (program.colorkey.occurences)
	{
		for (t_uint i = 0; i < program.tiles_palettes_amount; ++i)
		{
			if (program.tiles_palettes[i].colors[0] != program.colorkey.index)
			{
				t_sint match = Palette_Find(&program.tiles_palettes[i], program.colorkey.index);
				if (match >= 0)
				{
					Memory_Swap(
						&program.tiles_palettes[i].colors[0],
						&program.tiles_palettes[i].colors[match],
						sizeof(t_u8));
					QuickSort_Compare_Color(program.tiles_palettes[i].colors + 1, PAL_SUB_COLORS - 1);
				}
				else
				{
					for (t_sint j = PAL_SUB_COLORS - 1; j > 0; --j)
					{
						program.tiles_palettes[i].colors[j] = program.tiles_palettes[i].colors[j - 1];
					}
					program.tiles_palettes[i].colors[0] = program.colorkey.index;
				}
			}
		}
	}

	// set the output palettes from the most popular palettes
	t_size length = program.tiles_palettes_amount;
	if (length > PAL_SUB_AMOUNT)
		length = PAL_SUB_AMOUNT;
	Memory_Copy(&program.output_palettes, program.tiles_palettes, length * sizeof(s_palette));

	return (OK);
}



static
int		FindOutputPalette(int index_tile, t_bool user_palette)
{
	s_palette const* result;

	if (user_palette)
	{
		result = Palette_GetNearest(
			program.tiles_palettes[index_tile],
			program.output_palettes, PAL_SUB_AMOUNT);
		return (result - program.output_palettes);
	}
	else
	{
		while (program.tiles_colors[index_tile].palette.duplicate >= 0)
		{
			if (index_tile == program.tiles_colors[index_tile].palette.duplicate)
				break;
			index_tile = program.tiles_colors[index_tile].palette.duplicate;
		}
		for (t_uint i = 0; i < PAL_SUB_AMOUNT; ++i)
		{
			if (index_tile == program.output_palettes[i].duplicate)
				return (i);
		}
		for (t_uint i = 0; i < program.tiles_palettes_amount; ++i)
		{
			if (index_tile == program.tiles_palettes[i].duplicate)
			{
				result = Palette_GetNearest(
					program.tiles_palettes[i],
					program.output_palettes, PAL_SUB_AMOUNT);
				return (result - program.output_palettes);
			}
		}
	}
	return (-1);
}

static
int		FindOutputColor(t_u8 pixel, t_argb32 const* colors, t_size length)
{
	t_argb32 const* color = Color_ARGB32_GetNearest(program.ref_palette[pixel], colors, length);
	if (color == NULL)
		return (-1);
	return (color - colors);
}

static
int		ConvertBitmap_ApplyOutputPalettes(t_bool user_palette)
{
	Log_Message(&program.logger, "Applying final palette colors to the bitmap...");
	Log_Message(&program.logger,
		"The final set of %i palettes of %i colors each (chosen by popularity):",
		PAL_SUB_AMOUNT,
		PAL_SUB_COLORS);
	for (t_uint i = 0; i < PAL_SUB_AMOUNT; ++i)
	{
		char* tmp = GetPalette(&program.output_palettes[i]);
		Log_Message(&program.logger,
			"%3i | palette: %s\toccurences: %i\tie: %.1f%%", i, tmp,
			program.output_palettes[i].popularity,
			program.output_palettes[i].popularity / (NAM_TILES / 100.));
		String_Delete(&tmp);
	}
	t_u8*	pixels = (t_u8*)program.bitmap->pixels;
	t_u8	pixel;
	int		index_color;
	int		index_palette;
	t_u32	index;
	t_u32	index_tile = 0;
	t_argb32 output_colors[PAL_SUB_AMOUNT][PAL_SUB_COLORS];
	SDL_Point tile = { .x=0, .y=0 };
	for (int i = 0; i < PAL_SUB_AMOUNT; ++i)
	for (int j = 0; j < PAL_SUB_COLORS; ++j)
	{
		output_colors[i][j] = program.ref_palette[program.output_palettes[i].colors[j]];
	}
	for (tile.y = 0; tile.y < NAM_H_TILES; ++tile.y)
	for (tile.x = 0; tile.x < NAM_W_TILES; ++tile.x)
	{
		index_palette = FindOutputPalette(index_tile, user_palette);
		if (index_palette < 0)
		{
			Log_Error(&program.logger, 0, "Could not find palette for tile at (x:%i, y:%i)",
				(tile.x * NAM_TILE),
				(tile.y * NAM_TILE));
			continue;
		}
		for (int y = 0; y < NAM_TILE; ++y)
		for (int x = 0; x < NAM_TILE; ++x)
		{
			index = (tile.y * NAM_TILE + y) * NAM_W + (tile.x * NAM_TILE + x);
			pixel = pixels[index];
			index_color = FindOutputColor(pixel, output_colors[index_palette], PAL_SUB_COLORS);
			if (index_color < 0)
			{
				Log_Error(&program.logger, 0, "Could not find color for pixel at (x:%i, y:%i)",
					(tile.x * NAM_TILE + x),
					(tile.y * NAM_TILE + y));
				continue;
			}
			pixels[index] = index_palette * (PAL_SUB_COLORS * sizeof(t_u8)) + index_color;
		}
		++index_tile;
	}
	SDL_Palette* palette = program.bitmap->format->palette;
	t_argb32 color;
	Memory_Clear(palette->colors, palette->ncolors * sizeof(SDL_Color));
	for (int i = 0; i < PAL_SUB_AMOUNT; ++i)
	for (int j = 0; j < PAL_SUB_COLORS; ++j)
	{
		index_color = program.output_palettes[i].colors[j];
		color = program.ref_palette[index_color];
		palette->colors[i * PAL_SUB_COLORS + j] = (SDL_Color)
		{
			.r = Color_ARGB32_Get_R(color),
			.g = Color_ARGB32_Get_G(color),
			.b = Color_ARGB32_Get_B(color),
			.a = 0
		};
	}
	return (OK);
}



/*
** ************************************************************************** *|
**                            Main Program Functions                          *|
** ************************************************************************** *|
*/

static
int	init(char const* name)
{
	program.called = name;

	// logger initiliazing
	program.logger = (s_logger)
	{
//		.in_use			= FALSE,
		.silence_logs	= FALSE,
		.silence_errors	= FALSE,
		.timestamp		= FALSE,
		.verbose		= FALSE,
		.obfuscated		= FALSE,
		.append			= FALSE,
		.format			= LOGFORMAT_ANSI,
		.fd				= STDOUT,
		.path			= NULL,
	};
	Logger_Init(&program.logger);
	return (OK);
}



#ifdef main
#undef main
#endif
int main(int argc, char** argv)
{
	char*	tmp;

	if (init(argv[0]))
		return (ERROR);

	if (HandleArgs(argc, argv))
		return (ERROR);

	if (program.file_output == NULL)
	{	// create default output filepath if not provided
		int extension = String_IndexOf_R_Char(program.file_input, '.');
		program.file_output = String_Sub(program.file_input, 0, extension);
	}
	else if (String_Equals_IgnoreCase(program.file_output + String_Length(program.file_output) - 4, ".nam"))
	{	// remove ".nam" file extension if provided
		((char*)program.file_output)[String_Length(program.file_output) - 4] = '\0';
	}

	if (CheckBitmap_LoadReferencePalette())
		return (ERROR);

	if (CheckBitmap_PixelFormat())
		return (ERROR);
	if (CheckBitmap_Dimensions())
		return (ERROR);
	if (CheckBitmap_LoadColors())
		return (ERROR);

	if (ConvertBitmap_ApplyRefPalette())
		return (ERROR);

	if (CheckBitmap_LoadColors())
		return (ERROR);
	if (CheckBitmap_TotalColors())
		return (ERROR);
	if (CheckBitmap_TilesColors())
		return (ERROR);

	if (ConvertBitmap_TotalColorReduction())
		return (ERROR);
	if (ConvertBitmap_TilesColorReduction())
		return (ERROR);

	if (CheckBitmap_LoadColors())
		return (ERROR);
	if (CheckBitmap_TotalColors())
		return (ERROR);
	if (CheckBitmap_TilesColors())
		return (ERROR);
	// TODO skip this step if the user has provided output palettes
	if (program.output_palettes[0].length == 0)
	{
		if (CheckBitmap_DuplicatePalettes())
			return (ERROR);
		if (ConvertBitmap_AssertOutputPalettes())
			return (ERROR);
		if (ConvertBitmap_ApplyOutputPalettes(FALSE))
			return (ERROR);
	}
	else
	{
		if (ConvertBitmap_ApplyOutputPalettes(TRUE))
			return (ERROR);
	}

	tmp = String_Join(program.file_output, ".bmp");
	if (SDL_SaveBMP(program.bitmap, tmp))
	{
		String_Delete(&tmp);
		Log_Error(&program.logger, 0, "Could not save BMP file => %s\n", SDL_GetError());
		return (ERROR);
	}
	Log_Success(&program.logger, "Wrote output file: %s", tmp);
	String_Delete(&tmp);

	tmp = String_Join(program.file_output, ".nam");
	// TODO code here
	String_Delete(&tmp);
	return (OK);
}
