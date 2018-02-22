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

/* Struct constructor for type drug pointer */
extern drug* 
newDrug(char* dName, int* dDoses, short isNG)
{
    drug* p = malloc(sizeof(drug));
    
    p->name = dName; 
    p->doses = dDoses; 
    p->isNanoGram = isNG;
    
    return(p);
}

/* Struct destructor for drugs */
extern void
drugioDestructor(drug* arrPtr[])
{
    for (int i = 0; arrPtr[i] != NULL; i++) free(*(arrPtr + i));
    exit(EXIT_SUCCESS);
    
}

/* Make Selection (read user input) */
static int 
makeSelection(char* lastObj)
{
    char c[20];

    do
    {
        printf("> ");
        if (!fgets(c, 20, stdin))
        {
            DRUGIO_ERR(DRUGIO_EOF); /* Print error end-of-file */
            exit(EXIT_FAILURE); /* Bail out */
        }
        else
        {
            if (!strncmp(c, "exit", 4) || !strncmp(c, "quit", 4)) return(-1);
            if (!strncmp(c, "back", 4) || !strncmp(c, "Back", 4)) return(-2);
            if (!strncmp(c, "help", 4) || !strncmp(c, "Help", 4))
            {
              puts("============================================================\n"
                   "Help menu:\n\n"
                   "\tType \"exit\" or \"quit\" to exit the program\n"
                   "\tType \"back\" to go back to the previous menu\n"
                   "\tType \"help\" to show this menu\n\n"
                   "============================================================\n"
                  );
              return(-2);
            }
            if (c[0] >= 'a' && c[0] <= (*lastObj - 1)) return((int) (c[0] - 'a'));
            else
            {
                DRUGIO_ERR(DRUGIO_OOR); /* Print error out of range to console */
                return(-2); /* Try again */
            }
        } 
    } while (c + strlen(c) != NULL);
}

/* Print drugs */
static diPtr 
drugioMenu(drug* ptr[])
{
    char ident;
    int i;
    int d;
    drug* idedDrug;
    diPtr dip; dip.promise = 0;

DRUGIO_MENU:
    d = 0;
    while(d >= 0)
    {
        puts("Please type the letter conresponding to the drug taken");
        puts("then press the enter key. Type \"help\" for help.\n");

        /* Print drug names */
        for (ident = 'a', i = 0; ptr[i] != NULL; ++i, ++ident) printf("[%c] %s\n", ident, ptr[i]->name);

        d = makeSelection(&ident);
        
        /* Check d */
        if (d < 0) break;
        else
        {
            /* Early out if only one dose for selected drug */
            if (!ptr[d]->doses[1])
            {
                dip.iPtr = 0;
                dip.dPtr = ptr[d];
                break;
            }
            else idedDrug = ptr[d];

            printf("\nDoses for %s:\n\n", idedDrug->name);
            
            /* Print the drug doses */
            for (ident = 'a', i = 0; idedDrug->doses[i] != 0 ; ++i, ++ident)
            {
                if (idedDrug->isNanoGram) printf("[%c] %2g mg\n", ident, (idedDrug->doses[i] / 1000.0));
                else printf("[%c] %d mg\n", ident, idedDrug->doses[i]);
            }

            d = makeSelection(&ident);
            
            /* Check d */
            if (d < 0) break;
            else 
            {
                dip.iPtr = d;
                dip.dPtr = idedDrug;
                break;
            }
        } /* else no early out */
    } /* while(d >= 0) */
    
    if (d == -2) goto DRUGIO_MENU;
    if (d == -1) dip.promise = -1;
    
    return(dip);
}

/* Date parsing and formatting */
static char* 
formattedDate(short isFullFormat)
{
    size_t strftime(char *, size_t, const char *, const struct tm *);

    time_t rawtime; struct tm *info;
    time(&rawtime); info = localtime(&rawtime);
    

    static char fileDate[19]
              , fullDate[19]
              , fullTime[6]
              ;
    
    switch (isFullFormat) 
    {
        default:
        case 0 : 
            strftime(fileDate, 19, "log-%F.txt", info);
            return(fileDate);
        case 1 : 
            strftime(fullDate, 19, "%F - %H:%M", info);
            return(fullDate);
        case 2 :
            strftime(fullTime, 6, "%H:%M", info);
            return(fullTime);
    }
}

/* Ask to run again */
static short 
runAgain()
{
    char c[4];

    printf("Do you want to run this again? (Y/N): ");
    if(!fgets(c, 4, stdin))
    {
        DRUGIO_ERR(DRUGIO_EOF);
        exit(EXIT_FAILURE);
    }
    else
    {
        if (c[0] == 'y' || c[0] == 'Y') return(1);
        else return(0);
    }
}

/* Set path of file to log to */
extern void 
drugioSetPath(char* s)
{
    drugioFilePath = s;
}

/* Show yesterday's log */
static void 
showLogs(char** fpString)
{
    FILE* f = fopen(*fpString, "r");
    int c;
    
    /* Go to eof */
    fseek(f, 0, SEEK_END);
    /* Check if eof is at 0 (aka empty file) */
    if (ftell(f))
    {
        printf("—————————————————————————————————————————————\n"
                "|%*s%*c\n"
                "—————————————————————————————————————————————\n",
                24, (formattedDate(2)), 20, '|'
              );

        fseek(f, 0, SEEK_SET);
        while(1)
        {
            c = fgetc(f);
            if (feof(f)) break;
            printf("%c", c);
        }
        printf("\n—————————————————————————————————————————————\n");
    }
    fclose(f);
}

/* Print the end result */
extern void
printd(drug* arrPtr[])
{
    char* theDate = formattedDate(1);
    char* theFileDate = formattedDate(0);
    
    char* buffer = malloc((strlen(theFileDate) + strlen(drugioFilePath) + 1));
    strcpy(buffer, drugioFilePath); strcat(buffer, theFileDate);

    FILE *ftoday; ftoday = fopen(buffer, "a+");
    
    do
    {
        showLogs(&buffer);
        
        diPtr dip = drugioMenu(arrPtr);
        if (dip.promise == -1) break;
        else
        {
            drug* p = dip.dPtr; 
            int d = dip.iPtr;

            if (p->isNanoGram) 
                fprintf(DRUGIO_USE_FILE,"[%s] %s %2g mg\n", theDate, p->name, (p->doses[d] / 1000.0));
            else 
                fprintf(DRUGIO_USE_FILE,"[%s] %s %d mg\n", theDate, p->name, p->doses[d]);
        }
    } while (runAgain());

    fclose(ftoday); 
    free(buffer);
}
