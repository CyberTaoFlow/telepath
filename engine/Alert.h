
#include <string>

using namespace std;

class AlertValToInsert{
public:
	unsigned long long RID;
	string rule_name;
	float score;
	string shard;
	unsigned int counter;

	AlertValToInsert();
        AlertValToInsert(unsigned long long,string,unsigned int,string &);
	void print();
};

class SessionAlertValToInsert : public AlertValToInsert{
public:
	SessionAlertValToInsert();
	SessionAlertValToInsert(unsigned long long,string &,unsigned int,string &);
	void print();
};

