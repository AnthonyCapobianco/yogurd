# 1.0.3

* Removed unnecessary type castings. 
* Fixed malloc(x + y + sizeof(char)) since sizeof(char) is always 1
* Fixed function definitions 

# 1.0.4

* Minor changes
* printd changed to void
* drugioDestructor is now called from main since the calls to newDrug are made there.
* If something goes wrong main will return EXIT_FAILIURE (-1)