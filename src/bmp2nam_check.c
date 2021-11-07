
#include <libccc.h>
#include <libccc/color.h>
#include <libccc/memory.h>
#include <libccc/string.h>
#include <libccc/sys/logger.h>
/*
#include <libccc/math/math.h>
#include <libccc/math/sort.h>
*/
#include "SDL.h"

#include "bmp2nam.h"



/*
** ************************************************************************** *|
**                           Core Program Functions                           *|
** ************************************************************************** *|
*/

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
			tmp = ANSI_GetColor(program.ref_palette[index]);
			String_Merge(&str, &tmp);
			++index;
		}
		Log_Message(&program.logger, "\t%s", str);
		String_Delete(&str);
	}
	return (OK);
}



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

int		CheckBitmap_TilesColors(void)
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
					ANSI_GetColor(program.tiles_colors[index].colors[i].color),
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
