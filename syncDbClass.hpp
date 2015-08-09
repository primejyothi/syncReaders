#ifndef __SYNCDBCLASS_H
#define __SYNCDBCLASS_H
//! \file syncDbClass.hpp
//! \brief SyncDb, CalibreDb and ReaderDb class declarations.


#include <sqlite3.h>
//! Abstract base class for CalibreDb and ReaderDb.
class SyncDb
{
public :

	//! Method to get customStatePresent flag.
	bool getCustomStatePresent (void);

	//! Flag indicating custom states for Calibre.
	bool customStatePresent;

	//! SyncDb destructor
	virtual ~SyncDb ();

	//! Connect to SQLiteDB.
	virtual int connectToDB (char *dbFileName) = 0;

	//! Disconnect DB.
	virtual int disconnectDB (void) = 0;

	//! Prepare SQL statements for DB ops.
	virtual int setupDbStmts (void) = 0;

	//! Finalize prepared statements.
	virtual int finalizeStmts (void) = 0;

	//! Fetch records from DB.
	virtual int fetchRecords (SyncClass *rec) = 0;

	//! Get the book info from the DB.
	virtual int getBookInfo (SyncClass *rec, string bookTitle) = 0;

	//! Method to set customStatePresent flag.
	int setCustomStatePresent (bool val);

	//! Method to update the rating.
	virtual int updateRating (SyncClass *newData)  = 0;

	//! Method to insert the rating.
	virtual int insertRating (SyncClass *newData)  = 0;

	//! Method to update the state.
	virtual int updateState (SyncClass *newData) = 0;
};

//! Class for Calibre
class CalibreDb : public SyncDb
{
private :
	//! DB Handle.
	sqlite3 *dbPtr;

	//! FetchRecord statement.
	sqlite3_stmt *cFetchRecordsStmt;

	//! fetch statement trail.
	const char *cFetchStmtTrail;

	//! State info statement.
	sqlite3_stmt *calibreStateStmt;

	//! Book info statement
	sqlite3_stmt *cGetBookInfStmt;

	//! Book info statement trail
	const char *cGetBookInfStmtTrail;

	//! Calibre rating update statement.
	sqlite3_stmt *cUpdateRatingStmt;

	//! Calibre rating update statement trail.
	const char *cUpdateRatingStmtTrail;

	//! Calibre rating insert statement
	sqlite3_stmt *cInsRatingStmt;

	//! Calibre rating insert statement trail.
	const char *cInsRatingStmtTrail;

	//! Calibre state update statement.
	sqlite3_stmt *cUpdateStateStmt;

	//! Calibre state update statement trail.
	const char *cUpdateStateStmtTrail;

	//! Table id for custom states.
	int tabId;

public :

	//! CalibreDb constructor
	CalibreDb();

	//! CalibreDb destructor
	virtual ~CalibreDb();

	//! Connect to Calibre database.
	int connectToDB (char *fName);

	//! Close the Calibre DB
	int disconnectDB ();

	//! Prepare the SQL statements.
	int setupDbStmts (void);

	//! Finalize prepared statements of Calibre Db.
	int finalizeStmts (void);

	//! Fetch records from Calibre db
	int fetchRecords (SyncClass *cRec);

	//! Method to find the state info.
	int getCustomTabId (string stateFName, int *tabId);

	//! Method to prepare SQL statements for fetching Calibre state info.
	int setupStateOps (int tabId);

	//! Fetch the read state info from the Calibre Db.
	int getCalibreStateInfo (int id, int *state);

	//! Finalize state info statements.
	int finalizeStateOps (void);

	//! Load the Read state info into a map.
	int loadReadState (int tabId, map<int, string>& cStates);

	//! Load the Rating ids into a  map.
	int loadRatingIds (map<int, int>& cRates);

	//! Get the book info from the Calibre db.
	int getBookInfo (SyncClass *cRec, string bookTitle);

	//! Update the rating in the Calibre db.
	int updateRating (SyncClass *newData);

	//! Insert rating in Calibre db.
	int insertRating (SyncClass *newData);

	//! Method to update the state in Calibre DB.
	int updateState (SyncClass *newData);

	//! Method to set table id.
	void setTabId (int tId);

	//! Get method for custom table id.
	int getTabId (void);
};

//! Class for Reader
class ReaderDb : public SyncDb
{
private :
	//! FetchRecord statement
	sqlite3_stmt *rFetchRecordsStmt;

	//! fetch statement trail.
	const char *rFetchStmtTrail;

	//! Book info statement for Reader
	sqlite3_stmt *rGetBookInfStmt;

	//! Book info statement trail for Reader
	const char *rGetBookInfStmtTrail;

	//! Update flags statement for Reader.
	sqlite3_stmt *rUpdateFlagStmt;

	//! Update flag statment trail for Reader.
	const char *rUpdateFlagStmtTrail;

	//! DB Handle.
	sqlite3 *dbPtr;

	//! Flags - specific to Reader
	int flags;

public :

	//! ReaderDb constructor
	ReaderDb();

	//! ReaderDb destructor
	virtual ~ReaderDb();

	//! Connect to Reader database.
	int connectToDB (char *fName);

	//! Close the Reader database.
	int disconnectDB ();

	//! Prepare the SQL statements.
	int setupDbStmts (void);

	//! Finalize prepared statements of Calibre Db.
	int finalizeStmts (void);

	//! Fetch records from Reader Db.
	int fetchRecords (SyncClass *rRec);

	//! Fetch book info from Reader db.
	int getBookInfo (SyncClass *rRec, string bookTitle);

	//! Update the rating in the Reader db.
	int updateRating (SyncClass *newData);

	//! Insert rating in Reader db.
	int insertRating (SyncClass *newData);

	//! Method to update the state in Calibre DB.
	int updateState (SyncClass *newData);
};
#endif
