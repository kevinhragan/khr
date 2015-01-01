use 5.00503;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'		=> 'Mail::DeliveryStatus::BounceParser',
    'VERSION_FROM'	=> 'BounceParser.pm', # finds $VERSION
    'PREREQ_PM'		=> {
       "MIME::Entity"  => 5.404,
     },
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'BounceParser.pm', # retrieve abstract from module
       AUTHOR     => 'Meng Weng Wong <mengwong+bounceparser@pobox.com>')
      : ()),
     );

