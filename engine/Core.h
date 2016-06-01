#ifndef _Core_h
#define _Core_h

#include <vector>
#include <boost/unordered_set.hpp>

using namespace std;

class rule_core{
public:
	unsigned short emission;
	boost::unordered_set <vector <unsigned int> > imagination;

	rule_core();
};

#endif
