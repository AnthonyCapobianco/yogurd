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
#include "drugio.h"

/* Struct constructor for type Drug pointer */
extern Drug* 
newDrug(char* dName, int* dDoses, bool isNG)
{
        Drug *p = malloc(sizeof(*p));
        
        if (p == NULL)
        {
                DRUGIO_RET_NULL("malloc");
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
static ParsedDateAndTime tStruct;

static void
refresh_time_struct(void)
{
        size_t strftime(char *, size_t, const char *, const struct tm *);

        time_t rawTimeNow; time(&rawTimeNow);
        struct tm *timeNow; timeNow = localtime(&rawTimeNow);

        strftime(tStruct.theTime, 6, "%H:%M", timeNow);
        strftime(tStruct.theDate, 11, DRUGIO_DATE_FORMAT, timeNow);
}

/* parse string to uint */
static unsigned int
parse_string_to_uint(char *buffer)
{
        static long numberToCastToUint = 0;

        char *endPtr;

        numberToCastToUint = strtol(buffer, &endPtr, 10);

        if (numberToCastToUint > UINT_MAX || numberToCastToUint < UINT_MIN)
        {
                DRUGIO_ERR("ERROR #00: number too big to be an unsigned int");
                exit(EXIT_FAILURE);
        }
        else return (unsigned int) numberToCastToUint;
}

/* Get agreement from user */
static bool
does_user_agree(void)
{
        char c[4];

        if (fgets(c, 4, stdin) == NULL)
        {
                *c = '\0';
                DRUGIO_RET_NULL("fgets");
                exit(EXIT_FAILURE);
        }
        else if (c[0] == 'Y' || c[0] == 'y') return true;
        else return false;
}


/*************************************************************************/
/*                        START OF DB INTEGRATION                        */
/*************************************************************************/


/* Global variable for sqlite */
static char *zErrMsg = 0;
static const char* data = "Callback function called";
static bool isFirstSqliteStatement = true;
static sqlite3_int64 lastIdInTable = 1;
static sqlite3_int64 numberOfRowsPrinted = 0;
static int logDatabaseHandler;
static sqlite3 *dbPtr;

/* Default callback function for sqlite3 */
static int
callback(void *NotUsed, int argc, char **argv, char **azColName)
{
        static short i = 0; i++;
        char *endPtr;
        /* Must be a boat otherwise it sinks */
        float doseBoat = strtof(argv[4], &endPtr);

        if (i & 1) printf("\x1b[40m");
        
        if (!strncmp(argv[1], tStruct.theDate, 6)) printf("[%s] %-10s %6g mg\t\t\t\t"COLOR_RESET"\n", argv[2], argv[3], doseBoat);
        else printf("[%s - %s] %s\t%6g mg\t\t"COLOR_RESET"\n", argv[1], argv[2], argv[3], doseBoat);
                
        if (isFirstSqliteStatement) numberOfRowsPrinted++;
        
        return 0;
}

/* Find last id in table and set variable for it */
static int
set_last_id(void *NotUsed, int argc, char **argv, char **azColName)
{
        lastIdInTable = parse_string_to_uint(argv[0]);
        return 0;
}

/* Print logs from a date */
/* At the moment the date is always today. 
 * That will change in the future 
 *
 * TO DO: print doses by user-defined date 
 */
static int
print_logs_from_date(char *date)
{
        char *sqlStatement = sqlite3_mprintf("SELECT * FROM logs WHERE theDate is '%q'", date);

        if ((logDatabaseHandler = sqlite3_exec(DB_EXEC_CALLBACK)) != SQLITE_OK) SQLITE_NOT_OK(dbPtr);

        free(sqlStatement);

        return 0;
}

/* Print logs from an id */
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

        if ((logDatabaseHandler = sqlite3_exec(DB_EXEC_CALLBACK)) != SQLITE_OK) SQLITE_NOT_OK(dbPtr);

        free(sqlStatement);

        return 0;
}

/* Insert the drug, dose, date & time in the db */
static int
add_to_logs(sqlite3 *dbPtr, int logDatabaseHandler, char* theDate, char* theTime, char* drug, float dose)
{
        static char *preStatement = "INSERT INTO logs(theDate, theTime, name, dose) VALUES('%q','%q','%s','%2g')";
        
        char *sqlStatement = sqlite3_mprintf(preStatement, theDate, theTime, drug, dose);

        if ((logDatabaseHandler = sqlite3_exec(DB_EXEC_CALLBACK)) != SQLITE_OK) SQLITE_NOT_OK(dbPtr);

        free(sqlStatement);

        return 0;
}

/* if user typed logs get how many to print then print them */
static void
get_limit_then_print_logs(char *string)
{
        static unsigned char a = 0;
        static unsigned char b = 0;
        
        static char stringCopy[10];
       
        static size_t stringLength; stringLength = strlen(string);
 
        while (a++ < stringLength)
        {
                if (string[a] >= '0' && string[a] <= '9')
                {
                        stringCopy[b] = string[a];
                        b++;
                }
        }
        
        static unsigned int idLimit; idLimit = parse_string_to_uint(stringCopy);
        
        puts(BOLD_LINE);

        /* Silent error handling. We just want to keep this reasonable */
        if (idLimit > 50) idLimit = 50;
        else if (idLimit < 1) idLimit = 1;
        
        print_logs_from_ID((sqlite3_int64) idLimit);

        puts(BOLD_LINE);
}

/* Remove last log entry*/
static int
rm_last_entry_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
        printf("Are you sure you want to remove the last entry of %s at %s (Y/N): ", argv[3], argv[2]);
        return 0;
}

static void
rm_last_entry_from_database(void)
{
        #define SELECT_LAST_ENTRY dbPtr, "SELECT * FROM logs WHERE ID = (SELECT MAX(ID) FROM logs);"
        #define DELETE_LAST_ENTRY dbPtr, "DELETE FROM logs WHERE ID = (SELECT MAX(ID) FROM logs);"
        #define DB_OBJECTS (void*) data, &zErrMsg
        
        /* This calls the function just above this one aka `rm_last_entry_callback` */
        logDatabaseHandler = sqlite3_exec(SELECT_LAST_ENTRY, rm_last_entry_callback, DB_OBJECTS);
        if (logDatabaseHandler != SQLITE_OK) SQLITE_NOT_OK(dbPtr);
        
        if (does_user_agree())
        {
                /* This deletes the last entry */
                logDatabaseHandler = sqlite3_exec(DELETE_LAST_ENTRY, callback, DB_OBJECTS);
                
                /* If sql error */
                if (logDatabaseHandler != SQLITE_OK) SQLITE_NOT_OK(dbPtr);
                else puts("Succesfully removed last entry from logs");

        }
        else return;
}


/*************************************************************************/
/*                          END OF DB INTEGRATION                        */
/*************************************************************************/

/* Show today's log */
static void
show_logs(void)
{
        printf("\n\n %s\n", tStruct.theTime);
        puts(BOLD_LINE);

        print_logs_from_date(tStruct.theDate);

        isFirstSqliteStatement = false;

        puts(BOLD_LINE);
}

/* Clear screen and print everything again. */
static void
refresh_screen(void)
{
        CLEAR_SCREEN();
        refresh_time_struct();
        printf(COLOR_RESET);
        show_logs();
}

/* Print help menu */
static void
print_help_menu(void)
{
        puts("\n\n"BOLD_LINE);
        
        printf( "Help menu:\n\n"
                "\tType \"exit\" or \"quit\" to exit the program\n"
                "\tType \"back\" to go back to the previous menu\n"
                "\tType \"logs <N>\" to show N log entries before today\n"
                "\tType \"rmlast\" to remove the last log entry\n"
                "\tType \"clear\" to clear the screen\n"
                "\tType \"help\" to show this menu\n\n"
              );
        
        puts(BOLD_LINE);
}

/* Make Selection (read user input) */
static int
read_user_input(char* lastObj)
{
        char c[13];
        char *ptr;

        printf("\n> ");
        if (fgets(c, 13, stdin) == NULL)
        {
                *c = '\0';
                DRUGIO_RET_NULL("fgets");
                exit(EXIT_FAILURE);
        }
        else
        {
                /* Remove the \n from fgets */
                if ((ptr = strchr(c, '\n')) != NULL) *ptr = '\0';
                
                if (c[1])
                {
                        if (c[0] >= 'e')
                        {
                                if (c[0] == 'e' && !strncmp(c, "exit", 4)) return -1;
                                if (c[0] == 'q' && !strncmp(c, "quit", 4)) return -1;
                                
                                if (!strncmp(c, "help", 4))
                                {
                                        print_help_menu();
                                        return -2;
                                }
                                if (!strncmp(c, "logs", 4))
                                {
                                        get_limit_then_print_logs(c);
                                        return -2;
                                }
                                if (!strncmp(c, "rmlast", 6))
                                {
                                        rm_last_entry_from_database();
                                        return -2;
                                }
                        }
                        else if (c[0] < 'e')
                        {
                        
                                if (!strncmp(c, "back", 4) || !strncmp(c, "cls", 3) || !strncmp(c, "clear", 5))
                                {
                                        refresh_screen();
                                        return -2;
                                }
                        }
                }
                else if (c[0] >= 'a' && c[0] <= (*lastObj - 1)) return((int) (c[0] - 'a'));
                else DRUGIO_ERR(DRUGIO_OOR); /* Error: out of range */
                
                return -2;
        }
}

/* Print drugs */
static DrugAndDoseToPrint 
drugio_menu(Drug* drugList[])
{
        Drug *dPtr = NULL;
        DrugAndDoseToPrint dip; dip.promise = true;
          
        char ident = 'a';
        
        for (int i = 0, d = 0; ; d = 0)
        {
                while (true)
                {
                        printf("Please type the letter conresponding to the Drug taken\n"
                               "then press the enter key. Type \"help\" for help.\n\n"
                              );

                        /* Print Drug names */
                        for (i = 0, ident = 'a'; drugList[i] != NULL; ++i)
                        {
                                printf("[%c] %s\n", ident, drugList[i]->name);
                                
                                DRUGIO_IDENT_SWITCH(ident)
                        }

                        d = read_user_input(&ident);

                        if (d < 0 || drugList[d] == NULL) break;
                                        
                        dPtr = drugList[d];
                        dip.drugName = dPtr->name;

                        /* Early out if only one dose for selected Drug */
                        if (!dPtr->doses[1]) d = 0;
                        else
                        {
                                printf("\nDoses for %s:\n\n", dPtr->name);
                
                                /* Print the Drug doses */
                                for (ident = 'a', i = 0; dPtr->doses[i] != 0 ; ++i)
                                {
                                        printf("[%c] ", ident);
                                        
                                        if (!dPtr->isNanoGram) printf("%d mg\n", dPtr->doses[i]);
                                        else printf("%-2g mg\n", (float) (dPtr->doses[i] / 1000.0f));
                                                
                                        DRUGIO_IDENT_SWITCH(ident)
                                }
                                
                                d = read_user_input(&ident);
                        }
                        break;
                } /* while (d >= 0) */

                switch (d)
                {
                        case -2: continue;
                        case -1: dip.promise = false; return dip;
                        default:
                        {
                                if (dPtr == NULL) continue;
                                else if (!dPtr->isNanoGram) dip.drugDose = (float) dPtr->doses[d] / 1.0f;
                                else dip.drugDose = (float) dPtr->doses[d] / 1000.0f;
                                return dip;
                        }
                }
        } /* for (int i = 0, d = 0; ; d = 0) */
}

/* Ask to run again */
static bool
does_user_want_to_run_again(void)
{
        printf("Do you want to run this again? (Y/N): ");
        return does_user_agree();
}

/* Print the end result */
extern void
do_fprintd(const char* dbPath, Drug* drugList[])
{
        logDatabaseHandler = sqlite3_open(dbPath, &dbPtr);

        if (logDatabaseHandler != SQLITE_OK) SQLITE_NOT_OK(dbPtr);
        else do
        {
                refresh_screen();

                DrugAndDoseToPrint dip = drugio_menu(drugList);

                if (!dip.promise) break;
                else add_to_logs(dbPtr, logDatabaseHandler, tStruct.theDate, tStruct.theTime, dip.drugName, dip.drugDose);

        } while (does_user_want_to_run_again());

        sqlite3_close(dbPtr);
}
