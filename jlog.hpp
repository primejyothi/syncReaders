#ifndef __JLOG_H
#define __JLOG_H

#include <istream>
#include <iomanip>
#include <iostream>

//! \file jlog.hpp
//! \brief Simple Error/Log/Trace/Debug macros.
//!
//! Follows syntax of std::cout, but the arguments have to be
//! enclosed in parentheses. Newlines and endl operators are not 
//! required. Messages are prefixed with Message type, file name, line
//! and two characters indicating the type of message.
//!
//! Type  | Message | Value |Indicators
//! ------|---------|-------|----------
//! Fatal | FATAL   |  1    | XX
//! Error | ERROR   |  2    | ==
//! Warn  | WARN    |  4    | =-
//! Log   | LOG     |  8    | --
//! Info  | INFO    | 16    | -+
//! Debug | DEBUG   | 32    | +-
//! Trace | TRACE   | 64    | ++
//!

//! Available log levels.
typedef enum {
//! Fatal messages
FATAL = 1,

//! Error messages
ERROR = 2,

//! Warning messages
WARN = 4,

//! Log messages
LOG = 8,

//! Info messages
INFO = 16,

//! Debug messages
DBG = 32,

//! Trace messages 
TRACE = 64} LVLS;

extern int JLFWIDT;
extern int JLLWIDT;
extern int JLOGLVL;
extern int JMINLVL;

//! Width for file name in debug statements.
#define FILE_W 13
//! Width for line number in debug statements.
#define LINE_W 4

//! Set the log level.

//! MIN Level ensures that critical messages are not turned off by mistake.
#define SETMSGLVL(lvl) \
	{ \
		JLOGLVL = JMINLVL | lvl; \
	}

		// jLOG ("Setting log level to " << #lvl << " : " << lvl); 

//! Turn on a particular log level.
#define TURNON(lvl) \
	{ \
		SETMSGLVL (JLOGLVL | lvl);  \
	}

//! Turn off a particular log level.
#define TURNOFF(lvl) \
	{ \
		SETMSGLVL ( ((~lvl) & JLOGLVL)); \
	}

//! Set the width for file names in log messages.
#define SETFWDT(width) do {JLFWIDT = width;} while (0)

//! Set the width for line numbers in log messages.
#define SETLWDT(width) do {JLLWIDT = width;} while (0)


//! Displays Fatal message
#define jFATAL(x) \
	do \
	{ \
		std::cout << "[FATAL :" << setw (JLFWIDT) <<  __FILE__ << ":" \
		<< std::setw (JLLWIDT)  <<  __LINE__ << "] " \
		<< "XX [" << x << "]" << std::endl; \
	} \
	while (0)

//! Displays Error message
#define jERR(x) \
	do \
	{ \
		std::cout << "[ERROR :" << setw (JLFWIDT) <<  __FILE__ << ":" \
		<< std::setw (JLLWIDT)  <<  __LINE__ << "] " \
		<< "== [" << x << "]" << std::endl; \
	} \
	while (0)

//! Displays warning message
#define jWARN(x) \
	do \
	{ \
		std::cout << "[WARN  :" << setw (JLFWIDT) <<  __FILE__ << ":" \
		<< std::setw (JLLWIDT)  <<  __LINE__ << "] " \
		<< "=- [" << x << "]" << std::endl; \
	} \
	while (0)

//! Displays Log message
#define jLOG(x) \
	do \
	{ \
		if (JLOGLVL >= LOG) \
		{ \
			std::cout << "[LOG   :" << setw (JLFWIDT) <<  __FILE__ << ":" \
			<< std::setw (JLLWIDT)  <<  __LINE__ << "] " \
			<< "-- [" << x << "]" << std::endl; \
		} \
	} \
	while (0)

//! Displays Info message
#define jINFO(x) \
	do \
	{ \
		if (JLOGLVL >= INFO) \
		{ \
			std::cout << "[INFO  :" << setw (JLFWIDT) <<  __FILE__ << ":" \
			<< std::setw (JLLWIDT)  <<  __LINE__ << "] " \
			<< "-+ [" << x << "]" << std::endl; \
		} \
	} \
	while (0)

//! Displays Debug message
#define jDBG(x) \
	do \
	{ \
		if (JLOGLVL >= DBG) \
		{ \
			std::cout << "[DEBUG :" << setw (JLFWIDT) <<  __FILE__ << ":" \
			<< std::setw (JLLWIDT)  <<  __LINE__ << "] " \
			<< "+- [" << x << "]" << std::endl; \
		} \
	} \
	while (0)

//! Displays Trace message
#define jTRACE(x) \
	do \
	{ \
		if (JLOGLVL >= TRACE) \
		{ \
			std::cout << "[TRACE :" << setw (JLFWIDT) <<  __FILE__ << ":" \
			<< std::setw (JLLWIDT)  <<  __LINE__ << "] " \
			<< "++ [" << x << "]" << std::endl; \
		} \
	} \
	while (0)

//! Displays Function entry message
#define jFNTRY(x) \
	do \
	{ \
		if (JLOGLVL >= TRACE) \
		{ \
			std::cout << "[TRACE :" << setw (JLFWIDT) <<  __FILE__ << ":" \
			<< std::setw (JLLWIDT)  <<  __LINE__ << "] " \
			<< "++ [Entering " << __PRETTY_FUNCTION__ << "]" << std::endl; \
		} \
	} \
	while (0)

//! Displays Function exit message
#define jFX(x) \
	do \
	{ \
		if (JLOGLVL >= TRACE) \
		{ \
			std::cout << "[TRACE :" << setw (JLFWIDT) <<  __FILE__ << ":" \
			<< std::setw (JLLWIDT)  <<  __LINE__ << "] " \
			<< "++ [Leaving " << __PRETTY_FUNCTION__ << "]" << std::endl; \
		} \
	} \
	while (0)

#endif
