#include <stdio.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include "tdparser.h"
#include <assert.h>
#include <signal.h>
#include <ctype.h>
#include <math.h>

//
//	everything is cool, except the following:
//
//	all of idaho is in the mountain time zone, but there is no area-code split at present.
//	same for north dakota and south dakota. In all other cases there is no area-code that spans timezones.
//

static struct gvtz timezonestable[] = {
{ "201", 60*5, 1, "NJ" }, { "202", 60*5, 1, "DC" }, { "203", 60*5, 1, "CT" }, { "204", 60*6, 1, "Manitoba" },
{ "205", 60*6, 1, "AL" }, { "206", 60*8, 1, "WA" }, { "207", 60*5, 1, "ME" }, { "208", 60*7, 1, "ID" },
{ "209", 60*8, 1, "CA" }, { "210", 60*6, 1, "TX" }, { "212", 60*5, 1, "NY" }, { "213", 60*8, 1, "CA" },
{ "214", 60*6, 1, "TX" }, { "215", 60*5, 1, "PA" }, { "216", 60*5, 1, "OH" }, { "217", 60*6, 1, "IL" },
{ "218", 60*6, 1, "MN" }, { "219", 60*5, 0, "IN" }, { "224", 60*6, 1, "IL" }, { "225", 60*6, 1, "LA" },
{ "228", 60*6, 1, "MS" }, { "229", 60*5, 1, "GA" }, { "231", 60*5, 1, "MI" }, { "234", 60*5, 1, "OH" },
{ "239", 60*5, 1, "FL" }, { "240", 60*5, 1, "MD" }, { "248", 60*5, 1, "MI" }, { "250", 60*8, 1, "British Columbia" },
{ "251", 60*6, 1, "AL" }, { "252", 60*5, 1, "NC" }, { "253", 60*8, 1, "WA" }, { "254", 60*6, 1, "TX" },
{ "256", 60*6, 1, "AL" }, { "260", 60*5, 0, "IN" }, { "262", 60*6, 1, "WI" }, { "267", 60*5, 1, "PA" },
{ "269", 60*5, 1, "MI" }, { "270", 60*6, 1, "KY" }, { "276", 60*5, 1, "VA" }, { "281", 60*6, 1, "TX" },
{ "289", 60*5, 1, "Ontario" }, { "301", 60*5, 1, "MD" }, { "302", 60*5, 1, "DE" }, { "303", 60*7, 1, "CO" },
{ "304", 60*5, 1, "WV" }, { "305", 60*5, 1, "FL" }, { "306", 60*7, 1, "Saskatchewan" }, { "307", 60*7, 1, "WY" },
{ "308", 60*7, 1, "NE" }, { "309", 60*6, 1, "IL" }, { "310", 60*8, 1, "CA" }, { "312", 60*6, 1, "IL" },
{ "313", 60*5, 1, "MI" }, { "314", 60*6, 1, "MO" }, { "315", 60*5, 1, "NY" }, { "316", 60*6, 1, "KS" },
{ "317", 60*5, 0, "IN" }, { "318", 60*6, 1, "LA" }, { "319", 60*6, 1, "IA" }, { "320", 60*6, 1, "MN" },
{ "321", 60*5, 1, "FL" }, { "323", 60*8, 1, "CA" }, { "325", 60*6, 1, "TX" }, { "330", 60*5, 1, "OH" },
{ "334", 60*6, 1, "AL" }, { "336", 60*5, 1, "NC" }, { "337", 60*6, 1, "LA" }, { "339", 60*5, 1, "MA" },
{ "340", 60*5, 0, "USVI" }, { "347", 60*5, 1, "NY" }, { "351", 60*5, 1, "MA" }, { "352", 60*5, 1, "FL" },
{ "360", 60*8, 1, "WA" }, { "361", 60*6, 1, "TX" }, { "386", 60*5, 1, "FL" }, { "401", 60*5, 1, "RI" },
{ "402", 60*6, 1, "NE" }, { "403", 60*7, 1, "Alberta" }, { "404", 60*5, 1, "GA" }, { "405", 60*6, 1, "OK" },
{ "406", 60*7, 1, "MT" }, { "407", 60*5, 1, "FL" }, { "408", 60*8, 1, "CA" }, { "409", 60*6, 1, "TX" },
{ "410", 60*5, 1, "MD" }, { "412", 60*5, 1, "PA" }, { "413", 60*5, 1, "MA" }, { "414", 60*6, 1, "WI" },
{ "415", 60*8, 1, "CA" }, { "416", 60*5, 1, "Ontario" }, { "417", 60*6, 1, "MO" }, { "418", 60*5, 1, "Quebec" },
{ "419", 60*5, 1, "OH" }, { "423", 60*6, 1, "TN" }, { "425", 60*8, 1, "WA" }, { "430", 60*6, 1, "TX" },
{ "432", 60*6, 1, "TX" }, { "434", 60*5, 1, "VA" }, { "435", 60*7, 1, "UT" }, { "440", 60*5, 1, "OH" },
{ "443", 60*5, 1, "MD" }, { "450", 60*5, 1, "Quebec" }, { "469", 60*6, 1, "TX" }, { "478", 60*5, 1, "GA" },
{ "479", 60*6, 1, "AR" }, { "480", 60*7, 0, "AZ" }, { "484", 60*5, 1, "PA" }, { "501", 60*6, 1, "AR" },
{ "502", 60*5, 1, "KY" }, { "503", 60*8, 1, "OR" }, { "504", 60*6, 1, "LA" }, { "505", 60*7, 1, "NM" }, { "506", 60*4, 1, "New Brunswick" },
{ "507", 60*6, 1, "MN" }, { "508", 60*5, 1, "MA" }, { "509", 60*8, 1, "WA" }, { "510", 60*8, 1, "CA" }, { "512", 60*6, 1, "TX" },
{ "513", 60*5, 1, "OH" }, { "514", 60*5, 1, "Quebec" }, { "515", 60*6, 1, "IA" }, { "516", 60*5, 1, "NY" }, { "517", 60*5, 1, "MI" },
{ "518", 60*5, 1, "NY" }, { "519", 60*5, 1, "Ontario" }, { "520", 60*7, 0, "AZ" }, { "530", 60*8, 1, "CA" }, { "540", 60*5, 1, "VA" },
{ "541", 60*8, 1, "OR" }, { "551", 60*5, 1, "NJ" }, { "559", 60*8, 1, "CA" }, { "561", 60*5, 1, "FL" }, { "562", 60*8, 1, "CA" },
{ "563", 60*6, 1, "IA" }, { "567", 60*5, 1, "OH" }, { "570", 60*5, 1, "PA" }, { "571", 60*5, 1, "VA" }, { "573", 60*6, 1, "MO" },
{ "574", 60*5, 0, "IN" }, { "580", 60*6, 1, "OK" }, { "585", 60*5, 1, "NY" }, { "586", 60*5, 1, "MI" }, { "601", 60*6, 1, "MS" },
{ "602", 60*7, 0, "AZ" }, { "603", 60*5, 1, "NH" }, { "604", 60*8, 1, "British Columbia" }, { "605", 60*6, 1, "SD" }, { "606", 60*5, 1, "KY" },
{ "607", 60*5, 1, "NY" }, { "608", 60*6, 1, "WI" }, { "609", 60*5, 1, "NJ" }, { "610", 60*5, 1, "PA" }, { "612", 60*6, 1, "MN" },
{ "613", 60*5, 1, "Ontario" }, { "614", 60*5, 1, "OH" }, { "615", 60*5, 1, "TN" }, { "616", 60*5, 1, "MI" }, { "617", 60*5, 1, "MA" },
{ "618", 60*6, 1, "IL" }, { "619", 60*8, 1, "CA" }, { "620", 60*6, 1, "KS" }, { "623", 60*7, 0, "AZ" }, { "626", 60*8, 1, "CA" },
{ "630", 60*6, 1, "IL" }, { "631", 60*5, 1, "NY" }, { "636", 60*6, 1, "MO" }, { "641", 60*6, 1, "IA" }, { "646", 60*5, 1, "NY" },
{ "647", 60*5, 1, "Ontario" }, { "650", 60*8, 1, "CA" }, { "651", 60*6, 1, "MN" }, { "660", 60*6, 1, "MO" }, { "661", 60*8, 1, "CA" },
{ "662", 60*6, 1, "MS" }, { "671", 60*14, 1, "GU" }, { "678", 60*5, 1, "GA" }, { "682", 60*6, 1, "TX" }, { "684", 60*12, 0, "AS" },
{ "701", 60*6, 1, "ND" }, { "702", 60*8, 1, "NV" }, { "703", 60*5, 1, "VA" }, { "704", 60*5, 1, "NC" }, { "705", 60*5, 1, "Ontario" },
{ "706", 60*5, 1, "GA" }, { "707", 60*8, 1, "CA" }, { "708", 60*6, 1, "IL" }, { "709", 60*4, 1, "Newfoundland" }, { "712", 60*6, 1, "IA" },
{ "713", 60*6, 1, "TX" }, { "714", 60*8, 1, "CA" }, { "715", 60*6, 1, "WI" }, { "716", 60*5, 1, "NY" }, { "717", 60*5, 1, "PA" },
{ "718", 60*5, 1, "NY" }, { "719", 60*7, 1, "CO" }, { "720", 60*7, 1, "CO" }, { "724", 60*5, 1, "PA" }, { "727", 60*5, 1, "FL" },
{ "731", 60*6, 1, "TN" }, { "732", 60*5, 1, "NJ" }, { "734", 60*5, 1, "MI" }, { "740", 60*5, 1, "OH" }, { "754", 60*5, 1, "FL" },
{ "757", 60*5, 1, "VA" }, { "760", 60*8, 1, "CA" }, { "763", 60*6, 1, "MN" }, { "765", 60*5, 0, "IN" }, { "770", 60*5, 1, "GA" },
{ "772", 60*5, 1, "FL" }, { "773", 60*6, 1, "IL" }, { "774", 60*5, 1, "MA" }, { "775", 60*8, 1, "NV" }, { "778", 60*8, 1, "British Columbia" },
{ "780", 60*7, 1, "Alberta" }, { "781", 60*5, 1, "MA" }, { "785", 60*6, 1, "KS" }, { "786", 60*5, 1, "FL" }, { "787", 60*5, 0, "Puerto Rico" },
{ "801", 60*7, 1, "UT" }, { "802", 60*5, 1, "VT" }, { "803", 60*5, 1, "SC" }, { "804", 60*5, 1, "VA" }, { "805", 60*8, 1, "CA" },
{ "806", 60*6, 1, "TX" }, { "807", 60*5, 1, "Ontario" }, { "808", 60*10, 0, "HI" }, { "810", 60*5, 1, "MI" }, { "812", 60*5, 0, "IN" },
{ "813", 60*5, 1, "FL" }, { "814", 60*5, 1, "PA" }, { "815", 60*6, 1, "IL" }, { "816", 60*6, 1, "MO" }, { "817", 60*6, 1, "TX" },
{ "818", 60*8, 1, "CA" }, { "819", 60*5, 1, "Quebec" }, { "828", 60*5, 1, "NC" }, { "830", 60*6, 1, "TX" }, { "831", 60*8, 1, "CA" },
{ "832", 60*6, 1, "TX" }, { "843", 60*5, 1, "SC" }, { "845", 60*5, 1, "NY" }, { "847", 60*6, 1, "IL" }, { "848", 60*5, 1, "NJ" },
{ "850", 60*6, 1, "FL" }, { "856", 60*5, 1, "NJ" }, { "857", 60*5, 1, "MA" }, { "858", 60*8, 1, "CA" }, { "859", 60*5, 1, "KY" },
{ "860", 60*5, 1, "CT" }, { "862", 60*5, 1, "NJ" }, { "863", 60*5, 1, "FL" }, { "864", 60*5, 1, "SC" }, { "865", 60*5, 1, "TN" },
{ "867", 60*8, 1, "Yukon  NW Terr.  Nunavut" }, { "870", 60*6, 1, "AR" }, { "878", 60*5, 1, "PA" }, { "901", 60*6, 1, "TN" }, { "902", 60*4, 1, "Nova Scotia" },
{ "903", 60*6, 1, "TX" }, { "904", 60*5, 1, "FL" }, { "905", 60*5, 1, "Ontario" }, { "906", 60*5, 1, "MI" }, { "907", 60*9, 1, "AK" },
{ "908", 60*5, 1, "NJ" }, { "909", 60*8, 1, "CA" }, { "910", 60*5, 1, "NC" }, { "912", 60*5, 1, "GA" }, { "913", 60*6, 1, "KS" },
{ "914", 60*5, 1, "NY" }, { "915", 60*6, 1, "TX" }, { "916", 60*8, 1, "CA" }, { "917", 60*5, 1, "NY" }, { "918", 60*6, 1, "OK" },
{ "919", 60*5, 1, "NC" }, { "920", 60*6, 1, "WI" }, { "925", 60*8, 1, "CA" }, { "928", 60*7, 0, "AZ" }, { "931", 60*6, 1, "TN" },
{ "936", 60*6, 1, "TX" }, { "937", 60*5, 1, "OH" }, { "939", 60*5, 0, "Puerto Rico" }, { "940", 60*6, 1, "TX" }, { "941", 60*5, 1, "FL" },
{ "947", 60*5, 1, "MI" }, { "949", 60*8, 1, "CA" }, { "951", 60*8, 1, "CA" }, { "952", 60*6, 1, "MN" }, { "954", 60*5, 1, "FL" },
{ "956", 60*6, 1, "TX" }, { "970", 60*7, 1, "CO" }, { "971", 60*8, 1, "OR" }, { "972", 60*6, 1, "TX" }, { "973", 60*5, 1, "NJ" },
{ "978", 60*5, 1, "MA" }, { "979", 60*6, 1, "TX" }, { "980", 60*5, 1, "NC" }, { "985", 60*6, 1, "LA" }, { "989", 60*5, 1, "MI" },
{NULL, 0, 0, NULL}
};

struct gvtz *GetTimeData(char *tendigit)
{
	struct gvtz *ptz;

	if( tendigit == NULL )
		return NULL;

	if( strlen(tendigit) != 10 )
		return NULL;

	for(ptz = timezonestable; ptz->areacode; ptz++ )
	{
		if( strncmp(ptz->areacode, tendigit, 3) == 0 )
			break;
	}

	if( ptz->areacode )
		return ptz;
	else
		return NULL;
}

time_t AdjustGmt(char *tendigit, time_t julian)
{
	struct gvtz *ptz;

	if( tendigit == NULL )
		return julian;

	ptz = GetTimeData(tendigit);
	if( ptz == NULL )
		return julian;

	time_t adjusted = julian;

	struct tm longnow = *localtime(&adjusted);
	time_t startdst, enddst;
	time_t correction = 0;

	longnow.tm_sec = 0;
	longnow.tm_min = 4;
	longnow.tm_hour = 2;
	longnow.tm_mday = 1;
	longnow.tm_mon = 3;	// April 1st.

	startdst = mktime(&longnow);
	startdst += 86400*((7-longnow.tm_wday)%7); // first sunday in april

	longnow.tm_sec = 0;
	longnow.tm_min = 4;
	longnow.tm_hour = 2;
	longnow.tm_mday = 31;
	longnow.tm_mon = 9;	// October 31st.

	enddst = mktime(&longnow);
	enddst -= 86400*(longnow.tm_wday%7);	// last sunday in october

	if( (adjusted >= startdst) && (adjusted < enddst) )
	{
		// Daylight Saving Time applies
		if( ptz->dstused )
			correction = ptz->minswest + 60;
		else
			correction = ptz->minswest;
	}
	else
	{
		correction = ptz->minswest;
	}

	adjusted += correction*60;	

	return adjusted;
}

//
//	return the local time, in the areacode of the supplied ten digit phone number.
//
struct tm *GetAdjustedTime(char *tendigit, time_t *julian = NULL)
{
	struct gvtz *ptz;

	ptz = GetTimeData(tendigit);
	if( ptz == NULL )
		return NULL;

	time_t now;

	if( julian )
		now = *julian;
	else
		time(&now);

	struct tm longnow = *localtime(&now);
	time_t startdst, enddst;
	time_t correction = 0;

	longnow.tm_sec = 0;
	longnow.tm_min = 4;
	longnow.tm_hour = 2;
	longnow.tm_mday = 1;
	longnow.tm_mon = 3;	// April 1st.

	startdst = mktime(&longnow);
	startdst += 86400*((7-longnow.tm_wday)%7); // first sunday in april

	longnow.tm_sec = 0;
	longnow.tm_min = 4;
	longnow.tm_hour = 2;
	longnow.tm_mday = 31;
	longnow.tm_mon = 9;	// October 31st.

	enddst = mktime(&longnow);
	enddst -= 86400*(longnow.tm_wday%7);	// last sunday in october

	if( (now >= startdst) && (now < enddst) )
	{
		// Daylight Saving Time applies
		if( ptz->dstused )
			correction = ptz->minswest - 60;
		else
			correction = ptz->minswest;
	}
	else
	{
		correction = ptz->minswest;
	}

#ifdef STANDALONE
	printf("correction = %d\n", correction);
#endif
	now -= correction*60;	
	if( julian ) *julian = now;

	return gmtime(&now);
}
		
#ifdef STANDALONE
main(int ac, char **av)
{
	struct gvtz *ptz;
	struct tm *corrected;

	if( ac > 1 )
	{
		ptz = GetTimeData(av[1]);

		if( ptz )
		{
			printf("%s -> \"%s\" %d %d\n", av[1], ptz->name, ptz->minswest, ptz->dstused);
		}

		time_t now;

		time(&now);

		corrected = localtime(&now);
		printf("%s", asctime(corrected));
		corrected = GetAdjustedTime(av[1]);
		printf("%s", asctime(corrected));
	}
}
#endif
