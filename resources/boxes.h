/*
 *  This project provides an easy way for users to 
 *  create boxes to surround their text. 
 *  Copyright (C) 2018 Anthony Capobianco
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef _BOXES_H_INCLUDED_
#define _BOXES_H_INCLUDED_

#define P_BOX_TOP_LEFT "╔"
#define P_BOX_BOTTOM_LEFT "╚"
#define P_BOX_TOP_RIGHT "╗"
#define P_BOX_BOTTOM_RIGHT "╝"

#define P_BOX_VERTICAL_LINE "║"
#define P_BOX_HORIZONTAL_LINE "═"

#define POSITION unsigned char
#define P_TOP 0
#define P_BOTTOM 1

typedef struct A
{
    char *string;
} StringArray;

/* Functions prototypes */
extern void _mkBox(const char* string, const unsigned long stringLength, const unsigned char isCustomLength);
extern void drawHorizontalLine(const unsigned int length);

#define mkBox(x) _mkBox(x, (strlen(x) + 1), 0)
#define makeBox(x) mkBox(x)
#define mkLargerBox(string, length) _mkBox(string, (length - (strlen(string) - 1)), 1)

#ifndef stringList
#define stringList(...) StringArray* stringList[] = {__VA_ARGS__, NULL}
#endif

extern StringArray* newLine(char *string);

extern void _multilineBox(StringArray *stringList[]);
extern void stringArrayDestructor(StringArray *stringList[]);

#define multilineBox(...) stringList(__VA_ARGS__); _multilineBox(stringList); stringArrayDestructor(stringList)


#endif
