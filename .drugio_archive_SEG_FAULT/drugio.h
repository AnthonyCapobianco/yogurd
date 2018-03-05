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
#include <stdlib.h> /* Need it for NULL */

#ifndef DRUGIO_H_INCLUDED
#define DRUGIO_H_INCLUDED

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
#define mg(...) (int[]) {__VA_ARGS__, 0}, 0
#else
    #ifndef drugio_mg
        #define drugio_mg(...) (int[]) {__VA_ARGS__, 0}, 0
    #endif
#endif

#ifndef ng
#define ng(...) (int[]) {__VA_ARGS__, 0}, 1
#else
    #ifndef drugio_ng
        #define drugio_ng(...) (int[]) {__VA_ARGS__, 0}, 1
    #endif
#endif

#ifndef drugList
#define drugList(...) drug* drugList[] = {__VA_ARGS__, NULL}
#else
    #ifndef drugio_drugList
        #define drugio_drugList(...) drug* drugList[] = {__VA_ARGS__, NULL}
    #endif
#endif

#ifndef DRUGIO_USE_FILE
#define DRUGIO_USE_FILE (DRUGIO_DEBUG) ? stdout : ftoday
#endif

/* Type drug of type struct */
typedef struct DRUG
{   char* name;
    int* doses;
    short isNanoGram;
} drug;

/* Type diPtr of type struct */
typedef struct PRT_DRUG_INT
{    int iPtr;
     drug* dPtr;
} diPtr;

/* Functions prototypes */
extern drug* newDrug( char*, int*, short);
static void drugioDestructor(drug* arrPtr[]);
int makeSelection(char*);
static diPtr drugioMenu(drug* ptr[]);
char* formatedDate(short);
short runAgain(void);
void showLogs(char**);
void printd(drug* arrPtr[]);
void drugioSetPath(char*);

/* Global constant */
const char* drugioFilePath;

#endif
