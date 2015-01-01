#!/bin/sh

VIP_CRITICAL_TIMEOUT_ABORT=1
VIP_CRITICAL_TIMEOUT_COMPLETE=1000
VIP_WARNING_TIMEOUT_ABORT=1
VIP_WARNING_TIMEOUT_COMPLETE=900

REG_CRITICAL_TIMEOUT_ABORT=10
REG_CRITICAL_TIMEOUT_COMPLETE=1000
REG_WARNING_TIMEOUT_ABORT=6
REG_WARNING_TIMEOUT_COMPLETE=900

MIN_MESSAGES=0

VENDOR=""
IS_VIP=""



while getopts v:VA:a:C:c: opt ; do
    case "$opt" in
	v)VENDOR="$OPTARG";;
	V)IS_VIP="vip";;

	A)
	  VIP_CRITICAL_TIMEOUT_ABORT="$OPTARG"
	  REG_CRITICAL_TIMEOUT_ABORT="$OPTARG"
	  ;;
	a)
	  VIP_WARNING_TIMEOUT_ABORT="$OPTARG"
	  REG_WARNING_TIMEOUT_ABORT="$OPTARG"
	  ;;
	C)
	  VIP_CRITICAL_TIMEOUT_COMPLETE="$OPTARG"
	  REG_CRITICAL_TIMEOUT_COMPLETE="$OPTARG"
	  ;;
	c)
	  VIP_WARNING_TIMEOUT_COMPLETE="$OPTARG"
	  REG_WARNING_TIMEOUT_COMPLETE="$OPTARG"
	  ;;
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
set @fromdate = (SELECT SUBTIME(now(),'01:00:00'));
set @todate = now();
set @vendor = \"${VENDOR}\";

SELECT DISTINCT tq.vendor AS VendorName,
(SELECT count(*) FROM transcription_queue
    WHERE arrival_date >= @fromdate AND arrival_date <= @todate AND complete_date != '0000-00-00 00:00:00'
    AND vendor = VendorName AND status = 'timeout-abort') AS T_O,
(SELECT count(*) FROM transcription_queue
    WHERE arrival_date >= @fromdate AND arrival_date <= @todate AND complete_date != '0000-00-00 00:00:00'
    AND vendor = VendorName AND status = 'timeout-complete') AS T_C,
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
# <vendor>        <t_o>   <t-c>   <total_messages>
# nairobi-regular 0       1       50


#Vendor info
#CURRENT_LINE=`echo "${DATA}" | egrep -e "^${VENDOR}[^0-9a-zA-Z_\-]"`
CURRENT_LINE="${DATA}"

#for VENDOR in ${VENDOR_LIST} ; do
if [ -z "${VENDOR}" ] ; then
    NAGIOS_STATUS="CRITICAL"
    NAGIOS_DESC=" - Don't understand vendor ${VENDOR}."

# VENDOR is set.
elif [ -n "${CURRENT_LINE}" ] ; then
    # Vendor Timeout Abort, and Timeout Complete.
    VENDOR_TIMEOUT_ABORT=`echo "${CURRENT_LINE}" | awk '{print $2}'`
    VENDOR_TIMEOUT_COMPLETE=`echo "${CURRENT_LINE}" | awk '{print $3}'`

    # Number of messages during period.
    VENDOR_MSG_PROC=`echo "${CURRENT_LINE}" | awk '{print $4}'`


    if [ -n "${IS_VIP}" ] ; then
	NAGIOS_DESC=" - VIP Vendor"
    else
	NAGIOS_DESC=" - Vendor"
    fi

    # Construct the message description.  It is the same regardless of if it is a problem or not.
    NAGIOS_DESC="${NAGIOS_DESC} ${VENDOR}'s timeout aborts : ${VENDOR_TIMEOUT_ABORT}, timeout completes : ${VENDOR_TIMEOUT_COMPLETE} for ${VENDOR_MSG_PROC} messages."


    if [ "${VENDOR_MSG_PROC}" -ge "${MIN_MESSAGES}" ] ; then
	if [ -n "${IS_VIP}" ] ; then
	    if [ "${VENDOR_TIMEOUT_ABORT}" -ge "${VIP_CRITICAL_TIMEOUT_ABORT}" -o "${VENDOR_TIMEOUT_COMPLETE}" -ge "${VIP_CRITICAL_TIMEOUT_COMPLETE}" ] ; then
		NAGIOS_STATUS="CRITICAL"
	    elif [ "${VENDOR_TIMEOUT_ABORT}" -ge "${VIP_WARNING_TIMEOUT_ABORT}" -o "${VENDOR_TIMEOUT_COMPLETE}" -ge "${VIP_WARNING_TIMEOUT_COMPLETE}" ] ; then
		NAGIOS_STATUS="WARNING"
	    fi
	else
	    if [ "${VENDOR_TIMEOUT_ABORT}" -ge "${REG_CRITICAL_TIMEOUT_ABORT}" -o "${VENDOR_TIMEOUT_COMPLETE}" -ge "${REG_CRITICAL_TIMEOUT_COMPLETE}" ] ; then
		NAGIOS_STATUS="CRITICAL"
	    elif [ "${VENDOR_TIMEOUT_ABORT}" -ge "${REG_WARNING_TIMEOUT_ABORT}" -o "${VENDOR_TIMEOUT_COMPLETE}" -ge "${REG_WARNING_TIMEOUT_COMPLETE}" ] ; then
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
