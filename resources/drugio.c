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
#include <sqlite3.h>
#include "boxes.h"
#include "drugio.h"

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
                        if (!dPtr->doses[1]) d = 0;
                        else
                        {
                                printf("\nDoses for %s:\n\n", dPtr->name);
                                
                                /* Print the Drug doses */
                                for (ident = 'a', i = 0; dPtr->doses[i] != 0 ; ++i, ++ident)
                                {
                                        printf("[%c] ", ident);
                                        
                                        if (!dPtr->isNanoGram) printf("%d mg\n", dPtr->doses[i]);
                                        else printf("%-2g mg\n", (float) (dPtr->doses[i] / 1000.0f));
                                }

                                d = readUserInput(&ident);
                        }
                        break;
                } /* else of: if (d < 0) */
        } /* while(d >= 0) */

        switch (d)
        {
                case -2: goto DRUGIO_MENU; break;
                case -1: dip.promise = false; break;
                default:
                        if (!dPtr->isNanoGram) dip.drugDose = (float) dPtr->doses[d] / 1.0f;
                        else dip.drugDose = (float) dPtr->doses[d] / 1000.0f;
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
        

        static char fullDate[11]
                  , fullTime[6]
                  ;
        
        switch (isFullFormat) 
        {
                default:
                case 0 : strftime(fullTime, 6, "%H:%M", timeNow); 
                         return fullTime;
                case 1 : strftime(fullDate, 11, DRUGIO_DATE_FORMAT, timeNow); 
                         return fullDate;
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

/* Database integration */
static int
callback(void *NotUsed, int argc, char **argv, char **azColName)
{
        char *endPtr;
        float boat = strtof(argv[4], &endPtr);
        
        if (argc) printf("[%s - %s] %s %-2g mg\n", argv[1], argv[2], argv[3], boat); 
        return 0;
}

static int
printLogsFromDate(sqlite3 *dbPtr, int logDatabaseHandler, char *date)
{
        char *zErrMsg = 0;
        const char* data = "Callback function called";

        char *sqlStatement = sqlite3_mprintf("SELECT * FROM logs WHERE theDate is '%q'", date);
        
        logDatabaseHandler = sqlite3_exec(dbPtr, sqlStatement, callback, (void*) data, &zErrMsg);

        if (logDatabaseHandler != SQLITE_OK) 
        {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
        }
        
        free(sqlStatement);
        
        return 0;
}

static int
addToLogs(sqlite3 *dbPtr, int logDatabaseHandler, char* theDate, char* theTime, char* drug, float dose)
{
        char *zErrMsg = 0;
        const char* data = "Callback function called"; /* For debug */

        if (logDatabaseHandler != SQLITE_OK) SQLITE_NOT_OK(dbPtr);

        char *sqlStatement = sqlite3_mprintf("INSERT INTO logs(theDate, theTime, name, dose) VALUES('%q','%q','%s','%2g')", 
                                             theDate, theTime, drug, dose
                                            );


        logDatabaseHandler = sqlite3_exec(dbPtr, sqlStatement, callback, (void*) data, &zErrMsg);

        if (logDatabaseHandler != SQLITE_OK) 
        {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
        }

        free(sqlStatement);   
        
        return 0;
}

/* Show today's log */
static int 
showLogs(sqlite3 *dbPtr, int logDatabaseHandler, char* theTime, char* theDate)
{
        
        mkLargerBox(theTime, BOX_SIZE);
        
        int ret = printLogsFromDate(dbPtr, logDatabaseHandler, theDate);
        
        drawHorizontalLine(BOX_SIZE + 6);
        
        return ret;
}

/* Print the end result */
extern void
_fprintd(const char* dbPath, Drug* drugList[])
{
        char* theDate = formatDateToString(1);
        char* theTime = formatDateToString(0);

        sqlite3 *dbPtr;

        int logDatabaseHandler = sqlite3_open(dbPath, &dbPtr);

        if (logDatabaseHandler != SQLITE_OK) SQLITE_NOT_OK(dbPtr);
        else do
        {
                showLogs(dbPtr, logDatabaseHandler, theTime, theDate);

                DrugAndDoseToPrint dip = drugioMenu(drugList);

                if (!dip.promise) break;
                else addToLogs(dbPtr, logDatabaseHandler, theDate, theTime, dip.drugName, dip.drugDose);

        } while (doesUserWantToRunAgain());

        sqlite3_close(dbPtr);
}
