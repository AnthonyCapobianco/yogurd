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