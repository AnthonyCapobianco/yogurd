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
new_drug(char* d_name, int* d_doses, bool is_nG)
{
        Drug *p = malloc(sizeof(*p));
        
        if (p == NULL)
        {
                DRUGIO_RET_NULL("malloc");
                exit(EXIT_FAILURE);
        }
        
        p->name = d_name; 
        p->doses = d_doses; 
        p->is_nano_gram = is_nG;
        
        return p;
}

/* Struct destructor for drugs */
extern void
free_Drug_array(Drug* drug_list[])
{
        for (int i = 0; drug_list[i] != NULL; i++) free(*(drug_list + i));
        exit(EXIT_SUCCESS);
        
}

/* Date parsing and formatting */
static Parsed_date_and_time t_struct;

static void
refresh_time_struct(void)
{
        size_t strftime(char *, size_t, const char *, const struct tm *);

        time_t raw_time_now; time(&raw_time_now);
        struct tm *time_now; time_now = localtime(&raw_time_now);

        strftime(t_struct.the_time, 6, "%H:%M", time_now);
        strftime(t_struct.the_date, 11, DRUGIO_DATE_FORMAT, time_now);
}

/* parse string to uint */
static unsigned int
parse_string_to_uint(char *buffer)
{
        static long number_to_cast_to_uint = 0;

        char *end_ptr;

        number_to_cast_to_uint = strtol(buffer, &end_ptr, 10);

        if (number_to_cast_to_uint > UINT_MAX || number_to_cast_to_uint < UINT_MIN)
        {
                DRUGIO_ERR("ERROR #00: number too big to be an unsigned int");
                exit(EXIT_FAILURE);
        }
        else return (unsigned int) number_to_cast_to_uint;
}

/* Get agreement from user */
static bool
does_user_agree(void)
{
        char c[4];

        if (fgets(c, 4, stdin) == NULL)
        {
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
static char *z_err_msg = 0;
static const char* data = "Callback function called";
static bool is_first_sqlite_statement = true;
static sqlite3_int64 last_id_in_table = 1;
static sqlite3_int64 number_of_rows_printed = 0;
static int log_database_handler;
static sqlite3 *db_ptr;

/* Default callback function for sqlite3 */
static int
callback(void *Not_used, int argc, char **argv, char **az_col_name)
{
        static short i = 0; i++;
        char *end_ptr;
        /* Must be a boat otherwise it sinks */
        float dose_boat = strtof(argv[4], &end_ptr);

        if (i & 1) printf("\x1b[40m");
        
        if (!strncmp(argv[1], t_struct.the_date, 6)) printf("[%s] %-10s %6g mg\t\t\t\t"COLOR_RESET"\n", argv[2], argv[3], dose_boat);
        else printf("[%s - %s] %s\t%6g mg\t\t"COLOR_RESET"\n", argv[1], argv[2], argv[3], dose_boat);
                
        if (is_first_sqlite_statement) number_of_rows_printed++;
        
        return 0;
}

/* Find last id in table and set variable for it */
static int
set_last_id(void *Not_used, int argc, char **argv, char **az_col_name)
{
        last_id_in_table = parse_string_to_uint(argv[0]);
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
        char *sql_statement = sqlite3_mprintf("SELECT * FROM logs WHERE theDate is '%q'", date);

        if ((log_database_handler = sqlite3_exec(DB_EXEC_CALLBACK)) != SQLITE_OK) SQLITE_NOT_OK(db_ptr);

        free(sql_statement);

        return 0;
}

/* Print logs from an id */
static int
print_logs_from_ID(sqlite3_int64 limit)
{
        log_database_handler = sqlite3_exec(db_ptr, "SELECT MAX(ID) FROM logs;", set_last_id, (void*) data, &z_err_msg);

        if (log_database_handler != SQLITE_OK) SQLITE_NOT_OK(db_ptr);

        static sqlite3_int64 relative_position = 0;
        static sqlite3_int64 id_to_start_from = 1;

        if ((last_id_in_table - number_of_rows_printed) >= 1)
        {
                relative_position = last_id_in_table - number_of_rows_printed;

                if (relative_position < limit) limit = relative_position;

                if ((relative_position - limit) >= 1) id_to_start_from = (relative_position - limit);
        }

        char *sql_statement = sqlite3_mprintf("SELECT * FROM logs WHERE ID >= %lli LIMIT %lli;", id_to_start_from, limit);

        if ((log_database_handler = sqlite3_exec(DB_EXEC_CALLBACK)) != SQLITE_OK) SQLITE_NOT_OK(db_ptr);

        free(sql_statement);

        return 0;
}

/* Insert the drug, dose, date & time in the db */
static int
add_to_logs(sqlite3 *db_ptr, int log_database_handler, char* the_date, char* the_time, char* drug, float dose)
{
        static char *pre_statement = "INSERT INTO logs(theDate, theTime, name, dose) VALUES('%q','%q','%s','%2g')";
        
        char *sql_statement = sqlite3_mprintf(pre_statement, the_date, the_time, drug, dose);

        if ((log_database_handler = sqlite3_exec(DB_EXEC_CALLBACK)) != SQLITE_OK) SQLITE_NOT_OK(db_ptr);

        free(sql_statement);

        return 0;
}

/* if user typed logs get how many to print then print them */
static void
get_limit_then_print_logs(char *string)
{
        int a = 0;
        int b = 0;
        
        char string_copy[10] = "";
       
        size_t string_length = strlen(string);
 
        while (a++ < string_length)
        {
                if (string[a] >= '0' && string[a] <= '9')
                {
                        string_copy[b] = string[a];
                        b++;
                }
        }
        
        unsigned int id_limit = parse_string_to_uint(string_copy);
        
        puts(BOLD_LINE);

        /* Silent error handling. We just want to keep this reasonable */
        if (id_limit > 50) id_limit = 50;
        else if (id_limit < 1) id_limit = 1;
        
        print_logs_from_ID((sqlite3_int64) id_limit);

        puts(BOLD_LINE);
}

/* Remove last log entry*/
static int
rm_last_entry_callback(void *Not_used, int argc, char **argv, char **az_col_name)
{
        printf("Are you sure you want to remove the last entry of %s at %s (Y/N): ", argv[3], argv[2]);
        return 0;
}

static void
rm_last_entry_from_database(void)
{
        #define SELECT_LAST_ENTRY db_ptr, "SELECT * FROM logs WHERE ID = (SELECT MAX(ID) FROM logs);"
        #define DELETE_LAST_ENTRY db_ptr, "DELETE FROM logs WHERE ID = (SELECT MAX(ID) FROM logs);"
        #define DB_OBJECTS (void*) data, &z_err_msg
        
        /* This calls the function just above this one aka `rm_last_entry_callback` */
        log_database_handler = sqlite3_exec(SELECT_LAST_ENTRY, rm_last_entry_callback, DB_OBJECTS);
        if (log_database_handler != SQLITE_OK) SQLITE_NOT_OK(db_ptr);
        
        if (does_user_agree())
        {
                /* This deletes the last entry */
                log_database_handler = sqlite3_exec(DELETE_LAST_ENTRY, callback, DB_OBJECTS);
                
                /* If sql error */
                if (log_database_handler != SQLITE_OK) SQLITE_NOT_OK(db_ptr);
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
        printf("\n\n %s\n", t_struct.the_time);
        puts(BOLD_LINE);

        print_logs_from_date(t_struct.the_date);

        if (is_first_sqlite_statement) is_first_sqlite_statement = false;

        puts(BOLD_LINE);
}

static inline void
reset_number_of_rows_printed(void)
{
        is_first_sqlite_statement = true;
        number_of_rows_printed = 0;
}

/* Clear screen and print everything again. */
static void
refresh_screen(void)
{
        CLEAR_SCREEN();
        refresh_time_struct();
        reset_number_of_rows_printed();
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
read_user_input(char* last_obj)
{
        static char c[13];
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
                else if (c[0] >= 'a' && c[0] <= (*last_obj - 1)) return((int) (c[0] - 'a'));
                else DRUGIO_ERR(DRUGIO_OOR); /* Error: out of range */
                
                return -2;
        }
}

/* Print drugs */
static Drug_and_dose_to_print 
drugio_menu(Drug* drug_list[])
{
        Drug *d_ptr = NULL;
        Drug_and_dose_to_print dip; dip.promise = true;
          
        char ident = 'a';
        
        for (int i = 0, d = 0;;)
        {
                while (true)
                {
                        printf("Please type the letter conresponding to the Drug taken\n"
                               "then press the enter key. Type \"help\" for help.\n\n"
                              );

                        /* Print Drug names */
                        for (i = 0, ident = 'a'; drug_list[i] != NULL; ++i)
                        {
                                printf("[%c] %s\n", ident, drug_list[i]->name);
                                
                                DRUGIO_IDENT_SWITCH(ident);
                        }

                        d = read_user_input(&ident);

                        if (d < 0 || drug_list[d] == NULL) break;
                                        
                        d_ptr = drug_list[d];
                        dip.drug_name = d_ptr->name;

                        /* Early out if only one dose for selected Drug */
                        if (!d_ptr->doses[1]) d = 0;
                        else
                        {
                                printf("\nDoses for %s:\n\n", d_ptr->name);
                
                                /* Print the Drug doses */
                                for (ident = 'a', i = 0; d_ptr->doses[i] != 0 ; ++i)
                                {
                                        printf("[%c] ", ident);
                                        
                                        if (!d_ptr->is_nano_gram) printf("%d mg\n", d_ptr->doses[i]);
                                        else printf("%-2g mg\n", (float) (d_ptr->doses[i] / 1000.0f));
                                                
                                        DRUGIO_IDENT_SWITCH(ident);
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
                                if (d_ptr == NULL) continue;
                                else if (!d_ptr->is_nano_gram) dip.drug_dose = (float) d_ptr->doses[d] / 1.0f;
                                else dip.drug_dose = (float) d_ptr->doses[d] / 1000.0f;
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
do_fprintd(const char* db_path, Drug* drug_list[])
{
        log_database_handler = sqlite3_open(db_path, &db_ptr);

        if (log_database_handler != SQLITE_OK) SQLITE_NOT_OK(db_ptr);
        else do
        {
                refresh_screen();

                Drug_and_dose_to_print dip = drugio_menu(drug_list);

                if (!dip.promise) break;
                else add_to_logs(db_ptr, log_database_handler, t_struct.the_date, t_struct.the_time, dip.drug_name, dip.drug_dose);

        } while (does_user_want_to_run_again());

        sqlite3_close(db_ptr);
}
