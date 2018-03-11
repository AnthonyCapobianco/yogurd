# 1.0.3

* Removed unnecessary type castings. 
* Fixed malloc(x + y + sizeof(char)) since sizeof(char) is always 1
* Fixed function definitions 

# 1.0.4

* Minor changes
* printd changed to void
* drugioDestructor is now called from main since the calls to newDrug are made there.
* If something goes wrong main will return EXIT_FAILIURE (-1)

# 1.0.5

* Minor improvements (more descriptive names)
* printd now directly takes the path argument instead of having a global variable 
* drugioMenu doesn't create a drug pointer anymore. 

# 1.0.6

* Fixed log not updating when user choses to run again.

# 1.0.7

* Fixed seg fault if file doesn't exist while showLogs() is called.

# 1.0.8

* Improved functions to make them easier to understand.
* Now using bools instead of shorts
* The type (struct) DrugAndDoseToPrint now only contains the name of
  the medication and the dose to be printed (as well as the promise boolean)
* Reduced the use of pointers where not necessary.
* showHelp is now void. It used to only return -2 just to save a line in the menu function. That was improper and bad practice.
* There is no need to add the destructor in the main file. It's done through preprocessing (atm.)
* The size of the "box" surrounding the time (at the top of the program) is now defined in the drugio.c header instead of being static in the showLogs() function. The default is 39 but it varies if you don't use a monospace font in your shell.
* For the medications where the doses are in micrograms,
  the doses are now displayed alligned to the left instead of being all of being all over the place. 
