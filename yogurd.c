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
#include "drugio.h"

int main(void)
{    
    /* Doses are in microgram */
    drugList( newDrug("Ritalin", doses(5000, 10000, 15000, 20000))
            , newDrug("Concerta", doses(36000, 72000))
            , newDrug("Effexor", doses(225000))
            , newDrug("Xanax", doses(125, 250, 500, 1000, 1500, 2000))
            , newDrug("Akton", doses(1000, 2000, 3000, 4000))
            , newDrug("Zolpidem", doses(5000, 10000, 15000))
            );
    
    printd(drugList, "./logs/");
    
    drugioDestructor(drugList);
    
    return(EXIT_FAILURE);
}
