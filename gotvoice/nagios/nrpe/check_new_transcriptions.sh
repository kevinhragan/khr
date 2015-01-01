#!/bin/sh

WARN_AT=20
CRIT_AT=25

while getopts w:c: opt ; do
    case "$opt" in
	w)WARN_AT="$OPTARG";;
	c)CRIT_AT="$OPTARG";;
    esac
done



#####
# Get the data for the first potential problem.
NEW_COUNT_CMD=`mysql -s -u gvuser -pgotvoice -e "select count(*) from transcriptions.transcription_queue where status='new';" 2>&1`

# Check to see if we are connecting to the database.
TEST=`echo "${NEW_COUNT_CMD}" | grep -c -i ERROR`
if [ "${TEST}" -le 0 ] ; then

    NEW_COUNT=`echo "${NEW_COUNT_CMD}" | awk '{print $1}'`

    ####
    # Figure out the results

    NAGIOS_STATUS="OK"
    NAGIOS_DESC=""


    # Check to see if things are really bad.
    if [ "${NEW_COUNT}" -ge "${CRIT_AT}" ] ; then
	NAGIOS_STATUS="CRITICAL"
    elif [ "${NEW_COUNT}" -ge "${WARN_AT}" ] ; then
	NAGIOS_STATUS="WARNING"
    fi

    # Generate the messages that reports our status.
    NAGIOS_DESC="${NAGIOS_DESC} - Currently we have ${NEW_COUNT} new (unassigned) transcriptions."

else
    NAGIOS_STATUS="CRITICAL"
    NAGIOS_DESC="${NAGIOS_DESC} - Can't connect to the database!"
fi


echo "${NAGIOS_STATUS}${NAGIOS_DESC}"

if [ "${NAGIOS_STATUS}" == "OK" ] ; then
    exit 0
elif [ "${NAGIOS_STATUS}" == "WARNING" ] ; then
    exit 1
fi

exit 2
