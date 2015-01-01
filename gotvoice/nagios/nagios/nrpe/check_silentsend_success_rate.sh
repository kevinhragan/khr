#!/bin/sh

CRITICAL_PERCENT=10
WARNING_PERCENT=80

VENDOR=""


while getopts v:P:p: opt ; do
    case "$opt" in
	v)VENDOR="$OPTARG";;
	P)CRITICAL_PERCENT="$OPTARG";;
	p)WARNING_PERCENT="$OPTARG";;
    esac
done




####
# Get the data for the second potential problem.
QUERY="select email_identifier from vendor where vendorcode = '${VENDOR}';"
VENDOR_DOMAIN=`mysql -s -u gvuser -pgotvoice -D gv -h server1 --execute="${QUERY}"`

if [ -n "${VENDOR_DOMAIN}" ] ; then
    QUERY="
select AVG(messages_sent)*100, count(*) from iface_foxradio_callback
where caller_email like '%${VENDOR_DOMAIN}%'
and delivery_time > SUBTIME(now(),'05:00:00') and messages_sent >= 0;
"
    DATA=`mysql -s -u gvuser -pgotvoice -D gv -h server1 --execute="${QUERY}"`
fi



####
# Figure out the results

NAGIOS_STATUS="OK"
NAGIOS_DESC=""


#for VENDOR in ${VENDOR_LIST} ; do
if [ -z "${VENDOR}" -o -z "${VENDOR_DOMAIN}" ] ; then
    NAGIOS_STATUS="CRITICAL"
    NAGIOS_DESC=" - Don't understand vendor ${VENDOR}."

# VENDOR is set.
elif [ -n "${DATA}" ] ; then

    # Vendor Percent
    VENDOR_PERCENT=`echo "${DATA}" | awk -F. '{print $1}'`

    # Vendor Message Count
    VENDOR_MSG_PROC=`echo "${DATA}" | awk '{print $2}'`

    # Make sure we have some valid numbers.
    if [ "${VENDOR_MSG_PROC}" -gt 0 ] ; then

        # Construct the message description.  It is the same regardless of if it is a problem or not.
	NAGIOS_DESC=" - Vendor ${VENDOR}'s completion percent is at ${VENDOR_PERCENT}% for ${VENDOR_MSG_PROC} messages."

	if [ "${VENDOR_PERCENT}" -lt "${CRITICAL_PERCENT}" ] ; then
	    NAGIOS_STATUS="CRITICAL"
	elif [ "${VENDOR_PERCENT}" -lt "${WARNING_PERCENT}" ] ; then
	    NAGIOS_STATUS="WARNING"
	fi
    fi
fi


# If we have not results, then there was no valid data.
if [ -z "${NAGIOS_DESC}" ] ; then
    NAGIOS_STATUS="OK"
    NAGIOS_DESC=" - No activity for vendor ${VENDOR}."
fi



# Return status.
echo "${NAGIOS_STATUS}${NAGIOS_DESC}"

if [ "${NAGIOS_STATUS}" == "OK" ] ; then
    exit 0
elif [ "${NAGIOS_STATUS}" == "WARNING" ] ; then
    exit 1
fi

exit 2
