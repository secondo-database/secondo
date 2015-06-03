#!/bin/bash

rsync --progress -l -H -p -D -t -r -v -e "ssh" . nidzwetzki@132.176.69.181:/export/homes/nidzwetzki/dsecondo/secondo/Algebras/Cassandra/tools/dsecondo-gui
