#!/bin/sh

WARN_LEVEL=20
CRIT_LEVEL=10

while getopts w:c: opt ; do
    case "$opt" in
	w)WARN_LEVEL="$OPTARG";;
	c)CRIT_LEVEL="$OPTARG";;
    esac
done



#####
# Get the data for the first potential problem.
MAX_CONNECTIONS=`mysql -s -u gvuser -pgotvoice -e "show variables like 'max_connections';" 2>&1`

# Check to see if we are connecting to the database.
TEST=`echo "${MAX_CONNECTIONS}" | grep -c -i ERROR`
if [ "${TEST}" -le 0 ] ; then

    MAX_CONNECTIONS=`echo "${MAX_CONNECTIONS}" | awk '{print $2}'`

    CURRENT_CONNECTIONS=`mysql -s -u gvuser -pgotvoice -e "show processlist;" 2>&1 | wc -l`

    AVAILABLE_CONNECTIONS=`expr ${MAX_CONNECTIONS} - ${CURRENT_CONNECTIONS}`
    PERCENT_AVAILABLE=`expr 100 - \( \( 100 \* ${CURRENT_CONNECTIONS} \) / ${MAX_CONNECTIONS} \)`


    ####
    # Figure out the results

    NAGIOS_STATUS="OK"
    NAGIOS_DESC=""


    # Check to see if things are really bad.
    if [ "${PERCENT_AVAILABLE}" -le "${CRIT_LEVEL}" ] ; then
	NAGIOS_STATUS="CRITICAL"
    elif [ "${PERCENT_AVAILABLE}" -le "${WARN_LEVEL}" ] ; then
	NAGIOS_STATUS="WARNING"
    fi

    # Generate the messages that reports our status.
    NAGIOS_DESC="${NAGIOS_DESC} - Currently we have ${PERCENT_AVAILABLE}% of our database conections free.  We have ${AVAILABLE_CONNECTIONS} out of ${MAX_CONNECTIONS} free connections."

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
