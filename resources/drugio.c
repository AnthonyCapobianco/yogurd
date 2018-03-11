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
#include <string.h>
#include <time.h>
#include "drugio.h"
#include "boxes.h"

/* Struct constructor for type Drug pointer */
extern Drug* 
newDrug(char* dName, int* dDoses, bool isNG)
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
static void
printHelpMenu()
{
    printf("============================================================\n"
           "Help menu:\n\n"
           "\tType \"exit\" or \"quit\" to exit the program\n"
           "\tType \"back\" to go back to the previous menu\n"
           "\tType \"help\" to show this menu\n\n"
           "============================================================\n"
          );
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
        if (!strncmp(c, "help", 4) || !strncmp(c, "Help", 4))
        {
            printHelpMenu();
            return -2;
        }
        
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
    Drug *dPtr = NULL;
    DrugAndDoseToPrint dip; dip.promise = true;

    char ident;
    int i;
    int d;


DRUGIO_MENU:
    i = 0; d = 0; ident = 'a';
    while(d >= 0)
    {
        printf("Please type the letter conresponding to the Drug taken\n"
               "then press the enter key. Type \"help\" for help.\n\n"
              );

        /* Print Drug names */
        for (; drugList[i] != NULL; ++i, ++ident) printf("[%c] %s\n", ident, drugList[i]->name);

        d = readUserInput(&ident);
        
        if (d < 0) break;
        else
        {
            dPtr = drugList[d];
            dip.drugName = dPtr->name;

            /* Early out if only one dose for selected Drug */
            if (!dPtr->doses[1])
            {
                if (!dPtr->isNanoGram) dip.drugDose = (float) dPtr->doses[0];
                else dip.drugDose = (float) dPtr->doses[0] / 1000.0;
                break;
            }
            
            printf("\nDoses for %s:\n\n", dPtr->name);
            
            /* Print the Drug doses */
            for (ident = 'a', i = 0; dPtr->doses[i] != 0 ; ++i, ++ident)
            {
                printf("[%c] ", ident);
                
                if (!dPtr->isNanoGram) printf("%d mg\n", dPtr->doses[i]);
                else printf("%-2g mg\n", (dPtr->doses[i] / 1000.0));
            }

            d = readUserInput(&ident);
            
            break;
        } /* else of: if (d < 0) */
    } /* while(d >= 0) */

    switch (d)
    {
        case -2: goto DRUGIO_MENU; break;
        case -1: dip.promise = false; break;
        default:
            if (!dPtr->isNanoGram) dip.drugDose = (float) dPtr->doses[d];
            else dip.drugDose = (float) dPtr->doses[d] / 1000.0;
            break;
    }
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
static bool
doesUserWantToRunAgain()
{
    char c[4];

    printf("Do you want to run this again? (Y/N): ");
    if(!fgets(c, 4, stdin))
    {
        DRUGIO_ERR(DRUGIO_EOF);
        exit(EXIT_FAILURE);
    }
    else if (c[0] == 'Y' || c[0] == 'y') return true;
    else return false;
}

/* Show today's log */
static int 
showLogs(char** filePathString)
{
    
    mkLargerBox(formatDateToString(2), BOX_SIZE);
    
    FILE* f = fopen(*filePathString, "r");
    
    if (f == NULL) return -1;
    else
    {
        /* Go to eof */
        fseek(f, 0, SEEK_END);

       /* Check where the file pointer is.
        * Only excecute if not at 0 since it should be at eof.
        * And if eof is at 0 there is nothing to print.
        */
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
            drawHorizontalLine(BOX_SIZE + 6);
        }
        fflush(f); fclose(f);
        return 0;
    }
}

/* Print the end result */
extern void
_fprintd(const char* drugioLogFolderPath, Drug* drugList[])
{
    char* theDate = formatDateToString(1);
    char* theFileName = formatDateToString(0);
    
    char* completePathToFile = malloc((strlen(theFileName) + strlen(drugioLogFolderPath) + 1));
    
    strcpy(completePathToFile, drugioLogFolderPath); 
    strcat(completePathToFile, theFileName);

    do
    {
        showLogs(&completePathToFile);
        
        DrugAndDoseToPrint dip = drugioMenu(drugList);
        
        if (!dip.promise) break;
        else
        {
            FILE *logFile = fopen(completePathToFile, "a+");

            fprintf(DRUGIO_USE_FILE,"[%s] %s %-2g mg\n", theDate, dip.drugName, dip.drugDose);
            
            fflush(logFile); fclose(logFile); 
        }
        
    } while (doesUserWantToRunAgain());

    free(completePathToFile);
}
