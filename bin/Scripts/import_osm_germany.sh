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
#regions="baden-wuerttemberg bayern berlin brandenburg bremen hamburg hessen mecklenburg-vorpommern niedersachsen nordrhein-westfalen rheinland-pfalz saarland sachsen sachsen-anhalt schleswig-holstein thueringen"
regions="berlin"

# The name of the database in SECONDO
database="germany"

# Datadir
datadir="osm-germany"

cd $SECONDO_BUILD_DIR/bin

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
      filename="$region-latest-free.shp.zip"
      echo "Downloading region $region"
      wget $baseurl/$filename
      unzip $filename
      cd ..
   fi

   # Remove invaid chars for region name
   secondo_region=$(echo $region | sed s/-//g)

   echo "delete Natural_$secondo_region;" >> $tmpfile
   echo "delete Roads_$secondo_region;" >> $tmpfile
   echo "delete Buildings_$secondo_region;" >> $tmpfile

   echo "let Natural_$secondo_region = dbimport2('../bin/$datadir/$region/gis_osm_natural_free_1.dbf') addcounter [No , 1] shpimport2 ('../bin/$datadir/$region/gis_osm_natural_free_1.shp') namedtransformstream [GeoData ] addcounter [No2, 1] mergejoin [No, No2] remove [No, No2 ] filter [ isdefined ( bbox(.GeoData ))] validateAttr consume;" >> $tmpfile
   echo "let Roads_$secondo_region = dbimport2('../bin/$datadir/$region/gis_osm_roads_free_1.dbf') addcounter [No , 1] shpimport2 ('../bin/$datadir/$region/gis_osm_roads_free_1.shp') namedtransformstream [GeoData ] addcounter [No2, 1] mergejoin [No, No2] remove [No, No2 ] filter [ isdefined ( bbox(.GeoData ))] validateAttr consume;" >> $tmpfile
   echo "let Buildings_$secondo_region = dbimport2('../bin/$datadir/$region/gis_osm_buildings_a_free_1.dbf') addcounter [No , 1] shpimport2 ('../bin/$datadir/$region/gis_osm_buildings_a_free_1.shp') namedtransformstream [GeoData ] addcounter [No2, 1] mergejoin [No, No2] remove [No, No2 ] filter [ isdefined ( bbox(.GeoData ))] validateAttr consume;" >> $tmpfile
   echo "let Landuse_$secondo_region = dbimport2('../bin/$datadir/$region/gis_osm_landuse_a_free_1.dbf') addcounter [No , 1] shpimport2 ('../bin/$datadir/$region/gis_osm_landuse_a_free_1.shp') namedtransformstream [GeoData ] addcounter [No2, 1] mergejoin [No, No2] remove [No, No2 ] filter [ isdefined ( bbox(.GeoData ))] validateAttr consume;" >> $tmpfile
done

# Merge regions
arr=($secondo_regions)
firstregion=${arr[0]};
echo "delete Natural;" >> $tmpfile
echo "delete Roads;" >> $tmpfile
echo "delete Buildings;" >> $tmpfile
echo "delete Landuse;" >> $tmpfile

natural="let Natural=Natural_$firstregion feed " 
roads="let Roads=Roads_$firstregion feed " 
buildings="let Buildings=Buildings_$firstregion feed " 
landuse="let Landuse=Landuse_$firstregion feed " 

for ((i=1; i<${#arr[*]}; i++)); do
   natural="$natural Natural_${arr[i]} feed concat " 
   roads="$roads Roads_${arr[i]} feed concat "
   buildings="$buildings Buildings_${arr[i]} feed concat "
   landuse="$landuse Landuse_${arr[i]} feed concat "
done
echo "$natural trimAllUndef consume;" >> $tmpfile
echo "$roads trimAllUndef consume;" >> $tmpfile
echo "$buildings trimAllUndef consume;" >> $tmpfile
echo "$landuse trimAllUndef consume;" >> $tmpfile

# Create forest region
echo "delete Forest;" >> $tmpfile
echo "let Forest = Landuse feed filter[.Fclass contains 'forest'] consume;" >> $tmpfile

# Create database in SECONDO
cat $tmpfile
cd $SECONDO_BUILD_DIR/bin
./SecondoBDB -i $tmpfile 
rm $tmpfile

