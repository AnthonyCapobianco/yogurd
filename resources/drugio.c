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
#include <string.h>
#include <time.h>
#include <sqlite3.h>
#include "boxes.h"
#include "drugio.h"

/* Global variable for sqlite */
char *zErrMsg = 0;
const char* data = "Callback function called";
static bool isFirstSqliteStatement = true;
static sqlite3_int64 lastIdInTable = 1;
static sqlite3_int64 numberOfRowsPrinted = 0;
static int logDatabaseHandler;
static sqlite3 *dbPtr;

/* Struct constructor for type Drug pointer */
extern Drug* 
newDrug(char* dName, int* dDoses, bool isNG)
{
        Drug* p = malloc(sizeof(Drug));
        
        if (p == NULL)
        {
                DRUGIO_ERR("ERROR: malloc failed.");
                exit(EXIT_FAILURE);
        }
        
        p->name = dName; 
        p->doses = dDoses; 
        p->isNanoGram = isNG;
        
        return p;
}

/* Struct destructor for drugs */
extern void
free_Drug_array(Drug* drugList[])
{
        for (int i = 0; drugList[i] != NULL; i++) free(*(drugList + i));
        exit(EXIT_SUCCESS);
        
}

/* Date parsing and formatting */
static char*
format_date_to_string(short isFullFormat)
{
        size_t strftime(char *, size_t, const char *, const struct tm *);

        time_t rawTimeNow; time(&rawTimeNow);
        struct tm *timeNow; timeNow = localtime(&rawTimeNow);

        static char fullDate[11]
                  , fullTime[6]
                  ;

        switch (isFullFormat)
        {
            default:
            case 0: strftime(fullTime, 6, "%H:%M", timeNow);
                    return fullTime;
            case 1: strftime(fullDate, 11, DRUGIO_DATE_FORMAT, timeNow);
                    return fullDate;
        }
}

/* parse string to uint */
static unsigned int
parse_string_to_uint(char *buffer)
{
        static volatile long numberToCastToUint = 0;

        char *endPtr;

        numberToCastToUint = strtol(buffer, &endPtr, 10);

        if (numberToCastToUint > UINT_MAX || numberToCastToUint < UINT_MIN)
        {
                DRUGIO_ERR("ERROR #00: NUMBER TOO BIG TO BE AN UNSIGNED INT");
                exit(EXIT_FAILURE);
        }
        else return (unsigned int) numberToCastToUint;
}

/* Database integration */
static int
callback(void *NotUsed, int argc, char **argv, char **azColName)
{

        char *endPtr;
        /* Must be a boat otherwise it sinks */
        float doseBoat = strtof(argv[4], &endPtr);

        if (argc)
        {
                printf("[%s - %s] %s %-2g mg\n", argv[1], argv[2], argv[3], doseBoat);
                if (isFirstSqliteStatement) numberOfRowsPrinted++;
        }
        return 0;
}

static int
set_last_id(void *NotUsed, int argc, char **argv, char **azColName)
{
        lastIdInTable = parse_string_to_uint(argv[0]);
        return 0;
}

static int
print_logs_from_date(char *date)
{
        char *sqlStatement = sqlite3_mprintf("SELECT * FROM logs WHERE theDate is '%q'", date);

        if ((logDatabaseHandler = sqlite3_exec(dbPtr, sqlStatement, callback, (void*) data, &zErrMsg)) != SQLITE_OK) SQLITE_NOT_OK(dbPtr);

        free(sqlStatement);

        return 0;
}

static int
print_logs_from_ID(sqlite3_int64 limit)
{
        logDatabaseHandler = sqlite3_exec(dbPtr, "SELECT MAX(ID) FROM logs;", set_last_id, (void*) data, &zErrMsg);

        if (logDatabaseHandler != SQLITE_OK) SQLITE_NOT_OK(dbPtr);

        static sqlite3_int64 relativePosition;
        static sqlite3_int64 idToStartFrom = 1;

        if ((lastIdInTable - numberOfRowsPrinted) >= 1)
        {
                relativePosition = lastIdInTable - numberOfRowsPrinted;

                if (relativePosition < limit) limit = relativePosition;

                if ((relativePosition - limit) >= 1) idToStartFrom = (relativePosition - limit);
        }

        char *sqlStatement = sqlite3_mprintf("SELECT * FROM logs WHERE ID >= %lli LIMIT %lli;", idToStartFrom, limit);

        if ((logDatabaseHandler = sqlite3_exec(dbPtr, sqlStatement, callback, (void*) data, &zErrMsg)) != SQLITE_OK) SQLITE_NOT_OK(dbPtr);

        free(sqlStatement);

        return 0;
}

static int
add_to_logs(sqlite3 *dbPtr, int logDatabaseHandler, char* theDate, char* theTime, char* drug, float dose)
{
        static char *preStatement = "INSERT INTO logs(theDate, theTime, name, dose) VALUES('%q','%q','%s','%2g')";
        
        char *sqlStatement = sqlite3_mprintf(preStatement, theDate, theTime, drug, dose);

        if ((logDatabaseHandler = sqlite3_exec(dbPtr, sqlStatement, callback, (void*) data, &zErrMsg)) != SQLITE_OK) SQLITE_NOT_OK(dbPtr);

        free(sqlStatement);

        return 0;
}

/* Show today's log */
static int
show_logs(char* theTime, char* theDate)
{
        mk_larger_box(theTime, BOX_SIZE);

        int ret = print_logs_from_date(theDate);

        isFirstSqliteStatement = false;

        draw_horizontal_line(BOX_SIZE + 6);

        return ret;
}

/* Print help menu */
static void
print_help_menu()
{
        printf("============================================================\n"
               "Help menu:\n\n"
               "\tType \"exit\" or \"quit\" to exit the program\n"
               "\tType \"back\" to go back to the previous menu\n"
               "\tType \"logs <N>\" to show N log entries before today\n"
               "\tType \"help\" to show this menu\n\n"
               "============================================================\n"
              );
}

/* if user typed logs get how many to print then pint them */
static void
get_limit_then_print_logs(char *string)
{
        unsigned char a = 0
                    , b = 0
                    ;

        static char stringCopy[10];
        
        while (a++ < strlen(string))
        {
                if (string[a] >= '0' && string[a] <= '9')
                {
                        stringCopy[b] = string[a];
                        b++;
                }
        }
        
        static unsigned int idLimit; idLimit = parse_string_to_uint(stringCopy);
        
        draw_horizontal_line(BOX_SIZE);

        if (idLimit > 50)
        {
                idLimit = 50;
                DRUGIO_ERR("ERROR: limited to 50. Printing 50 last log entries.\n");
        }
        else if (idLimit < 1)
        {
                idLimit = 1;
                DRUGIO_ERR("ERROR: minimum 1 log entry. Printing last log entry.\n");
        }
        print_logs_from_ID((sqlite3_int64) idLimit);

        draw_horizontal_line(BOX_SIZE);
}

/* Make Selection (read user input) */
static int
read_user_input(char* lastObj)
{
        char c[13];

        printf("> ");
        if (!fgets(c, 13, stdin))
        {
                DRUGIO_ERR(DRUGIO_EOF);
                exit(EXIT_FAILURE);
        }
        else
        {
                /* Remove the \n from fgets */
                c[strlen(c) - 1] = 0;
                
                if (!strncmp(c, "exit", 4) || !strncmp(c, "quit", 4)) return -1;
                if (!strncmp(c, "back", 4) || !strncmp(c, "Back", 4)) return -2;
                if (!strncmp(c, "help", 4) || !strncmp(c, "Help", 4))
                {
                        print_help_menu();
                        return -2;
                }
                if (!strncmp(c, "logs", 4) || !strncmp(c, "Logs", 4))
                {
                        get_limit_then_print_logs(c);
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
drugio_menu(Drug* drugList[])
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

                d = read_user_input(&ident);
                

                if (d >= 0 && drugList[d] != NULL)
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

                                d = read_user_input(&ident);
                        }
                        break;
                }
                else break;
        } /* while(d >= 0) */

        switch (d)
        {
                case -2: goto DRUGIO_MENU;
                case -1: dip.promise = false; break;
                default:
                {
                        if (dPtr == NULL) goto DRUGIO_MENU;
                        else
                        {
                                if (!dPtr->isNanoGram) dip.drugDose = (float) dPtr->doses[d] / 1.0f;
                                else dip.drugDose = (float) dPtr->doses[d] / 1000.0f;
                        }
                        break;
                }
        }
        
        return dip;
}

/* Ask to run again */
static bool
does_user_want_to_run_again()
{
        char c[4];

        printf("Do you want to run this again? (Y/N): ");
        if (!fgets(c, 4, stdin))
        {
                DRUGIO_ERR(DRUGIO_EOF);
                exit(EXIT_FAILURE);
        }
        else if (c[0] == 'Y' || c[0] == 'y') return true;
        else return false;
}

/* Print the end result */
extern void
do_fprintd(const char* dbPath, Drug* drugList[])
{
        char *theDate = format_date_to_string(1);
        char *theTime = format_date_to_string(0);

        logDatabaseHandler = sqlite3_open(dbPath, &dbPtr);

        if (logDatabaseHandler != SQLITE_OK) SQLITE_NOT_OK(dbPtr);
        else do
        {
                
                system(CLEAR_SCREEN);
                
                show_logs(theTime, theDate);

                DrugAndDoseToPrint dip = drugio_menu(drugList);

                if (!dip.promise) break;
                else add_to_logs(dbPtr, logDatabaseHandler, theDate, theTime, dip.drugName, dip.drugDose);

        } while (does_user_want_to_run_again());

        sqlite3_close(dbPtr);
}
