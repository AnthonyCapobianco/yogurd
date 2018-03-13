PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;

CREATE TABLE 
IF NOT EXISTS logs(ID INTEGER PRIMARY KEY, theDate TEXT, theTime TEXT, name TEXT, dose blob);

INSERT INTO logs(theDate, theTime, name, dose) VALUES('12-03-2018','03:04','ritalin',5.0);

COMMIT;
