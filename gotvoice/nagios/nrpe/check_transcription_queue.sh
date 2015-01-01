#!/bin/sh

WARN_LEVEL=5
CRIT_LEVEL=10

while getopts w:c: opt ; do
    case "$opt" in
	w)WARN_LEVEL="$OPTARG";;
	c)CRIT_LEVEL="$OPTARG";;
    esac
done



#####
# Get the data for the first potential problem.
DATA=`mysql -s -u gvuser -pgotvoice -D gv <<EOF
select \\\`date\\\`, \\\`new\\\`, assigned, complete, deleted, timeout, \\\`timeout-complete\\\` from transcription_queue_stats order by date desc limit 1;
EOF`

STATUS_NEW=`echo "${DATA}" | awk '{print $3}'`
STATUS_TIMEOUT=`echo "${DATA}" | awk '{print $7}'`


####
# Figure out the results

NAGIOS_STATUS="OK"
NAGIOS_DESC=""


# Check to see if things are really bad.
if [ "${STATUS_NEW}" -ge "${CRIT_LEVEL}" ] ; then
    NAGIOS_STATUS="CRITICAL"
    NAGIOS_DESC="${NAGIOS_DESC} - Transcription queue is getting full : ${STATUS_NEW} new"
elif [ "${STATUS_NEW}" -ge "${WARN_LEVEL}" ] ; then
    NAGIOS_STATUS="WARNING"
    NAGIOS_DESC="${NAGIOS_DESC} - Transcription queue on the rise : ${STATUS_NEW} new"
fi


# Generate OK status message, if everything is ok.
if [ "${NAGIOS_STATUS}" == "OK" ] ; then
    NAGIOS_DESC="${NAGIOS_DESC} - We are keeping up with transcriptions."
fi

echo "${NAGIOS_STATUS}${NAGIOS_DESC}"

if [ "${NAGIOS_STATUS}" == "OK" ] ; then
    exit 0
elif [ "${NAGIOS_STATUS}" == "WARNING" ] ; then
    exit 1
fi

exit 2
