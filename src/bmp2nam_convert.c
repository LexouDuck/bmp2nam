
#include <libccc.h>
#include <libccc/color.h>
#include <libccc/memory.h>
#include <libccc/string.h>
#include <libccc/sys/logger.h>

#include "SDL.h"

#include "bmp2nam.h"



/*
** ************************************************************************** *|
**                           Core Program Functions                           *|
** ************************************************************************** *|
*/

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
	t_char* str;
	for (t_uint i = 0; i < program.tiles_palettes_amount; ++i)
	{
		palette = &program.tiles_palettes[i];
		str = ANSI_GetPalette(palette);
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

int		ConvertBitmap_ApplyOutputPalettes(t_bool user_palette)
{
	Log_Message(&program.logger, "Applying final palette colors to the bitmap...");
	Log_Message(&program.logger,
		"The final set of %i palettes of %i colors each (chosen by popularity):",
		PAL_SUB_AMOUNT,
		PAL_SUB_COLORS);
	for (t_uint i = 0; i < PAL_SUB_AMOUNT; ++i)
	{
		t_char* tmp = ANSI_GetPalette(&program.output_palettes[i]);
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
