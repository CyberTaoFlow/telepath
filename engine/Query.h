#include <vector>

#include "Parameter.h"

using namespace std;

class Query{
public:
	//!  Storing the Total Query score.
	/*!*/
	double result;

	//!  Defult Constrator.
	/*!
		Initializing the object fields with their default values.
	*/
	Query();

	//!  Culculating Query Score.
	/*!
		Culculating a total query score for a given vector of parameters.

		For Example: \n \n

		Input: \n
		params = [ \n
			{score=0.5,length=0,weight=1},       (User-Agent) \n
			{score=0.75,length=0.5,weight=0.25}, (Host) \n
			{score=0,length=0,weight=1},         (Content-length) \n
			{score=0.1,length=0,weight=1}        (Accept)  \n
		] \n
		presence = 0.3 \n \n
		Output : result = ( ( 0.2423<average_score> + 0.0384<average_length> + 0.3<presence> )/3 ) = 0.1935
	*/
	Query(vector <Parameter> &,double);
};

