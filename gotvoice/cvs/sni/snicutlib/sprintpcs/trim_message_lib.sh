#!/bin/sh
# NEVER USE exit.  USE return.
# return values:
#   0: script ran normally
#   2: invalid parameters
#
#
trim_message()
{
    _TM_ORIG_MESSAGE="$1"
    _TM_SNI_PROVIDER_LIB="$2"

    # Return 0 is true, everything is false or error.
    # Ideally each return code will be unique.
    _TM_RETURN_CODE="0"


    # Do a bunch of error checking.
    # original message exists.
    if [ -z "${_TM_ORIG_MESSAGE}" -o ! -r "${_TM_ORIG_MESSAGE}" ] ; then
	_TM_RETURN_CODE="2"
	echo `date`" Invalid message to trim ${_TM_ORIG_MESSAGE}."
    else
	_TM_MSG_LOC="`dirname ${_TM_ORIG_MESSAGE}`"
	_TM_MSG_BASE="`echo ${_TM_ORIG_MESSAGE} | sed -e 's/.wav$//g'`"
    fi


    # provider info exists for the snicut
    _TM_SNICUTLIB_CHECK="`ls -1d ${_TM_SNI_PROVIDER_LIB} 2> /dev/null`"
    if [ -z "${_TM_SNICUTLIB_CHECK}" -o ! -r "${_TM_SNI_PROVIDER_LIB}" ] ; then
	_TM_RETURN_CODE="2"
	echo `date`" Invalid snicut libraries ${_TM_SNI_PROVIDER_LIB}."
    fi


    if [ "${_TM_RETURN_CODE}" -eq "0" ] ; then
	pushd "${_TM_MSG_LOC}" > /dev/null

	snicut_output="`/usr/local/bin/snicut -l${_TM_SNI_PROVIDER_LIB} ${_TM_ORIG_MESSAGE} -BspPM3,spAM 2>&1`"
	echo `date`" snicut output ${snicut_output}"

	#Get sizes
	_TM_ORIG_FILE_SIZE="`_tm_calc_size ${_TM_ORIG_MESSAGE}`"
	_TM_NEW_FILE_SIZE="`_tm_calc_size ${_TM_MSG_BASE}_000.wav`"
	_TM_FILE_SIZE_DIFF="`expr ${_TM_ORIG_FILE_SIZE} - ${_TM_NEW_FILE_SIZE}`"

	#Determine success.
	if [ "${_TM_NEW_FILE_SIZE}" -gt "60" -a "${_TM_FILE_SIZE_DIFF}" -gt "0" -a "${_TM_FILE_SIZE_DIFF}" -lt "240000" ] ; then
	    move_output="`mv ${_TM_MSG_BASE}_000.wav ${_TM_ORIG_MESSAGE} 2>&1`"
	    echo `date`" Using snicut file. ${move_output}"
	else
	    remove_output="`rm ${_TM_MSG_BASE}_000.wav 2>&1`"
	    echo `date`" Discarding snicut file. ${remove_output}"
	    
	    #Try another way?
            ###snicut_output="`/usr/local/bin/snicut -l${_TM_SNI_PROVIDER_LIB} ${_TM_ORIG_MESSAGE} -BspPM3,spAM -EspReplay 2>&1`"

	    _TM_RETURN_CODE="10"

	    if [ "${_TM_FILE_SIZE_DIFF}" -lt "0" -o "${_TM_FILE_SIZE_DIFF}" -gt "240000" ] ; then

		# Setting the error code even higher.  The caller should send an email when the return code is above 100
		_TM_RETURN_CODE="101"
		echo `date`" snicut made a really bad cut.  diff is ${_TM_FILE_SIZE_DIFF}"
	    fi
	fi

	# return from where we whence came.
	popd > /dev/null
    fi

    return "${_TM_RETURN_CODE}"
}


_tm_calc_size()
{
    _TM_FILE="$1"

    if [ -n "${_TM_FILE}" ] ; then
	_TM_SIZE="`ls -l ${_TM_FILE} 2> /dev/null | awk '{print $5}'`"
    fi    
    if [ -z "${_TM_SIZE}" ] ; then
	_TM_SIZE="0"
    fi

    echo "${_TM_SIZE}"
}


if [ -n "$1" -a -n "$2" ] ; then
    trim_message "$1" "$2"
fi
