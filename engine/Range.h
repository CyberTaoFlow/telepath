#ifndef _Range_h
#define _Range_h

#include <string> 

using namespace std;

//!  Converting IP string to a decimal number.
/*!
	\param ip as a C++ string argument.
	\return The decimal IP number(or zero in a case of an illeagal IP string).
*/
unsigned int ipToNum(string &);

//!  Checking the IP Range.
/*!
	This Fucntion checks if a given string("IP") is an IP Range or a single IP.
	\param IP as a C++ string argument.
	\param from as a unsigned integer argument.
	\param to as a unsigned integer argument.
	\return true or false as a boolean argument.
	\n      true - The given string is an IP range.
	\n      false - The given string is a single IP.
*/
bool ipSingleOrRange(string &,string &,string &);

//! Range class
/*!
	This class stores IP range in a decimal representation.
*/
class Range{
public:

	//!  A 4 bytes number that saves the lower decimal range. 
	/*!*/
	unsigned int from;

	//!  A 4 bytes number that saves the higher decimal range.
	/*!*/
	unsigned int to;

	//!  Defult Constrator.
	/*!
		Initializing the object fields with their default values.
	*/
	Range();

	//!  Constrator.
	/*!
		Initializing all the object fields(from & to) with the argument values .
		\param from as a unsigned integer argument.
		\param to as a unsigned integer argument.
	*/
	Range(unsigned int,unsigned int);

	//!  Initializing Object Values..
	/*!
		Initializing all the object fields(from & to) with the argument values .
		\param from as a unsigned integer argument.
		\param to as a unsigned integer argument.
	*/
	void init(unsigned int,unsigned int);

	//!  Checking If a decimal IP is in the Range.
	/*!
		\param decimalIP as a unsigned integer argument
		\return true or false as a boolean argument.
		\n	true - The IP is in the range.
		\n 	false - The IP is out of range.
	*/
	bool inRange(unsigned int);

	//!  Printing the class variables.
	/*!
		Printing to stdout the object fields.
		\param none
		\return void
	*/
	void print();

	//!  Printing the class variables.
	/*!
		Printing to syslog the object fields.
		\param none
		\return void
	*/
	void print_syslog();
};

#endif
