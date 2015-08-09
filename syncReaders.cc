/*!
 * \mainpage syncReaders
 * \brief Synchronize the SQLite databases of Calibre and Cool Reader
 * \author Prime Jyothi (primejyothi [at] gmail [dot] com)
 * \date 2014-02-06
 * \copyright Copyright Prime Jyothi
 *
 * The syncReaders synchronizes the Calibre ebook manager's SQLite database
 * in from the Linux system and the Cool Reader SQLite database from Android
 * device. The SQLite database files of both Calibre and Cool Reader needs
 * to be present in the Linux system.
 */

using namespace std;

#include <string>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <cstdlib>
#include "jlog.hpp"
#include "syncClass.hpp"
#include "syncDbClass.hpp"

//! \file syncReaders.cc Synchronize Calibre and Cool Reader database files.
//! \brief Synchronize Calibre and Cool Reader SQLite database files.

//! syncReaders synchronizes the SQLite Database files of the Calibre e-book
//! manager and Cool Reader e-book reader. The rating and read state are
//! synchronized.
//!
//! The SQLite data files of Cool Reader need to be transferred the Linux
//! system manually. Once the synchronization is complete the modified SQLite
//! database file of Cool Reader has to be transferred to the Android device
//! manually.


// Calibre related entities referred as Calibre xyz or prefixed with 'C'.
// Cool Reader related entities referred as Reader xyz or prefixed with 'R'.

// Function prototypes.
int processArgs (int argc, char **argv, char *CDbFile, char *RDbFile,
	string& direction, string& stateVal,string& lvl);
void help (char *progName);
int setupDbOps (CalibreDb& cDB, ReaderDb& rDB, char *CDbFile,
	char *RDbFile, string stateVal, int *tabId);
int clearDbOps (CalibreDb& cDb, ReaderDb& rDb);
int updateData (SyncClass *Source, SyncClass *Dest, SyncDb *DestDb, SyncClass *newData);

//! \fn int main (int argc, char **argv)
//! \brief Starting point for syncReaders.
//!
//! syncReaders supports following command line options:
//! \arg \c [ \c -h, \c \--help \c] Display help message
//! \arg \c [ \c -l, \c \--log \c DBG \c | \c TRACE] Set message level to DBG
//! or TRACE. By default Fatal, Error, Warning, Log & Info messages are printed
//! \arg \c -c, \c \--calibredb \c CalibreDBFile The Calibre SQLite Database
//! File
//! \arg \c -r, \c \--readerdb \c CoolReaderDbFile The CoolReader SQLite
//! Database File
//! \arg \c -d, \c \--direction  \c cal2reader | \c reader2cal Direction of
//! synchronization. The option cal2reader will synchronize the data from
//! Calibre Db to CoolReader DB and the option reader2cal will synchronize
//! the data from CoolReader DB to Calibre DB.
//! \arg \c -s \c customColumnName Name of the custom status column defined in
//! Calibre. Calibre does not have a read state column by default. In order
//! to support read state in Calibre, a custom column is required. Using the 
//! "Add your own columns" option, create a new custom column to store the 
//! read status of a book in Calibre DB. The column type should be text and
//! the "Lookup Name" should be passed as the customColumnName.
//! For more details on how custom columns are processed, please refer
//! CalibreDb::getCustomTabId
//!
//! \see LVLS
//! \see CalibreDb::getCustomTabId
//
int main (int argc, char **argv)
{
	char CDbFile[PATH_MAX];
	char RDbFile[PATH_MAX];
	string direction;
	string stateVal;
	string lvl;

	int retVal;

	SETFWDT (15);
	SETLWDT (5);
	// SETMSGLVL (TRACE);
	SETMSGLVL (LOG);

	stateVal.clear ();

	retVal = processArgs (argc, argv, CDbFile, RDbFile, direction,
		stateVal, lvl);
	if (retVal != SUCCESS)
	{
		return FAIL;
	}
	if (lvl == "DBG")
	{
		SETMSGLVL (DBG);
	}
	else if (lvl == "TRACE")
	{
		SETMSGLVL (TRACE);
	}
	jTRACE ("processArgs retVal = " << retVal);

	jDBG ("CDbFile [" << CDbFile << "] RDbFile [" << RDbFile);
	

	// Db Classes.
	CalibreDb cDb;
	ReaderDb rDb;

	// Title info.
	Calibre cData;
	Calibre newCalData;
	Reader rData;
	Reader newRdrData;

	SyncClass *Source = 0; // Source
	SyncClass *Dest = 0; // Destination
	SyncClass *NewData = 0; // Class with updated rating/state.

	SyncDb *sourceDB = 0; // Source db
	SyncDb *destDB = 0; // Destination db

	// Set the customStatePresent flag.
	if (stateVal.length () != 0)
	{
		cDb.setCustomStatePresent (true);
		rDb.setCustomStatePresent (true);

		cData.setCustomStatePresent (true);
		rData.setCustomStatePresent (true);
	}

	int tabId; // Custom table id for state.

	int retval = setupDbOps (cDb, rDb, CDbFile, RDbFile, stateVal, &tabId);
	if (retval != SUCCESS)
	{
		jERR ("setupDbOps failed");
		return FAIL;
	}

	//! Get the Rating ids and the ratings.
	map <int, int> r;
	retval = cDb.loadRatingIds (r);
	if (retval != SUCCESS)
	{
		jERR ("loadRatingIds failed");
		clearDbOps (cDb, rDb);
		return FAIL;
	}

	retval = cData.createRatingLookup (r);
	if (retval != SUCCESS)
	{
		jERR ("Calibre createRefLookup failed");
		return FAIL;
	}
	else
	{
		jDBG ("Found " << r.size () << " ratings.");
		for (map<int, int>::iterator i = cData.ratingIdMap.begin ();
			i != cData.ratingIdMap.end (); ++i)
		{
			jDBG ("Rating id [" << (*i).first << "] rating [" << (*i).second);
		}
	}

	if (cDb.customStatePresent == true)
	{
		map<int, string> t;
		// Get the custom Read State values from the Calibre db.
		retval = cDb.loadReadState (tabId, t);
		if (retval != SUCCESS)
		{
			jERR ("Calibre loadReadState failed.");
			return FAIL;
		}
		else
		{

			jDBG ("Found " << t.size () << " states.");
			for (map <int, string>::iterator i = t.begin ();
				i != t.end (); ++i)
			{
				jDBG ("State id [" << (*i).first << "] name [" << (*i).second);
			}

			jDBG ("Creating ref look up");
			retval = cData.createRefLookup (t);
			if (retval != SUCCESS)
			{
				jERR ("Calibre createRefLookup failed.");
				return FAIL;
			}

		}
	}
	if (direction == "cal2reader")
	{
		// Sync data from Calibre to Reader, set up source and
		// destination variables.
		Source = &cData;
		Dest = &rData;

		// Destination is reader, new class should be of type Reader.
		NewData = &newRdrData;

		sourceDB = &cDb;
		destDB = &rDb;
		jLOG ("Syncing data from Calibre DB to CoolReader DB.");
	}
	else if (direction == "reader2cal")
	{
		// Sync data from Reader to Calibre, set up source and
		// destination variables.
		Source = &rData;
		Dest = &cData;

		// Destination is Calibre, new class should be of type Calibre.
		NewData = &newCalData;

		sourceDB = &rDb;
		destDB = &cDb;
		jLOG ("Syncing data from CoolReader DB to Calibre DB.");
	}

	while (1)
	{
		//! Fetch the data from the source db.
		retval = sourceDB->fetchRecords (Source);
		if (retval != SUCCESS)
		{
			break;
		}

		//! Fetch data from the dest Db for the source record.
		retVal = destDB->getBookInfo (Dest, Source->getTitle());
		if (retVal != SUCCESS)
		{
			// The book may not be present in the destination DB, skip it.
			continue;
		}

		if ( (Dest->getStdState () < Source->getStdState ()) ||
			 (Dest->getStdRating () < Source->getStdRating ())
		)
		{
			jINFO ("");
			Source->displayData ();
			Dest->displayData ();

			retVal = updateData (Source, Dest, destDB, NewData);
		}
	}
	jLOG ("Finished syncing.");

	// Clear the DB connections and statements.
	clearDbOps (cDb, rDb);
	jTRACE ("Exiting========================================");
	return (0);
}

//! \fn int processArgs (int argc, char **argv, char *CDbFile,
//!	char *RDbFile, string& direction, string& stateVal,string& lvl)
//! \brief Process and validate the input arguments and parameters.
//! Process and validate the input arguments and parameters. The program
//! expects three mandatory parameters : -c, -r and -d.
//! \param [in] argc argc from main().
//! \param [in] argv argv from main().
//! \param [out] CDbFile Name of the Calibre database file.
//! \param [out] RDbFile Name of the Cool Reader database file.
//! \param [out] direction Sync direction (cal2reader, reader2cal).
//! \param [out] stateVal State field in Calibre Db.
//! \param [out] lvl The log level (DBG, TRACE).
int processArgs (int argc, char **argv, char *CDbFile, char *RDbFile,
	string& direction, string& stateVal, string& lvl)
{
	static struct option glyphOptions[] = 
	{
		{"calibredb",		required_argument,	0, 'c'},
		{"readerdb",		required_argument,	0, 'r'},
		{"direction",		required_argument,	0, 'd'},
		{"state",			required_argument,	0, 's'},
		{"log",				required_argument,	0, 'l'},
		{"help",			no_argument, 		0, 'h'},
		{0,					0,					0, 0}
	};

	int helpFlag = 0;
	int optIdx = 0;
	jFNTRY ();

	while (1)
	{
		int c = 0;
		c = getopt_long (argc, argv, "c:r:d:s:l:h", glyphOptions, &optIdx);
		jDBG ("optIdx " << optIdx);
		if ( -1 == c )
		{
			break;
		}

		switch (c)
		{
			case 'h' :
				helpFlag = 1;
				jDBG ("Help option found");
				break;
			case 'c' :
				jDBG ("i: name = " << glyphOptions[optIdx].name
						<<", optarg = "<< optarg);
				strcpy (CDbFile, optarg);
				// jDBG ("inFile " << CDbFile);
				break;
			case 'r' :
				jDBG ("r: name = " << glyphOptions[optIdx].name
						<<", optarg = "<< optarg);
				strcpy (RDbFile, optarg);
				break;
			case 'd' :
				jDBG ("d: name = " << glyphOptions[optIdx].name
						<<", optarg = "<< optarg);
				direction = optarg;
				break;
			case 's' :
				jDBG ("s: name = " << glyphOptions[optIdx].name
						<<", optarg = "<< optarg);
				stateVal = optarg;
				break;
			case 'l' :
				jDBG ("l: name = " << glyphOptions[optIdx].name
						<<", optarg = "<< optarg);
				lvl = optarg;
				// If trace/debug statements from this functions are required.
				if (lvl == "TRACE")
				{
					SETMSGLVL (TRACE);
				}
				jDBG ("Log level " << lvl);
				break;
			case '?' :
				jDBG ("Try " << argv[0] << " --help for more information");
				exit (2);
				break;
		}
		if (helpFlag)
		{
			help (argv[0]);
			exit (1);
		}

	}

	if (strlen (CDbFile) == 0)
	{
		jERR ("Calibre DB file not specified, try " << argv[0] << " -h");
		exit (1);
	}

	if (strlen (RDbFile) == 0)
	{
		jERR ("Cool Reader DB file not specified, try " << argv[0] << " -h");
		exit (1);
	}

	if (direction.length() != 0)
	{
		if ((direction != "cal2reader") && (direction != "reader2cal"))
		{
			jERR ("Invaid direction");
			exit (1);
		}
	}
	else
	{
		jERR ("Sync direction not specified, try " << argv[0] << " -h");
		exit (1);
	}
	jFX ();
	return SUCCESS;
}

//! \fn void help (char *progName)
//! \brief Display the help text.
void help (char *progName)
{
	cout << "Usage : " << progName <<
		" -c CalibreDbFile -r CoolReaderDbFile -d direction" << endl;
	cout << "\t -c, --calibredb  CalibreDbFile" << endl;
	cout << "\t -r, --readerdb   CoolReaderDbFile" << endl;
	cout << "\t -d, --direction  Sync direction (cal2reader | reader2cal)" << endl;
	cout << "\t -s, --state      customColumnName" << endl;
	cout << "\t [-l, --log]      MessageLevel (DBG | TRACE)" << endl;
	cout << "\t [-h, --help]     Display this help message" << endl;
}

//! int setupDbOps (CalibreDb& cDb, ReaderDb& rDb, char *CDbFile,
//! char *RDbFile, string stateVal, int *tabId)
//! \brief Open DB, setup statements
//! Open the SQLite Database files of Calibre and Reader and invokes the
//! setupDbStmts to prepare various SQLite statements.
//! \param [in] cDb Calibre db access class.
//! \param [in] rDb Reader db access class.
//! \param [in] CDbFile Calibre SQLite Database file name
//! \param [in] RDbFile Reader SQLite Database file name
//! \param [in] stateVal Name of the custom state field
//! \param [in] tabId The id of the custom table.
int setupDbOps (CalibreDb& cDb, ReaderDb& rDb, char *CDbFile,
	char *RDbFile, string stateVal, int *tabId)
{
	int retval;

	jFNTRY ();

	// Connect to the Calibre DB
	retval = cDb.connectToDB (CDbFile);
	if (retval != SUCCESS)
	{
		jFATAL ("Unable to open Calibre DB [" << CDbFile << "]");
		return FAIL;
	}
	else
	{
		jTRACE ("Connected to Calibre db");
	}

	if (cDb.customStatePresent == true)
	{
		// Need to proceed only if custom state is specified from command line.
		retval = cDb.getCustomTabId (stateVal, tabId);
		if (retval != SUCCESS)
		{
			jFATAL ("Calibre getCustomTabId failed");
			return FAIL;
		}
		retval = cDb.setupStateOps (*tabId);
		if (retval != SUCCESS)
		{
			jFATAL ("Calibre setupStateOps failed");
			jFX ();
			return FAIL;
		}
	}

	// Prepare the statements for Calibre db
	retval = cDb.setupDbStmts ();
	if (retval != SUCCESS)
	{
		jFATAL ("Calibre setupDbStmts failed");
		return FAIL;
	}


	// Connect to the Reader DB
	retval = rDb.connectToDB (RDbFile);
	if (retval != SUCCESS)
	{
		jFATAL ("Unable to open Reader DB [" << RDbFile << "]");
		return FAIL;
	}
	else
	{
		jTRACE ("Connected to Reader db");
	}

	// Prepare the statement for Reader db
	retval = rDb.setupDbStmts ();
	if (retval != SUCCESS)
	{
		jFATAL ("Reader setupDbStmts failed");
		return (FAIL);
	}

	jFX ();
	return SUCCESS;
}

//! \fn int clearDbOps (CalibreDb& cDb, ReaderDb& rDb)
//! \brief Clear the DB connections and statements.
int clearDbOps (CalibreDb& cDb, ReaderDb& rDb)
{
	int retval;

	jDBG ("clearDbOps");
	retval = cDb.finalizeStmts ();
	if (retval != SUCCESS)
	{
		jLOG ("cDb.finalizeStmts failed");
	}

	cDb.finalizeStateOps ();
	if (retval != SUCCESS)
	{
		jLOG ("cDb.finalizeStateOps failed");
	}

	retval = cDb.disconnectDB ();
	if (retval != SUCCESS)
	{
		jLOG ("cDb.disconnectDB failed");
	}

	retval = rDb.finalizeStmts ();
	if (retval != SUCCESS)
	{
		jLOG ("rDb.finalizeStmts failed");
	}
	retval = rDb.disconnectDB ();
	if (retval != SUCCESS)
	{
		jLOG ("rDb.disconnectDB failed");
	}
	return SUCCESS;
}

//! \fn int updateData (SyncClass *Source, SyncClass *Dest, SyncDb *DestDb, SyncClass *newData)
//! \brief Update the rating and state (if applicable) of Dest based on Source
int updateData (SyncClass *Source, SyncClass *Dest, SyncDb *DestDb, SyncClass *newData)
{
	// jFNTRY ();
	int retval;

	bool updateFlag = false;


	//! Use newData variable to store the data to be updated in
	//! the destination DB.
	*newData = *Dest;

	//! If the source rating is greater than target, update the target rting.
	if (Source->getStdRating () > Dest->getStdRating ())
	{
		//! Set the rating in newData if the destination rating is less
		//! than that of source.
		updateFlag = true;

		int srcStdRate = Source->getStdRating ();
		int targetDbRating = Dest->stdRateToDBRate (srcStdRate);

		newData->setRating (targetDbRating);
		newData->setStdRating (srcStdRate);

		if (Dest->getLinkId () < 0)
		{
			//! Data corresponding to this book is not vailable in
			// books_ratings_link table. Insert the data.
			retval = DestDb->insertRating (newData);
			if (retval != SUCCESS)
			{
				jWARN ("Inserting the rating data failed for "
					<< newData->getTitle());
			}
		}
		else
		{
			//! Update the existing rating data in books_ratings_link table.
			retval = DestDb->updateRating (newData);
			if (retval != SUCCESS)
			{
				jWARN ("Updating the rating failed for " << newData->getTitle());
			}
		}
	}

	//! If destination state is lower than that of soruce, update it.
	if (Source->getCustomStatePresent () )
	{
		//! Update the state info in newData only if custom state
		//! is provided from the command line.
		if (Source->getStdState () > Dest->getStdState ())
		{
			//! Set the state info in newData if the standard State value is
			//! lesser in the Dest than Source. First find the state text
			//! from the Source, use it to find the state value of
			//! destination and update it in the newData. The translation is
			//! required as Calibre and Reader use different values for states.
			string sStateText = Source->getStateText ();
			int dState = Dest->textTostate (sStateText);
			newData->setState (dState);
			
			// State text is not required for DB update, but required for the
			// displayData function.
			newData->setStateText (sStateText); 
			updateFlag = true;

			retval = DestDb->updateState (newData);
			if (retval != SUCCESS)
			{
				jWARN ("Updating the state failed for " << newData->getTitle());
			}
		}
	}

	if (updateFlag)
	{
		newData->displayData ();
	}

	// jFX ();
	return SUCCESS;
}
