#include "CompressedPage.h"

using namespace std;

CompressedPage::CompressedPage(){}
CompressedPage::CompressedPage(unsigned int index){
	this->index= index ;
	this->emission=1;
	this->link_sample=1;
}

void CompressedPage::print(){
	cout << "index:" << this->index << "   emission:" << this->emission << "   link_sample" << this->link_sample << endl;
}


