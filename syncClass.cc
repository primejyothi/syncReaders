using namespace std;
#include "jlog.hpp"
#include "syncClass.hpp"
#include <vector>
//! \file syncClass.cc
//! \brief SyncClass, Calibre & Reader class implementation.

/*
int decodeRating (int flags);
int decodeState (int flags);
*/

// SyncClass methods. ////////////////////////////////////
//! SyncClass constructor.
SyncClass::SyncClass ()
{
	customStatePresent = false;
	stdState = 0;
	stdRating = 0;
	id = 0;
	rating = 0;
	state = 0;
	flags = 0;
	linkId = 0;
}

//! SyncClass destructor.
SyncClass::~SyncClass ()
{
	jTRACE ("SyncClass destructor");
}

SyncClass& SyncClass::operator = (const SyncClass& rhs)
{
	title = rhs.title;
	id = rhs.id;
	rating = rhs.rating;
	stdRating = rhs.stdRating;
	state = rhs.state;
	stateText = rhs.stateText;
	stdState = rhs.stdState;
	customStatePresent = rhs.customStatePresent;
	flags = rhs.flags;

	states = rhs.states;
	ratingIdMap = rhs.ratingIdMap;
	linkId = rhs.linkId;

	return *this;
}

//! Set method for title.
void SyncClass::setTitle (string name)
{
	title = name;
}

//! Get method for title.
string SyncClass::getTitle (void)
{
	return title;
}


//! Set method for id.
void SyncClass::setId (int i)
{
	id = i;
}

//! Get method for id.
int SyncClass::getId (void)
{
	return id;
}

//! Set method for link id.
void SyncClass::setLinkId (int lId)
{
	linkId = lId;
}

//! Get method for link id.
int SyncClass::getLinkId (void)
{
	return (linkId);
}

//! Set method for rating
void SyncClass::setRating (int r)
{
	rating = r;

	int sRating;
	sRating = findStdRating (r);
	stdRating = sRating;
}

//! Get method for rating
int SyncClass::getRating (void)
{
	return rating;
}

//! Get method for std Rating
int SyncClass::getStdRating (void)
{
	return stdRating;
}

//! Set method for std Rating
void SyncClass::setStdRating (int rating)
{
	stdRating = rating;
}

//! Get method for flags.
int SyncClass::getFlags (void)
{
	return flags;
}

//! Set method for state.
void SyncClass::setState (int s)
{
	state = s;
}

//! Get method for state.
int SyncClass::getState (void)
{
	return state;
}

//! Get method for stdState
int SyncClass::getStdState (void)
{
	return stdState;
}

void SyncClass::setFlags (int f)
{
	flags = f;
}

//! \fn int SyncClass::setStateText (string name)
//! brief Set method for state text.
//! Set the state text and standard state.
int SyncClass::setStateText (string name)
{
	stateText = name;
	int stateVal;
	stateVal = findStdState (name);
	stdState = stateVal;
	return SUCCESS;
}

//! \fn string SyncClass::getStateText (void)
//! \brief set method for state text
string SyncClass::getStateText (void)
{
	return stateText;
}

int SyncClass::setCustomStatePresent (bool val)
{
	customStatePresent = val;
	return SUCCESS;
}

bool SyncClass::getCustomStatePresent (void)
{
	return customStatePresent;
}

// Calibre methods //////////////////////////////////////
//! Calibre destructor.
Calibre::~Calibre ()
{
	jTRACE ("Calibre destructor");
}

//! \fn void Calibre::displayData (void)
//! \brief Display the Calibre data.
void Calibre::displayData ()
{
	if (getCustomStatePresent () == true)
	{
		jINFO ("Calibre Id =" << setw(5) << getId () << ", State = "
		<< setw(8) << getStateText () << " (" << getStdState () << ","
		<< getState () << "), Rating = " << getStdRating () << "("
		<< getRating () << ")" << ", Title = " << getTitle ());
	}
	else
	{
		jINFO ("Calibre Id =" << setw(5) << getId ()  << ", State = "
		<< setw(8) << getStateText () << "  (" << "NA" << "), Rating = "
		<< getStdRating () << "(" << getRating () << ")" << ", Title = "
		<< getTitle());
	}
}

//! \fn int Calibre::createRefLookup (map<int, string>& t)
//! \brief Create a lookup table for Calibre based on the values read from DB.
int Calibre::createRefLookup (map<int, string>& t)
{
	states = t;
	return SUCCESS;
}

int Calibre::createRatingLookup (map<int, int>& dbRatings)
{
	ratingIdMap = dbRatings;
	return SUCCESS;
}

//! \fn int Calibre::findStdState (string stateName)
//! \brief Find the state value given the Calibre state name.
int Calibre::findStdState (string stateName)
{
	int lState = -1;
	map<string, int> stdState;
	stdState["Unread"] = 0;
	stdState["To Read"] = 1;
	stdState["Reading"] = 2;
	stdState["Finished"] = 3;

	map<string, int>::iterator res;

	res = stdState.find (stateName);
	if (res != stdState.end())
	{
		lState = stdState[stateName];
	}
	return (lState);

}

//! \fn string Calibre::stateToText (int state)
//! Translate the Read state values to text
//! The text value of Calibre Text is returned from the map. If the input
//! value is out of bounds, "Unknown" will be returned.
string Calibre::stateToText (int state)
{
	string text;
	map<int, string>::iterator i = states.find (state);
	if (i != states.end ())
	{
		text = (*i).second;
	}
	else
	{
		text = "Unknown";
	}
	return (text);
}

//! \fn int Calibre::textTostate (string stateName)
//! \brief Get the Calibre state value given the state name.
int Calibre::textTostate (string stateName)
{
	int lState = -1;
	// jFNTRY ();
	for (map <int, string>::iterator i = states.begin ();
		i != states.end (); ++i)
	{
		if ( (*i).second == stateName )
		{
			lState = (*i).first;
			break;
		}
	}
	// jFX ();
	return lState;
}

//! \fn int Calibre::findStdRating (int dbRating)
//! Convert Calibre rating to a standard rating.
//! Calibre does not use straight forward values for ratings. Instead
//! it uses values stored in the DB.
int Calibre::findStdRating (int dbRating)
{
	//! Calibre store the ratings in the ratings table. The ratings can
	//! range from 0 to 10. The id of this rating is linked to the 
	//! books_ratings_link table. The stars are derived as rating / 2.
	int sRating = 0;

	for (map<int, int>::iterator i = ratingIdMap.begin ();
		i != ratingIdMap.end (); ++i)
	{
		if ((*i).first == dbRating)
		{
			sRating = (*i).second;
		}
	}

	// jTRACE ("Translating Calibre Rating from "<< dbRating << " to "
	// << sRating);
	return (sRating);
}


//! \fn int Calibre::stdRateToDBRate (int stdRate)
//! \brief Translate the standard rating to Calibre Rating.
int Calibre::stdRateToDBRate (int stdRate)
{
	int dbRate = 0;

	for (map<int, int>::iterator j = ratingIdMap.begin ();
		j != ratingIdMap.end (); ++j)
	{
		if ((*j).second == stdRate)
		{
			dbRate = (*j).first;
		}
	}
	return dbRate;
}

// Reader methods ///////////////////////////////////////
//
//! Reader constructor
Reader::Reader ()
{
	map<int, string> t;
	t[0] = "Unread";
	t[1] = "To Read";
	t[2] = "Reading";
	t[3] = "Finished";
	createRefLookup (t);
	setId (0);
	jDBG ("Calling setRating from Reader constructor");
	setRating (0);
	setStdRating (0);
	setState (0);
}

//! Reader destructor.
Reader::~Reader ()
{
	jTRACE ("Reader destructor");
}

//! \fn void Reader::displayData (void)
//! \brief Display the Reader data.
void Reader::displayData ()
{

	jINFO ("Reader Id  =" << setw(5) << getId ()  << ", State = " << setw(8)
	<< getStateText () << " (" << getStdState () << "," << getState ()
	<< "), Rating = " << getStdRating () << "(" << getRating () << ")"
	<< ", Title = " << getTitle());
}


//! Read state look up for Reader.
int Reader::createRefLookup (map<int, string>& t)
{
	states = t;
	return SUCCESS;
}


//! \fn string Reader::stateToText (int state)
//! \brief Find the text value for a given state.
//! The text value of Reader state is returned from the map.
string Reader::stateToText (int state)
{
	string lText;
	
	map<int, string>::iterator res = states.find (state);
	if (res == states.end ())
	{
		lText = "Unknown";
	}
	else
	{
		lText = states[state];
	}
	
	return (lText);
}

//! \fn int Reader::textTostate (string state)
//! \brief Get the Reader state value given the state name.
int Reader::textTostate (string stateName)
{
	int lState = -1;
	// jFNTRY ();
	for (map <int, string>::iterator i = states.begin ();
		i != states.end (); ++i)
	{
		if ( (*i).second == stateName )
		{
			lState = (*i).first;
			break;
		}
	}
	// jFX ();
	return lState;
}

//! \fn int Reader::findStdState (string stateName)
//! Find the Reader state value for the given state name.
int Reader::findStdState (string stateName)
{
	int lState;
	map<string, int> stdState;
	stdState["Unread"] = 0;
	stdState["To Read"] = 1;
	stdState["Reading"] = 2;
	stdState["Finished"] = 3;

	map<string, int>::iterator res = stdState.find (stateName);
	if (res == stdState.end ())
	{
		lState = -1;
	}
	else
	{
		lState = stdState[stateName];
	}

	return (lState);
}

//! Find the standard rating for the given Reader rating.
int Reader::findStdRating (int dbRating)
{
	//! Unlike Calibre, Cool Reader does not do any translations for rating.
	return (dbRating);
}

int Reader::stdRateToDBRate (int stdRate)
{
	return (stdRate);
}

//! \fn void Reader::setRateNState (int f)
//! \brief Set method for flags.
//! The Reader stores the rating and the state in flags. Extract the rating
//! and state from flags well and set them.
void Reader::setRateNState (int f)
{
	int lState = decodeRState (f);
	setState (lState);

	string lStateText = stateToText (lState);
	setStateText (lStateText);

	int lRating = decodeRRating (f);
	setRating (lRating);
}

//! \fn int Reader::decodeRRating (int flags)
//! \brief Decode the Reader rating from the flags.
//! The bits 20 to 23 of flags indicate the rating.
int Reader::decodeRRating (int flags)
{
	int rating;

	rating = (flags >> RATE_SHIFT) & RATE_MASK;
	return rating;
}

//! \fn int Reader::decodeRState (int flags)
//! \brief Decode the Reader state from the flags.
//! The bits 16 to 19 of the flags indicate the state.
int Reader::decodeRState (int flags)
{
	int state;

	state = (flags >> STATE_SHIFT) & STATE_MASK;
	return state;
}


