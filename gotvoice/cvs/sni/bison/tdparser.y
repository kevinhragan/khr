/* Parser for GotVoice time/date strings 

	martind@gotvoice.com 3/4/07

*/
%{

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "tdparser.h"

#define YYSTYPE time_t

int yylex(void);
void yyerror( char *s );

struct timeval utc;
time_t now;
struct tm *longnow;

char *options = "?R:D:U:";
char *routing = NULL;

#define BOOL int
#define TRUE 1
#define FALSE 0
BOOL bDebug = FALSE;
int nDebug = 0;
extern char *statement;

%}

%token ORD NUM REC AT TD YD AM PM OH MON YEAR DOW

%% /* Grammar rules and actions follow */

input : /* empty string */
	| input line
	| input error
	;

line :  exp { char *gt = asctime(gmtime(&$1)); gt[24] = '\0'; printf("%u\t%s\t%s\t%s\n", AdjustGmt(routing, $1), routing, gt, statement); }
	;

exp : 	REC date AT time AM	{ $$ = $2 + $4; if( bDebug && (nDebug >= 0) )printf("$$ = %d %s\n", $$, asctime(gmtime(&$$))); }
	| REC date AT time PM	{ $$ = $2 + $4 + 3600*12; if( bDebug && (nDebug >= 0) )printf("$$ = %d %s\n", $$, asctime(gmtime(&$$))); }
	| REC AT time AM	{ $$ = (now - (now%(24*3600))) + $3; 
					if( bDebug && (nDebug >= 0) )printf("$$ = %d %s\n", $$, asctime(gmtime(&$$))); }
	| REC AT time PM	{ $$ = (now - (now%(24*3600))) + $3 + 3600*12; 
					if( bDebug && (nDebug >= 0) )printf("$$ = %d %s\n", $$, asctime(gmtime(&$$))); }
	;

date :	TD	{ 
			struct tm *ltmp, *gtmp; 
			int lmd, gmd;
			time_t offset = 0;

			ltmp = localtime(&now); lmd = ltmp->tm_mday;

			gtmp = gmtime(&now); gmd = gtmp->tm_mday;

			if( gmd != lmd )
				offset = 24*3600;
			
			$$ = now - (now%(24*3600)) - offset; 

			if( bDebug && (nDebug >= 0) )
				printf("$$ = %d %s", $$, asctime(gmtime(&$$)) ); 
		}
	| YD	{ 
			struct tm *ltmp, *gtmp; 
			int lmd, gmd;
			time_t offset = 0;

			ltmp = localtime(&now); lmd = ltmp->tm_mday;

			gtmp = gmtime(&now); gmd = gtmp->tm_mday;

			if( gmd != lmd )
				offset = 24*3600;
			
			$$ = now - (now%(24*3600)) - 24*3600 - offset; 

			if( bDebug && (nDebug >= 0) )
				printf("$$ = %d %s\n", $$, asctime(gmtime(&$$))); 
		}
	| DOW	{ 
			struct tm *ltmp, *gtmp; 
			int lmd, gmd;
			time_t offset = 0;

			ltmp = localtime(&now); lmd = ltmp->tm_mday;

			gtmp = gmtime(&now); gmd = gtmp->tm_mday;

			if( gmd != lmd )
				offset = 24*3600;
			
			$$ = now - (now%(24*3600)) - (((7 - $1) + longnow->tm_wday)*24*3600) - offset; 

			if( bDebug && (nDebug >= 0) )
				printf("$$ = %d %s\n", $$, asctime(gmtime(&$$))); 
		}
	| DOW monthdate	{ 
			$$ = $2; 

			if( bDebug && (nDebug >= 0) )
				printf("$$ = %d %s\n", $$, asctime(gmtime(&$$))); 
		}
	;

monthdate : MON ORD 
	{
		// we calculate the time to midnight (am) on the day in question, then we add the time later
		// Here we construct a tm structure and then cast it into a julian time.

		struct tm monthtime;
		struct tm *local = localtime(&now);
		time_t pt;

		// midnight on the day in question

		monthtime.tm_sec = 0;
		monthtime.tm_min = 0;
		monthtime.tm_hour = 0;
		monthtime.tm_mday = $2;
		monthtime.tm_mon = $1;
		monthtime.tm_year = local->tm_year;
		monthtime.tm_isdst = 0;

		pt = mktime(&monthtime);

		if( pt > now )
		{
			monthtime.tm_year--;
			pt = mktime(&monthtime);
		}

		$$ = pt - (pt%(24*3600));;
	}
	| MON NUM ORD
	{
		// we calculate the time to midnight (am) on the day in question, then we add the time later
		// Here we construct a tm structure and then cast it into a julian time.

		struct tm monthtime;
		struct tm *local = localtime(&now);
		time_t pt;

		// midnight on the day in question

		monthtime.tm_sec = 0;
		monthtime.tm_min = 0;
		monthtime.tm_hour = 0;
		monthtime.tm_mday = $2 + $3;
		monthtime.tm_mon = $1;
		monthtime.tm_year = local->tm_year;
		monthtime.tm_isdst = 0;

		pt = mktime(&monthtime);

		if( pt > now )
		{
			monthtime.tm_year--;
			pt = mktime(&monthtime);
		}

		$$ = pt - (pt%(24*3600));;
	}
	| monthdate YEAR
	{
		// we calculate the time to midnight (am) on the day in question, then we add the time later
		// Here we construct a tm structure and then cast it into a julian time.

		struct tm *local = localtime(&now);
		struct tm *gmt = gmtime(&$1);
		struct tm monthtime = *gmt;

		time_t pt;

		monthtime.tm_year = $2;

		pt = mktime(&monthtime);

		if( pt > now )
		{
			monthtime.tm_year--;
			pt = mktime(&monthtime);
		}

		$$ = pt - (pt%(24*3600));;
	}
	;

time :	NUM	{ $$ = $1%12 * 3600; if( bDebug && (nDebug >= 0) )printf("$$ = %d %s\n", $$, asctime(gmtime(&$$))); }
	| NUM OH NUM { $$ = $1%12*3600 + $3*60; if( bDebug && (nDebug >= 0) )printf("$$ = %d %s\n", $$, asctime(gmtime(&$$))); }
	| NUM NUM { $$ = $1%12*3600 + $2*60; if( bDebug && (nDebug >= 0) )printf("$$ = %d %s\n", $$, asctime(gmtime(&$$))); }
	| NUM NUM NUM { $$ = $1%12*3600 + $2*60 + $3*60; if( bDebug && (nDebug >= 0) )printf("$$ = %d %s\n", $$, asctime(gmtime(&$$))); }  
	;

%%

/* Lexical analyzer returns a double floating point 
number on the stack and the token NUM, or the ASCII
character read if not a number.  Skips all blanks
and tabs, returns 0 for EOF. */

main(int ac, char **av)
{
	int oc;

	time(&now);

	while( (oc = getopt(ac, av, options)) > 0 )
	{
		switch( oc )
		{
		case '?':
		default:
			printf("parse -R<routing>\n");
			exit(0);

		case 'R':
			routing = strdup(optarg);
			break;

		case 'U':
			now = atoi(optarg);
			break;

		case 'D':
			bDebug = TRUE;
			nDebug = atoi(optarg);
			break;
		}
	}

	if( bDebug && (nDebug >= 0) )
		printf("UTC now = (%d) %s", now, asctime(gmtime(&now)));

	longnow = GetAdjustedTime(routing, &now);

	if( bDebug && (nDebug >= 0) )
		printf("adjusted now = (%d) %s", now, asctime(gmtime(&now)));

	yyparse ();
}

void yyerror (char *s)  /* Called by yyparse on error */
{
	printf ("oops : %s\n", s);
}
