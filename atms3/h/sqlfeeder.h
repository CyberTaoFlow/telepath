#ifndef _sqlfeeder_h
#define _sqlfeeder_h
//------------------------!

#include <vector>
#include <map>
#include <stdarg.h>
#include "../h/atmstypes.h"
#include "../h/log.h"
#include "../h/hregex.h"
#include <my_global.h>
#include <mysql.h>


#include "config.h"

/*
 * DAL
 * Wrapping functions for MySQL and Postgres
 */

//#define DAL_POSTGRES

#ifdef DAL_POSTGRES
	#include <libpq-fe.h>
#endif


using namespace std;

/*
 * SQLFeeder object provides database wrapping functions for MySQL and Postgres.
 * Contains all database functions
 */

class SQLFeeder
{
//connection parameters
	string db_addr, db_username, db_psswd, db_name, db_type;

//MYSQL variables - for MYSQL DAL layer
	MYSQL_RES *MYSQL_res;
	MYSQL_ROW row;
	MYSQL mysql;
	MYSQL *MYSQL_conn;
	unsigned long long last_selected_row;

//POSTGRES variables - for POSTGRES DAL layer
#ifdef DAL_POSTGRES
	PGconn     *PG_conn;
	PGresult   *PG_res;
#endif

	unsigned long long rows_count, fields_count;

protected:
//Wrapping functions for native libmysql functions
	int DAL_MYSQL_select(const char * q);
	void DAL_MYSQL_query(const char * q);
	void DAL_MYSQL_clear_result();
	char* DAL_MYSQL_get_cell_value(unsigned long long, unsigned long long);

//Wrapping functions for native postgres functions
	int DAL_POSTGRES_select(const char * q);
	void DAL_POSTGRES_query(const char * q);
	void DAL_POSTGRES_clear_result();
	char *DAL_POSTGRES_get_cell_value(unsigned long long, unsigned long long );

//Pointers to corresponding functions
	int (SQLFeeder::*DAL_Select_pointer)(const char *);
	void (SQLFeeder::*DAL_query_pointer)(const char *);
	void (SQLFeeder::*DAL_clear_result_pointer)();
	char* (SQLFeeder::*DAL_get_cell_value_pointer)(unsigned long long, unsigned long long);

	void open_sql_connection(string,string,string,string);
	void close_sql_connection();

public:

	SQLFeeder();
	SQLFeeder(string, string, string, string, string);
	SQLFeeder(map<string,string> &);
	~SQLFeeder();

	void sqlError(const char * q);
	short sqlError(string q,short duplicateF);

//Database type transparent functions
	int DAL_select(const char * q);
	void DAL_query(const char * q);
	void DAL_clear_result();
	char* DAL_get_cell_value(unsigned long long, unsigned long long);

//called from addobject()
	//void insert_session_data(string, TeleObject &);

//called from TeleCache()
	void get_app_groupheaders(map<string, struct AppData >&);
	void get_url_detect_regex(string&);

	void get_op_mod_logtodb(int&,u64&);

//ATMS
	void get_dontmappage(string&);
	void get_all_agent_conf(set<string>&,bool);
	void get_agent_conf(string, string&);
	void select_sqlblacklist(string&);

	string get_regex(string);

	void get_list_of_agents(map<string, struct AgentStatus>&);
	void get_list_of_filters(map<string, struct FilterStatus>& );
	void get_loadbalance_settings(size_t&, set<string>&, vector<string> &);
	void get_apps_list(unsigned int *iAddApp, map<class AppKey, struct AppData, AppMatchPredicate> & mAD, map<string, AppIpData> & mAppIPData, set<unsigned int> & mSslPorts);

// thread_sock_receive()
	void update_agent_stat(string &, const char * status);
	void update_filter_stat(string &, const char * status);
	void update_revision(const char* svn_revision);

//thread_agent_conf()
	void get_atms_config_flag(set<int> &);

//init_atms()
	void get_excluded_ip(set<string>&);
	unsigned long long add_app(const string & appname);

//gui_thread()
	void sqllog(string &, string &);
	


/*
	void query2(string&);
	void select_query(char*);
	void delete_query(string);
	void delete_query(char*);
	void insert_query(string);
	void update_query(string);
	void clear_result();
*/

};


#endif
