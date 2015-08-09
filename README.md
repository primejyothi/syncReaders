syncReaders
========

Synchronize the SQLite databases of Calibre and Cool Reader.

The syncReaders synchronizes the Calibre ebook manager's SQLite database in from the Linux system and the Cool Reader SQLite database from Android device. The SQLite database files of both Calibre and Cool Reader needs to be present in the Linux system.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

#### Running syncReaders
syncReaders
syncReaders [-l DBG |TRACE] -c Calibre_database_file -r CoolReader_database_file -d cal2reader | reader2cal -s Custom_name_for_read_state

	-h : Display the help message
	-d : Display debug messages.
	-i : Input data file
	-o : Output XML file
	
	
syncReaders supports following command line options:

    -h, --help      : Display help message
    -l, --log       : Set message level to DBG or TRACE. By default Fatal, Error, Warning, Log & Info messages are printed
    -c, --calibredb : CalibreDBFile The Calibre SQLite Database File
    -r, --readerdb  : CoolReaderDbFile The CoolReader SQLite Database File
    -d, --direction : Direction of synchronization, cal2reader or reader2cal. The option cal2reader will synchronize the data from Calibre Db to CoolReader DB and the option reader2cal will synchronize the data from CoolReader DB to Calibre DB.
    -s              : Name of the custom status column defined in Calibre. Calibre does not have a read state column by default. In order to support read state in Calibre, a custom column is required. Using the "Add your own columns" option, create a new custom column to store the read status of a book in Calibre DB. The column type should be text and the "Lookup Name" should be passed as the customColumnName.



The database file for Calibre is usually called metadata.db and is located in the Calibre Library folder. To find the Calibre Library location, click the "Calibre Library" icon in Calibre.

The database file for CoolReader is called cr3db.sqlite and can be found in the .cr3 folder in the Android device.


Copy metadata.db and cr3db.sqlite to a local folder. Ensure that you have back up copies should something go wrong.

If you want to synchronize read state between CoolReader and Calibre, you need to set up a custom column in Calibre. Please see the notes on the -s option for more details on how to add custom columns in Calibre.

To synchronize information from Calibre to CoolReader, run syncReaders as follows:
```
syncReaders -c metadata.db -r cr3db.sqlite -d cal2Reader -s readstate

```
To synchronize information from CoolReader to Calibre, run syncReaders as follows:
```
syncReaders -c metadata.db -r cr3db.sqlite -d reader2Cal -s readstate
```

Copy the updated database files back to respective locations to see the updated information in respective software.
