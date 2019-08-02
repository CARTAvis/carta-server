#!/usr/bin/perl

use strict;
use POSIX 'setsid';
use CGI::Carp qw(fatalsToBrowser);
use CGI qw(:standard);
use JSON;

my $username = $ENV{LOGNAME} || getpwuid($<) || $ENV{USER}; 

use MongoDB ();
use Data::Dumper qw(Dumper);
my $client = MongoDB::MongoClient->new(host => 'localhost', port => 27017);
my $db = $client->get_database( 'CARTA'  );

my $people_coll = $db->get_collection('userconf');
my $people = $people_coll->find;
my $tester = 1;
my $token = "no_token";
my $socket = 3002;
my $sockstring = "socket3002";

while (my $p = $people->next) {
    if ( $p->{"username"} eq $username ) {
	$token = $p->{"token"};
	$socket = $p->{"socket"};
	$tester = 0;
    }
}

$sockstring = "socket" . $socket;

my %json_hash= ('username' => $username, 'token' => $token, 'socket' => $sockstring);

print header('application/json');
my $json1 = encode_json \%json_hash;
print "$json1";

open(my $outfile, '>', "logs/carta-perl.$$") or die "can't open log file";
print $outfile "$json1 \n";
close $outfile;

defined (my $kid = fork) or die "Cannot fork: $!\n";
if ($kid) {
    waitpid($kid,0);
} else {
    defined (my $grandkid = fork) or die "Kid cannot fork: $!\n";
    if ($grandkid) {
	CORE::exit(0);
    } else {
	my $string = "export CARTA_USER_PORT=$socket ; /usr/lib/cgi-bin/carta_run_local.sh";
	
	open STDIN, '/dev/null'  or die "Can't read /dev/null: $!";
	open STDOUT, '>/dev/null'
	    or die "Can't write to /dev/null: $!";
	open STDERR, '>&STDOUT'  or die "Can't dup stdout: $!";
	setsid or die "Can't start a new session: $!";

	exec("/usr/local/CARTA/bin/carta_run_local.sh", $username, $socket);
	CORE::exit(0);
    }
}

