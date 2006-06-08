#!/bin/sh
#
# 08.06.2006 M. Spiekermann
# 
# A shell script with hourly updates group membership for
# CVS repository files

chgrp -R cvs-dipl /var/lock/cvs/secondo

chgrp -R cvs-pub /var/lock/cvs/secondo-data
chgrp -R cvs-pub /home/cvsroot/secondo-data

chgrp -R cvs-dipl /var/lock/cvs/students
chgrp -R cvs-dipl /home/cvsroot/students

