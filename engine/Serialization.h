using namespace std;

string filepath = "/opt/telepath/db/kb_bin";
//string filepath = "/home/roi/atms_eng";

short writeB5(Path & path,unsigned int group_id){
	short flag;
	char filename[300];
	sprintf(filename,"%s/behavior/%u",(char*)filepath.c_str(),group_id );

	syslog(LOG_NOTICE,"SAVE Behavior - %u", group_id );
	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << path;
			flag=1;
		}else{
			flag=0;
		}
	}
	return flag;
}

short readB5(Path & path,unsigned int group_id){
	short flag=0;
	char filename[300];
	sprintf(filename,"%s/behavior/%u",(char*)filepath.c_str(),group_id );

	syslog(LOG_NOTICE,"LOAD BEHAVIOR - %u", group_id );
	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> path;
				flag=1;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing behavior");
			}
		}else{
			flag=0;
		}
	}

	return flag;
}

void readB(map<unsigned int,myAtt> & myAttMap){
	char filename[300];
	sprintf(filename,"%s/attributes/all_attributes",(char*)filepath.c_str());

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> myAttMap;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing attributes ");
			}
		}else{	}
	}

	/*cout<<"---------LOAD-----------"<<endl;
	myAttMap[1].printCapacity();
	cout<<"---------------------------"<<endl;*/
}

void readB(map<string,myAtt> & myAttUserMap){
	char filename[300];
	sprintf(filename,"%s/user_attributes/all_user_attributes",(char*)filepath.c_str());

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> myAttUserMap;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing user attributes" );
			}
		}else{	}
	}
}

short readB4(Path & path,string domain){
	short flag=0;
	char filename[300];


	sprintf(filename,"%s/paths/%s",(char*)filepath.c_str(),domain.c_str() );

	#ifdef DEBUG
		syslog (LOG_NOTICE, "LOADING PATH- %s",domain.c_str() );
	#endif

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> path;
				flag=1;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing path- %s",domain.c_str() );
			}
		}else{
			flag=0;
		}
	}

	/*cout<<"---------------------------"<<endl;
	path.printCapacity();
	cout<<"---------------------------"<<endl;*/

	return flag;
}

void readB(map<string,Path> pathUser){
	char filename[300];
	sprintf(filename,"%s/user_paths/all_user_paths",(char*)filepath.c_str());

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> pathUser;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing user_paths");
			}
		}else{	}
	}

}


short readB(map <string,ScoresNumeric> & n){
	short flag=0;
	char filename[300];
	sprintf(filename,"%s/globals/production_pages",(char*)filepath.c_str() );

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> n;
				flag=1;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing global pages");
			}
		}else{
			flag=0;
		}
	}
	return flag;
}

short readB(map <string,ScoresData> & n){
	short flag=0;
	char filename[300];
	sprintf(filename,"%s/globals/learning_pages",(char*)filepath.c_str() );

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> n;
				flag=1;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing global pages");
			}
		}else{
			flag=0;
		}
	}
	return flag;
}

short readB6(map <unsigned int,ScoreNumericAtt> & n){
	short flag=0;
	char filename[300];
	sprintf(filename,"%s/globals/production_attributes",(char*)filepath.c_str() );

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> n;
				flag=1;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing global attributes(Production)");
			}
		}else{
			flag=0;
		}
	}
	return flag;
}

short readB7(map <unsigned int,ScoreAtt> & n){
	short flag=0;
	char filename[300];
	sprintf(filename,"%s/globals/learning_attributes",(char*)filepath.c_str() );

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> n;
				flag=1;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing global attributes(Learning)");
			}
		}else{
			flag=0;
		}
	}
	return flag;
}

short readB(map <string,ScoreNumericAtt> & n){
	short flag=0;
	char filename[300];
	sprintf(filename,"%s/globals/user_attributes",(char*)filepath.c_str() );

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> n;
				flag=1;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing global user attributes");
			}
		}else{
			flag=0;
		}
	}
	return flag;
}

short readB(map <string,ScoreAtt> & n){
	short flag=0;
	char filename[300];
	sprintf(filename,"%s/globals/user_attributes",(char*)filepath.c_str() );

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> n;
				flag=1;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing global user attributes");
			}
		}else{
			flag=0;
		}
	}
	return flag;
}

short readB2(map <string,unsigned int> & n){
	short flag=0;
	char filename[300];
	sprintf(filename,"%s/globals/sessions",(char*)filepath.c_str() );

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> n;
				flag=1;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing global user sessions");
			}
		}else{
			flag=0;
		}
	}
	return flag;
}

short readB(map<unsigned long,unsigned int> & n){
	short flag=0;
	char filename[300];
	sprintf(filename,"%s/flows/pid_web_service",(char*)filepath.c_str() );

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> n;
				flag=1;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing web service");
			}
		}else{
			flag=0;
		}
	}
	return flag;
}

short readB3(map<unsigned int,unsigned int> & n,string name){
	short flag=0;
	char filename[300];
	sprintf(filename,"%s/flows/%s",(char*)filepath.c_str(),(char*)name.c_str() );

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> n;
				flag=1;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing flows");
			}
		}else{
			flag=0;
		}
	}
	return flag;
}

short readB(map <unsigned int, Operation > & n){
	short flag=0;
	char filename[300];
	sprintf(filename,"%s/globals/operations",(char*)filepath.c_str() );

	{
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		if( ifs.good() ){
			try{
				boost::archive::binary_iarchive ia(ifs);
				ia >> n;
				flag=1;
			}catch(boost::archive::archive_exception& ex){
				syslog(LOG_NOTICE, "Archive Exception during deserializing operations");
			}
		}else{
			flag=0;
		}
	}
	return flag;
}

void writeB(map<unsigned int,myAtt> & myAttMap){
	char filename[300];
	sprintf(filename,"%s/attributes/all_attributes",(char*)filepath.c_str());
	std::ofstream ofs(filename, std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << myAttMap;
		}else{	}
	}

	/*cout<<"**********SAVE**********"<<endl;
	myAttMap[1].printCapacity();
	cout<<"************************"<<endl;*/

}

void writeB(map<string,myAtt> & myAttUserMap){
	char filename[300];
	sprintf(filename,"%s/user_attributes/all_user_attributes",(char*)filepath.c_str());

	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << myAttUserMap;
		}else{	}
	}
}

short writeB4(Path & path,string domain){
	short flag;
	char filename[300];

	sprintf(filename,"%s/paths/%s",(char*)filepath.c_str(),domain.c_str() );

	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << path;
			flag=1;
		}else{
			flag=0;
		}
	}

	return flag;
}


void writeB(map<string,Path> pathUser){
	char filename[300];
	sprintf(filename,"%s/user_paths/all_user_paths",(char*)filepath.c_str() );

	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << pathUser;
		}else{	}
	}
}

short writeB(map <string,ScoresNumeric> & n){
	short flag;
	char filename[300];
	sprintf(filename,"%s/globals/production_pages",(char*)filepath.c_str() );

	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << n;
			flag=1;
		}else{
			flag=0;
		}
	}

	return flag;
}

short writeB(map <string,ScoresData> & n){

	short flag;
	char filename[300];
	sprintf(filename,"%s/globals/learning_pages",(char*)filepath.c_str() );

	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << n;
			flag=1;
		}else{
			flag=0;
		}
	}

	return flag;
}

short writeB6(map <unsigned int,ScoreNumericAtt>  & n){
	short flag;
	char filename[300];
	sprintf(filename,"%s/globals/production_attributes",(char*)filepath.c_str() );

	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << n;
			flag=1;
		}else{
			flag=0;
		}
	}

	return flag;
}

short writeB7(map <unsigned int,ScoreAtt>  & n){

	short flag;
	char filename[300];
	sprintf(filename,"%s/globals/learning_attributes",(char*)filepath.c_str() );

	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << n;
			flag=1;
		}else{
			flag=0;
		}
	}

	return flag;
}

short writeB(map <string,ScoreNumericAtt>  & n){
	short flag;
	char filename[300];
	sprintf(filename,"%s/globals/user_attributes",(char*)filepath.c_str() );

	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << n;
			flag=1;
		}else{
			flag=0;
		}
	}
	return flag;
}

short writeB(map <string,ScoreAtt>  & n){
	short flag;
	char filename[300];
	sprintf(filename,"%s/globals/user_attributes",(char*)filepath.c_str() );

	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << n;
			flag=1;
		}else{
			flag=0;
		}
	}
	return flag;
}

short writeB2(map <string,unsigned int> & n){
	short flag;
	char filename[300];
	sprintf(filename,"%s/globals/sessions",(char*)filepath.c_str() );

	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << n;
			flag=1;
		}else{
			flag=0;
		}
	}
	return flag;
}

short writeB(map<unsigned long,unsigned int> & n){
	short flag;
	char filename[300];
	sprintf(filename,"%s/flows/pid_web_service",(char*)filepath.c_str() );

	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << n;
			flag=1;
		}else{
			flag=0;
		}
	}
	return flag;
}


short writeB3(map<unsigned int,unsigned int> & n,string name){
	short flag;
	char filename[300];
	sprintf(filename,"%s/flows/%s",(char*)filepath.c_str(),(char*)name.c_str() );

	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << n;
			flag=1;
		}else{
			flag=0;
		}
	}
	return flag;
}

short writeB(map <unsigned int, Operation > & n){
	short flag;
	char filename[300];
	sprintf(filename,"%s/globals/operations",(char*)filepath.c_str() );

	std::ofstream ofs(filename,std::ios::out | std::ios::binary);
	{
		if( ofs.good() ){
			boost::archive::binary_oarchive oa(ofs);
			oa << n;
			flag=1;
		}else{
			flag=0;
		}
	}
	return flag;
}

