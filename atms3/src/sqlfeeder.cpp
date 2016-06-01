#include "../h/sqlfeeder.h"
//#define DEBUG
//------------------------------------
//---connection parameters------------

//extern string log_path;
#define SQL_CONN_TRIES 10
//-----------------------------------
//-----------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#undef min
#undef max
#include <algorithm>

#ifndef WIN32
#include <arpa/inet.h>
#endif

using namespace std;

void beforeDecode(string & s){
	std::size_t found;
	while(1){
		found = s.find("%%");
		if (found!=std::string::npos){
			s.erase(s.begin()+found);
		}else{
			break;
		}
	}
}


const char HEX2DEC[256] =
{
    /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
    /* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

    /* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

    /* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

    /* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};


std::string uriDecode(const std::string & sSrc){


	// Note from RFC1630: "Sequences which start with a percent
	// sign but are not followed by two hexadecimal characters
	// (0-9, A-F) are reserved for future extension"

	const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
	const int SRC_LEN = sSrc.length();
	const unsigned char * const SRC_END = pSrc + SRC_LEN;
	// last decodable '%'
	const unsigned char * const SRC_LAST_DEC = SRC_END - 2;

	char * const pStart = new char[SRC_LEN];
	char * pEnd = pStart;

	while (pSrc < SRC_LAST_DEC)
	{
		if (*pSrc == '%')
		{
			char dec1, dec2;
			if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])	&& -1 != (dec2 = HEX2DEC[*(pSrc + 2)]) )
			{
				*pEnd++ = (dec1 << 4) + dec2;
				pSrc += 3;
				continue;
			}
		}

		*pEnd++ = *pSrc++;
	}

	// the last 2- chars
	while (pSrc < SRC_END)
		*pEnd++ = *pSrc++;

	std::string sResult(pStart, pEnd);
	delete [] pStart;
	return sResult;
}



extern RSA * loadRSA(std::string & priv_key, std::string & pwd);

SQLFeeder::SQLFeeder()
{
	MYSQL_conn = NULL;
}
/*
 * SQLFeeder constructor takes list of parameters(made for ATMS).
 * Creates connection depended on database type.
 * Sets pointers to appropriate database type functions.
 */
SQLFeeder::SQLFeeder(map<string,string> &c)
{
#ifdef DEBUG
		printf("\nSQLFeeder::SQLFeeder(map<string,string> *c)\n");
#endif

	db_addr=c["database_address"];
	db_username=c["username"];
	db_psswd=c["password"];
	db_name=c["database_name"];
	db_type=c["database_type"];
//	cout << "db_addr "<< db_addr<<"; db_username "<<db_username<<"; db_psswd "<<db_psswd<<"; db_name "<<db_name<<endl;

	cout<<"database_type: "<<db_type<<endl;
	if(db_type.size() == 0 || db_type=="MYSQL")
	{
		open_sql_connection(db_addr,db_username,db_psswd,db_name);

		DAL_Select_pointer=&SQLFeeder::DAL_MYSQL_select;
		DAL_get_cell_value_pointer=&SQLFeeder::DAL_MYSQL_get_cell_value;
		DAL_clear_result_pointer=&SQLFeeder::DAL_MYSQL_clear_result;
		DAL_query_pointer=&SQLFeeder::DAL_MYSQL_query;
		
		DAL_query("SET NAMES UTF8;");
		last_selected_row=0;

		//cout<<"pointer initialized"<<endl;
	}
#ifdef DAL_POSTGRES
	else if(db_type=="POSTGRESQL")
	{

	  //conn=PQsetdbLogin(const char *pghost,const char *pgport,const char *pgoptions,const char *pgtty,const char *dbName,const char *login,const char *pwd);
		PG_conn = PQsetdbLogin(db_addr.c_str(),"","","",db_name.c_str(),db_username.c_str(),db_psswd.c_str());
	    if (PQstatus(PG_conn) != CONNECTION_OK)
	    {
	        fprintf(stderr, "Connection to database failed: %s",
	        PQerrorMessage(PG_conn));
	        PQfinish(PG_conn);
	        exit(1);
	    }

		DAL_Select_pointer=&SQLFeeder::DAL_POSTGRES_select;
		DAL_get_cell_value_pointer=&SQLFeeder::DAL_POSTGRES_get_cell_value;
		DAL_clear_result_pointer=&SQLFeeder::DAL_POSTGRES_clear_result;
		DAL_query_pointer=&SQLFeeder::DAL_POSTGRES_query;
	}
#endif
}

/*
 * SQLFeeder constructor takes set of parameters.
 * Creates connection depended on database type.
 * Sets all variables and pointers to appropriate database type functions.
 */
SQLFeeder::SQLFeeder(string db_addr, string u_name, string passwd, string db_name, string type)
{
#ifdef DEBUG
		printf("\nSQLFeeder::SQLFeeder(string db_addr, string u_name, string passwd, string db_name)\n");
#endif
		//mTable["learning_data"]=Table("learning_data",&DBConn);

	this->db_addr=db_addr;
	this->db_username=u_name;
	this->db_psswd=passwd;
	this->db_name=db_name;
	this->db_type=type;

	if(db_addr.size()==0 || u_name.size()==0 || db_name.size()==0)
		cout<<"Database address, name or username is empty"<<endl;

	if(this->db_type=="MYSQL")
	{
		DAL_Select_pointer=&SQLFeeder::DAL_MYSQL_select;
		DAL_get_cell_value_pointer=&SQLFeeder::DAL_MYSQL_get_cell_value;
		DAL_clear_result_pointer=&SQLFeeder::DAL_MYSQL_clear_result;
		DAL_query_pointer=&SQLFeeder::DAL_MYSQL_query;

		open_sql_connection(db_addr, u_name, passwd, db_name);
		last_selected_row=0;
	}
#ifdef DAL_POSTGRES
	else if(this->db_type=="POSTGRESQL")
	{

	  //conn=PQsetdbLogin(const char *pghost,const char *pgport,const char *pgoptions,const char *pgtty,const char *dbName,const char *login,const char *pwd);
		PG_conn = PQsetdbLogin(db_addr.c_str(),"","","",db_name.c_str(),db_username.c_str(),db_psswd.c_str());
		if (PQstatus(PG_conn) != CONNECTION_OK)
		{
			fprintf(stderr, "Connection to database failed: %s",
			PQerrorMessage(PG_conn));
			PQfinish(PG_conn);
			exit(1);
		}

		DAL_Select_pointer=&SQLFeeder::DAL_POSTGRES_select;
		DAL_get_cell_value_pointer=&SQLFeeder::DAL_POSTGRES_get_cell_value;
		DAL_clear_result_pointer=&SQLFeeder::DAL_POSTGRES_clear_result;
		DAL_query_pointer=&SQLFeeder::DAL_POSTGRES_query;
		cout<<"POSTGRES constructor worked"<<endl;

	}
#endif
}

/*
 * Opens SQL connection, sets all MySQL variables
 *
 */
void SQLFeeder::open_sql_connection(string db_addr, string u_name, string passwd, string db_name)
{

#ifdef DEBUG
		printf("\nSQLFeeder::open_sql_connection()\n");
#endif

	MYSQL_conn = mysql_init(NULL);
	while(MYSQL_conn == NULL) {sqlError("");}

	//tries to connect SQL_CONN_TRIES times
	for(int i=0; i<SQL_CONN_TRIES; i++)
	{
		if (mysql_real_connect(MYSQL_conn, db_addr.c_str(), u_name.c_str(), passwd.c_str(), db_name.c_str(), 0, NULL, CLIENT_MULTI_STATEMENTS) == NULL)
		{
			sqlError("");
		}
		else
			break;
		if(i==SQL_CONN_TRIES-1)
			//log((char*)"atmsd: ERROR: Can't connect to database\n",log_path.c_str());
			syslog (LOG_NOTICE, "%s","atmsd: ERROR: Can't connect to database\n");
	}
}

/*
 * void SQLFeeder::close_sql_connection()
 * 		Closes connection to MySQL database. Uses native libmysql functions
 * Parameters:
 * 		No
 * Return value:
 * 		No
 */
void SQLFeeder::close_sql_connection()
{
#ifdef DEBUG	
	printf("\nSQLFeeder::close_sql_connection()\n");
#endif

	mysql_close(MYSQL_conn);
	MYSQL_conn = NULL;
}

/*
 * =============DAL WRAPPING FUNCTIONS============================================================================================
 * ==========================================================================================================
 */
/*
 * int SQLFeeder::DAL_select(string q)
 * 		Makes SELECT request to database
 * Parameters:
 * 		q - request
 * Return value:
 * 		0 - if select returned not empty set.
 */
int SQLFeeder::DAL_select(const char * q)
{
#ifdef DEBUG
	printf("\nSQLFeeder::DAL_select(string q)\n");
#endif
	if ((this->*DAL_Select_pointer)(q)==1)
		return 1;
	else return 0;

#ifdef DEBUG
	printf("\nSQLFeeder::DAL_select(string q) end \n");
#endif
}

/*
 * char* SQLFeeder::DAL_get_cell_value(unsigned long long row_num, unsigned long long field_num)
 * 		Returns data from cell[row_num, field_num]
 * 		Can be used after SELECT statement
 * Parameters:
 * 		row_num - row number of cell in dataset
 * 		field_num - column number of cell in dataset
 * Return value:
 * 		Pointer to fetched data.
 */

char* SQLFeeder::DAL_get_cell_value(unsigned long long row_num, unsigned long long field_num)
{
#ifdef DEBUG
	printf("\nSQLFeeder::DAL_get_cell_value(unsigned long long row_num, unsigned long long j) %llu %llu\n", row_num, j);
#endif

	return (this->*DAL_get_cell_value_pointer)(row_num,field_num);
}


/*
 * void SQLFeeder::DAL_query(string q)
 * 		Makes UPDATE, DELETE, INSERT, etc. requests(which are not return dataset) to database
 * Parameters:
 * 		q - request
 * Return value:
 * 		Doesn't return value
 */
void SQLFeeder::DAL_query(const char * q)
{
#ifdef DEBUG
	printf("\nSQLFeeder::DAL_select(string q)\n");
#endif

	(this->*DAL_query_pointer)(q);

#ifdef DEBUG
	printf("\nSQLFeeder::DAL_select(string q) end \n");
#endif
}

/*
 * void SQLFeeder::DAL_clear_result()
 * 		Frees memory after SELECT statement. Must be used after successful int SQLFeeder::DAL_select(string q)
 * Parameters:
 * 		No
 * Return value:
 * 		Doesn't return value
 */
void SQLFeeder::DAL_clear_result()
{
	(this->*DAL_clear_result_pointer)();
}
/*
 * ==========================================================================================================
 * ==========================================================================================================
 */



/*
 * =====================DAL MYSQL FUNCTIONS==================================================================
 * ==========================================================================================================
 */

/*
 * int SQLFeeder::DAL_MYSQL_select(string q)
 * 		Making SELECT query.
 * Parameters:
 * 		string q - query to run
 * Return value:
 * 		int - 0 on success; -1 - otherwise
 */
int SQLFeeder::DAL_MYSQL_select(const char * q)
{
#ifdef DEBUG
	printf("\nSQLFeeder::DAL_select(string q)\n");
#endif

	while(mysql_query(MYSQL_conn, q))
		{sqlError(q);}
	if (!(MYSQL_res = mysql_store_result(MYSQL_conn)))	//storing result
	{
		return 1;
	}
	rows_count=mysql_num_rows(MYSQL_res);	//fetching row_count

	if(rows_count == 0)			//if row_count != 0 - we have some result
	{
		return 1;
	}
	fields_count=mysql_num_fields(MYSQL_res);	//fetching fields(columns) count

	//setting row to 0th line, (something like precaching)
	mysql_data_seek(MYSQL_res, 0);
	last_selected_row=0;

	row = mysql_fetch_row(MYSQL_res);

	return 0;

#ifdef DEBUG
	printf("\nSQLFeeder::DAL_select(string q) end \n");
#endif
}

/*
 * void SQLFeeder::DAL_MYSQL_query(string q)
 * 		Making query: UPDATE, INSERT, DELETE etc. Which doesn't return value
 * Parameters:
 * 		string q - query to run
 * Return value:
 * 		No
 */
void SQLFeeder::DAL_MYSQL_query(const char * q)
{
#ifdef DEBUG
	printf("\nvoid SQLFeeder::DAL_MYSQL_update(string q))\n");
#endif
	while(mysql_query(MYSQL_conn, q ))
	{sqlError(q);}
}


/*
 * char* SQLFeeder::DAL_MYSQL_get_cell_value(unsigned long long row_num, unsigned long long field_num)
 * 		Get exact value from result.
 * Parameters:
 * 		unsigned long long row_num 		-	from which row to select
 * 		unsigned long long field_num	-	from which column to select
 * Return value:
 * 		char*	-	exact value
 */
char* SQLFeeder::DAL_MYSQL_get_cell_value(unsigned long long row_num, unsigned long long field_num)
{
#ifdef DEBUG
	printf("\nSQLFeeder::DAL_MYSQL_get_cell_value(unsigned long long row_num, unsigned long long j) %llu %llu\n", row_num, j);
#endif

	//checking if desired row and column exist in result
	if(row_num>=this->rows_count || field_num>=this->fields_count)
	{
		cout<<"Wrong request to row or field: you are trying to access row or field index, which does not exist"<<endl;
		exit(1);
	}
	if(last_selected_row!=row_num)
	{
		last_selected_row=row_num;			//saving last selected row
		mysql_data_seek(MYSQL_res, row_num);	//looking for row in result
		row = mysql_fetch_row(MYSQL_res);	//fetching row from result
	}
	return row[field_num];//returning value
}


/*
 * void SQLFeeder::DAL_MYSQL_clear_result()
 * 		Clearing fetch result
 * Parameters:
 * 		No
 * Return value:
 * 		No
 */
void SQLFeeder::DAL_MYSQL_clear_result()
{
	while(1)
	{
		if(MYSQL_res)
			mysql_free_result(MYSQL_res);
		if(mysql_next_result(MYSQL_conn)==-1)
			break;
		else
			MYSQL_res=mysql_store_result(MYSQL_conn);
	}
	rows_count=-1;
	fields_count=-1;
}
/*
 * ==========================================================================================================
 * ==========================================================================================================
 */



#ifdef DAL_POSTGRES
/*
 * =============DAL POSTGRES FUNCTIONS=======================================================================
 * ==========================================================================================================
 */

/*
 * int SQLFeeder::DAL_POSTGRES_select(string q)
 * 		Makes SELECT request to Postgres
 * Parameters:
 * 		q - request
 * Return value:
 * 		0 - if select returned not empty set.
 */
int SQLFeeder::DAL_POSTGRES_select(const char * q)
{
    PG_res = PQexec(PG_conn, q);				//executing request
    if (PQresultStatus(PG_res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "FETCH ALL failed: %s", PQerrorMessage(PG_conn));
        PQclear(PG_res);
        PQfinish(PG_conn);
        exit(1);
    }

    rows_count=PQntuples(PG_res);		//saving rows count. If count == 0, empty result was returned
    if(rows_count == 0)
    {
        return -1;
    }
    fields_count= PQnfields(PG_res);	//saving columns count

    return 0;
}

/*
 * char * SQLFeeder::DAL_POSTGRES_get_cell_value(unsigned long long row_num, unsigned long long field_num)
 * 		Fetching exact cell value
 * Parameters:
 * 		unsigned long long row_num
 * 		unsigned long long field_num
 * Return value:
 * 		char*
 */
char * SQLFeeder::DAL_POSTGRES_get_cell_value(unsigned long long row_num, unsigned long long field_num)
{
	if(row_num>=this->rows_count || field_num>=this->fields_count)
	{
		cout<<"SQLFeeder::DAL_POSTGRES_get_cell_value:: Wrong request to row or field: you are trying to access row or field index, which not exists"<<endl;
		return NULL;
	}
	return PQgetvalue(PG_res, row_num, field_num);
}

/*
 * void SQLFeeder::DAL_POSTGRES_query(string q)
 * 		Executing request to database: UPDATE, INSERT, DELETE etc. Which doesn't return value
 * Parameters:
 * 		string q - request
 * Return value:
 * 		No
 */
void SQLFeeder::DAL_POSTGRES_query(const char * q)
{
	PG_res = PQexec(PG_conn, q);			//executing request to database
	if(PQresultStatus(PG_res)!=PGRES_COMMAND_OK)
	{
		printf("Postgresql query error: %s", PQresultErrorMessage(PG_res));
	}
}

/*
 * void SQLFeeder::DAL_POSTGRES_clear_result()
 * 		Clear memory of saved result
 * Parameters:
 * 		No
 * Return value:
 * 		No
 */
void SQLFeeder::DAL_POSTGRES_clear_result()
{

	PQclear(PG_res);
	rows_count=fields_count=-1;
}
#endif
/*
 * ==========================================================================================================
 * ==========================================================================================================
 */

/*
 * string SQLFeeder::get_regex(string AgentName)
 * 		Getting exclude data from agent_regex table for agent
 * Parameters:
 * 		string AgentName
 * Return value:
 * 		string - configuration to send to agent
 */
string SQLFeeder::get_regex(string AgentName)
{
#ifdef DEBUG
	printf("string SQLFeeder::get_regex()\n");
#endif


//	result="SELECT agent_id, ServerIP, AgentPort, MaxHeadersLength, MaxGetParamsLength, MaxDefaultLength, StatusInterval, newConfig, CookieExpireTime, SessionCookie, ConcurrentSessions, MessagesQueueSize, SessionCookieName, NetworkInterface  FROM agents WHERE agent_id='"+agent+"';";
	string quer;
	string q;
	string sTmp;
	string sResult;
	string sResult2;

	char digit[20];
	vector<string> v;
	int i=0;


	//getting list of attributes to exclude from table agents_regex
	quer="SELECT att_id FROM agents_regex WHERE agent_id='"+AgentName+"';";
	if(DAL_select(quer.c_str())==0)
	{
		for(unsigned long long i=0; i<rows_count; ++i)
		{
			sTmp=string(DAL_get_cell_value(i,0));
			if(sTmp.length()>0)
				v.push_back(sTmp);
		}
		DAL_clear_result();
	}
	else
	{
		cout << "string SQLFeeder::get_regex()"<<endl;
		cout << "SELECT att_id FROM agents_regex WHERE agent_id='"+AgentName<<endl;
	}

	if (v.size()!=0)
	{
		for(vector<string>::iterator vi=v.begin();vi!=v.end();++vi)
		{
			q+=string("att_id='")+*vi+string("' OR ");
		}
		if(q.size())
		{
			if( (q.size()-3) < q.size() ){
				q.erase(q.size()-3,4);
			}
			q+=string(");");
		}

		//getting all needed parameters for excluded attributes
		quer=string("SELECT att_name, att_source, pages.path FROM attributes, pages WHERE attributes.page_id=pages.page_id AND (")+q;
		sTmp.clear();
		sResult.clear();

		if(DAL_select(quer.c_str())==0)
		{
			for(unsigned long long ii=0; ii<rows_count; ++ii)
			{
				memset(digit,'\0',sizeof(digit));
				sprintf(digit,"%d",i);
				sTmp+=xml_node("Name",string(DAL_get_cell_value(ii,0)));
				sTmp+=xml_node("Type",string(DAL_get_cell_value(ii,1)));
				sTmp+=xml_node("Page",string(DAL_get_cell_value(ii,3)));
				sResult+=xml_node(string("Param")+string(digit),sTmp);
				sTmp.clear();
				++i;
			}

			memset(digit,'\0',sizeof(digit));
			sprintf(digit,"%d",i);
			sResult=xml_node("Param_Quantity",string(digit))+sResult;

			DAL_clear_result();
		}
		else
		{
			sResult=xml_node("Param_Quantity",int_to_string(0));
		}
	}
	else
		sResult=xml_node("Param_Quantity",int_to_string(0));

	sTmp.clear();
	i=0;

	quer="SELECT header_name, regex, cond FROM agents_regex WHERE agent_id='"+AgentName+"';";

	if(DAL_select(quer.c_str())==0)
	{
		for(unsigned long long ii=0; ii<rows_count; ++ii)
		{
			memset(digit,'\0',sizeof(digit));
			sprintf(digit,"%d",i);
			sTmp+=xml_node("Type",string(DAL_get_cell_value(ii,0)));
			sTmp+=xml_node("RegexExpression",string(DAL_get_cell_value(ii,1)));
			sTmp+=xml_node("Condition",string(DAL_get_cell_value(ii,2)));
			sResult2+=xml_node(string("Field")+string(digit),sTmp);
			sTmp.clear();
			++i;
		}

		memset(digit,'\0',sizeof(digit));
		sprintf(digit,"%d",i);
		sResult2=xml_node("Field_Quantity",string(digit))+sResult2;

		DAL_clear_result();
	}
	else
	{
		sResult2=xml_node("Field_Quantity",int_to_string(0));
	}

	return sResult+sResult2;

#ifdef DEBUG
	printf("string SQLFeeder::get_regex() - finish \n");
#endif
}


/*
 * void SQLFeeder::get_dontmappage(string& r)
 * 		Read dontMapPage parameter. (It's status code.)
 * Parameters:
 * 		string& r - string passed by reference for result
 * Return value:
 * 		No
 */
void SQLFeeder::get_dontmappage(string& r)
{
	static const char * q = "SELECT value FROM config WHERE name='dontMapPage';";
	if( DAL_select(q)==0)
	{
		r= string(DAL_get_cell_value(0,0));
		DAL_clear_result();
	}
	else
	{
		cout << "void SQLFeeder::get_dontmappage(string& r)"<<endl;
		cout << "Request returned 0 rows"<<endl;
		cout << "SELECT value FROM `config` WHERE name='dontMapPage';"<<endl;
	}
}

/*
 * void SQLFeeder::get_all_agent_conf(set<string> &sConfigs)
 * 		Function to get configurations of all agents
 * Parameters:
 * 		set<string> &sConfigs - all configurations
 * Return value:
 * 		No
 */

/*void SQLFeeder::get_all_agent_conf(set<string> &sConfigs,bool get_only_edited)
{
	static const char * query="SELECT newConfig, agent_id FROM agents WHERE 1=1;";
	string q;
	set<string> AgentToConf;

//reading table "agents" and save agent_id, for which newConfig==1
	if (DAL_select(query)==0)
	{
		for(unsigned long long i=0; i<rows_count; ++i)
		{
			if (get_only_edited){
				if(string(DAL_get_cell_value(i,0))=="1")
				{
					AgentToConf.insert(string(DAL_get_cell_value(i,1)));
				}
			}
			else{
				AgentToConf.insert(string(DAL_get_cell_value(i,1)));
			}
		}
		DAL_clear_result();

		//for each agent_id, read configuration from database and insert into resulting set
		for(set<string>::iterator i=AgentToConf.begin(); i!=AgentToConf.end(); ++i)
		{
			get_agent_conf(*i, q);
			sConfigs.insert(q);
		}
	}
	else
	{
		cout << "void SQLFeeder::get_all_agent_conf(set<string> &sConfigs) "<<endl;
		cout << "SELECT newConfig, agent_id FROM agents WHERE 1=1;"<<endl;
	}
}*/


/*
 * int SQLFeeder::get_agent_conf(string agent, string &result)
 * 		Builds configuration string for desired agent
 * Parameters:
 * 		string agent	-	agent id
 * 		string &result	-	string with result
 * Return value:
 * 		No
 */

/*void SQLFeeder::get_agent_conf(string agent, string &result)
{
//	static unsigned int s=0;
//	srand( (unsigned)time( NULL ) +s);
//	++s;

	string q, fingerprint_headers, sTmp, sList;
	string update_q;
	char rand_id[11];
	string query = "SELECT FilterExpression ,agent_id, ATMSIP, ATMSPort, MaxHeadersLength, MaxGetParamsLength, MaxPostLength, StatusInterval, CookieExpireTime, ConcurrentSessions, MessagesQueueSize, NetworkInterface, fingerprint_headers  FROM agents WHERE agent_id='";
	query.append(agent);
	query.append("';");
	//selecting all parameters from "agents" table
	if (DAL_select(query.c_str())==0)
	{
		q.clear();
		sprintf(rand_id,"%d",rand());
		q+=xml_node("ConfigID",string(rand_id));

		q+=xml_node("FilterExpression",string(DAL_get_cell_value(0,0)));
		q+=xml_node("AgentID",string(DAL_get_cell_value(0,1)));
		q+=xml_node("ATMSIP",string(DAL_get_cell_value(0,2)));
		q+=xml_node("ATMSPort",string(DAL_get_cell_value(0,3)));
		q+=xml_node("MaxHeadersLength",string(DAL_get_cell_value(0,4)));
		q+=xml_node("MaxGetParamsLength",string(DAL_get_cell_value(0,5)));
		q+=xml_node("MaxPostLength",string(DAL_get_cell_value(0,6)));
		q+=xml_node("StatusInterval",string(DAL_get_cell_value(0,7)));
		q+=xml_node("CookieExpireTime",string(DAL_get_cell_value(0,8)));
		//q+=xml_node("SessionCookie",parse_str(result));
		q+=xml_node("ConcurrentSessions",string(DAL_get_cell_value(0,9)));
		q+=xml_node("MessagesQueueSize",string(DAL_get_cell_value(0,10)));
		q+=xml_node("NetworkInterface",string(DAL_get_cell_value(0,11)));

		fingerprint_headers=string(DAL_get_cell_value(0,12));

		DAL_clear_result();

		int i=0;
		string sI, sCookieList,AppName, strCookie,cpt_name,cpt_val;

//selecting all cookie's names for all applications from table "applications" and adding to configuration
		if (DAL_select("SELECT app_domain,AppCookieName,cpt_name,cpt_val FROM applications WHERE 1=1")==0)
		{
			for(unsigned long long ii=0; ii<rows_count; ++ii)
			{
				sI=int_to_string(i);
				AppName=string(DAL_get_cell_value(ii,0));
				strCookie=string(DAL_get_cell_value(ii,1));
				cpt_name=string(DAL_get_cell_value(ii,2));
				cpt_val=string(DAL_get_cell_value(ii,3));
				if(strCookie.size()==0)
					continue;
				parse_url(AppName);

				sCookieList+=xml_node("App"+sI,AppName);
				sCookieList+=xml_node("Cookie"+sI,strCookie);
				sCookieList+=xml_node("Cpt"+sI,cpt_name);
				sCookieList+=xml_node("CptVal"+sI,cpt_val);
				++i;
			}

			DAL_clear_result();

			sCookieList=xml_node("Quantity",int_to_string(i))+sCookieList;
			sCookieList=xml_node("SessionsCookies",sCookieList);

			q=q+sCookieList;
			i=0;

			while(fingerprint_headers.size())
			{
				sI=int_to_string(i);
				sTmp=parse_str_delim(fingerprint_headers,',');
				sList+=xml_node("Header"+sI,sTmp);
				++i;
			}

			sTmp=xml_node("HeadersQuantity",int_to_string(i))+sList;
			sTmp=xml_node("HeadersFingerPrint",sTmp);

			q+=sTmp;

		}
		else
		{			
			cout << "void "<<endl;
			cout << "SELECT app_domain,AppCookieName FROM applications WHERE 1=1"<<endl;
		}
		string bodyxml="", headerxml="";
		char inttochar[10];

		q=xml_node("Settings", q );
		bodyxml=xml_node("Body",q);

		int size=bodyxml.length();
		sprintf(inttochar,"%d",size);

		headerxml+=(xml_node("Type","Settings")+xml_node("Size",string(inttochar)));
		headerxml=xml_node("Headers",headerxml);

		result=xml_node("Transmission", headerxml+bodyxml);
		update_q = "UPDATE agents SET newConfig=0 WHERE agent_id='";
		update_q.append(agent);
		update_q.append("';");
		DAL_query(update_q.c_str());
	}
	else
	{
		cout << "int SQLFeeder::get_agent_conf(string agent, string &result)"<<endl;
		cout << "SELECT FilterExpression ,agent_id, ATMSIP, ATMSPort, MaxHeadersLength, MaxGetParamsLength, MaxPostLength, StatusInterval, CookieExpireTime, ConcurrentSessions, MessagesQueueSize, NetworkInterface  FROM agents WHERE agent_id='"+agent+"';"<<endl;
		DAL_clear_result();
	}
	printf("\nProgram Configuration: %s\n", result.c_str());
}*/



/*
 * void SQLFeeder::select_sqlblacklist(string &result)
 * 		Selecting expression for checking for sql injections from config table
 * Parameters:
 * 		string &result - string to return
 * Return value:
 * 		No
 */
void SQLFeeder::select_sqlblacklist(string &result)
{
	if( DAL_select("SELECT value FROM config WHERE name='sqlblacklist';") ==0)
	{
		//result=string(DAL_get_cell_value(0,0));
		char * r = DAL_get_cell_value(0,0);
		if (r && *r != '\0')
			result = r;
		else
			result.clear();
		DAL_clear_result();
	}
	else
	{
		cout << "void SQLFeeder::select_sqlblacklist(string &)"<<endl;
		cout << "SELECT value FROM config WHERE name='sqlblacklist';"<<endl;
	}
}

/*
 * void SQLFeeder::get_list_of_agents(map<string, struct AgentStatus>& list)
 * 		Reads list of agents from database, from table "agents"
 * Parameters:
 * 		map<string, struct AgentStatus>& list - list to fill in with data
 * Return value:
 * 		No
 */

void SQLFeeder::get_list_of_agents(map<string, struct AgentStatus>& list)
{
	static const char * q = "SELECT agent_id, StatusInterval FROM agents WHERE 1=1;";
	struct AgentStatus AStat;
	string sTmp;
	map<string, struct AgentStatus> tmpList;

	if (DAL_select(q)==0)
	{
		for(unsigned long long i=0; i<rows_count; ++i)
		{
			sTmp=string(DAL_get_cell_value(i,0));
			if(list.find(sTmp)==list.end())
			{
				AStat.iSockNum=-1;
				AStat.iStatus=0;
				AStat.iTimeout=atoi(DAL_get_cell_value(i,1))+5;
				AStat.iConfigCounter=0;
				tmpList.insert(pair<string,struct AgentStatus>(DAL_get_cell_value(i,0),AStat));
			}
			else
			{
				tmpList.insert(pair<string,struct AgentStatus>(sTmp,list[sTmp]));
			}
		}
		DAL_clear_result();
		list.clear();
		list = tmpList;
	}
	else
	{
		cout << "void SQLFeeder::get_list_of_agents(map<string, struct AgentStatus>& list)"<<endl;
		cout << "SELECT agent_id, StatusInterval FROM agents WHERE 1=1;"<<endl;
	}
}


/*
 * void SQLFeeder::get_list_of_filters(map<string, struct FilterStatus>& list)
 * 		Reads list of filters from database, from table "filters"
 * Parameters:
 * 		map<string, struct FilterStatus>& list - list to fill in with data
 * Return value:
 * 		No
 */
void SQLFeeder::get_list_of_filters(map<string, struct FilterStatus>& list)
{
	static const char * q = "SELECT filter_id, StatusInterval FROM filters WHERE 1=1;";
	struct FilterStatus FStat;
	string sTmp;
	map<string, struct FilterStatus> tmpList;

	if (DAL_select(q)==0)
	{
		for(unsigned long long i=0; i<rows_count; ++i)
		{
			sTmp=string(DAL_get_cell_value(i,0));
			if(list.find(sTmp)==list.end())
			{
				FStat.iSockNum=-1;
				FStat.iStatus=0;
				FStat.iTimeout=atoi(DAL_get_cell_value(i,1))+15;
				FStat.iSignalCounter=0;
				tmpList.insert(pair<string,struct FilterStatus>(DAL_get_cell_value(i,0),FStat));
			}
			else
			{
				tmpList.insert(pair<string,struct FilterStatus>(sTmp,list[sTmp]));
			}
		}
		DAL_clear_result();

		list.clear();
		list=tmpList;
	}
	else
	{
		cout << "void SQLFeeder::get_list_of_filters(map<string, struct FilterStatus>& list)"<<endl;
		cout << "SELECT agent_id, StatusInterval FROM filters WHERE 1=1;"<<endl;
	}
}

/*
 * void SQLFeeder::get_apps_list(unsigned int *iAddApp, map<string, struct AppData> & mAD, vector<string> &vA)
 * 		Reads list of applications from table "applications"
 * Parameters:
 * 		unsigned int *iAddApp				-	flag to add unknown applications to database or not
 * 		map<string, struct AppData> & mAD	-	map of structures to keep application data
 * 		vector<string> &vA					-	vector to store DESCENDING sorted names of applications
 * Return value:
 * 		No
 */
void SQLFeeder::get_apps_list(unsigned int *iAddApp, map<class AppKey, struct AppData, AppMatchPredicate> & mAppData, map<string, AppIpData> & mAppIPData, set<unsigned int> & mSslPorts)
{
	map<class AppKey, struct AppData, AppMatchPredicate>::iterator itr;
	map<class AppKey, struct AppData, AppMatchPredicate> mAD;
	string ips;
	string ip;
	const char * c;
	static const char * q = "SELECT value FROM config WHERE name='addUnknownApp';";
	static const char * q2 = "SELECT app_domain, app_id, AppCookieName, ssl_flag, ssl_server_port, app_ips, ssl_certificate_password, certificate, private_key,form_authentication_body_value FROM applications WHERE 1=1;";
	AppKey app;
	string cookie_names;
	char* tmp_cookie;
	string tmp_cookie_str;
	app.search_subdomain = false;
	//reading flag "addUnknownApp"
	if( DAL_select(q) ==0)
	{
		*iAddApp=atoi(DAL_get_cell_value(0,0));
		syslog(LOG_INFO,"Changing Auto-learn New Applications flag to %d",*iAddApp);
		printf("Changing Auto-learn New Applications flag to %d\n",*iAddApp);
		DAL_clear_result();
	}
	else
	{
		cout << "void SQLFeeder::get_apps_list(unsigned int *iAddApp, map<string, struct AppData> & mAD)"<<endl;
		cout << "SELECT value FROM config WHERE name='addUnknownApp';"<<endl;
	}

	//reading list of application from table applications
	mAD.clear();
	struct AppData sAppD;
	string appname;
	if (DAL_select(q2)!=0)
	{
		cout << "No applications found in db"<< endl;
	}
	mSslPorts.clear();
	//mSslPorts.insert( ntohs( 8080 ) );
	mSslPorts.insert( ntohs( 443 ) );
	mSslPorts.insert( ntohs( 80 ) );
	syslog(LOG_INFO,"========================Applications===========================");
	mAppIPData.clear();
	for(unsigned long long i=0; i<rows_count; ++i)
	{
		appname=string(DAL_get_cell_value(i,0));
		sAppD.id=(unsigned int) atoi(DAL_get_cell_value(i,1));
/*		sAppD.cookie=string(DAL_get_cell_value(i,2));
		syslog(LOG_INFO,"Hostname:%s",appname.c_str());
		//printf("loading %s\n", appname.c_str());
		transform(appname.begin(),appname.end(),appname.begin(),::tolower);
		transform(sAppD.cookie.begin(),sAppD.cookie.end(),sAppD.cookie.begin(),::tolower);*/
		cookie_names = string(DAL_get_cell_value(i,2));
		tmp_cookie = strtok ((char*)cookie_names.c_str(),", ");
		while (tmp_cookie != NULL)
		{
			//printf ("%s\n",tmp_cookie);
			tmp_cookie_str = string(tmp_cookie);
			transform(tmp_cookie_str.begin(),tmp_cookie_str.end(),tmp_cookie_str.begin(),::tolower);
			//printf ("\tCookie for %s : %s\n",appname.c_str(),tmp_cookie_str.c_str());
			sAppD.cookie_names.insert(tmp_cookie_str);
			tmp_cookie = strtok (NULL, ", ");
		}
		//printf("loading %s\n", appname.c_str());
		transform(appname.begin(),appname.end(),appname.begin(),::tolower);
		//transform(sAppD.cookie.begin(),sAppD.cookie.end(),sAppD.cookie.begin(),::tolower);
		sAppD.ssl_enabled = false;
		c = DAL_get_cell_value(i,3);
		// server ip
		if (DAL_get_cell_value(i,5) != NULL)
		{
			ips = DAL_get_cell_value(i,5);
		}

		if (c != NULL && *c == '1')
		{
			printf("Trying to load SSL certificate for %s\n", appname.c_str());
			//int port = 443;
			// server port for ssl
			if (DAL_get_cell_value(i,4) != NULL && *DAL_get_cell_value(i,4) != 0)	
			{
				sAppD.ssl_server_port = atoi(DAL_get_cell_value(i,4));
				sAppD.ssl_server_port_net = ntohs(sAppD.ssl_server_port);
			} else {
				sAppD.ssl_server_port = 443;
				sAppD.ssl_server_port_net = ntohs(443);
			}
			// certificate password
			if (DAL_get_cell_value(i,6) != NULL)
			{
				sAppD.certificate_password = DAL_get_cell_value(i,6);
			}
			if (DAL_get_cell_value(i,7) != NULL)
			{
				sAppD.certificate = DAL_get_cell_value(i,7);
			}
			if (DAL_get_cell_value(i,8) != NULL)		
			{
				sAppD.private_key = DAL_get_cell_value(i,8);
			}
			//printf("Checking prv key length\n");
			if (sAppD.private_key.size() != 0)
			{
				printf("Check if this certificate already loaded\n");
				// check if this certificate already loaded !
				itr = mAppData.find(app);
				if (itr != mAppData.end())
				{
					printf("app found\n");
					if (itr->second.rsa != NULL && itr->second.private_key == sAppD.private_key)
					{
						printf("Reusing old loaded cert\n");
						// only if the key is the same !
						sAppD.rsa = itr->second.rsa;
					} else {
						printf("will load new cert\n");
						sAppD.rsa = loadRSA(sAppD.private_key, sAppD.certificate_password);
					}
				} else {
					printf("will load new cert2\n");
					sAppD.rsa = loadRSA(sAppD.private_key, sAppD.certificate_password);
				}
				if (sAppD.rsa)
				{
					printf("cert loaded\n");
					sAppD.ssl_enabled = true;
					mSslPorts.insert( sAppD.ssl_server_port_net );
				}
			}
		} else {
			//printf("no ssl info found\n");
			sAppD.certificate_password.clear();
			sAppD.certificate.clear();
			sAppD.private_key.clear();
			sAppD.rsa = NULL;
		}
		sAppD.login_msg = DAL_get_cell_value(i,9);

		app.name = ".";
		app.name += appname;
		reverse(app.name.begin(), app.name.end());

		mAD.insert(pair<AppKey,struct AppData>(app,sAppD));

		if (ips.size() == 0)
			continue;

		// load ip data
		AppIpData d;
		d.id = sAppD.id;
		d.rsa = sAppD.rsa;
		d.net_port = sAppD.ssl_server_port_net;
		if (d.rsa == NULL && d.net_port == 0)
		{
			d.net_port = ntohs(80);
		}
		
		while (ips.size())
		{
			ip = parse_str_delim(ips, ',');
			if (ip.size())
			{
				if (mAppIPData.find(ip) == mAppIPData.end())
				{
					mAppIPData[ip] = d;
				}
			}
		}
	}
	DAL_clear_result();
	mAppData = mAD;
}


/*
 * void SQLFeeder::get_atms_config_flag(set<int> &sConfCodes)
 * 		Reads ATMS action codes from "config" table, fiels - atms_actions
 * Parameters:
 * 		set<int> &sConfCodes - set of codes
 * Return value:
 * 		No
 */
void SQLFeeder::get_atms_config_flag(set<int> &sConfCodes)
{
	static const char * q = "SELECT value FROM config WHERE name='atmsActions';";
	static const char * update_q = "UPDATE config SET value=0 WHERE name='atmsActions';";
	string sCodes;

	sConfCodes.clear();

	//making request to database
	if (DAL_select(q)==0)
	{
		sCodes=string(DAL_get_cell_value(0,0));
		DAL_clear_result();
	}
	else
	{
		cout << "void SQLFeeder::get_atms_config_flag(set<int> &sConfCodes)"<<endl;
		cout << "SELECT value FROM `config` WHERE name='atmsActions';"<<endl;
	}

	if(sCodes!="0")
	{
		DAL_query(update_q);
		//parsing codes from databse
		while(sCodes.size())
		{
			sConfCodes.insert(atoi(parse_str_delim(sCodes,',').c_str()));
		}
	}
	else
	{
		sConfCodes.clear();
		return;
	}

}


/*
 * void SQLFeeder::sqllog(string source, string message)
 * 		Send log data to database, table "logs"
 * Parameters:
 * 		string source	-	data to write to "source" field
 * 		string message	-	data to write to "message" field
 * Return value:
 * 		No
 */
void SQLFeeder::sqllog(string & source, string & message)
{
	time_t date = time(NULL);
	char *creq;

	creq=new char[100+message.size()+source.size()];
	if (!creq)
	{
		return;
	}
	sprintf(creq,"INSERT INTO logs (date,source,message) VALUES (%s,'%s','%s');",
			int_to_string((unsigned int)date).c_str(),source.c_str(),message.c_str());
	DAL_query(creq);

	delete []creq;
}

/*
 * void SQLFeeder::update_agent_stat(string agent_id, string status)
 * 		Updates agent status in table "agents"
 * Parameters:
 * 		string agent_id		- id of agent we want to update
 * 		string status		- status to write (usually alive or dead)
 * Return value:
 * 		No
 */
void SQLFeeder::update_agent_stat(string & agent_id, const char * status)
{
	time_t date = time(NULL);
	char creq[600];
	sprintf(creq,"UPDATE agents SET time=%d, status='%s' WHERE agent_id='%s';",
			(unsigned int)date,status,agent_id.c_str());
	DAL_query(creq);
}


/*
 * void SQLFeeder::update_filter_stat(string agent_id, string status)
 * 		Updates agent status in table "filters"
 * Parameters:
 * 		string filter_id		- id of filter we want to update
 * 		string status		- status to write (usually alive or dead)
 * Return value:
 * 		No
 */
void SQLFeeder::update_filter_stat(string & agent_id, const char * status)
{
	time_t date = time(NULL);
	char creq[500];
	sprintf(creq,"UPDATE filters SET time=%d, status='%s' WHERE filter_id='%s';",
			(unsigned int)date,status,agent_id.c_str());
	DAL_query(creq);
}

/*
 * void SQLFeeder::update_revision()
 * 		Updates atms svn revision on the database
 * Parameters:
 * 		string svn_revision -  the revision to update
 * Return value:
 * 		No
 */
void SQLFeeder::update_revision(const char* svn_revision){
	char creq[500];
	sprintf(creq,"UPDATE config SET value=%s WHERE name='atms_version';",svn_revision);
	DAL_query(creq);	
}


/*
 * void SQLFeeder::get_op_mod_logtodb(int &op_mode, int &logdb, u64 &init_rid)
 * 		Gets operation_mode, logging_to_database flag and initial RID from database
 * Parameters:
 * 		int &op_mode		-	operation mode
 * 		int &logdb			-	log to database flag (to learning_data and http_traffic)
 * 		u64 &init_rid		-	initial rid - to continue count from last largest
 * Return value:
 * 		No
 */
void SQLFeeder::get_op_mod_logtodb(int &op_mode, u64 &init_rid)
{
	static const char * q = "SELECT value FROM config WHERE name='operation_mode';";
	static const char * q_max = "SELECT MAX(RID) as value FROM request_scores;";

	//fetching operation mode
	if (DAL_select(q)==0){
		op_mode=atoi(DAL_get_cell_value(0,0));
		DAL_clear_result();
	}

	//fetching "initial RID" parameter
	if (DAL_select(q_max)==0){
		char * v = DAL_get_cell_value(0,0);
		if (v)
			init_rid=atoi(v);
		else
			init_rid = 1;
		DAL_clear_result();
	}


}

/*
 * void SQLFeeder::get_maxsession(int &msess)
 * 		Read "urlDetectRegex" parameter from database.
 * 		Used to detect, if value of attribute is URL.
 * Parameters:
 * 		string &reg		-	string for database
 * Return value:
 * 		No
 */
void SQLFeeder::get_url_detect_regex(string &reg)
{
#ifdef DEBUG
	printf("void SQLFeeder::get_url_detect_regex(string &reg)\n");
#endif

	if (DAL_select("SELECT value FROM config WHERE name='urlDetectRegex';")==0)
	{
		reg=string(DAL_get_cell_value(0,0));
		DAL_clear_result();
	}
	else
	{
		cout << "void SQLFeeder::get_url_detect_regex(string &reg)"<<endl;
		cout << "SELECT value FROM config WHERE name='urlDetectRegex';"<<endl;
	}
}

/*
 * void SQLFeeder::get_excluded_ip(set<string>& exclIP)
 * 		Read read list of excluded IPs.
 * Parameters:
 * 		set<string>& exclIP	-	list of IPs
 * Return value:
 * 		No
 */
void SQLFeeder::get_excluded_ip(set<string>& exclIP)
{

	string sExclIPs;
	if (DAL_select("SELECT value FROM config WHERE name='excludedIPs';")==0)
	{
		sExclIPs=string(DAL_get_cell_value(0,0));
		DAL_clear_result();

		exclIP.clear();
		while(sExclIPs.size())
		{
			exclIP.insert(parse_str_delim(sExclIPs,','));
		}
	}
	else
	{
		cout << "void SQLFeeder::get_excluded_ip(set<string>& exclIP)"<<endl;
		cout << "SELECT value FROM config WHERE name='excludedIPs';"<<endl;
	}

}


/*
 * void SQLFeeder::get_excluded_ip(set<string>& exclIP)
 * 		Read read list of excluded IPs.
 * Parameters:
 * 		set<string>& exclIP	-	list of IPs
 * Return value:
 * 		No
 */
unsigned long long SQLFeeder::add_app(const string & appname)
{

	char cq[1000];
	unsigned long long id = 0;

	sprintf(cq, "SELECT app_id from applications where app_domain='%s' or app_domain like '%%.%s';", appname.c_str(), appname.c_str());
	if (DAL_select(cq) == 0)
	{
		if (rows_count)
		{
			id = atol(DAL_get_cell_value(0,0));
			DAL_clear_result();
			return id;
		}
	}

	string tmp = appname;
	beforeDecode(tmp);
	string app_utf8 = uriDecode(tmp);

	sprintf(cq, "INSERT INTO applications (app_domain) VALUES('%s');", app_utf8.c_str());
	DAL_select(cq);
	id = mysql_insert_id(MYSQL_conn);
	if (id == 0)
	{
		// it is possible that other process or user created this application
		sprintf(cq, "SELECT app_id from applications where app_domain='%s';", app_utf8.c_str());
		if (DAL_select(cq) == 0)
		{
			id = atol(DAL_get_cell_value(0,0));
		}
	}	

	return id;
}

/*
 * void SQLFeeder::get_app_groupheaders(map<string, struct AppData >& m)
 * 		Read read list of excluded IPs.
 * Parameters:
 * 		map<string, struct LoginAtt>& mLoginA 	- structure holds parameters to detect login/logout actions
 * 		map<string, struct LoggedIn>& mLoggedA	- structure holds parameters to detect if user is logged in currently
 * Return value:
 * 		No
 */

void SQLFeeder::get_app_groupheaders(map<string, struct AppData >& m)
{

	string app_id;
	string headers;
	//string global_pages;

//fetching data
//global pages - page which meets conditions will be added to index only one time
	if (DAL_select("SELECT app_id, exclude_group_headers, global_pages, global_per_app FROM applications WHERE 1=1;")==0)
	{
		for(unsigned long long i=0; i<rows_count; ++i)
		{
			app_id=string(DAL_get_cell_value(i,0));
			headers=string(DAL_get_cell_value(i,1));
			//global_pages=string(DAL_get_cell_value(i,2));
			m[app_id].global_per_app=atoi(DAL_get_cell_value(i,3));
			//m[app_id].global=atoi(row[4]);
			while(headers.size())
			{
				m[app_id].sExcludeGlobalHeaders.insert(parse_str_delim(headers,','));
			}
			//while(global_pages.size())
			//{
			//	//saving regexes for pages
			//	m[app_id].mdynPageRegex2[parse_str_delim(global_pages,',')];
			//}
		}

		DAL_clear_result();
	}
	else
	{
		cout << "void SQLFeeder::get_app_groupheaders(map<string, struct AppData >& m)"<<endl;
		cout << "SELECT app_id, exclude_group_headers, global_pages, global_per_app FROM applications WHERE 1=1;"<<endl;
	}

//if in config.globalHeaders==1, headers will be global for all applications!
	if (DAL_select("SELECT value FROM config WHERE name='globalHeaders';")==0)
	{
		m["0"].global=atoi(DAL_get_cell_value(0,0));
		DAL_clear_result();
	}
	else
	{
		cout << "void SQLFeeder::get_app_groupheaders(map<string, struct AppData >& m)"<<endl;
		cout << "SELECT value FROM config WHERE name='globalHeaders';"<<endl;
	}

//excluding headers from all applications
	m["0"].sExcludeGlobalHeaders.clear();
	if (DAL_select("SELECT value FROM config WHERE name='excludeGlobal';")==0)
	{
		for(unsigned long long i=0; i<rows_count; ++i)
		{
			headers=string(DAL_get_cell_value(0,0));
			while(headers.size())
			{
				m["0"].sExcludeGlobalHeaders.insert(parse_str_delim(headers,','));
			}
		}

		DAL_clear_result();
	}
	else
	{
		cout << "void SQLFeeder::get_app_groupheaders(map<string, struct AppData >& m)"<<endl;
		cout << "SELECT value FROM config WHERE name='excludeGlobal';"<<endl;
	}
}

/*
 * void SQLFeeder::get_loadbalance_settings(size_t &flag, set<string> &sLB_IP, string &header)
 * 		Reads set of loadbalancer settings from table.
 * Parameters:
 * 		size_t &flag			-	flag if we have loadbalancer
 * 		set<string> &sLB_IP		-	IPs of loadbalancers
 * 		string &header			-	header where we should look for user IP if, there is load balancer
 * Return value:
 * 		No
 */
void SQLFeeder::get_loadbalance_settings(size_t &flag, set<string> &sLB_IP, vector<string> &headers)
{

	string ips;
	string ip;
	//gets flag, if load_balancer is used
	if (DAL_select("SELECT value FROM config WHERE name='load_balancer_on';")==0)
	{
		flag=atoi(DAL_get_cell_value(0,0));
		DAL_clear_result();
	}
	else
	{
		cout << "void SQLFeeder::get_loadbalance_settings(size_t &flag, set<string> &sLB_IP, string &header)\n"<<endl;
		cout << "SELECT value FROM config WHERE name='load_balancer_on';"<<endl;
	}

	//if load balancer is used gets IPs of it
	if(!flag)
	{
		printf("Load Balancer Flag is Off\n");
		syslog (LOG_NOTICE, "Load Balancer Flag is Off");
		return;
	}
	printf("========================LOAD BALANCER IP's===========================\n");
	syslog(LOG_INFO,"========================LOAD BALANCER IP's===========================");

	printf("Load Balancer Flag is On\n");
	if (DAL_select("SELECT value FROM config WHERE name='load_balancer_ip';")==0)
	{
		ips=string(DAL_get_cell_value(0,0));
		DAL_clear_result();
	}
	else
	{
		cout << "void SQLFeeder::get_loadbalance_settings(size_t &flag, set<string> &sLB_IP, string &header)\n"<<endl;
		cout << "SELECT value FROM config WHERE name='load_balancer_ip';"<<endl;
	}

	sLB_IP.clear();
	transform(ips.begin(), ips.end(), ips.begin(), ::tolower);
	while (ips.size())
	{
		ip = parse_str_delim(ips, ',');
		if (ip.size())
		{
			printf("Load Balancer IP: %s\n",ip.c_str());
			syslog(LOG_INFO,"Load Balancer IP: %s",ip.c_str());
			sLB_IP.insert( ip );
		}
	}

	//and name of the header
	if (DAL_select("SELECT value FROM config WHERE name='load_balancer_header';")==0)
	{
		string h;
		string header=string(DAL_get_cell_value(0,0));
		transform(header.begin(), header.end(), header.begin(), ::tolower);

		while (header.size())
		{
			h = parse_str_delim(header, ',');
			if (h.size())
			{
				printf("Load Balancer Header: %s\n",h.c_str());
				syslog(LOG_INFO,"Load Balancer Header: %s",h.c_str());
				headers.push_back(h);
			}
		}
		DAL_clear_result();
	}
	else
	{
		cout << "void SQLFeeder::get_loadbalance_settings(size_t &flag, set<string> &sLB_IP, string &header)\n"<<endl;
		cout << "SELECT value FROM config WHERE name='load_balancer_header';"<<endl;
	}
	printf("====================================================================\n");
	syslog(LOG_INFO,"====================================================================");
}


/*
 * void SQLFeeder::sqlError(string q)
 * 		Logging and printing sqlerror
 * Parameters:
 * 		string q - request
 * Return value:
 * 		No
 */
void SQLFeeder::sqlError(const char * q)
{
	char fbuf[2000];

	printf("Error %u: %s\n", mysql_errno(MYSQL_conn), mysql_error(MYSQL_conn));
	cout<<"Query: "<<q<<endl;
      	
    	sprintf(fbuf,"Error %u: %s\nQuery: %s\n", mysql_errno(MYSQL_conn), mysql_error(MYSQL_conn),q);
	syslog (LOG_NOTICE, "%s",fbuf);

	if(mysql_errno(MYSQL_conn) == 2006){ // MYSQL HAS GONE AWAY.
		while(1){
			sleep(5);
			if (mysql_real_connect(MYSQL_conn,(char*)db_addr.c_str(),(char*)db_username.c_str(),(char*)db_psswd.c_str(),(char*)db_name.c_str(), 0, NULL, 0) == NULL){syslog (LOG_NOTICE, "WAIT TO RESTORE CONNECTION!!!");}
			else{return;}
		}
	}else{
		exit(1);
	}


}

short SQLFeeder::sqlError(string q,short duplicateF)
{
	char fbuf[2000];
	printf("Error %u: %s\n", mysql_errno(MYSQL_conn), mysql_error(MYSQL_conn));
	cout<<"Query: "<<q<<endl;

    	sprintf(fbuf,"Error %u: %s\nQuery: %s\n", mysql_errno(MYSQL_conn), mysql_error(MYSQL_conn),q.c_str());
	syslog (LOG_NOTICE, "%s",fbuf);

	if(mysql_errno(MYSQL_conn) == 1062){  // Duplicate entry.
		return 1;
	}

	if(mysql_errno(MYSQL_conn) == 2006){ // MYSQL HAS GONE AWAY.
		while(1){
			sleep(5);
			if (mysql_real_connect(MYSQL_conn,(char*)db_addr.c_str(),(char*)db_username.c_str(),(char*)db_psswd.c_str(),(char*)db_name.c_str(), 0, NULL, 0) == NULL){syslog (LOG_NOTICE, "WAIT TO RESTORE CONNECTION!!!");}
			else{return 0;}
		}
	}else{
		exit(1);
	}


}

/*
 * SQLFeeder::~SQLFeeder()
 * 		SQLFeeder destructor
 * Parameters:
 * 		No
 * Return value:
 * 		No
 */

SQLFeeder::~SQLFeeder()
{
#ifdef DEBUG
	printf("\nSQLFeeder::~SQLFeeder()\n");
#endif

	if(db_type=="MYSQL")
	{
		close_sql_connection();
	}
	else if(db_type=="POSTGRESQL")
	{
#ifdef DAL_POSTGRES
		PQfinish(PG_conn);
#endif
	}
}




