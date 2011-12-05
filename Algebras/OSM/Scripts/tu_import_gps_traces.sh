#!/bin/sh

################################################################################
### Description:
### - This shell-script imports gps traces by calling a secondo-script.
###
### Author:
### - Thomas Uchdorf, thomas.uchdorf(at)fernuni-hagen.de
################################################################################

bin_dir_path="${SECONDO_BUILD_DIR}/bin"
script_dir_path="${SECONDO_BUILD_DIR}/Algebras/OSM/Scripts"

# Importing GPS-traces from csv-files to secondo
${bin_dir_path}/SecondoTTYBDB -i "${script_dir_path}/TuImportGpsTracesFromCsvFiles.sec"
