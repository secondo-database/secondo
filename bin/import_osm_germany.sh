#!/bin/bash
#
# Due to some limitations of the SHP file format 
# (filesize must be < 2 GB) only the SHP files 
# for the german states are available at geofabrik.de. 
# 
# This script download, import and merge the 16 german 
# states into the database $database
#
# Author: Jan Nidzwetzki
#####################################################

# The base url
baseurl="http://download.geofabrik.de/europe/germany/"

# The regions to download
regions="baden-wuerttemberg bayern berlin brandenburg bremen hamburg hessen mecklenburg-vorpommern niedersachsen nordrhein-westfalen rheinland-pfalz saarland sachsen sachsen-anhalt schleswig-holstein thueringen"

# The name of the database in SECONDO
database="germany"

# Datadir
datadir="osm-germany"

if [ ! -x ./SecondoBDB ]; then
    echo "You are not inside the SECONDO dir, exting..."
    exit -1
fi

if [ ! -d $datadir ]; then
    echo "Creating the download directory"
    mkdir osm-germany
fi

cd $datadir

# Create the secondo script
tmpfile=$(mktemp)
echo "create database $database;" > $tmpfile
echo "open database $database;" >> $tmpfile

# Remove invaid chars for region names
secondo_regions=$(echo $regions | sed s/-//g)

# Import the regions
for region in $regions; do
   # Download the region if needed
   if [ ! -d $region ]; then
      mkdir $region
      cd $region
      filename="$region-latest.shp.zip"
      echo "Downloading region $region"
      wget $baseurl/$filename
      unzip $filename
      cd ..
   fi

   # Remove invaid chars for region name
   secondo_region=$(echo $region | sed s/-//g)

   echo "delete Natural_$secondo_region;" >> $tmpfile
   echo "delete Roads_$secondo_region;" >> $tmpfile
   echo "let Natural_$secondo_region = dbimport2('../bin/$datadir/$region/natural.dbf') addcounter [No , 1] shpimport2 ('../bin/$datadir/$region/natural.shp') namedtransformstream [GeoData ] addcounter [No2, 1] mergejoin [No, No2] remove [No, No2 ] filter [ isdefined ( bbox(.GeoData ))] validateAttr consume;" >> $tmpfile
   echo "let Roads_$secondo_region = dbimport2('../bin/$datadir/$region/roads.dbf') addcounter [No , 1] shpimport2 ('../bin/$datadir/$region/roads.shp') namedtransformstream [GeoData ] addcounter [No2, 1] mergejoin [No, No2] remove [No, No2 ] filter [ isdefined ( bbox(.GeoData ))] validateAttr consume;" >> $tmpfile
done

# Merge natural regions
arr=($secondo_regions)
firstregion=${arr[0]};
echo "delete Natural;" >> $tmpfile
echo -n "let Natural=Natural_$firstregion feed " >> $tmpfile 

for ((i=1; i<${#arr[*]}; i++)); do
   echo -n "Natural_${arr[i]} feed concat " >> $tmpfile 
done
echo "consume;" >> $tmpfile

# Create forest region
echo "delete Forest;" >> $tmpfile
echo "let Forest = Natural feed filter [.Type contains 'forest'] consume;" >> $tmpfile

# Merge road regions
arr=($secondo_regions)
firstregion=${arr[0]};
echo "delete Roads;" >> $tmpfile
echo -n "let Roads=Roads_$firstregion feed " >> $tmpfile 

for ((i=1; i<${#arr[*]}; i++)); do
   echo -n "Roads_${arr[i]} feed concat " >> $tmpfile 
done
echo "consume;" >> $tmpfile

# Create database in SECONDO
cat $tmpfile
cd $SECONDO_BUILD_DIR/bin
./SecondoBDB -i $tmpfile 
rm $tmpfile

