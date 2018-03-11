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
#include <stdbool.h>
#include <stdlib.h> /* Need it for NULL */

#ifndef DRUGIO_H_INCLUDED
#define DRUGIO_H_INCLUDED

/* If the box above the logs (where the time appears) is scewed
 * you'll need to find the value for the box sizing that works 
 * with your font (the one of the shell you use to run this)
 */
#define BOX_SIZE 39

#ifndef DRUGIO_ARR_END
#define DRUGIO_ARR_END 0
#endif

#ifndef DRUGIO_DEBUG
#define DRUGIO_DEBUG 0
#endif

#ifndef DRUGIO_ERR
#define DRUGIO_ERR(x) fprintf(stderr, x)
#endif

#ifndef DRUGIO_EOF
#define DRUGIO_EOF "Error: Your choice isn't correct\n"
#endif

#ifndef DRUGIO_OOR
#define DRUGIO_OOR "Error: The character you entered is not part of the selection\n"
#endif

#ifndef mg
#define mg(...) (int[]) {__VA_ARGS__, 0}, false
#endif

#ifndef ng
#define ng(...) (int[]) {__VA_ARGS__, 0}, true
#endif

#ifndef drugList
#define drugList(...) Drug* drugList[] = {__VA_ARGS__, NULL}
#endif

#define DRUGIO_USE_FILE (DRUGIO_DEBUG) ? stdout : logFile

/* Type Drug of type struct */
typedef struct _Drug
{   char* name;
    int* doses;
    bool isNanoGram;
} Drug;

/* Type DrugAndDoseToPrint of type struct */
typedef struct _DrugAndDoseToPrint
{    float drugDose;
     char* drugName;
     bool promise;
} DrugAndDoseToPrint;

/* Functions prototypes */
extern Drug* newDrug(char* drugName, int* drugDoses, bool isNanoGram);
extern void _fprintd(const char* drugioLogFolderPath, Drug* drugList[]);
extern void drugioDestructor(Drug* drugList[]);

#define fprintd(P, L) _fprintd(P, L); drugioDestructor(L)

#endif
