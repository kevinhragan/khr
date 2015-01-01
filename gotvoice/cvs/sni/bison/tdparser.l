%{
#include "tdparser.tab.h"
#include <string.h>

char *statement = NULL;

#define LOGS(string_to_add) \
	if( statement == NULL ){\
		statement = (char *)malloc(strlen(string_to_add)+2); \
		sprintf(statement, "%s ", string_to_add);\
	} else {\
		statement = (char *)realloc(statement, strlen(statement) + strlen(string_to_add) + 2); \
		strcat(statement, string_to_add); \
		strcat(statement, " "); \
	}


extern int bDebug, nDebug;
%}

%option noyywrap
%%

[ \t\n]+

rec	if(statement)free(statement); statement = NULL; LOGS(yytext); if( bDebug && (nDebug >= 0) )printf("REC : %s(%d)\n", yytext, yylval); return REC;
at	LOGS(yytext); if( bDebug && (nDebug >= 0) )printf("AT : %s(%d)\n", yytext, yylval); return AT;
td	LOGS(yytext); if( bDebug && (nDebug >= 0) )printf("TD : %s(%d)\n", yytext, yylval); return TD;
yd	LOGS(yytext); if( bDebug && (nDebug >= 0) )printf("YD : %s(%d)\n", yytext, yylval); return YD;
am	LOGS(yytext); if( bDebug && (nDebug >= 0) )printf("AM : %s(%d)\n", yytext, yylval); return AM;
pm	LOGS(yytext); if( bDebug && (nDebug >= 0) )printf("PM : %s(%d)\n", yytext, yylval); return PM;

20[0-9][0-9]	LOGS(yytext); yylval = atoi(yytext) - 1900; if( bDebug && (nDebug >= 0) )printf("YEAR : %s(%d)\n", yytext, yylval); return YEAR;
oh	LOGS(yytext); yylval = 0; if( bDebug && (nDebug >= 0) )printf("OH : %s(%d)\n", yytext, yylval); return OH;
[0-9]+th	LOGS(yytext); yylval = atoi(yytext); if( bDebug && (nDebug >= 0) )printf("ORD : %s(%d)\n", yytext, yylval); return ORD;
1st	LOGS(yytext); yylval = atoi(yytext); if( bDebug && (nDebug >= 0) )printf("ORD : %s(%d)\n", yytext, yylval); return ORD;
2nd	LOGS(yytext); yylval = atoi(yytext); if( bDebug && (nDebug >= 0) )printf("ORD : %s(%d)\n", yytext, yylval); return ORD;
3rd	LOGS(yytext); yylval = atoi(yytext); if( bDebug && (nDebug >= 0) )printf("ORD : %s(%d)\n", yytext, yylval); return ORD;
[0-9]+	LOGS(yytext); yylval = atoi(yytext); if( bDebug && (nDebug >= 0) )printf("NUM : %s(%d)\n", yytext, yylval); return NUM;

jan 	LOGS(yytext); yylval = 0; if( bDebug && (nDebug >= 0) )printf("MON : %s(%d)\n", yytext, yylval); return MON;
feb 	LOGS(yytext); yylval = 1; if( bDebug && (nDebug >= 0) )printf("MON : %s(%d)\n", yytext, yylval); return MON;
mar 	LOGS(yytext); yylval = 2; if( bDebug && (nDebug >= 0) )printf("MON : %s(%d)\n", yytext, yylval); return MON;
apr 	LOGS(yytext); yylval = 3; if( bDebug && (nDebug >= 0) )printf("MON : %s(%d)\n", yytext, yylval); return MON;
may 	LOGS(yytext); yylval = 4; if( bDebug && (nDebug >= 0) )printf("MON : %s(%d)\n", yytext, yylval); return MON;
jun 	LOGS(yytext); yylval = 5; if( bDebug && (nDebug >= 0) )printf("MON : %s(%d)\n", yytext, yylval); return MON;
jul 	LOGS(yytext); yylval = 6; if( bDebug && (nDebug >= 0) )printf("MON : %s(%d)\n", yytext, yylval); return MON;
aug 	LOGS(yytext); yylval = 7; if( bDebug && (nDebug >= 0) )printf("MON : %s(%d)\n", yytext, yylval); return MON;
sep 	LOGS(yytext); yylval = 8; if( bDebug && (nDebug >= 0) )printf("MON : %s(%d)\n", yytext, yylval); return MON;
oct 	LOGS(yytext); yylval = 9; if( bDebug && (nDebug >= 0) )printf("MON : %s(%d)\n", yytext, yylval); return MON;
nov 	LOGS(yytext); yylval = 10; if( bDebug && (nDebug >= 0) )printf("MON : %s(%d)\n", yytext, yylval); return MON;
dec 	LOGS(yytext); yylval = 11; if( bDebug && (nDebug >= 0) )printf("MON : %s(%d)\n", yytext, yylval); return MON;

sun	LOGS(yytext); yylval = 0; if( bDebug && (nDebug >= 0) )printf("DOW : %s(%d)\n", yytext, yylval); return DOW;
mon	LOGS(yytext); yylval = 1; if( bDebug && (nDebug >= 0) )printf("DOW : %s(%d)\n", yytext, yylval); return DOW;
tue	LOGS(yytext); yylval = 2; if( bDebug && (nDebug >= 0) )printf("DOW : %s(%d)\n", yytext, yylval); return DOW;
wed	LOGS(yytext); yylval = 3; if( bDebug && (nDebug >= 0) )printf("DOW : %s(%d)\n", yytext, yylval); return DOW;
thu	LOGS(yytext); yylval = 4; if( bDebug && (nDebug >= 0) )printf("DOW : %s(%d)\n", yytext, yylval); return DOW;
fri	LOGS(yytext); yylval = 5; if( bDebug && (nDebug >= 0) )printf("DOW : %s(%d)\n", yytext, yylval); return DOW;
sat	LOGS(yytext); yylval = 6; if( bDebug && (nDebug >= 0) )printf("DOW : %s(%d)\n", yytext, yylval); return DOW;

%%
