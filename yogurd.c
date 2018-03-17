/*
 *  This program is designed to provide an easy way for
 *  users to log doses of the medication(s) they use.
 *  Copyright (C) 2018 Anthony Capobianco
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, eithyer version 3 of the License, or
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
#define DRUGIO_FOLDER_LOCATION "/Users/Muddy/Desktop/"
#include "resources/drugio.h"

int 
main(void)
{        
        drugList( newDrug("Ritalin", mg(5, 10, 15, 20))
                , newDrug("Concerta", mg(36, 72, 108))
                , newDrug("Effexor", mg(225))
                , newDrug("Xanax", ng(125, 250, 500, 1000, 1500, 2000))
                , newDrug("Akton", mg(1, 2, 3, 4))
                , newDrug("Zolpidem", mg(5, 10, 15))
                , newDrug("Pantomed", mg(20, 40))
                , newDrug("Dafalgan", mg(500, 1000))
                , newDrug("DXM", mg(50, 100, 150, 200, 250))
                , newDrug("chlorphenamine", ng(3400))
                , newDrug("Weed", mg(125, 250, 500))
                );
        
        return EXIT_FAILURE;
}
