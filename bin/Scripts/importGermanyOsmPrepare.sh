#!/bin/bash

# for s in $(cat newton2-5); do ssh ralf@$s rm /home/ralf/Daten/germany-latest.osm ; done

for s in $(cat newton2-5); do ssh ralf@$s rm /home/ralf/Daten/germany-latest.osm.bz2 ; done

date

scp ralf@newton1:/home/ralf/Daten/germany-latest.osm.bz2 ralf@newton2:/home/ralf/Daten/ & 
pid2=$!
scp ralf@newton1:/home/ralf/Daten/germany-latest.osm.bz2 ralf@newton3:/home/ralf/Daten/ & 
pid3=$!
scp ralf@newton1:/home/ralf/Daten/germany-latest.osm.bz2 ralf@newton4:/home/ralf/Daten/ & 
pid4=$!
scp ralf@newton1:/home/ralf/Daten/germany-latest.osm.bz2 ralf@newton5:/home/ralf/Daten/ & 
pid5=$!

echo "wait for copying ..."

wait $pid2 $pid3 $pid4 $pid5

echo "finished"

date

# 2:42 min

ssh ralf@newton1 bunzip2 /home/ralf/Daten/germany-latest.osm.bz2 & 
pid11=$!
ssh ralf@newton2 bunzip2 /home/ralf/Daten/germany-latest.osm.bz2 & 
pid12=$!
ssh ralf@newton3 bunzip2 /home/ralf/Daten/germany-latest.osm.bz2 & 
pid13=$!
ssh ralf@newton4 bunzip2 /home/ralf/Daten/germany-latest.osm.bz2 & 
pid14=$!
ssh ralf@newton5 bunzip2 /home/ralf/Daten/germany-latest.osm.bz2 & 
pid15=$!

echo "wait for unpacking ..."

wait $pid11 $pid12 $pid13 $pid14 $pid15

echo "finished"

date

# 20:54 min

# ralf@ralf-ubuntu1:~/secondo/bin/Scripts$ ./importGermanyOsmPrepare.sh 
# Di 2. Aug 17:57:09 CEST 2016
# wait for copying ...
# finished
# Di 2. Aug 17:59:51 CEST 2016
# wait for unpacking ...
# finished
# Di 2. Aug 18:20:45 CEST 2016
# ralf@ralf-ubuntu1:~/secondo/bin/Scripts$ 




















