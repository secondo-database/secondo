#!/usr/bin/perl 
# 
# cvspass.pl <newpassword> - creates an encrypted password string for cvs
#
# 2003 M. Spiekermann (adapted from examples found in the internet)  
 
# Grab password from command line 
my $newpasswd = shift; 
 
# Generate encrypted password 
srand (time()); 
my $randletter = "(int (rand (26)) + (int (rand (1) + .5) % 2 ? 65 : 97))"; 
my $salt = sprintf ("%c%c", eval $randletter, eval $randletter); 
my $crypttext = crypt ($newpasswd, $salt); 
printf("<%s>,<%s>\n", $newpasswd, $crypttext)
