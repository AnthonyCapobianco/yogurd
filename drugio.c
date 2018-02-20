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
static void 
drugioDestructor(drug* arrPtr[])
{
    for (int i = 0; arrPtr[i] != NULL; i++) free(*(arrPtr + i));
}

/* Struct constructor for type dDate */
/* TODO Finish this */
/*
dDate newDate(short isToday)
{
    time_t today
         , yesterday
         ;
         
    struct tm *tmToday
            , *tmYesterday
            ;
    
    time(&yesterday); time(&today);
    yesterday -= 86400;
    tmYesterday = localtime(&yesterday); tmToday = localtime(&today); 
    
    dDate t = 
    { .d = tmToday->tm_mday
    , .m = (tmToday->tm_mon + 1)
    , .y = (tmToday->tm_year + 1900)
    , .h = tmToday->tm_hour
    , .min = tmToday->tm_min
    };
    
    dDate y = 
    { .d = tmYesterday->tm_mday
    , .m = (tmYesterday->tm_mon + 1)
    , .y = (tmYesterday->tm_year + 1900)
    , .h = tmYesterday->tm_hour
    , .min = tmYesterday->tm_min
    };
    if (isToday) return(t);
    else return(y);
}
*/

/* Make Selection (read user input) */
int 
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
            if (!strncmp(c, "exit", 4) || !strncmp(c, "quit", 4)) exit(0);
            if (!strncmp(c, "back", 4)) return(-1);
            if (!strncmp(c, "help", 4) || !strncmp(c, "Help", 4))
            {
              puts("============================================================");
              puts("Help menu:\n");
              
              puts("\tType \"exit\" or \"quit\" to exit the program");
              puts("\tType \"back\" to go back to the previous menu");
              puts("\tType \"help\" to show this menu\n");
              
              puts("============================================================\n");
              return(-1);
            }
            if (c[0] >= 'a' && c[0] <= (*lastObj - 1)) return((int) (c[0] - 'a'));
            else
            {
                DRUGIO_ERR(DRUGIO_OOR); /* Print error out of range to console */
                return(-1); /* Try again */
            }
        }
    } while (c + strlen(c) != NULL); /* While nothing is horribly wrong */
}

/* Print drug names */
static diPtr 
drugioMenu(drug* ptr[])
{
    char ident;
    int i;
    int d;
    drug* idedDrug;

DRUGIO_MENU:
    while(1)
    {
        puts("Please type the letter conresponding to the drug taken");
        puts("then press the enter key. Type help for help.\n");

        for (ident = 'a', i = 0; ptr[i] != NULL; ++i, ++ident) printf("[%c] %s\n", ident, ptr[i]->name);

        d = (int) (makeSelection(&ident));
        /* User typed "back" */
        if (d == -1) goto DRUGIO_MENU;
        /* Early out if only one dose */
        else if (!ptr[d]->doses[1]) return((diPtr) { .iPtr = 0, .dPtr = ptr[d]});
        else idedDrug = ptr[d];

        printf("\nDoses for %s:\n\n", idedDrug->name);
        for (ident = 'a', i = 0; idedDrug->doses[i] != 0 ; ++i, ++ident)
        {
            if (idedDrug->isNanoGram) printf("[%c] %2g mg\n", ident, (float) (idedDrug->doses[i] / 1000.0));
            else printf("[%c] %d mg\n", ident, idedDrug->doses[i]);
        }

        d = (int) (makeSelection(&ident));
        if (d == -1) goto DRUGIO_MENU;
        else return((diPtr) { .iPtr = d, .dPtr = idedDrug});
    }
}

/* Date parsing and formating */
char* 
formatedDate(short isFullFormat)
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
short 
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
void 
drugioSetPath(char* s)
{
    drugioFilePath = s;
}

/* Show yesterday's log */
void 
showLogs(char** fpString)
{
    FILE* f = fopen(*fpString, "r");
    int c;
    
    /* Go to eof */
    fseek(f, 0, SEEK_END);
    /* Check if eof is at 0 (aka empty file) */
    if (ftell(f) == 0);
    else
    {
        printf("—————————————————————————————————————————————\n"
               "|%*s%*c\n"
               "—————————————————————————————————————————————\n", 
                24, (char *) (formatedDate(2)), 20, '|'
              );
        fseek(f, 0, SEEK_SET);
        while(1) 
        {
            c = fgetc(f); 
            if (feof(f)) break;
            printf("%c", c);
        }
        printf("—————————————————————————————————————————————\n");
    }
    fclose(f);
}

/* Print the end result */
void 
printd(drug* arrPtr[])
{
    char* theDate = (char *) (formatedDate(1));
    char* theFileDate = (char*) (formatedDate(0));
    
    char* buffer = (char*) malloc((strlen(theFileDate) + strlen(drugioFilePath) + sizeof(char)));
    strcpy(buffer, drugioFilePath); 
    strcat(buffer, theFileDate);
    
    do
    {
        showLogs(&buffer);
        
        FILE *ftoday; ftoday = fopen(buffer, "a+");
        
        diPtr dip = drugioMenu(arrPtr);
        drug* p = dip.dPtr; int d = dip.iPtr;

        if (p->isNanoGram) fprintf(DRUGIO_USE_FILE,"[%s] %s %2g mg\n", theDate, p->name, ((float) p->doses[d] / 1000));
        else fprintf(DRUGIO_USE_FILE,"[%s] %s %d mg\n", theDate, p->name, p->doses[d]);
      
        fclose(ftoday); 
    } while (runAgain());
    
    free(buffer); drugioDestructor(arrPtr); /* Free objects we used malloc for */
}
