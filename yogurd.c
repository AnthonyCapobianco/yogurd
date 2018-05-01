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

#define DRUGIO_DEBUG 0

#define DRUGIO_FOLDER_LOCATION "/Users/tony/Desktop/"
#include "resources/drugio.h"

int 
main(void)
{        
        drug_list( new_drug("Ritalin", mg(5, 10, 15, 20))
                 , new_drug("Concerta", mg(36, 72, 108))
                 , new_drug("Effexor", mg(225))
                 , new_drug("Xanax", ng(125, 250, 500, 1000, 1500, 2000))
                 , new_drug("Akton", mg(1, 2, 3, 4))
                 , new_drug("Zolpidem", ng(2500, 5000, 10000, 15000))
                 , new_drug("Pantomed", mg(20, 40))
                 , new_drug("Dafalgan", mg(500, 1000))
                 , new_drug("DXM", mg(50, 100, 150, 200, 250))
                 , new_drug("chlorphenamine", ng(3400))
                 , new_drug("Ibuprofen", mg(200, 300, 400, 600))
                 , new_drug("Weed", mg(125, 250, 500))
                 );
        
        return EXIT_FAILURE;
}
