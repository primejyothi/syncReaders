#ifndef __SYNCCLASS_H
#define __SYNCCLASS_H

#include <map>
//! \file syncClass.hpp
//! \brief SyncClass, Calibre & Reader class declarations.

//! Return value for Success
#define SUCCESS 0

//! Return value for failure
#define FAIL 1

//! Return value for no data.
#define NO_DATA -2

//! Length of book titles.
#define TITLE_LEN 200

//! The Read State is bytes 16-19 in Flags.
#define STATE_SHIFT 16

//! Mask for extracting state
#define STATE_MASK 0x0F

//! Rating is bytes 20-23 in Flags
#define RATE_SHIFT 20

//! Mask for extrating rating
#define RATE_MASK 0x0F

//! Abstract base class for the Calibre & Reader classes.
class SyncClass
{
private :
	//! The book title
	string title;

	//! Book id
	int id;

	//! Link id
	int linkId;

	//! Book rating
	int rating;

	//! Standard Rating
	int stdRating;

	//! Read state
	int state;

	//! Rating text.
	string stateText;

	//! Standard state
	int stdState;

	//! Flag indicating custom states for Calibre.
	bool customStatePresent;

	//! Flags - specific to Reader
	int flags;

public :

	//! Possible read states.
	map<int,string> states;

	//! Ratings id and ratings map.
	map<int, int> ratingIdMap;

	SyncClass ();
	virtual ~SyncClass ();

	//! Assignment operator.
	SyncClass& operator = (const SyncClass& rhs);

	//! Display the data.
	virtual void displayData (void) = 0;

	//! Create read state look up table.
	virtual int createRefLookup (map<int, string>& stateLookup) = 0;

	//! Method to find the state text from state value.	
	virtual string stateToText (int state) = 0; 

	//! Method to find the state value from state text. 
	virtual int textTostate (string state) = 0; 

	//! Method to find the DB rating value from standard rating.
	virtual int stdRateToDBRate (int stdRate) = 0;

	//! Set method for title.
	void setTitle (string name);

	//! Get method for title.
	string getTitle (void);

	//! Set method for id.
	void setId (int i);

	//! Get method for id.
	int getId (void);

	//! Set method for Link id.
	void setLinkId (int i);

	//! Get method for Link id.
	int getLinkId (void);

	//! Set method for rating
	void setRating (int r);

	//! Get method for rating
	int getRating (void);

	//! Set method for flags.
	void setFlags (int f);

	//! Get method for flags.
	int getFlags (void);

	//! Set method for state
	void setState (int s);

	//! Get method for state
	int getState (void);

	//! Method to decode Rating from Reader flags.
	virtual int decodeRRating (int flags) {return FAIL;};

	//! Method to set the state Text.
	int setStateText (string name);

	//! Method to get the state text.
	string getStateText (void);

	//! Method to convert the state to standard state.
	virtual int findStdState (string stateName) = 0;

	//! Method to convert the rating to standard rating.
	virtual int findStdRating (int rating) = 0;

	//! Method to get the standard state.
	int getStdState (void);

	//! Method to set the standard rating
	void setStdRating (int);

	//! Method to get the standard rating
	int getStdRating (void);

	//! Method to set customStatePresent flag.
	int setCustomStatePresent (bool val);

	//! Method to get customStatePresent flag.
	bool getCustomStatePresent (void);

	//! Method to extract and set State and Rating for Reader.
	virtual void setRateNState (int flags){};
};

//! Class for Calibre
class Calibre : public SyncClass
{
public :
	virtual ~Calibre ();
	Calibre& operator = (const Calibre& rhs);
	void displayData (void);
	int createRefLookup (map<int, string>& stateLookup);
	int createRatingLookup (map<int, int>& dbRatings);
	string stateToText (int state); 
	int textTostate (string state); 
	int stdRateToDBRate (int stdRate);
	int findStdState (string name);
	int findStdRating (int dbRating);

};

//! Class for Cool Reader
class Reader : public SyncClass
{
public :
	Reader ();
	virtual ~Reader ();
	Reader& operator = (const Reader& rhs);
	void displayData (void);
	int createRefLookup (map<int, string>& stateLookup);
	string stateToText (int state); 
	int textTostate (string state); 
	int stdRateToDBRate (int stdRate);
	int findStdState (string name);
	int findStdRating (int dbRating);
	int decodeRRating (int flags);

	//! Method to decode State from Reader flags.
	int decodeRState (int flags);

	//! Extract rating and state from the flags and set it.
	void setRateNState (int flags);
};

#endif 
