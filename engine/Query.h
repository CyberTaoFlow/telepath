#include <vector>

#include "Parameter.h"

using namespace std;

class Query{
public:
	double result;
	Query();
	Query(vector <Parameter> &,double);
};

