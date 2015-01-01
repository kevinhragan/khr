struct gvtz
{
	char *areacode;
	int minswest;
	int dstused;
	char *name;
};
extern struct gvtz *GetTimeData(char *tendigit);
extern time_t AdjustGmt(char *tendigit, time_t julian);
extern struct tm *GetAdjustedTime(char *tendigit, time_t *julian);
