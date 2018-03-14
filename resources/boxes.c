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
#define drawLine(x) _drawLine(&boxLength, x)

static unsigned char
didDrawLine(unsigned long *boxLength)
{
    for (unsigned int i = 0; i < *boxLength; i++) printf(P_BOX_HORIZONTAL_LINE);
    return 1;
}

static void
_drawLine(unsigned long *boxLength, POSITION isBottom)
{
    if (isBottom) printf(P_BOX_BOTTOM_LEFT);
    else printf(P_BOX_TOP_LEFT); 
    
    if (didDrawLine(boxLength) && isBottom) puts(P_BOX_BOTTOM_RIGHT); 
    else puts(P_BOX_TOP_RIGHT);
}

static void
printLine(char **stringPtr)
{
    printf(P_BOX_VERTICAL_LINE "%*c%s%*c", 5, ' ', *stringPtr, 5, ' '); puts(P_BOX_VERTICAL_LINE);
}

static void
printMultiLine(unsigned long lineLength , char **stringPtr)
{
    int spacing = (int) ((lineLength) - strlen(*stringPtr)) + 9;
    
    printf(P_BOX_VERTICAL_LINE " %s%-*c", *stringPtr, spacing, ' '); puts(P_BOX_VERTICAL_LINE);
}

static int 
biggestOfTwoNumber(int n1, int n2)
{
    return((n1 > n2) ? n1 : n2);
}

static unsigned long
longestString(size_t numberOfNumbersToCompare, StringArray* arrayOfStrings[])
{
        static unsigned int result;
        static int *numberArray;
        
        static size_t i
                    , j 
                    ;
        
        numberArray = malloc(numberOfNumbersToCompare * sizeof(int));
        
        for (i = 0; i < numberOfNumbersToCompare; i++)
            numberArray[i] = (int) strlen(arrayOfStrings[i]->string);
        
        result = numberArray[0];
                
        for (j = (numberOfNumbersToCompare - 1); j > 0; j -= 2) 
                result = biggestOfTwoNumber(result, biggestOfTwoNumber(numberArray[j], numberArray[j - 1]));
             
        free(numberArray);
        
        return result;
}

extern StringArray*
newLine(char *string)
{
    StringArray *p = malloc(sizeof(StringArray));
    
    p->string = string;
    
    return(p);
}

extern void 
stringArrayDestructor(StringArray *stringList[])
{
    for (int i = 0; stringList[i] != NULL; i++) free(stringList[i]);
}

extern void
_mkBox(const char* string, const unsigned long stringLength, const unsigned char isCustomLength)
{
    static unsigned long boxLength; boxLength = stringLength + 9;
    
    static char *staticStringCopy; staticStringCopy = malloc(stringLength);
    strlcpy(staticStringCopy, string, stringLength);
    drawLine(P_TOP); 
    
    if (isCustomLength) printMultiLine((stringLength - 1), &staticStringCopy);
    else printLine(&staticStringCopy); 
    
    drawLine(P_BOTTOM);
    
    free(staticStringCopy);
}

extern void
_multilineBox(StringArray *stringList[])
{
    unsigned long boxLength; 
    unsigned long lineLength; 
    
    size_t k = 0; while (stringList[k] != NULL) k++;
    
    lineLength = longestString(k, stringList); 
    boxLength = lineLength + 10;
    
    drawLine(P_TOP); 
    
    for (size_t i = 0; i < k; i++) printMultiLine(lineLength, &stringList[i]->string); 
    
    drawLine(P_BOTTOM);
}

extern void
drawHorizontalLine(const unsigned int length)
{
    for (unsigned int i = 0; i < length; i++) printf(P_BOX_HORIZONTAL_LINE);
    puts("");
}
