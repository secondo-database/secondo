#!/bin/sh
#
# 08.06.2006 M. Spiekermann
# 
# A shell script wich updates group membership for
# CVS repository files. This is needed since access
# privileges could only be managed by group memberships

lockRoot="/tmp/cvslock"
cvsHome="/home/cvsroot"

chgrp -R cvs-dipl $lockRoot/secondo

chgrp -R cvs-dipl $lockRoot/students
chgrp -R cvs-dipl $cvsHome/students

#chgrp -R cvs-pub /var/lock/cvs/secondo-data
#chgrp -R cvs-pub /home/cvsroot/secondo-data

