
#define SECS_DAY (24L * 60L * 60L)
#define EPOCH_YR 1970
#define YEAR0 1900
#define LEAPYEAR(year) (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year) (LEAPYEAR(year) ? 366 : 365)

using namespace std;

const int _ytab[2][12] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

string getDate(unsigned int timer)
{
	static struct tm br_time;
	register struct tm *tmbuf = &br_time;
	time_t time = timer;
	unsigned long dayno;
	int year = EPOCH_YR;

	dayno = (unsigned long) time / SECS_DAY;

	while (dayno >= (unsigned long) YEARSIZE(year)) {
		dayno -= YEARSIZE(year);
		year++;
	}
	tmbuf->tm_year = year - YEAR0;
	tmbuf->tm_yday = dayno;
	tmbuf->tm_mon = 0;
	while (dayno >= (unsigned long) _ytab[LEAPYEAR(year)][tmbuf->tm_mon]) {
		dayno -= _ytab[LEAPYEAR(year)][tmbuf->tm_mon];
		tmbuf->tm_mon++;
	}

	tmbuf->tm_year += YEAR0;
	tmbuf->tm_mon++;
	tmbuf->tm_mday = dayno + 1;

	char buff[50];
	if(tmbuf->tm_mon < 10){
		if(tmbuf->tm_mday < 10){
			sprintf(buff,"%u0%u0%u",(unsigned)tmbuf->tm_year,(unsigned)tmbuf->tm_mon,(unsigned)tmbuf->tm_mday);
		}else{
			sprintf(buff,"%u0%u%u",(unsigned)tmbuf->tm_year,(unsigned)tmbuf->tm_mon,(unsigned)tmbuf->tm_mday);
		}
	}else{
		if(tmbuf->tm_mday < 10){
			sprintf(buff,"%u%u0%u",(unsigned)tmbuf->tm_year,(unsigned)tmbuf->tm_mon,(unsigned)tmbuf->tm_mday);
		}else{
			sprintf(buff,"%u%u%u",(unsigned)tmbuf->tm_year,(unsigned)tmbuf->tm_mon,(unsigned)tmbuf->tm_mday);
		}
	}

	return string(buff);

}

void getHour(unsigned int timer,vector <unsigned int> & val)
{

        static struct tm br_time;
        register struct tm *timep = &br_time;
        register unsigned long dayclock;

        dayclock = (timer+ 3*3600) % (SECS_DAY );

        timep->tm_hour = dayclock / 3600;

	val.push_back( (unsigned int)timep->tm_hour );

}

void getWeekDay(unsigned int timer,vector <unsigned int> & val)
{	

        static struct tm br_time;
        register struct tm *timep = &br_time;
        register unsigned long dayno;

        dayno = (timer+3*3600) / (SECS_DAY );
        timep->tm_wday = (dayno + 4) % 7;     
	val.push_back( (unsigned int)timep->tm_wday );
}

unsigned int getHour2(unsigned int timer)
{

        static struct tm br_time;
        register struct tm *timep = &br_time;
        register unsigned long dayclock;

        dayclock = (timer+ 2*3600) % (SECS_DAY );
        timep->tm_hour = dayclock / 3600;

        return ( (unsigned int)timep->tm_hour );
}

static const char* DAYS_LOOKUP[7]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
string getWeekDay2(unsigned int timer)
{	

	static struct tm br_time;
	register struct tm *timep = &br_time;
	register unsigned long dayno;

	dayno = (timer+2*3600) / (SECS_DAY );
	timep->tm_wday = (dayno + 4) % 7;
	return( DAYS_LOOKUP[(unsigned int)timep->tm_wday] );

}

void print_time(unsigned int timer)
{

	static struct tm br_time;
	register struct tm *timep = &br_time;
	register unsigned long dayno;
	register unsigned long dayclock;

	int year = EPOCH_YR;

	dayno = (timer+3*3600) / (SECS_DAY );

        while (dayno >= YEARSIZE(year)) {
                dayno -= YEARSIZE(year);
                year++;
        }
	timep->tm_year = year - YEAR0;
	timep->tm_yday = dayno;
	timep->tm_mon = 0;
        while (dayno >= _ytab[LEAPYEAR(year)][timep->tm_mon]) {
                dayno -= _ytab[LEAPYEAR(year)][timep->tm_mon];
                timep->tm_mon++;
        }

	timep->tm_year += YEAR0;
	timep->tm_mon++;
	timep->tm_mday = dayno + 1;

        dayclock = (timer+ 3*3600) % (SECS_DAY );
        timep->tm_hour = dayclock / 3600;
	timep->tm_min = (dayclock % 3600) / 60;
	timep->tm_sec = dayclock % 60;

	timep->tm_wday = (dayno + 4) % 7;
	printf("      %u-%u-%u  %u:%u:%u  \n",timep->tm_mday,timep->tm_mon,timep->tm_year,timep->tm_hour,timep->tm_min,timep->tm_sec);

}


