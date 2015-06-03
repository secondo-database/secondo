#!/bin/bash

rsync --progress --delete -l -H -p -D -t -r -v -e "ssh" . nidzwetzki@132.176.69.181:/export/homes/nidzwetzki/dsecondo/secondo/Algebras/Cassandra/tools/dsecondo-gui
