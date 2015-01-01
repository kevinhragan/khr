#!/bin/sh

VIP_CRITICAL_PERCENT=70
VIP_CRITICAL_TAT=10
VIP_WARNING_PERCENT=80
VIP_WARNING_TAT=10

REG_CRITICAL_PERCENT=60
REG_CRITICAL_TAT=15
REG_WARNING_PERCENT=80
REG_WARNING_TAT=15

MIN_MESSAGES=10

VENDOR=""
IS_VIP=""



while getopts v:VP:p:T:t:m: opt ; do
    case "$opt" in
	v)VENDOR="$OPTARG";;
	V)IS_VIP="vip";;

	P)
	  VIP_CRITICAL_PERCENT="$OPTARG"
	  REG_CRITICAL_PERCENT="$OPTARG"
	  ;;
	p)
	  VIP_WARNING_PERCENT="$OPTARG"
	  REG_WARNING_PERCENT="$OPTARG"
	  ;;
	T)
	  VIP_CRITICAL_TAT="$OPTARG"
	  REG_CRITICAL_TAT="$OPTARG"
	  ;;
	t)
	  VIP_WARNING_TAT="$OPTARG"
	  REG_WARNING_TAT="$OPTARG"
	  ;;
	m)MIN_MESSAGES="$OPTARG";;
    esac
done



# Is this a VIP queue?
if [ -z "${IS_VIP}" ] ; then
    #CURRENT_LINE=`echo "${DATA}" | egrep -e "^${VENDOR}[^0-9a-zA-Z_\-]"`
    IS_VIP=`echo "${VENDOR}" | grep -i vip`
fi



####
# Get the data for the second potential problem.

QUERY="
set @fromdate = curdate();
set @todate = (SELECT SUBTIME(now(),'00:10:00'));
set @vendor = \"${VENDOR}\";

SELECT DISTINCT tq.vendor AS VendorName,
(100 * (SELECT count(*) FROM transcription_queue
        WHERE arrival_date >= @fromdate AND arrival_date <= @todate AND status = 'complete' AND vendor = VendorName)
     / (SELECT count(*) FROM transcription_queue
        WHERE arrival_date >= @fromdate AND arrival_date <= @todate AND vendor = VendorName) ) AS PercentComplete,
(SELECT SEC_TO_TIME( AVG( TIME_TO_SEC( TIMEDIFF(complete_date, assignment_date) ) ) )
    FROM transcription_queue WHERE arrival_date >= @fromdate AND arrival_date <= @todate
        AND complete_date != '0000-00-00 00:00:00' AND vendor = VendorName) AS AvgerageDelay,
count(*) AS TotalMessages
FROM transcription_queue as tq inner join transcription_vendor_queue as tv on (tq.vendor = tv.queue_name)
WHERE tq.arrival_date >= @fromdate AND tq.arrival_date <= @todate
AND tv.status = 'online' AND tq.vendor = @vendor
GROUP BY VendorName;
"


DATA=`mysql -s -u gvuser -pgotvoice -D gv --execute="${QUERY}"`



####
# Figure out the results

NAGIOS_STATUS="OK"
NAGIOS_DESC=""

#VENDOR_LIST=`echo "${DATA}" | awk '{print $1}'`
# wordzx  100.0000        00:04:25        1
# wordzx-vip      100.0000        00:04:34        1

#Vendor info
#CURRENT_LINE=`echo "${DATA}" | egrep -e "^${VENDOR}[^0-9a-zA-Z_\-]"`
CURRENT_LINE="${DATA}"

#for VENDOR in ${VENDOR_LIST} ; do
if [ -z "${VENDOR}" ] ; then
    NAGIOS_STATUS="CRITICAL"
    NAGIOS_DESC=" - Don't understand vendor ${VENDOR}."

# VENDOR is set.
elif [ -n "${CURRENT_LINE}" ] ; then
    # Vendor Percent
    VENDOR_PERCENT=`echo "${CURRENT_LINE}" | awk '{print $2}' | awk -F. '{print $1}'`

    # Vendor average time
    VEND_AVG_TIME=`echo "${CURRENT_LINE}" | awk '{print $3}'`
    VEND_HOUR=`echo "${VEND_AVG_TIME}" | awk -F: '{print $1}'`
    VEND_MIN=`echo "${VEND_AVG_TIME}" | awk -F: '{print $2}'`
    #VEND_SEC=`echo "${VEND_AVG_TIME}" | awk -F: '{print $3}'`

    # Number of messages during period.
    VENDOR_MSG_PROC=`echo "${CURRENT_LINE}" | awk '{print $4}'`


    if [ -n "${IS_VIP}" ] ; then
	NAGIOS_DESC=" - VIP Vendor"
    else
	NAGIOS_DESC=" - Vendor"
    fi

    # Construct the message description.  It is the same regardless of if it is a problem or not.
    NAGIOS_DESC="${NAGIOS_DESC} ${VENDOR}'s completion percent is at ${VENDOR_PERCENT}%, and average completion time is at ${VEND_AVG_TIME} for ${VENDOR_MSG_PROC} messages."


    if [ "${VENDOR_MSG_PROC}" -ge "${MIN_MESSAGES}" ] ; then
	if [ -n "${IS_VIP}" ] ; then
	    if [ "${VENDOR_PERCENT}" -le "${VIP_CRITICAL_PERCENT}" -o "${VEND_HOUR}" -ne "00" -o "${VEND_MIN}" -ge "${VIP_CRITICAL_TAT}" ] ; then
		NAGIOS_STATUS="CRITICAL"
	    elif [ "${VENDOR_PERCENT}" -le "${VIP_WARNING_PERCENT}" -o "${VEND_HOUR}" -ne "00" -o "${VEND_MIN}" -ge "${VIP_WARNING_TAT}" ] ; then
		NAGIOS_STATUS="WARNING"
	    fi
	else
	    if [ "${VENDOR_PERCENT}" -le "${REG_CRITICAL_PERCENT}" -o "${VEND_HOUR}" -ne "00" -o "${VEND_MIN}" -ge "${REG_CRITICAL_TAT}" ] ; then
		NAGIOS_STATUS="CRITICAL"
	    elif [ "${VENDOR_PERCENT}" -le "${REG_WARNING_PERCENT}" -o "${VEND_HOUR}" -ne "00" -o "${VEND_MIN}" -ge "${REG_WARNING_TAT}" ] ; then
		NAGIOS_STATUS="WARNING"
	    fi
	fi
    else
	NAGIOS_STATUS="OK"
	NAGIOS_DESC=" - Not enough data!${NAGIOS_DESC}"
    fi

# VENDOR is set, but CURRENT_LINE is not set.
else
    NAGIOS_STATUS="OK"
    NAGIOS_DESC=" - No activity for vendor ${VENDOR}."
fi

echo "${NAGIOS_STATUS}${NAGIOS_DESC}"

if [ "${NAGIOS_STATUS}" == "OK" ] ; then
    exit 0
elif [ "${NAGIOS_STATUS}" == "WARNING" ] ; then
    exit 1
fi

exit 2
