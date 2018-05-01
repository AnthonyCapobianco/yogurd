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
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <stdbool.h>
#include <stdlib.h>

#ifndef DRUGIO_H_INCLUDED
#define DRUGIO_H_INCLUDED

#define BOLD_LINE "════════════════════════════════════════════════════════"

/* Change the ORDER of those format parameters if 
 * you want the date to be displayed in a different manner.
 * Right now the order is day month year "%d-%m-%Y" 
 *
 * If you wanted MONTH DAY YEAR you'd use "%m-%d-%Y"
 */
#define DRUGIO_DATE_FORMAT "%d-%m-%Y"

#ifndef DRUGIO_DEBUG
#define DRUGIO_DEBUG 0
#endif

#if DRUGIO_DEBUG == 1
#define DRUGIO_DB_FILE_RELATIVE_PATH "yogurd/logs/debug.db"
#else
#define DRUGIO_DB_FILE_RELATIVE_PATH "yogurd/logs/logs.db"
#endif

#ifndef UINT_MIN
#define UINT_MIN 0
#endif

#ifndef UINT_MAX
#define UINT_MAX +4294967295
#endif

#define DRUGIO_ERR(x) fprintf(stderr, x)
#define DRUGIO_OOR "ERROR: The character you entered is not part of the selection\n"
#define DRUGIO_RET_NULL(x) fprintf(stderr, "ERROR: %s returned NULL in file '%s' at line: %i.", x, __FILE__, __LINE__)
#define SQLITE_NOT_OK(db_ptr) fprintf(stderr, "ERROR: SQLITE_ERR_MSG: %s\n", sqlite3_errmsg(db_ptr))

#define DB_EXEC_CALLBACK db_ptr, sql_statement, callback, (void*) data, &z_err_msg

#define mg(...) (int[]) {__VA_ARGS__, 0}, false
#define ng(...) (int[]) {__VA_ARGS__, 0}, true

#define drug_list(...)									\
											\
	Drug* drug_list[] = {__VA_ARGS__, NULL};					\
	do_fprintd(DRUGIO_FOLDER_LOCATION DRUGIO_DB_FILE_RELATIVE_PATH, drug_list);	\
	free_Drug_array(drug_list)							\

/*
 * The following `do {} while (false)` may seem odd but it generates the same code.
 * The difference is that you can use `;` at the end of the macro call. 
 */

#define DRUGIO_IDENT_SWITCH(ident)				\
	do							\
	{							\
		switch (ident)					\
		{						\
			case 'z': ident = 'A'; break;		\
			case 'Z': ident = '0'; break;		\
			default: ident++; break;		\
		}						\
	} while (false)						\

/* This is most likely right. May not be. */
#if defined(_WIN32) || defined(_WIN64)
	#define CLEAR_SCREEN() system("cls")
	#define COLOR_RESET "\x1b[0;37m"
#else
	#define CLEAR_SCREEN() printf("\x1B[2J")
	#define COLOR_RESET "\x1b[0;97m"
#endif

/* Type Drug of type struct */
typedef struct _Drug
{       char* name;
	int* doses;
	bool is_nano_gram;
} Drug;

/* Type Drug_and_dose_to_print of type struct */
typedef struct _Drug_and_dose_to_print
{       float drug_dose;
	char* drug_name;
	bool promise;
} Drug_and_dose_to_print;

/* Date and time strings */
typedef struct _Parsed_date_and_time
{       char the_date[11];
	char the_time[6];
} Parsed_date_and_time;

/* Functions prototypes */
extern Drug* new_drug(char*, int*, bool);
extern void do_fprintd(const char*, Drug* []);
extern void free_Drug_array(Drug* []);

#endif
