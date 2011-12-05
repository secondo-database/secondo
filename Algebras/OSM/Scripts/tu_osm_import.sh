#!/bin/sh

################################################################################
### Description:
### - This shell-script imports OSM data from the file
### http://download.geofabrik.de/osm/europe/germany/nordrhein-westfalen/
### <name>.shp.zip, from self-made shape-files, e.g. produced with tools like
### osm2shp from a xml-based osm-file, or directly from a osm-file.
### In case of zipped shape-files download the file, unzip and extract it.
### Afterwards make a few manual modifications in the shell-script at hand.
### First adapt the city name and the clipping rectangle, then set SRC_DIR_PATH
### to your local extraction directory.
### This shell script changes the settings in a companion SECONDO-script
### and executes another script which controls the import process.
###
### Author:
### - Thomas Uchdorf, thomas.uchdorf(at)fernuni-hagen.de
################################################################################

# --- feel free to perform manual changes in the following part
# Determining city-specific settings
# Muenster
#name='muenster'
#min_lon=7.52
#max_lon=7.74
#min_lat=51.93
#max_lat=51.99

# Dortmund
#name='dortmund'
#min_lon=7.303333
#max_lon=7.638889
#min_lat=51.416944
#max_lat=51.601389

# Duesseldorf
#name='hometown'
#name='duesseldorf'
#name='beispiel'
#min_lon=6.65
#max_lon=6.91
#min_lat=51.18
#max_lat=51.28
#min_lon=6.70
#max_lon=6.80
#min_lat=51.20
#max_lat=51.25

# bundeslaender
#name='baden-wuerttemberg'
#min_lon=7.5113934084
#max_lon=10.4918239143
#min_lat=47.5338000528
#max_lat=49.7913749328
#name='bayern'
#min_lon=8.9771580802
#max_lon=13.8350427083
#min_lat=47.2703623267
#max_lat=50.5644529365
#name='berlin'
#min_lon=13.0882097323
#max_lon=13.7606105539
#min_lat=52.3418234221
#max_lat=52.6697240587
#name='brandenburg'
#min_lon=11.2681664447
#max_lon=14.7647105012
#min_lat=51.3606627053
#max_lat=53.5579500214
name='bremen'
min_lon=8.4813576818
max_lon=8.9830477728
min_lat=53.0103701114
max_lat=53.6061664164
#name='hamburg'
#min_lon=8.4213643278
#max_lon=10.3242585128
#min_lat=53.3949251389
#max_lat=53.9644376366
#name='hessen'
#min_lon=7.7731704009
#max_lon=10.2340156149
#min_lat=49.3948229196
#max_lat=51.6540496066
#name='mv'
#name='mecklenburg-vorpommern'
#min_lon=10.5932460856
#max_lon=14.4122799503
#min_lat=53.1158637944
#max_lat=54.6849886830
#name='niedersachsen'
#min_lon=6.6545841239
#max_lon=11.59769814
#min_lat=51.2954150799
#max_lat=53.8941514415
#name='nrw'
#name='nordrhein-westfalen'
#min_lon=5.8659988131
#max_lon=9.4476584861
#min_lat=50.3226989435
#max_lat=52.5310351488
#name='rp'
#name='rheinland-pfalz'
#min_lon=6.1173598760
#max_lon=8.5084754437
#min_lat=48.9662745077
#max_lat=50.9404435711
#name='saarland'
#min_lon=6.3584695643
#max_lon=7.4034901078
#min_lat=49.1130992988
#max_lat=49.6393467247
#name='sachsen'
#min_lon=11.8723081683
#max_lon=15.0377433357
#min_lat=50.1715419914
#max_lat=51.6831408995
#name='sa'
#name='sachsen-anhalt'
#min_lon=10.5614755400
#max_lon=13.1865600846
#min_lat=50.9379979829
#max_lat=53.0421316033
#name='sh'
#name='schleswig-holstein'
#min_lon=7.8685145620
#max_lon=11.3132037822
#min_lat=53.3590675115
#max_lat=55.0573747014
#name='thueringen'
#min_lon=9.8778443239
#max_lon=12.6531964048
#min_lat=50.2042330625
#max_lat=51.6490678544

#fiktiv
#name='beispiel'
#min_lon=0
#max_lon=100
#min_lat=0
#max_lat=100

# Specifying the path to the directory in which the data is stored
#src_dir_path="/Users/fernuni-student/osm-data/shp-files/cloud-made/${name}/"
#src_dir_path="/Users/fernuni-student/osm-data/shp-files/geofabrik/${name}/"
#src_dir_path="/Users/fernuni-student/osm-data/shp-files/osm2shp/${name}/"
src_dir_path="/Users/fernuni-student/osm-data/shp-files/geofabrik/bundeslaender/${name}/"
#src_dir_path="/Users/fernuni-student/osm-data/osm-files/osm-api/${name}/"
#src_dir_path="/Users/fernuni-student/osm-data/osm-files/geofabrik/${name}/"
#src_dir_path="/Users/fernuni-student/osm-data/osm-files/cloud-made/${name}/"
#src_dir_path="/Users/fernuni-student/osm-data/osm-files/fiktiv/${name}/"

# Limiting the region of interest if desired
# north-west (NW), west (W), south-west (SW), north (N), centre (C), south (S),
# north-east (NE), east (E), south-east (SE), or another random string
#part='C'
part='City'

# Defining the format of the shape-file that is to be processed
file_type='shp_geofabrik'
#file_type='shp_osm2shp'
#file_type='osm'

# --- please only modify the subsequent lines if you exactly know what you are
#     doing
bin_dir_path="${SECONDO_BUILD_DIR}/bin"
script_dir_path="${SECONDO_BUILD_DIR}/Algebras/OSM/Scripts"
# Modifying a SECONDO-file that serves as template
#capitalizedName=`echo ${name} | cut -c1 | tr '[a-z]' '[A-Z]'``echo ${name} | cut -c2-`
inp='TuPreprocessImport.sec.tmpl'
outp="TuPreprocessImport.sec"
echo "Creating file \"${outp}\"..."
expr1="s#<name>#${name}#g"
expr2="s#<src_dir_path>#${src_dir_path}#g"
expr3="s#<min_lon>#${min_lon}#g"
expr4="s#<max_lon>#${max_lon}#g"
expr5="s#<min_lat>#${min_lat}#g"
expr6="s#<max_lat>#${max_lat}#g"
expr7="s#<part>#${part}#g"
sed -E ${expr1} "${script_dir_path}/${inp}"| sed -E  ${expr2} |
   sed -E  ${expr3}| sed -E  ${expr4}| sed -E  ${expr5}| 
   sed -E  ${expr6} | sed -E  ${expr7} > "${script_dir_path}/${outp}"

# Starting the import
if test ${file_type} = 'osm'; then
   ${bin_dir_path}/SecondoTTYNT -i "${script_dir_path}/TuOsmImport.sec"
elif test ${file_type} = 'shp_geofabrik'; then
   ${bin_dir_path}/SecondoTTYNT -i "${script_dir_path}/TuShpImport.sec"
else
   ${bin_dir_path}/SecondoTTYNT -i "${script_dir_path}/TuCustomShpImport.sec"
fi
