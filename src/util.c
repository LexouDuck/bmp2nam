
#include <libccc.h>
#include <libccc/color.h>
#include <libccc/memory.h>
#include <libccc/string.h>
#include <libccc/sys/logger.h>
#include <libccc/math/math.h>
#include <libccc/math/sort.h>

#include "SDL.h"

#include "bmp2nam.h"



/*
** ************************************************************************** *|
**                          Color Utility Functions                           *|
** ************************************************************************** *|
*/

t_u32 Color_Sum(t_argb32 color)
{
	return (
		(t_u32)Color_ARGB32_Get_R(color) +
		(t_u32)Color_ARGB32_Get_G(color) +
		(t_u32)Color_ARGB32_Get_B(color));
}



/*
** ************************************************************************** *|
**                         Sorting Utility Functions                          *|
** ************************************************************************** *|
*/

//! sort indexed colors of the `ref_palette`, by brightness
int Compare_ColorDiffs(s_colordiff c1, s_colordiff c2)
{
	return (c2.value - c1.value);
}
DEFINEFUNC_C_QUICKSORT(s_colordiff, Compare_ColorDiffs)

//! sort colors by popularity
int Compare_ColorUse(s_color_use c1, s_color_use c2)
{
	return (c1.occurences - c2.occurences);
}
DEFINEFUNC_C_QUICKSORT(s_color_use, Compare_ColorUse)

//! sort palettes by popularity
int Compare_Palette(s_palette c1, s_palette c2)
{
	return (c1.popularity - c2.popularity);
}
DEFINEFUNC_C_QUICKSORT(s_palette, Compare_Palette)

//! sort indexed colors of the `ref_palette`, by brightness
int Compare_Color(t_u8 c1, t_u8 c2)
{
	return (Color_Sum(program.ref_palette[c2]) - Color_Sum(program.ref_palette[c1]));
}
DEFINEFUNC_C_QUICKSORT(t_u8, Compare_Color)



/*
** ************************************************************************** *|
**                         Palette Utility Functions                          *|
** ************************************************************************** *|
*/

t_sint Palette_Find(s_palette const* palette, t_u8 color)
{
	for (int i = 0; i < palette->length; ++i)
	{
		if (palette->colors[i] == color)
			return (i);
	}
	return (-1);
}

t_bool Palette_Contains(s_palette const* palette, t_u8 color)
{
	for (int i = 0; i < palette->length; ++i)
	{
		if (palette->colors[i] == color)
			return (TRUE);
	}
	return (FALSE);
}

t_bool Palette_ContainsAll(s_palette const* palette, s_palette const* target)
{
	for (int i = 0; i < target->length; ++i)
	{
		if (!Palette_Contains(palette, target->colors[i]))
			return (FALSE);
	}
	return (TRUE);
}



s_palette Palette_GetMostUsedColors(s_color_use const* colors, t_u8 maxlength)
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
t_s64 GetSmallestColorDifference(t_u8 target, s_palette const* palette)
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

s_palette const* Palette_GetNearest(s_palette target, s_palette const* palettes, t_uint length)
{
	s_colordiff diffs[BMP_MAXCOLORS] = {0};
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
**                         Color ANSI stdout escapes                          *|
** ************************************************************************** *|
*/

t_char* ANSI_GetColor(t_argb32 color)
{
	return (String_Format(IO_COLOR_BG("%i")"  "IO_RESET, IO_GetColor(color)));
}



t_char* ANSI_GetPalette(s_palette* palette)
{
	t_char* tmp = NULL;
	t_char* result = String_New(0);
	if (result == NULL)
	{
		Log_Error(&program.logger, 0, "Could not allocate palette logging output string");
		return (NULL);
	}
	for (int j = 0; j < PAL_SUB_COLORS; ++j)
	{
		if (j < palette->length)
			tmp = ANSI_GetColor(program.ref_palette[palette->colors[j]]);
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



int PrintColorStats(s_color_use const* array, t_size length)
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
			ANSI_GetColor(sorted[i].color),
			sorted[i].index,
			sorted[i].index,
			sorted[i].color,
			sorted[i].occurences,
			sorted[i].occurences / total * 100.);

	}
	return (OK);
}
