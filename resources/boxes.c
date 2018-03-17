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
#include "boxes.h"
#define draw_line(x) do_draw_line(&boxLength, x)

static unsigned char
did_draw_line(unsigned long *boxLength)
{
        for (unsigned int i = 0; i < *boxLength; i++) printf(P_BOX_HORIZONTAL_LINE);
        return 1;
}

static void
do_draw_line(unsigned long *boxLength, POSITION isBottom)
{
        if (isBottom) printf(P_BOX_BOTTOM_LEFT);
        else printf(P_BOX_TOP_LEFT); 
        
        if (did_draw_line(boxLength) && isBottom) puts(P_BOX_BOTTOM_RIGHT); 
        else puts(P_BOX_TOP_RIGHT);
}

static void
print_line(char **stringPtr)
{
        printf(P_BOX_VERTICAL_LINE "%*c%s%*c", 5, ' ', *stringPtr, 5, ' '); puts(P_BOX_VERTICAL_LINE);
}

static void
print_multi_line(unsigned long lineLength , char **stringPtr)
{
        int spacing = (int) ((lineLength) - strlen(*stringPtr)) + 9;
    
        printf(P_BOX_VERTICAL_LINE " %s%-*c", *stringPtr, spacing, ' '); puts(P_BOX_VERTICAL_LINE);
}

static int 
biggest_of_two_number(int n1, int n2)
{
        return((n1 > n2) ? n1 : n2);
}

static unsigned long
longest_string(size_t numberOfNumbersToCompare, StringArray* arrayOfStrings[])
{
        if (numberOfNumbersToCompare > 2)
        {
                static unsigned int result = 0
                                  , firstQuery = 0
                                  ;
                static int *numberArray;

                static size_t i
                            , j
                            ;

                numberArray = malloc(numberOfNumbersToCompare * sizeof(int));

                for (i = 0; i < numberOfNumbersToCompare; i++)
                        numberArray[i] = (int) strlen(arrayOfStrings[i]->string);

                result = numberArray[0];

                for (j = (numberOfNumbersToCompare - 1); j > 0; j -= 2)
                {
                        firstQuery = biggest_of_two_number(numberArray[j], numberArray[j - 1]);
                        result = biggest_of_two_number(result, firstQuery);
                }


                free(numberArray);
                return result;
        }
        else return - 1;
}

extern StringArray*
new_line(char *string)
{
        StringArray *p = malloc(sizeof(StringArray));
    
        p->string = string;
    
        return(p);
}

extern void 
stringArray_destructor(StringArray *stringList[])
{
        for (int i = 0; stringList[i] != NULL; i++) free(stringList[i]);
}

extern void
mk_box(const char* string, const unsigned long stringLength, const unsigned char isCustomLength)
{
        static unsigned long boxLength; boxLength = stringLength + 9;

        static char *staticStringCopy; staticStringCopy = malloc(stringLength);
        strlcpy(staticStringCopy, string, stringLength);
        draw_line(P_TOP); 

        if (isCustomLength) print_multi_line((stringLength - 1), &staticStringCopy);
        else print_line(&staticStringCopy); 

        draw_line(P_BOTTOM);

        free(staticStringCopy);
}

extern void
multiline_box(StringArray *stringList[])
{
        if (stringList != NULL)
        {
                unsigned long boxLength = 0;
                unsigned long lineLength = 0;

                size_t k = 0; while (stringList[k] != NULL) k++;


                if (k) lineLength = longest_string(k, stringList);
                else lineLength = 0;
                boxLength = lineLength + 10;

                draw_line(P_TOP);

                for (size_t i = 0; i < k; i++) print_multi_line(lineLength, &stringList[i]->string);

                draw_line(P_BOTTOM);
        }
        else return;
}

extern void
draw_horizontal_line(const unsigned int length)
{
        for (unsigned int i = 0; i < length; i++) printf(P_BOX_HORIZONTAL_LINE);
        puts("");
}
