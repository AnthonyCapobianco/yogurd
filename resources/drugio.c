/*
 *  This program is designed to provide an easy way for
 *  users to log doses of the medication(s) they use.
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
#define DRUGIO_DEBUG 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "drugio.h"
#include "boxes.h"

/* Struct constructor for type Drug pointer */
extern Drug* 
newDrug(char* dName, int* dDoses, short isNG)
{
    Drug* p = malloc(sizeof(Drug));
    
    p->name = dName; 
    p->doses = dDoses; 
    p->isNanoGram = isNG;
    
    return p;
}

/* Struct destructor for drugs */
extern void
drugioDestructor(Drug* drugList[])
{
    for (int i = 0; drugList[i] != NULL; i++) free(*(drugList + i));
    exit(EXIT_SUCCESS);
    
}

/* Print help menu */
static int
printHelpMenu()
{
    printf("============================================================\n"
           "Help menu:\n\n"
           "\tType \"exit\" or \"quit\" to exit the program\n"
           "\tType \"back\" to go back to the previous menu\n"
           "\tType \"help\" to show this menu\n\n"
           "============================================================\n"
          );
        
    return -2;
}

/* Make Selection (read user input) */
static int 
readUserInput(char* lastObj)
{
    char c[6];

    printf("> ");
    if (!fgets(c, 6, stdin))
    {
        DRUGIO_ERR(DRUGIO_EOF);
        exit(EXIT_FAILURE);
    }
    else
    {
        if (!strncmp(c, "exit", 4) || !strncmp(c, "quit", 4)) return -1;
        if (!strncmp(c, "back", 4) || !strncmp(c, "Back", 4)) return -2;
        if (!strncmp(c, "help", 4) || !strncmp(c, "Help", 4)) return printHelpMenu();
        
        if (c[0] >= 'a' && c[0] <= (*lastObj - 1)) return((int) (c[0] - 'a'));
        else
        {
            DRUGIO_ERR(DRUGIO_OOR); /* Error: out of range */
            return -2; 
        }
    } 
}

/* Print drugs */
static DrugAndDoseToPrint 
drugioMenu(Drug* drugList[])
{
    char ident;
    int i;
    int d = 0;
    DrugAndDoseToPrint dip; dip.promise = 0;

DRUGIO_MENU:
    while(d >= 0)
    {
        printf("Please type the letter conresponding to the Drug taken\n"
               "then press the enter key. Type \"help\" for help.\n\n"
              );

        /* Print Drug names */
        for (ident = 'a', i = 0; drugList[i] != NULL; ++i, ++ident) printf("[%c] %s\n", ident, drugList[i]->name);

        d = readUserInput(&ident);
        
        if (d < 0) break;
        else
        {
            dip.dPtr = drugList[d];
            /* Early out if only one dose for selected Drug */
            if (!dip.dPtr->doses[1])
            {
                dip.doseOffset = 0;
                break;
            }
            
            printf("\nDoses for %s:\n\n", dip.dPtr->name);
            
            /* Print the Drug doses */
            for (ident = 'a', i = 0; dip.dPtr->doses[i] != 0 ; ++i, ++ident)
            {
                printf("[%c] ", ident);
                
                if (!dip.dPtr->isNanoGram) printf("%d mg\n", dip.dPtr->doses[i]);
                else printf("%2g mg\n", (dip.dPtr->doses[i] / 1000.0));
            }

            d = readUserInput(&ident);
            
            if (d >= 0) dip.doseOffset = d;
            break;
        } /* else of: if (d < 0) */
    } /* while(d >= 0) */
    
    if (d == -2) 
    {
        d = 0;
        goto DRUGIO_MENU;    
    }
    if (d == -1) dip.promise = -1;
    
    return dip;
}

/* Date parsing and formatting */
static char* 
formatDateToString(short isFullFormat)
{
    size_t strftime(char *, size_t, const char *, const struct tm *);

    time_t rawtime; struct tm *timeNow;
    time(&rawtime); timeNow = localtime(&rawtime);
    

    static char fileDate[19]
              , fullDate[19]
              , fullTime[6]
              ;
    
    switch (isFullFormat) 
    {
        default:
        case 0 : 
            strftime(fileDate, 19, "log-%F.txt", timeNow);
            return fileDate;
        case 1 : 
            strftime(fullDate, 19, "%F - %H:%M", timeNow);
            return fullDate;
        case 2 :
            strftime(fullTime, 6, "%H:%M", timeNow);
            return fullTime;
    }
}

/* Ask to run again */
static short 
doesUserWantToRunAgain()
{
    char c[4];

    printf("Do you want to run this again? (Y/N): ");
    if(!fgets(c, 4, stdin))
    {
        DRUGIO_ERR(DRUGIO_EOF);
        exit(EXIT_FAILURE);
    }
    else if (c[0] == 'y' || c[0] == 'Y') return 1;
    else return 0;
}

/* Show today's log */
static int 
showLogs(char** filePathString)
{
    
    mkLargerBox(formatDateToString(2), 39);
    
    FILE* f = fopen(*filePathString, "r");
    
    if (f == NULL) return -1;
    else
    {
        /* Go to eof */
        fseek(f, 0, SEEK_END);
        /* Check if eof is at 0 (aka empty file) */
        if (ftell(f))
        {
            int c;

            fseek(f, 0, SEEK_SET);
            while(1)
            {
                c = fgetc(f);
                if (feof(f)) break;
                else printf("%c", c);
            }
            drawHorizontalLine(39 + 6);
        }
        fflush(f); fclose(f);
        return 0;
    }
}

/* Print the end result */
extern void
printd(Drug* drugList[], char* drugioLogFolderPath)
{
    DrugAndDoseToPrint dip;
    
    int d;
    
    char* theDate = formatDateToString(1);
    char* theFileName = formatDateToString(0);
    
    char* completePathToFile = malloc((strlen(theFileName) + strlen(drugioLogFolderPath) + 1));
    
    strcpy(completePathToFile, drugioLogFolderPath); 
    strcat(completePathToFile, theFileName);

    do
    {
        showLogs(&completePathToFile);
        
        dip = drugioMenu(drugList);
        
        if (dip.promise == -1) break;
        else
        {
            d = dip.doseOffset;
            
            FILE *logFile = fopen(completePathToFile, "a+");

            fprintf(DRUGIO_USE_FILE,"[%s] %s ", theDate, dip.dPtr->name);
            
            if (!dip.dPtr->isNanoGram) fprintf(DRUGIO_USE_FILE,"%d mg\n", dip.dPtr->doses[d]);
            else fprintf(DRUGIO_USE_FILE,"%2g mg\n", (dip.dPtr->doses[d] / 1000.0));
            
            fflush(logFile); fclose(logFile); 
        }
        
    } while (doesUserWantToRunAgain());

    free(completePathToFile);
}
