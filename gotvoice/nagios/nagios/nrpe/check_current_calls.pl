#!/usr/bin/perl -w
#
# check_current_calls
# Checks how many calls are being made on the asterisk server.
# Author: Allen Caldwell

use strict;
use Getopt::Long qw(:config no_ignore_case bundling);
use lib "/usr/local/nagios/libexec";
use utils qw($TIMEOUT %ERRORS &print_revision &support &usage); # Import predefined status-codes and timeout variable
use File::Basename;

my $prog_name  = basename $0;
my $prog_ver   = "1.0";
my $status = 3; #do we want to start with UNKNOWN???
my $status_print = "";
my $warn_val = 50;  # Default - override with -w arg
my $crit_val = 75;  # Default - override with -c arg
my $help_val = 0;  # Off unless -h arg
my $verb_val = 0;  # Off unless -v arg
my $vers_val = 0;  # Off unless -V arg

# Gives us a way to print out verbose debug information to the screen when user
# passes in a -v argument.
# print_debug() should receive one parameter: a text string to print out.
sub print_debug() {
    my $message = shift;
    if ($verb_val == 1) {
        print "DEBUG: " . $message . "\n";
    }
}

# Show help information
sub show_help() {
    print '

Perl-Monitor the asterisk server for current calls.  A plugin for Nagios

Usage: $prog_name -w  -c  [-h] [-V] [-v]

-w, --warning=INTEGER
   Number of revisions behind current daily.cvd to generate a warning state (Default: 1)
-c, --critical=INTEGER
   Number of revisions behind current daily.cvd to generate a critical state (Default: 2)
-h, --help
   Show this help
-V, --version
   Output version information for the plugin
-v, --verbose
   Enable verbose output
'
}

GetOptions (
        "w=i" => \$warn_val, "warning=i" => \$warn_val,
        "c=i" => \$crit_val, "critical=i" => \$crit_val,
        "h" => \$help_val, "help" => \$help_val,
        "V" => \$vers_val, "version" => \$vers_val,
        "v" => \$verb_val, "verbose" => \$verb_val,
);
&print_debug("GetOptions: warning: $warn_val, critical: $crit_val, help value: $help_val, version: $vers_val
 ");

if ($help_val != 0) {
    &show_help;
    exit $ERRORS{'OK'};
}

if ($vers_val != 0) {
    &print_revision($prog_name,$prog_ver);
    exit $ERRORS{'OK'};
}

#Let's get the number of calls being made...
my @sentence = split " ",  `asterisk -rx "sip show channels" | grep active`;
my $how_many = $sentence[0];
&print_debug("Calls being made: $how_many");

#We've got our number, now let's figure out what to return...
#Nagios protocol requires a <= .
if ( $how_many <= $warn_val ) { $status = $ERRORS{'OK'};
                              $status_print = "OK"; }
elsif ( $how_many > $warn_val && $how_many <= $crit_val ) { $status = $ERRORS{'WARNING'};
                              $status_print = "WARNING";}
elsif ( $how_many > $crit_val ) { $status = $ERRORS{'CRITICAL'};
                              $status_print = "CRITICAL"; }
else { $status = $ERRORS{'UNKNOWN'} ;
           $status_print = "UNKNOWN" ; }

#let's send back the stuff we need to send back...single descriptive line printed and
#exit value for nagios.
print $status_print . " - " .
         $how_many . " call(s) being made on the asterisk server.\n" ;
exit $status;
