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

/* Struct constructor for type Drug pointer */
extern Drug* 
newDrug(char *medicationName, unsigned int medicationDoses[], size_t numberOfDoses)
{
    Drug* p = malloc(sizeof(unsigned int) + numberOfDoses + strlen(medicationName));
    
    p->medicationName = medicationName;
    p->numberOfDoses = numberOfDoses;

    for (int i = 0; i < numberOfDoses; i++) p->medicationDoses[i] = medicationDoses[i];
    
    return(p);
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
        
    return(-2);
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
        if (!strncmp(c, "exit", 4) || !strncmp(c, "quit", 4)) return(-1);
        if (!strncmp(c, "back", 4) || !strncmp(c, "Back", 4)) return(-2);
        if (!strncmp(c, "help", 4) || !strncmp(c, "Help", 4)) return(printHelpMenu());
        
        if (c[0] >= 'a' && c[0] <= (*lastObj - 1)) return((int) (c[0] - 'a'));
        else
        {
            DRUGIO_ERR(DRUGIO_OOR); /* Error: out of range */
            return(-2); 
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
    DrugAndDoseToPrint dip; dip.promise = 1;

DRUGIO_MENU:
    while(d >= 0)
    {
        printf("Please type the letter conresponding to the Drug taken\n"
               "then press the enter key. Type \"help\" for help.\n\n"
              );

        /* Print Drug names */
        for (ident = 'a', i = 0; drugList[i] != NULL; ++i, ++ident)
            printf("[%c] %s\n", ident, drugList[i]->medicationName);

        d = readUserInput(&ident);
        
        /* Check d */
        if (d < 0) break;
        else
        {
            dip.userSelectedMedication = drugList[d];
            /* Early out if only one dose for selected Drug */
            if (dip.userSelectedMedication->numberOfDoses == 1)
            {
                dip.userSelectedDosage = dip.userSelectedMedication->medicationDoses[0];
                break;
            }
            
            printf("\nDoses for %s:\n\n", dip.userSelectedMedication->medicationName);
            
            /* Print the Drug doses */
            for (ident = 'a', i = 0;  i < dip.userSelectedMedication->numberOfDoses ; ++i, ++ident)
                printf("[%c] %2g mg\n", ident, (dip.userSelectedMedication->medicationDoses[i] / 1000.0));

            d = readUserInput(&ident);
            
            /* Check d */
            if (d >= 0) dip.userSelectedDosage = dip.userSelectedMedication->medicationDoses[d];
            break;
        } /* else of: if (d < 0) */
    } /* while(d >= 0) */
    
    if (d == -2) 
    {
        d = 0;
        goto DRUGIO_MENU;    
    }
    if (d == -1) dip.promise = 0;
    
    return(dip);
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
            return(fileDate);
        case 1 : 
            strftime(fullDate, 19, "%F - %H:%M", timeNow);
            return(fullDate);
        case 2 :
            strftime(fullTime, 6, "%H:%M", timeNow);
            return(fullTime);
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
    else if (c[0] == 'y' || c[0] == 'Y') return(1);
    else return(0);
}

/* Show today's log */
static void 
showLogs(char** filePathString)
{
    FILE* f = fopen(*filePathString, "r");
    int c;
    
    /* Go to eof */
    fseek(f, 0, SEEK_END);
    /* Check if eof is at 0 (aka empty file) */
    if (ftell(f))
    {
        printf("—————————————————————————————————————————————\n"
               "|%*s%*c\n"
               "—————————————————————————————————————————————\n",
                24, (formatDateToString(2)), 20, '|'
              );

        fseek(f, 0, SEEK_SET);
        while(1)
        {
            c = fgetc(f);
            if (feof(f)) break;
            else printf("%c", c);
        }
        printf("—————————————————————————————————————————————\n");
    }
    fclose(f);
}

/* Print the end result */
extern void
printd(Drug* drugList[], char* drugioLogFolderPath)
{
    DrugAndDoseToPrint dip;

    char* theDate = formatDateToString(1);
    char* theFileName = formatDateToString(0);
    
    char* completePathToFile = malloc((strlen(theFileName) + strlen(drugioLogFolderPath) + 1));
    
    strcpy(completePathToFile, drugioLogFolderPath); 
    strcat(completePathToFile, theFileName);

    FILE *logFile = fopen(completePathToFile, "a+");
    
    #define DRUGIO_USE_FILE (DRUGIO_DEBUG) ? stdout : logFile

    do
    {
        showLogs(&completePathToFile);
        
        dip = drugioMenu(drugList);
        
        if (!dip.promise) break;
        else fprintf(DRUGIO_USE_FILE,"[%s] %s %2g mg\n", theDate
                    , dip.userSelectedMedication->medicationName, dip.userSelectedDosage / 1000.0);

    } while (doesUserWantToRunAgain());

    fclose(logFile); 
    free(completePathToFile);
}
