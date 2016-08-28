
#define DECIDE_THREADS 1 
#define NUM_OF_GET_ATT 16
#define NUM_OF_MARKOV_THREADS 16 
#define MIN_VALUES 25
#define INSERT_SUSPECT 15 
#define INSERT_REQ_AT_ONCE 1000
#define INSERT_ATT_AT_ONCE 50 
#define MAX_PERCENT 97
#define COOKIE_ANALYSIS_SAMPLES 10000
#define NUM_OF_COOKIE_SUGGESTIONS 10
#define	COEFFICIENT 0.0007
#define LOAD_RULES_BULK 300
#define LOAD_BOTS_BULK 575

using namespace std;

class ElasticData{
public:
	string action;
	string alerts;
	unsigned int counter_alerts;
	double threshold;
	boost::unordered_map<string,boost::unordered_set<unsigned int> > rule_names;

	ElasticData(){
		this->action.clear();
		this->alerts.clear();
		this->counter_alerts = 0;
		this->threshold=0;
	}
};

class ElasticAtt{
public:
	string attributes;
	unsigned short counter_atts;

	ElasticAtt(){
		attributes.clear();
		counter_atts = 1;
	}
};

TeleCache * TC;
Numeric   * markov_Length;
myAtt     * markov_Att;


string license_key;
string app_was_changed;

extern pthread_mutex_t mutexTeleObjQueue;
extern boost::unordered_map <char,string> const_types;
extern boost::unordered_map <string,unsigned short> global_header;
extern boost::unordered_set<string> domain_header;
extern boost::unordered_set<string>const_method;


unsigned int getRedis=0;
unsigned int insertElastic=0;
unsigned int dropApp=0;
unsigned int dropMethod=0;
unsigned int dropPage=0;
unsigned int dropURI=0;
unsigned int longSessionDrop=0;

double max_dist;
float noisePercent;
float opClusterPercent;
float operationPercent;

unsigned short write_to_syslog;
unsigned short addUnknownApp;

unsigned int moveToProductionAfter; // move to production after this number of requests.
unsigned int markov_i;
unsigned int noisyFlag=1;
unsigned int globalEngine=1; // Engine Mode.
unsigned short rules_table_was_changed;
unsigned short business_flow_was_changed;
unsigned short enable_weight;
unsigned short password_masking;
unsigned short cc_masking;
unsigned int sessionDelete=0;
unsigned int timeOutFlag=0;
unsigned int UserID_counter;
unsigned short slowOrFastLanding; //   0=Analyze Slow landing speed.  , 1=Analyze all kind of landing speed. , 2=Analyze fast landing speed.
unsigned short bot_intelligence;
//unsigned int maxSession;
unsigned int numRulesRotate;

pcre *cc_regex;

//pthread_mutex_t mutexDynamicCheck    = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutexDynPages        = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutexmarkovSession   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexMSatt           = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexOperation       = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSessions        = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexgetatt5         = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexBusinessLogic   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexscorenumericatt = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSidPerMin       = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexmarkov          = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexgetatt          = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexgetattPro       = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexTeleObjQueue    = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexInsertReq       = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVecCase         = PTHREAD_MUTEX_INITIALIZER; // mutex for vCases
pthread_mutex_t mutexlocation        = PTHREAD_MUTEX_INITIALIZER; // mutex for location.
pthread_mutex_t mutexScoreData       = PTHREAD_MUTEX_INITIALIZER;  // mutex for score_data object.
pthread_mutex_t mutex_mQuery         = PTHREAD_MUTEX_INITIALIZER;  // mutex for mQuery object.
pthread_mutex_t mutexLongTermRules   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexInsertFlowState = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexRepIPs          = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexRepUsers        = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexMyAttMap        = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexMyAttUserMap    = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSessionsPerHost = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexLogin           = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexLogout          = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSearch          = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexgeoip           = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexAddRules        = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexAddActions      = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexCommand         = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexCaptcha         = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexUnknownApp      = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexAppMode	     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexHashRules       = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexRidAtts         = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexRidAlertAndBusiness    = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t  getatt_cond    = PTHREAD_COND_INITIALIZER;
pthread_cond_t  getatt_condPro = PTHREAD_COND_INITIALIZER;

sem_t sem_insert_logins;
sem_t sem_markov_att_start;
sem_t sem_markov_att_end;
sem_t sem_markov_session_start;
sem_t sem_markov_session_end;
sem_t sem_tokenize1;
sem_t sem_tokenize2;
sem_t sem_log_iis;
sem_t sem_log_apache;
sem_t sem_log_json;
sem_t sem_insert_reqs;
sem_t sem_session_alerts;
sem_t sem_insert_attributes;
sem_t sem_command;
sem_t sem_captcha;

vector <char> markovAtt_values_flag;
vector <Session*> markovSession_;
vector <char> markovSession_values_flag;
vector <Path*> markovSession_path;
vector <Path*> markovSession_path_user;
vector <short> markovSession_mode;
vector <char> learning_values_flag;
vector <char> production_values_flag;
vector <LogFile> apache_logs;
vector <LogFile> iis_logs;
vector <Rule> rules;
//vector < Case > vCases;
vector <unsigned int> addRulesIDs;
vector <unsigned int> addActionIDs;

boost::unordered_set <unsigned int> setUserIDs;

set <unsigned int> sessionPerMinute;
set <unsigned int> MSatt;

queue <Login> loginQueue;
queue <Logout> logoutQueue;
queue <Search> searchQueue;
queue <string> valCommandQueue;
queue <string> valCaptchaQueue;



boost::unordered_set <unsigned int> setHashRules;
boost::unordered_set <unsigned int> setHashMasks;
bool noRulesToLoad=false;
bool noAppsToLoad=false;
bool noBotsToLoad=false;
vector <Action> businessFlowVec;


map <string,rule_group_data> mRulesGroup;
map <string,rule_group_data>::iterator itRulesGroup;

boost::unordered_map <string,AppMode> mAppMode;



boost::unordered_map <long long,ElasticAtt> mRidAtts;// save all Session data.
boost::unordered_map <long long,ElasticData> mRidAlertAndAction;// save all Session data.
//boost::unordered_map <string,BotIntelligence> mBotIntelligenceIP; // BotIntelligence For IPs
boost::unordered_set <string> sTorIntelligenceIP;
boost::unordered_set <string> sBotIntelligenceIP;
boost::unordered_set <long long> sRidSaveDB;
//boost::unordered_map <string,BotIntelligence>::iterator itBotIntelligence;
boost::unordered_map <unsigned int,boost::unordered_map <string,UserReputation> > mReputationUsers;
boost::unordered_map <unsigned int,Reputation> mReputationIPs;
map<unsigned int,Path> mPathPerGroup; // Path to any suspicious group.
map<unsigned int,Path> ::iterator itPathPerGroup;// iterator of mPathPerGroup.
map <string,string>::iterator itApp;
boost::unordered_map <unsigned int,struct Attribute>::iterator itAttUser;
boost::unordered_map <unsigned int,double>  mAttAvg;
boost::unordered_map <unsigned int,vector <Parameter> > mQuery;  // Query probability for RID.
boost::unordered_map <unsigned int,vector <Parameter> > mQueryUser;  // User query probability for RID.
map <unsigned int,myAtt> myAttMap;// save all attribute data.
map<string,myAtt> myAttUserMap;// save all attribute users data.
map<string,Path> mPath; // Path to any hostname.
map<string,Path> mPathUser; // Path to any user.
boost::unordered_map <unsigned int,Session> mSession;// save all Session data.


map<string,unsigned int> numOfSessionsPerDomain; // count how many sessions for each application. 
map<string,unsigned int>::iterator itSessionsPerDomain;



map <unsigned int, Operation > mOperation;
map <long long, OpRID > mOpRIDs;

map <string,string> config_data; // store database_address,username,password and database_name.


//--------Numeric Objects For Rules-----------------
map <string,ScoresData> score_data;
map <string,ScoresNumeric> score_numeric;
map <unsigned int,ScoreAtt> mScoreAtt;
map <unsigned int,ScoreNumericAtt> mScoreNumericAtt;
//--------------------------------------------------

multimap <unsigned int,string > topCookies;


using namespace std;



