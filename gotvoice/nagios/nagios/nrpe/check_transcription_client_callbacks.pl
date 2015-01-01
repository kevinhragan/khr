#!/usr/bin/perl -w

# check that callbacks to transcription clients (using transcription API) are not erroring out

use strict;
use DBI;

my $noReplyLimit = 5;

my $dbh = DBI->connect('DBI:mysql:transcriptions','gvuser','gotvoice');

my $query="select client_code, count(*) as count
	from transcription_client_queue where status='complete'
	and (reply_status='none' or reply_status='reply-error')
	and complete_date > date_sub(now(), interval 30 minute)
	and complete_date < date_sub(now(), interval 10 minute)
	group by client_code;";

my $query_output = $dbh->prepare($query);
$query_output->execute;
my $nagios_status="OK";
my $nagios_msg="";

while(my($client_code,$count)=$query_output->fetchrow_array) {
    if ($count > $noReplyLimit) {
       $nagios_status="WARNING"; 
       $nagios_msg=$nagios_msg . "Client $client_code has $count callbacks with no reply or reply errors in the last half-hour. ";
    }
}

print "$nagios_status $nagios_msg\n";
if ($nagios_status eq "OK") {
    exit 0;
} elsif ($nagios_status eq "WARNING") {
    exit 1;
} else {
    exit 2;
}

