
#include <libccc.h>
#include <libccc/memory.h>
#include <libccc/string.h>
#include <libccc/sys/logger.h>
#include <libccc/math/math.h>
#include <libccc/math/sort.h>

#include "SDL.h"

#include "bmp2nam.h"



s_program program = { 0 };

//! A special return value to signal when a help argument has been provided by the user
#define MATCHED_HELP    ((int)-1)



static
t_bool HandleArg_Verbose(t_char const* arg)
{
	if (arg == NULL) return (ERROR);
	program.logger.verbose = TRUE;
	return (OK);
}

static
t_bool HandleArg_BitmapWidth(t_char const* arg)
{
	if (arg == NULL) return (ERROR);
	program.expected_w = U32_FromString(arg);
	if (program.expected_w == 0)
		return (ERROR);
	return (OK);
}

static
t_bool HandleArg_BitmapHeight(t_char const* arg)
{
	if (arg == NULL) return (ERROR);
	program.expected_h = U32_FromString(arg);
	if (program.expected_h == 0)
		return (ERROR);
	return (OK);
}

static
t_bool HandleArg_Palette(t_char const* arg)
{
	if (arg == NULL) return (ERROR);
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

static
t_bool HandleArg_ColorKey(t_char const* arg)
{
	if (arg == NULL) return (ERROR);
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



//! This is the list of accepted program arguments
static s_program_arg const program_args[PROGRAM_ARGS_AMOUNT] =
{
	(s_program_arg){ NULL,                  'h', "help",     FALSE, "If provided, display only the program usage help and exit." },
	(s_program_arg){ HandleArg_Verbose,     'v', "verbose",  FALSE, "If provided, displays additional information while processing the BMP." },
	(s_program_arg){ HandleArg_BitmapWidth, 'w', "bitmap_w", FALSE, "(expects value, integer: `-w=256`) If provided, sets the expected bitmap width dimension." },
	(s_program_arg){ HandleArg_BitmapHeight,'h', "bitmap_h", FALSE, "(expects value, integer: `-h=240`) If provided, sets the expected bitmap height dimension." },
	(s_program_arg){ HandleArg_Palette,     'p', "palette",  TRUE,  "(expects value, filepath: `-p=./path/to/file.pal`) If provided, forces the output to use the given palette (must be a binary .pal file, containing at most 64 different 32-bit colors)." },
	(s_program_arg){ HandleArg_ColorKey,    'c', "colorkey", TRUE,  "(expects value, color: `-c=FF00FF`) If provided, the given color value will be present as the first color for all palettes."}
};



/*
** ************************************************************************** *|
**                      Output Printing Utility Functions                     *|
** ************************************************************************** *|
*/

static
void PrintUsage(void)
{
	IO_Output_Line(IO_TEXT_BOLD"USAGE"IO_RESET":");
	IO_Output_Line("\t""bmp2nam [OPTIONS] INPUTFILE [OUTPUTFILE]");
	IO_Output_Line("");
	IO_Output_Line(IO_TEXT_BOLD"INPUTFILE"IO_RESET": (necessary)");
	IO_Output_Line("\t""The filepath of the BMP file to read (it must be in 8BPP indexed palette format, and must have fewer than 16 colors total).");
	IO_Output_Line("");
	IO_Output_Line(IO_TEXT_BOLD"OUTPUTFILE"IO_RESET":");
	IO_Output_Line("\t""The filepath of the NAM file to create.");
	IO_Output_Line("\t""If not provided, this program will output a file with the same name as the given BMP `INPUTFILE`,");
	IO_Output_Line("\t""but will add a CHR/NAM file extension to the output filepath (only if needed).");
	IO_Output_Line("");
	IO_Output_Line(IO_TEXT_BOLD"OPTIONS"IO_RESET":");
	IO_Output_Line("\t""Here is the list of accepted options, both in `-c` short t_char format, and `--string` long string format:");
	for (int j = 0; j < PROGRAM_ARGS_AMOUNT; ++j)
	{
		IO_Output_Format("\t-%c, --%s\t%s\n",
			program_args[j].arg_char,
			program_args[j].arg_long,
			program_args[j].description);
	}
	IO_Output_Line("\n");
}



/*
** ************************************************************************** *|
**                     Program Argument Handling Functions                    *|
** ************************************************************************** *|
*/

static
t_bool HandleArgs_Option_Char(t_char const* arg)
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
						"Invalid argument syntax: expected '=' symbol immediately after \'%c\' option flag t_char",
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
t_bool HandleArgs_Option_String(t_char const* arg)
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
int HandleArgs_FilePath_Input(t_char const* arg)
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
int HandleArgs_FilePath_Output(t_char const* arg)
{
	program.file_output = arg;
	return (OK);
}

static
int HandleArgs(int argc, t_char** argv)
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
	int match;
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
**                            Main Program Functions                          *|
** ************************************************************************** *|
*/

static
int init(t_char const* name)
{
	program.called = name;

	// logger initiliazing
	program.logger = (s_logger)
	{
//      .in_use         = FALSE,
		.silence_logs   = FALSE,
		.silence_errors = FALSE,
		.timestamp      = FALSE,
		.verbose        = FALSE,
		.obfuscated     = FALSE,
		.append         = FALSE,
		.format         = LOGFORMAT_ANSI,
		.fd             = STDOUT,
		.path           = NULL,
	};
	Logger_Init(&program.logger);
	return (OK);
}



#ifdef main
#undef main
#endif
int main(int argc, t_char** argv)
{
	t_char* tmp;
	// perform initialisation of program state variables
	if (init(argv[0]))
		return (ERROR);
	// parse and handle commandline arguments
	if (HandleArgs(argc, argv))
		return (ERROR);
	// create default output filepath if not provided
	if (program.file_output == NULL)
	{
		int extension = String_IndexOf_R_Char(program.file_input, '.');
		program.file_output = String_Sub(program.file_input, 0, extension);
	}
	else if (String_Equals_IgnoreCase(program.file_output + String_Length(program.file_output) - 4, ".nam"))
	{   // remove ".nam" file extension if provided
		((t_char*)program.file_output)[String_Length(program.file_output) - 4] = '\0';
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
