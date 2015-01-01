#!/bin/sh

WARN_LEVEL=1
CRIT_LEVEL=3

while getopts w:c: opt ; do
    case "$opt" in
	w)WARN_LEVEL="$OPTARG";;
	c)CRIT_LEVEL="$OPTARG";;
    esac
done



#####
# Get the data for the first potential problem.
#TODAY_DATE=`date | sed -e 's/^\([A-Za-z]*[ \t]*[A-Za-z]*[ \t]*[0-9]*\).*/\1/g'`
TODAY_DATE=`date | awk '{print $1" "$2" "$3" "$4}' | awk -F: '{print $1}'`
TODAY_YEAD=`date | awk '{print $6}'`

TODAYS_RESULTS=`egrep -e "^${TODAY_DATE}.*${TODAY_YEAD}" /var/gv/spool/gvcall.log`
NUM_FAILURES=`echo "${TODAYS_RESULTS}" | grep -c "Couldn't retrieve wave file"`


####
# Figure out the results

NAGIOS_STATUS="OK"
NAGIOS_DESC=""


# Check to see if things are really bad.
if [ "${NUM_FAILURES}" -ge "${CRIT_LEVEL}" ] ; then
    NAGIOS_STATUS="CRITICAL"
    NAGIOS_DESC="${NAGIOS_DESC} - Some files are not being transfered : ${NUM_FAILURES} failures today."
elif [ "${NUM_FAILURES}" -ge "${WARN_LEVEL}" ] ; then
    NAGIOS_STATUS="WARNING"
    NAGIOS_DESC="${NAGIOS_DESC} - Some files are not being transfered : ${NUM_FAILURES} failures today."
elif [ -z "${TODAYS_RESULTS}" ] ; then
    NAGIOS_STATUS="WARNING"
    NAGIOS_DESC="${NAGIOS_DESC} - Not enough data."
fi


# Generate OK status message, if everything is ok.
if [ "${NAGIOS_STATUS}" == "OK" ] ; then
    NAGIOS_DESC="${NAGIOS_DESC} - GV File Pipeline is working normally."
fi

echo "${NAGIOS_STATUS}${NAGIOS_DESC}"

if [ "${NAGIOS_STATUS}" == "OK" ] ; then
    exit 0
elif [ "${NAGIOS_STATUS}" == "WARNING" ] ; then
    exit 1
fi

exit 2
