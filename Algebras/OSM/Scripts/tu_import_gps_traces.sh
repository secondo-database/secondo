#!/bin/sh

bin_dir_path="${SECONDO_BUILD_DIR}/bin"
script_dir_path="${SECONDO_BUILD_DIR}/Algebras/OSM/Scripts"

# Importing GPS-traces from csv-files to secondo
${bin_dir_path}/SecondoTTYBDB -i "${script_dir_path}/TuImportGpsTracesFromCsvFiles.sec"
