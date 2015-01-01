package Mail::DeliveryStatus::Report;

use Mail::Header;
use vars qw(@ISA);
@ISA = qw(Mail::Header);

# i just don't like how Mail::Header leaves a \n at the end of everything
# meng

sub get {
  my $string = $_[0]->SUPER::get($_[1]);
  chomp($string=(defined $string && length $string) ? $string : "");
  $string;
}

1;
