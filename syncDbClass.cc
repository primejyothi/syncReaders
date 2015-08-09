using namespace std;
#include "jlog.hpp"
#include "syncClass.hpp"
#include "syncDbClass.hpp"
#include <sqlite3.h>
#include <string.h>
#include <cstdio>

//! \file syncDbClass.cc
//! \brief SyncDb, CalibreDb & ReaderDb class implementation.

//! Various DB operations implementations for accessing Calibre and 
//! Cool Reader database.


// SyncDb methods ///////////////////////////////////////
//! SyncDb destructor
SyncDb::~SyncDb ()
{
	jTRACE ("SyncDb destructor");
}


int SyncDb::setCustomStatePresent (bool val)
{
	customStatePresent = val;
	return SUCCESS;
}

//! \fn bool SyncDb::getCustomStatePresent (void)
//! \brief Check if custom state is present.
//! Check if custom state is present. Based on the availability of this
//! field certain processes that involve custom state are performed.
//! \return True if custom state is present.
bool SyncDb::getCustomStatePresent (void)
{
	return customStatePresent;
}

// CalibreDb methods ///////////////////////////////////////
//! CalibreDb constructor
CalibreDb::CalibreDb ()
{
	dbPtr = 0;
	cFetchRecordsStmt = 0;
	cFetchStmtTrail = 0;
	calibreStateStmt = 0;
	cGetBookInfStmt = 0;
	customStatePresent = false;
	cGetBookInfStmtTrail = 0;
	cUpdateRatingStmt = 0;
	cUpdateRatingStmtTrail = 0;
	cInsRatingStmt = 0;
	cInsRatingStmtTrail = 0;
	cUpdateStateStmt = 0;
	cUpdateStateStmtTrail = 0;
	tabId = 0;
	jTRACE ("CalibreDb constructor");
}

//! \fn int CalibreDb::connectToDB (char *fName)
//! \brief Connect to Calibre database.
int CalibreDb::connectToDB (char *fName)
{
	int retVal;
	jDBG ("CalibreDb::connectToDB [" << fName << "]");
	jDBG ("SQL : CalibreDb open DB");
	retVal = sqlite3_open (fName, &dbPtr);
	if (SQLITE_OK != retVal)
	{
		return FAIL;
	}

	return SUCCESS;
}

//! \fn int CalibreDb::disconnectDB ()
//! \brief Disconnect the Calibre DB
int CalibreDb::disconnectDB ()
{
	jTRACE ("CalibreDb::disconnectDB");
	if (dbPtr)
	{
		int retVal;
		jDBG ("SQL : CalibreDB close  DB");
		retVal = sqlite3_close (dbPtr);
		if (SQLITE_OK != retVal)
		{
			jERR ("Error while closing Calibre db " << sqlite3_errmsg (dbPtr));
			return FAIL;
		}
		else
		{
			dbPtr = 0;
		}
	}

	jDBG ("Calibre DB closed");
	return SUCCESS;
}

//! CalibreDb destructor
CalibreDb::~CalibreDb ()
{
	jTRACE ("CalibreDb destructor");
	disconnectDB ();
}

//! \fn int CalibreDb::setupDbStmts (void)
//! \brief Prepare the SQL Statements for Calibre
//! Setup the SQL statements to fetch the book records and book information
//! from the Calibre db.
//! \returns SUCCESS or FAIL
int CalibreDb::setupDbStmts (void)
{
	int retVal;
	jFNTRY ();

	// Prepare the statement to fetch data from Calibre db
	jDBG ("SQL : cFetchRecordsStmt prepare");
	retVal = sqlite3_prepare_v2 (dbPtr,
		"select b.title, b.id, r.rating, r.id from books b left outer join "
		"books_ratings_link r on b.id = r.book",
		-1, &cFetchRecordsStmt,
		&cFetchStmtTrail);
	if (retVal != SQLITE_OK)
	{
		jERR ( "Prepare statement for cFetchRecordsStmt failed with error ["
		<< retVal << "] " <<  sqlite3_errmsg (dbPtr));
		return (FAIL);
	}

	// Prepare the statement to fetch the book info from Calibre db
	// for the given id
	jDBG ("SQL : cGetBookInfStmt prepare");

	retVal = sqlite3_prepare_v2 (dbPtr,
	"select b.title, b.id, r.rating, r.id from books b left outer join " 
	" books_ratings_link r on b.id = r.book where b.title = :calTitle",
			-1, &cGetBookInfStmt, &cGetBookInfStmtTrail);
	if (retVal != SQLITE_OK)
	{
		jERR ("Prepare statemnt for cGetBookInfStmt failed with error ["
				<< retVal << "]" << sqlite3_errmsg (dbPtr));
		return FAIL;
	}

	jDBG ("SQL : cUpdateRatingStmt prepare");
	retVal = sqlite3_prepare_v2 (dbPtr,
		"update books_ratings_link set rating = :rating where book = :book",
		-1, &cUpdateRatingStmt, &cUpdateRatingStmtTrail);
	if (retVal != SQLITE_OK)
	{
		jERR ("Prepare statemnt for cUpdateRatingStmt failed with error ["
				<< retVal << "]" << sqlite3_errmsg (dbPtr));
		return FAIL;
	}

	jDBG ("SQL : cInsRatingStmt prepare");
	retVal = sqlite3_prepare_v2 (dbPtr,
		"insert into books_ratings_link (book, rating) values "
		"(:cbookId, :cRating)", -1, &cInsRatingStmt, &cInsRatingStmtTrail);
	if (retVal != SQLITE_OK)
	{
		jERR ("Prepare statemnt for cInsRatingStmt failed with error ["
				<< retVal << "]" << sqlite3_errmsg (dbPtr));
		return FAIL;
	}

	char qry[255];
	int tabId;

	tabId = getTabId ();
	memset (qry, '\0', 255);
	sprintf (qry, "update books_custom_column_%d_link set value = :cState "
	"where book = :csBookId", tabId);

	jDBG ("SQL : cUpdateStateStmt prepare");
	retVal = sqlite3_prepare_v2 (dbPtr, qry,
		-1, &cUpdateStateStmt, &cUpdateStateStmtTrail);
	if (retVal != SQLITE_OK)
	{
		jERR ("Prepare statemnt for cUpdateStateStmt failed with error ["
				<< retVal << "]" << sqlite3_errmsg (dbPtr));
		return FAIL;
	}

	jFX ();
	return SUCCESS;
}

//! \fn int CalibreDb::finalizeStmts (void)
//! \brief Finalize prepared statements of Calibre Db.
int CalibreDb::finalizeStmts (void)
{
	jTRACE ("CalibreDb::finalizeStmts");
	if (cFetchRecordsStmt)
	{
		jDBG ("SQL : cFetchRecordsStmt finalize");
		sqlite3_finalize (cFetchRecordsStmt);
		cFetchRecordsStmt = 0;
	}

	if (cGetBookInfStmt)
	{
		jDBG ("SQL : cGetBookInfStmt finalize");
		sqlite3_finalize (cGetBookInfStmt);
		cGetBookInfStmt = 0;
	}

	if (cUpdateRatingStmt)
	{
		jDBG ("SQL : cUpdateRatingStmt finalize");
		sqlite3_finalize (cUpdateRatingStmt);
		cUpdateRatingStmt = 0;
	}

	if (cInsRatingStmt)
	{
		jDBG ("SQL : cInsRatingStmt finalize");
		sqlite3_finalize (cInsRatingStmt);
		cInsRatingStmt = 0;
	}

	if (cUpdateStateStmt)
	{
		jDBG ("SQL : cUpdateStateStmt finalize");
		sqlite3_finalize (cUpdateStateStmt);
		cUpdateStateStmt = 0;
	}

	return SUCCESS;
}

//! \fn int CalibreDb::fetchRecords (SyncClass *cRec)
//! \brief Fetch the records from the Calibre DB
int CalibreDb::fetchRecords (SyncClass *cRec)
{
	int retVal;

	char cTitle[TITLE_LEN];
	int cId;
	int cRating;
	int linkId; // Id from books_ratings_link table.

	// jTRACE ("CalibreDb::fetchRecords");
	// jFNTRY ();
	retVal = sqlite3_step (cFetchRecordsStmt);
	if (retVal != SQLITE_ROW)
	{
		jTRACE ("No data, returning");
		return NO_DATA;
	}

	strcpy (cTitle,  (char *) sqlite3_column_text (cFetchRecordsStmt, 0));
	cId = sqlite3_column_int (cFetchRecordsStmt, 1);
	cRating = sqlite3_column_int (cFetchRecordsStmt, 2);
	linkId = sqlite3_column_int (cFetchRecordsStmt, 3);

	cRec->setId (cId);
	cRec->setTitle (cTitle);
	cRec->setRating (cRating);
	cRec->setLinkId (linkId);

	if (cRec->getCustomStatePresent () == true)
	{
		int cState;
		retVal = getCalibreStateInfo (cId, &cState);
		if (retVal != SUCCESS)
		{
			jERR ("getCalibreStateInfo failed");
			return FAIL;
		}
		cRec->setState (cState);

		// Find the State text
		string stateText;
		stateText = cRec->stateToText (cState);
		cRec->setStateText (stateText);
	}

	// jLOG ("Title [" << cTitle << "] id [" << cId << "]");

	// jFX ();
	return SUCCESS;
}

//! \fn int CalibreDb::getBookInfo (SyncClass *cRec, string rTitle)
//! \brief Get the book info from Calibre DB.
int CalibreDb::getBookInfo (SyncClass *cRec, string rTitle)
{
	int idx;
	int retVal;
 	
	// jTRACE ("CalibreDb::getBookInfo");	

	// jFNTRY ();
	idx = sqlite3_bind_parameter_index (cGetBookInfStmt, ":calTitle");
	if (!idx)
	{
		jERR ("Lookup for calibreId failed");
		return (FAIL);
	}

	char title[TITLE_LEN];
	memset (title, '\0', TITLE_LEN);
	strcpy (title, rTitle.c_str ());
	retVal = sqlite3_bind_text (cGetBookInfStmt, idx, title, -1, 
		SQLITE_TRANSIENT);
	if (retVal != SQLITE_OK)
	{
		jERR ("Binding for calibreId failed");
		sqlite3_clear_bindings (cGetBookInfStmt);
		sqlite3_reset (cGetBookInfStmt);
		return (FAIL);
	}

	retVal = sqlite3_step (cGetBookInfStmt);
	if (retVal != SQLITE_ROW)
	{
		//! Set the Book id to 0 if data is not found in the database.
		cRec->setId (0); 
		// jWARN ("Could not fetch book info for book [" << rTitle <<
			// "] from Calibre DB.");
		sqlite3_clear_bindings (cGetBookInfStmt);
		sqlite3_reset (cGetBookInfStmt);
		return (NO_DATA);
	}

	memset (title, '\0', TITLE_LEN);
	strcpy (title, (char *) sqlite3_column_text (cGetBookInfStmt, 0));

	int id;
	id = sqlite3_column_int (cGetBookInfStmt, 1);

	int rating = 0;
	if (sqlite3_column_type (cGetBookInfStmt, 2) == SQLITE_NULL)
	{
		rating = -1;
	}
	{
		rating = sqlite3_column_int (cGetBookInfStmt, 2);
	}
	// jDBG ("Calibre rating from db [" << rating << "]");

	int linkId = 0;
	if (sqlite3_column_type (cGetBookInfStmt, 3) == SQLITE_NULL)
	{
		//! Query returned null for link id, will have to insert the data
		//! at a later point. Indicate the null bu setting the value as -1.
		linkId = -1;
	}
	else
	{
		linkId = sqlite3_column_int (cGetBookInfStmt, 3);
	}


	cRec->setId (id);
	cRec->setTitle (title);
	cRec->setRating (rating);
	cRec->setLinkId (linkId);

	if (cRec->getCustomStatePresent () == true)
	{
		int cState;
		retVal = getCalibreStateInfo (id, &cState);
		if (retVal != SUCCESS)
		{
			jERR ("getCalibreStateInfo failed");
			return FAIL;
		}
		cRec->setState (cState);

		// Get the state text.
		string stateText;
		stateText = cRec->stateToText (cState);
		cRec->setStateText (stateText);
	}

	// Clear the bindings.
	sqlite3_clear_bindings (cGetBookInfStmt);
	sqlite3_reset (cGetBookInfStmt);

	// jFX ();
	return SUCCESS;
}

//! \fn int CalibreDb::getCustomTabId (string stateFName, int *tabId)
//! \brief Find the custom table id for Read state.
//! Find the table id of the custom state table.
//! Calibre store the custom column details in a table called custom_columns.
//! The details of the custom column are stored in the table custom_column_n
//! where n is the id from the custom_columns table for the new column.
int CalibreDb::getCustomTabId (string stateFName, int *tabId)
{
	//! Find the id of the custom state name from the custom_columns table.
	int retVal;
	char qry[255];
	int id;

	sqlite3_stmt *tabIdStmt;
	const char *tabIdTrail;

	jFNTRY ();
	memset (qry, '\0', 255);
	sprintf (qry,
		"select id from custom_columns where label = \"%s\"",
		stateFName.c_str());

	jDBG ("SQL : tabIdStmt prepare");
	retVal = sqlite3_prepare_v2 (dbPtr, qry, -1, &tabIdStmt, &tabIdTrail);
	if (retVal != SQLITE_OK)
	{
		jERR ("Error while preparing statement for custom_columns. "
			<< retVal << "," << sqlite3_errmsg (dbPtr));
		return FAIL;
	}
	jDBG ("state id qry [" << qry << "]");

	// Get the id for the read state.
	retVal = sqlite3_step (tabIdStmt);
	if (retVal != SQLITE_ROW)
	{
		jERR ("Error while fetching data from custom_columns :"
			<< retVal << ", " << sqlite3_errmsg (dbPtr));
		jDBG ("SQL : tabIdStmt finalize");
		sqlite3_finalize (tabIdStmt);
		return FAIL;
	}
	id = sqlite3_column_int (tabIdStmt, 0);
	jDBG ("State id [" << id << "]");

	// Assigning the tab id to the parameter so that it can be used to
	// find the values of read states later.
	*tabId = id;

	// Finalize the tabIdStmt
	if (tabIdStmt)
	{
		jDBG ("SQL : tabIdStmt finalize");
		sqlite3_finalize (tabIdStmt);
		tabIdStmt = 0;
	}

	//! Store the custom state table id.
	setTabId (id);

	jFX ();
	return SUCCESS;
}

//! \fn int CalibreDb::setupStateOps (int tabId)
//! \brief Statements for finding the state info.
int CalibreDb::setupStateOps (int tabId)
{
	char qry[255];
	int retVal;

	const char *cStateTrail;	

	jFNTRY ();
	memset (qry, '\0', 255);
	sprintf (qry,
			"select b.id, r.value from books b left outer join "
			"books_custom_column_%d_link r on  b.id = r.book "
			"where b.id = :bookId", tabId);
	jDBG ("State Qry is [" <<  qry << "]");
	jDBG ("SQL : calibreStateStmt prepare");
	retVal = sqlite3_prepare_v2 (dbPtr, qry, -1,
			&calibreStateStmt, &cStateTrail);
	if (retVal != SQLITE_OK)
	{
		jERR ("Prepare for state select statement failed " << retVal <<
				", " <<  sqlite3_errmsg (dbPtr));
		return (FAIL);
	}

	jFX ();
	return SUCCESS;
}

//! \fn int CalibreDb::getCalibreStateInfo (int id, int *state)
//! \brief Get the state info from the Calibre DB for the given Book id.
int CalibreDb::getCalibreStateInfo (int id, int *state)
{
	// Get the state info from the Calibre DB for the given Book id.
	int retVal;
	int idx;
	
	idx = sqlite3_bind_parameter_index (calibreStateStmt, ":bookId");
	if (!idx)
	{
		jERR ("Look up for bookId failed"); 
		return (FAIL);
	}
	retVal = sqlite3_bind_int (calibreStateStmt, idx, id);
	if (retVal != SQLITE_OK)
	{
		jERR ("Binding for bookId failed\n"); 
		sqlite3_clear_bindings (calibreStateStmt);
		sqlite3_reset (calibreStateStmt);
		return (FAIL);
	}

	retVal = sqlite3_step (calibreStateStmt);
	if (retVal != SQLITE_ROW)
	{
		jERR ("Error fetching state info for book id [" << id << "]");
		sqlite3_clear_bindings (calibreStateStmt);
		sqlite3_reset (calibreStateStmt);
		return (NO_DATA);
	}

	if (sqlite3_column_type (calibreStateStmt, 1) ==  SQLITE_NULL)
	{
		*state = -1;
	}
	else
	{
		*state = sqlite3_column_int (calibreStateStmt, 1);
	}
	//  jDBG ("Calibre State [" << *state << "]");

	sqlite3_clear_bindings (calibreStateStmt);
	sqlite3_reset (calibreStateStmt);
	return (SUCCESS);

}


//! \fn int CalibreDb::finalizeStateOps (void)
//! \brief Finalize statements related to the state info.
int CalibreDb::finalizeStateOps (void)
{
	if (calibreStateStmt)
	{
		jDBG ("SQL : calibreStateStmt finalize");
		sqlite3_finalize (calibreStateStmt);
		calibreStateStmt = 0;
	}
	return SUCCESS;
}

//! \fn int CalibreDb::loadReadState (int tabId, map<int, string>& cStates)
//! \brief Load the Read state values into a map.
int CalibreDb::loadReadState (int tabId, map<int, string>& cStates)
{
	int retVal;
	char qry[255];
	char stateName[20];

	sqlite3_stmt *stateNameStmt;
	const char *stateNameTrail;

	bool customStateFlag;

	jFNTRY ();
	customStateFlag = getCustomStatePresent ();
	if (customStateFlag != true)
	{
		// Custom state column is not specified, return.
		return SUCCESS;
	}

	memset (qry, '\0', 255);
	sprintf (qry, "select id, value from custom_column_%d", tabId);
	jDBG ("state qry [" << qry << "]");

	jDBG ("SQL : stateNameStmt prepare");
	retVal = sqlite3_prepare_v2 (dbPtr, qry, -1, &stateNameStmt,
		&stateNameTrail);
	if (retVal != SQLITE_OK)
	{
		jERR ("Prepare statement for custom_column_" << tabId <<  "failed " << 
			retVal << ", " << sqlite3_errmsg (dbPtr));
		return (FAIL);
	}

	while (1)
	{
		retVal = sqlite3_step (stateNameStmt);
		if (retVal != SQLITE_ROW)
		{
			break;
		}
		int stateNum;
		stateNum = sqlite3_column_int (stateNameStmt, 0);
		strcpy (stateName,
				(char *) sqlite3_column_text (stateNameStmt, 1));

		// jDBG ("Fetched id [" << stateNum << "] name [" << stateName);
		cStates[stateNum] = stateName;
	}
	jDBG ("SQL : stateNameStmt finalize");
	sqlite3_finalize (stateNameStmt);

	jFX ();
	return SUCCESS;
}


//! \fn int CalibreDb::loadRatingIds (map<int, int>& cRates)
//! \brief Load the Rating ids and ratings into a map.
//
//! Calibre ratings can have values between 0 and 10. These values are stored
//! int the ratings table with associated ids. The real rating is taken as
//! the half of the value in the DB. The books_ratings_link table map the
//! book id and the rating id. This function load the rating id and the
//! rating/2 from the ratings table into a map which will be used to
//! translate the Calibre ratings for the books.
int CalibreDb::loadRatingIds (map<int, int>& cRates)
{
	int retVal;
	char qry[255];

	sqlite3_stmt *ratingIdStmt;
	const char *ratingIdTrail;

	jFNTRY ();

	memset (qry, '\0', 255);
	sprintf (qry, "select id, rating from ratings");
	jDBG ("state qry [" << qry << "]");

	jDBG ("SQL : ratingIdStmt prepare");
	retVal = sqlite3_prepare_v2 (dbPtr, qry, -1, &ratingIdStmt,
		&ratingIdTrail);

	if (retVal != SQLITE_OK)
	{
		jERR ("Prepare statement for rating failed " << retVal << ", "
			<< sqlite3_errmsg (dbPtr));
		return (FAIL);
	}

	while (1)
	{
		retVal = sqlite3_step (ratingIdStmt);
		if (retVal != SQLITE_ROW)
		{
			break;
		}

		int ratingId;
		int dbRating;

		ratingId = sqlite3_column_int (ratingIdStmt, 0);
		dbRating = sqlite3_column_int (ratingIdStmt, 1);

		// jDBG ("Fetched Rating id " << ratingId << " and rating "
		// << dbRating);

		//! Calibre can have ratings between 0 and 10. Divide the rating
		//! obtained from the DB by 2 to get the actual rating.
		cRates[ratingId] = dbRating / 2;
	}

	jDBG ("SQL : ratingIdStmt finalize");
	sqlite3_finalize (ratingIdStmt);

	return SUCCESS;
}


//! \fn int CalibreDb::updateRating (SyncClass *newData)
//! \brief Update the rating of the book in the Calibre DB.
int CalibreDb::updateRating (SyncClass *newData)
{
	int retVal;
	int idx;
	// jFNTRY ();

	idx = sqlite3_bind_parameter_index (cUpdateRatingStmt, ":rating");
	if (!idx)
	{
		jERR ("Look up for rating failed");
		return FAIL;
	}

	int newRating = newData->getRating ();
	retVal = sqlite3_bind_int (cUpdateRatingStmt, idx, newRating);
	if (retVal != SQLITE_OK)
	{
		jERR ("Binding for rating failed");
		sqlite3_clear_bindings (cUpdateRatingStmt);
		sqlite3_reset (cUpdateRatingStmt);
		return (FAIL);
	}

	idx = sqlite3_bind_parameter_index (cUpdateRatingStmt, ":book");
	if (!idx)
	{
		jERR ("Look up for book failed");
		return FAIL;
	}

	int newBookId = newData->getId ();
	retVal = sqlite3_bind_int (cUpdateRatingStmt, idx, newBookId);
	if (retVal != SQLITE_OK)
	{
		jERR ("Binding for book failed");
		sqlite3_clear_bindings (cUpdateRatingStmt);
		sqlite3_reset (cUpdateRatingStmt);
		return (FAIL);
	}

	jDBG ("Updating rating to " << newRating << " for bookid " << newBookId);
	retVal = sqlite3_step (cUpdateRatingStmt);
	if (retVal != SQLITE_DONE)
	{
		jERR ("Calibre rating update failed.");
		sqlite3_clear_bindings (cUpdateRatingStmt);
		sqlite3_reset (cUpdateRatingStmt);
		return (FAIL);
	}

	sqlite3_clear_bindings (cUpdateRatingStmt);
	sqlite3_reset (cUpdateRatingStmt);

	// jFX ();
	return SUCCESS;
}

//! \fn int CalibreDb::insertRating (SyncClass *newData)
//! \brief Insert the rating of the book in the Calibre DB.
int CalibreDb::insertRating (SyncClass *newData)
{
	// jFNTRY ();
	int retVal;
	int idx;

	idx = sqlite3_bind_parameter_index (cInsRatingStmt, ":cbookId");
	if (!idx)
	{
		jERR ("Lookup for cbookId failed");
		return FAIL;
	}
	int lBookId = newData->getId ();
	retVal = sqlite3_bind_int (cInsRatingStmt, idx, lBookId);
	if (retVal != SQLITE_OK)
	{
		jERR ("Binding for cbookId failed");
		sqlite3_clear_bindings (cInsRatingStmt);
		sqlite3_reset (cInsRatingStmt);
		return FAIL;
	}

	idx = sqlite3_bind_parameter_index (cInsRatingStmt, ":cRating");
	if (!idx)
	{
		jERR ("Lookup for cRating failed");
		return FAIL;
	}
	int lRating = newData->getRating ();
	retVal = sqlite3_bind_int (cInsRatingStmt, idx, lRating);
	if (retVal != SQLITE_OK)
	{
		jERR ("Binding for cRating failed");
		sqlite3_clear_bindings (cInsRatingStmt);
		sqlite3_reset (cInsRatingStmt);
		return FAIL;
	}

	jDBG ("Inserting rating for " << lBookId << ", Rating : " << lRating);
	retVal = sqlite3_step (cInsRatingStmt);
    if (retVal != SQLITE_DONE)
    {
        jERR ("Calibre rating insert failed");
        sqlite3_clear_bindings (cInsRatingStmt);
        sqlite3_reset (cInsRatingStmt);
        return (FAIL);
    }

	sqlite3_clear_bindings (cInsRatingStmt);
	sqlite3_reset (cInsRatingStmt);

	// jFX ();
	return SUCCESS;
}

int CalibreDb::updateState (SyncClass* newData)
{
	jFNTRY ();
	int lBook;
	int lState;
	
	int retVal;
	int idx;

	lBook =  newData->getId ();
	lState = newData->getState ();
	/*
	   "update books_custom_column_%d_link set value = :cState "
	   "where book = :csBookId", id);
	   */

	idx = sqlite3_bind_parameter_index (cUpdateStateStmt, ":cState");
	if (!idx)
	{
		jERR ("Look up for cState failed");
		return FAIL;
	}

	retVal = sqlite3_bind_int (cUpdateStateStmt, idx, lState);
	if (retVal != SQLITE_OK)
	{
		jERR ("Binding for cState failed");
		sqlite3_clear_bindings (cUpdateStateStmt);
		sqlite3_reset (cUpdateStateStmt);
		return FAIL;
	}

	idx = sqlite3_bind_parameter_index (cUpdateStateStmt, ":csBookId");
	if (!idx)
	{
		jERR ("Look up for csBookId failed");
		return FAIL;
	}

	retVal = sqlite3_bind_int (cUpdateStateStmt, idx, lBook);
	if (retVal != SQLITE_OK)
	{
		jERR ("Binding for csBookId failed");
		sqlite3_clear_bindings (cUpdateStateStmt);
		sqlite3_reset (cUpdateStateStmt);
		return FAIL;
	}

	jDBG ("Updating state for book id " << lBook << " State = " << lState);
	retVal = sqlite3_step (cUpdateStateStmt);
	if (retVal != SQLITE_DONE)
	{
		jERR ("Calibre state updated failed "
			<< sqlite3_errmsg (dbPtr));
		sqlite3_clear_bindings (cUpdateStateStmt);
		sqlite3_reset (cUpdateStateStmt);
		return FAIL;
	}

	sqlite3_clear_bindings (cUpdateStateStmt);
	sqlite3_reset (cUpdateStateStmt);

	jFX ();
	return SUCCESS;
}

//! Set method for custom tab id.
void CalibreDb::setTabId (int tId)
{
	tabId = tId;
}

//! Get method for custom tab id.
int CalibreDb::getTabId (void)
{
	return tabId;
}

// ReaderDb methods ///////////////////////////////////////
//! ReaderDb constructor.
ReaderDb::ReaderDb ()
{
	dbPtr = 0;
	rFetchRecordsStmt = 0;
	rFetchStmtTrail = 0;
	flags = 0;
	customStatePresent = false;
	rGetBookInfStmt = 0;
	rGetBookInfStmtTrail = 0;
	rUpdateFlagStmt = 0;
	rUpdateFlagStmtTrail = 0;
	jTRACE ("ReaderDb constructor");
}

//! Method to connect to the Reader database.
int ReaderDb::connectToDB (char *fName)
{
	int retVal;
	jDBG ("ReaderDb::connectToDB [" << fName << "]");

	jDBG ("SQL : Reader open DB");
	retVal = sqlite3_open (fName, &dbPtr);
	if (SQLITE_OK != retVal)
	{
		return FAIL;
	}

	return SUCCESS;
}

//! Disconnect the Reader DB
int ReaderDb::disconnectDB ()
{
	jDBG ("ReaderDb::disconnectDB");
	if (dbPtr)
	{
		int retVal;
		jDBG ("SQL : Reader close DB");
		retVal = sqlite3_close (dbPtr);
		if (SQLITE_OK != retVal)
		{
			jERR ("Error while closing Reader db");
			return FAIL;
		}
		else
		{
			dbPtr = 0;
		}
	}

	jDBG ("Reader DB closed");
	return SUCCESS;
}

//! ReaderDb destructor
ReaderDb::~ReaderDb ()
{
	jTRACE ("ReaderDb destructor");
	disconnectDB ();
}


//! \fn int ReaderDb::setupDbStmts (void)
//! \brief Prepare the SQL Statements for Reader.
//! Setup the SQL statements to fetch book records and book information
//! from the Reader database.
int ReaderDb::setupDbStmts (void)
{
	int retVal;
	jFNTRY ();

	// prepare the statement to fetch data from reader db.
	jDBG ("SQL : rFetchRecordsStmt prepare");
	retVal = sqlite3_prepare_v2 (dbPtr,
			"select id, title, flags from book where flags != 0", -1,
			&rFetchRecordsStmt, &rFetchStmtTrail);
	if (retVal != SQLITE_OK)
	{
		jERR ( "Prepare statement for rFetchRecordsStmt failed with error ["
		<< retVal << "] " <<  sqlite3_errmsg (dbPtr));
		return (FAIL);
	}

	jDBG ("SQL : rGetBookInfStmt prepare");
	retVal = sqlite3_prepare_v2 (dbPtr,
	"select id, title, flags from book where title = :rTitle",
			-1, &rGetBookInfStmt, &rGetBookInfStmtTrail);
	if (retVal != SQLITE_OK)
	{
		jERR ("Prepare statemnt for rGetBookInfStmt failed with error ["
				<< retVal << "]" << sqlite3_errmsg (dbPtr));
		return (FAIL);
	}

	//! SQL statement to update flags in book table.
	jDBG ("SQL : rGetBookInfStmt prepare");
	retVal = sqlite3_prepare_v2 (dbPtr,
		"update book set flags = :nFlags where id = :rId",
		-1, &rUpdateFlagStmt, &rUpdateFlagStmtTrail);
	if (retVal != SQLITE_OK)
	{
		jERR ("Prepare statemnt for rUpdateFlagStmt failed with error ["
				<< retVal << "]" << sqlite3_errmsg (dbPtr));
		return (FAIL);
	}
	
	jFX ();
	return SUCCESS;
}

//! \fn int ReaderDb::finalizeStmts (void)
//! \brief Finalize prepared statements of Reader Db.
int ReaderDb::finalizeStmts (void)
{
	jTRACE ("ReaderDb::finalizeStmts");
	if (rFetchRecordsStmt)
	{
		jDBG ("SQL : rFetchRecordsStmt finalize");
		sqlite3_finalize (rFetchRecordsStmt);
		rFetchRecordsStmt = 0;
	}

	if (rGetBookInfStmt)
	{
		jDBG ("SQL : rGetBookInfStmt finalize");
		sqlite3_finalize (rGetBookInfStmt);
		rGetBookInfStmt = 0;
	}

	if (rUpdateFlagStmt)
	{
		jDBG ("SQL : rUpdateFlagStmt finalize");
		sqlite3_finalize (rUpdateFlagStmt);
		rUpdateFlagStmt = 0;
	}
	return SUCCESS;
}

//! \fn int ReaderDb::fetchRecords (SyncClass *rRec)
//! Fetch records from the Reader DB
int ReaderDb::fetchRecords (SyncClass *rRec)
{
	int retVal;
	char rTitle[TITLE_LEN];
	int rId;
	int rFlags;


	// jTRACE ("ReaderDb::fetchRecords");

	retVal = sqlite3_step (rFetchRecordsStmt);
	if (retVal != SQLITE_ROW)
	{
		jTRACE ("No data, returning");
		return NO_DATA;
	}
	rId = sqlite3_column_int (rFetchRecordsStmt, 0);
	strcpy (rTitle, (char *) sqlite3_column_text (rFetchRecordsStmt, 1));
	rFlags = sqlite3_column_int (rFetchRecordsStmt, 2);

	rRec->setId (rId);
	rRec->setTitle (rTitle);
	rRec->setFlags (rFlags);
	
	// Extract the state and rating from flags and set them.
	rRec->setRateNState (rFlags);

	// jFX ();
	return SUCCESS;
}

//! \fn int ReaderDb::getBookInfo (SyncClass *rRec, string cTitle)
//! \brief Get the book info from Reader DB.
int ReaderDb::getBookInfo (SyncClass *rRec, string cTitle)
{
	// jFNTRY ();

	int idx;
	int retVal;

	idx = sqlite3_bind_parameter_index (rGetBookInfStmt, ":rTitle");
	if (!idx)
	{
		jERR ("Lookup for rTitle failed");
		return (FAIL);
	}

	char title[TITLE_LEN];
	memset (title, '\0', TITLE_LEN);
	strcpy (title, cTitle.c_str ());
	// jDBG ("Title [" << title << "]");


	retVal = sqlite3_bind_text (rGetBookInfStmt, idx, title, -1,
		SQLITE_TRANSIENT);
	if (retVal != SQLITE_OK)
	{
		jERR ("Binding for title failed");
		sqlite3_clear_bindings (rGetBookInfStmt);
		sqlite3_reset (rGetBookInfStmt);
		jERR ("SQL Error : " << sqlite3_errmsg (dbPtr));
		return (FAIL);
	}

	retVal = sqlite3_step (rGetBookInfStmt);
	if (retVal != SQLITE_ROW)
	{
		rRec->setId (0);
		// jWARN ("Could not fetch book info for book [" << cTitle <<
		// "] from Reader DB.");
		sqlite3_clear_bindings (rGetBookInfStmt);
		sqlite3_reset (rGetBookInfStmt);
		return (NO_DATA);
	}

	memset (title, '\0', TITLE_LEN);
	strcpy (title, (char *) sqlite3_column_text (rGetBookInfStmt, 1));

	int id;
	id = sqlite3_column_int (rGetBookInfStmt, 0);

	int dbFlags;
	dbFlags = sqlite3_column_int (rGetBookInfStmt, 2);
	// jDBG ("Flags [" << dbFlags << "]");

	rRec->setTitle (title);
	rRec->setId (id);
	rRec->setFlags (dbFlags);

	// Extract the state and rating from flags and set them.
	rRec->setRateNState (dbFlags);

	// int lRating = rRec->decodeRRating (dbFlags);
	// jDBG ("lRating [" << lRating << "]");
	// rRec->setRating (lRating);

	// int lState = rRec->decodeRState (dbFlags);
	// jDBG ("lState [" << lState << "]");
	// rRec->setState (lState);
	// string lStateTxt;
	// lStateTxt = rRec->stateToText (lState);
	// rRec->setStateText (lStateTxt);

	sqlite3_clear_bindings (rGetBookInfStmt);
	sqlite3_reset (rGetBookInfStmt);
	return SUCCESS;
}

//! \fn int ReaderDb::updateRating (SyncClass *newData)
//! brief Update the rating in Reader databse.
int ReaderDb::updateRating (SyncClass *newData)
{
	jFNTRY ();

	int curFlag;
	int newFlag;
	int newRating;

	int idx;
	int retVal;
	int lBookId;

	curFlag = 0;
	newFlag = 0;
	

	curFlag = newData->getFlags ();
	newRating = newData->getRating ();
	jDBG ("New Rating is " << newRating);
	jDBG ("Flags is " << curFlag);

	newFlag = (curFlag & ~(RATE_MASK << RATE_SHIFT)) |
		((newRating & RATE_MASK) << RATE_SHIFT);

	jDBG ("New flag [" << newFlag);

	idx = sqlite3_bind_parameter_index (rUpdateFlagStmt, ":nFlags");
	if (!idx)
	{
		jERR ("Look up for nFlags failed");
		return FAIL;
	}

	retVal = sqlite3_bind_int (rUpdateFlagStmt, idx, newFlag);
	if (retVal != SQLITE_OK)
	{
		jERR ("Binding for new flags failed");
		sqlite3_clear_bindings (rUpdateFlagStmt);
		sqlite3_reset (rUpdateFlagStmt);
		return (FAIL);
	}

	idx = sqlite3_bind_parameter_index (rUpdateFlagStmt, ":rId");
	if (!idx)
	{
		jERR ("Look up for rId failed");
		return FAIL;
	}

	lBookId = newData->getId ();
	retVal = sqlite3_bind_int (rUpdateFlagStmt, idx, lBookId);
	if (retVal != SQLITE_OK)
	{
		jERR ("Binding for book id failed");
		sqlite3_clear_bindings (rUpdateFlagStmt);
		sqlite3_reset (rUpdateFlagStmt);
		return (FAIL);
	}

	jDBG ("Updating flags");
	retVal = sqlite3_step (rUpdateFlagStmt);
	if (retVal != SQLITE_DONE)
	{
		jERR ("Reader flag update failed");
		sqlite3_clear_bindings (rUpdateFlagStmt);
		sqlite3_reset (rUpdateFlagStmt);
		return FAIL;
	}

	sqlite3_clear_bindings (rUpdateFlagStmt);
	sqlite3_reset (rUpdateFlagStmt);

	//! Update the new flags in newData so that the new rating is reflected
	//! in the flags which might be used for state updates.
	newData->setFlags (newFlag);

	jFX ();
	return SUCCESS;
}

//! Insert rating function for Reader, not used.
int ReaderDb::insertRating (SyncClass *newData)
{
	jFNTRY ();
	jFX ();
	return SUCCESS;
}

int ReaderDb::updateState (SyncClass* newData)
{
	jFNTRY ();
	jDBG ("Update State Flags [" << newData->getFlags());
	int curFlag;
	int newFlag;
	int newState;

	int idx;
	int retVal;
	int lBookId;

	curFlag = 0;
	newFlag = 0;
	

	curFlag = newData->getFlags ();
	newState = newData->getState ();
	jDBG ("New State is " << newState);
	jDBG ("Flags is " << curFlag);

	newFlag = (curFlag & ~(STATE_MASK << STATE_SHIFT)) |
		((newState & STATE_MASK) << STATE_SHIFT);

	jDBG ("New flag [" << newFlag);

	idx = sqlite3_bind_parameter_index (rUpdateFlagStmt, ":nFlags");
	if (!idx)
	{
		jERR ("Look up for nFlags failed");
		return FAIL;
	}

	retVal = sqlite3_bind_int (rUpdateFlagStmt, idx, newFlag);
	if (retVal != SQLITE_OK)
	{
		jERR ("Binding for new flags failed");
		sqlite3_clear_bindings (rUpdateFlagStmt);
		sqlite3_reset (rUpdateFlagStmt);
		return (FAIL);
	}

	idx = sqlite3_bind_parameter_index (rUpdateFlagStmt, ":rId");
	if (!idx)
	{
		jERR ("Look up for rId failed");
		return FAIL;
	}

	lBookId = newData->getId ();
	retVal = sqlite3_bind_int (rUpdateFlagStmt, idx, lBookId);
	if (retVal != SQLITE_OK)
	{
		jERR ("Binding for book id failed");
		sqlite3_clear_bindings (rUpdateFlagStmt);
		sqlite3_reset (rUpdateFlagStmt);
		return (FAIL);
	}

	jDBG ("Updating flags");
	retVal = sqlite3_step (rUpdateFlagStmt);
	if (retVal != SQLITE_DONE)
	{
		jERR ("Reader flag update failed");
		sqlite3_clear_bindings (rUpdateFlagStmt);
		sqlite3_reset (rUpdateFlagStmt);
		return FAIL;
	}

	sqlite3_clear_bindings (rUpdateFlagStmt);
	sqlite3_reset (rUpdateFlagStmt);

	//! Update the new flags in newData so that the new rating is reflected
	//! in the flags which might be used for state updates.
	newData->setFlags (newFlag);

	jFX ();
	return SUCCESS;
}
