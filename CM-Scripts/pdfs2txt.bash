#!/bin/bash
#
# Feb 2005, Markus Spiekermann

libFile="libutil.sh"
#include libutil.sh if present
buildDir=${SECONDO_BUILD_DIR}
scriptDir="."
if [ -z $buildDir ]; then
  printf "%s\n" "Error: I can't find file ${libUtil}."
  exit 1
else
  scriptDir=${buildDir}/CM-Scripts
fi

source ${scriptDir}/$libFile
if [ $? -ne 0 ]; then exit 1; fi

# default options
searchDir=$HOME
fileSep="+§+"
convertCmd="pdftotext"
searchPattern="*.pdf"

declare -i numOfArgs=$#
let numOfArgs++

while [ $# -eq 0 -o $numOfArgs -ne $OPTIND ]; do

  getopts "hd:s:c:p:" optKey
  if [ "$optKey" == "?" ]; then
    optKey="h"
  fi

  case $optKey in

   h) showGPL
      printf "\n%s\n" "Usage of ${0##*/}:" 
      printf "%s\n"   "  -h print this message and exit."
      printf "%s\n"   "  -p<search pattern> => \"*.pdf\" "
      printf "%s\n"   "  -d<directory> => \$HOME"
      printf "%s\n"   "  -s<separator> => \"${fileSep}\" "
      printf "%s\n\n" "  -c<pdf2text converter> => ${convertCmd}"
      printf "%s"     "The default option values are presented above. Normally you will "
      printf "%s"     "only use the -p -d options. "
      printf "%s"     "The filenames are not allowed to contain \"${fileSep}\" since this " 
      printf "%s"     "this is used internally to separate files, but it can be changed by "
      printf "%s"     " the -s option. Blanks in "
      printf "%s"     "input file names are allowed, but translated in the output file names "
      printf "%s"     "into underscores (_). An input tree of files is stored as a flat list "
      printf "%s\n"   "of files, files with the same basename are omitted. "
      exit 0;;
   
   s) fileSep=$OPTARG;;

   d) searchDir=$OPTARG;;

   c) convertCmd=$OPTARG;;

   p) searchPattern=$OPTARG;;

  esac

done

tempDir=${TEMP}/tmp_pdf2text_${date_ymd}_${date_HMS}

#find pdf files and substitute
#spaces with underscores 
printSep "Searching files in \"${searchDir}\" with pattern \"${searchPattern}\""

pdfs=$(find ${searchDir} -iname "$searchPattern" -printf "%p${fileSep}")

if [ $? -ne 0 ]; then exit 1; fi

# pattern replace {// */_} does not work
# hence we remove blanks in a loop
pdfs2=${pdfs// /_}
pdfs3=${pdfs2// /_}

while [ "$pdfs2" != "$pdfs3" ]
do
pdfs3=${pdfs3// /_}
done

pdfs3=${pdfs2//${fileSep}/ }

#Converting Files
printSep "Creating files in ${tempDir}"
mkdir $tempDir
cd $tempDir
if [ $? -ne 0 ]; then exit 1; fi

for inputFile in $pdfs3 
do
  outputFile="${inputFile##*/}.txt"
  if [ -f $outputFile ]; then
    printf "%s\n" "A converted file ${baseName} exists already. Input"
    printf "%s\n" "file ${file} will be omitted."
  else
    printf "%s\n" "Converting ${inputFile}"
    $convertCmd $inputFile $outputFile 
  fi
done

exit $?
