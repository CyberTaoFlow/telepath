#include "Probability.h"

using namespace std;


Probability::Probability(){

}
Probability::Probability(double prob,int exp)
{
	this->prob=prob;
	this->exp=exp;
	this->betterRepresentation();
}

void Probability::setProbability(double prob,int exp)
{
	this->prob = prob;
	this->exp = exp;
	this->betterRepresentation();
}

double Probability::normalized()
{
	return (this->prob * pow(2,this->exp) );
}

unsigned int Probability::operator==(Probability p)
{
	if(this->prob == p.prob && this->exp == p.exp)
		return 1;
	return 0;
}

void Probability::print()
{
	cout<<"("<<this->prob<<","<<this->exp<<")";
}

void Probability::betterRepresentation()
{
	while(this->prob < 0.01 || this->exp>0 ){ // better Representation for probabilities.
		if(this->prob==0)
			break;
		this->exp--;
		this->prob*=2;

	}
}


