#!/bin/bash
#
# Feb 2005, Markus Spiekermann

# default options
searchDir=$HOME
fileSep="+§+"
convertCmd="pdftotext"
searchPattern="*.pdf"

declare -i numOfArgs=$#
let numOfArgs++

while [ $numOfArgs -ne $OPTIND ]; do

  getopts "hd:s:c:p:" optKey
  if [ "$optKey" == "?" ]; then
    optKey="h"
  fi

  case $optKey in

   h) printf "\n%s\n" "Usage of ${0##*/}" 
      printf "%s\n" "  -h print this message and exit."
      printf "%s\n" "  -p<search pattern>=\"*.pdf\" "
      printf "%s\n" "  -d<directory>=\$HOME"
      printf "%s\n" "  -s<separator>=\"${fileSep}\" "
      printf "%s\n" "  -c<pdf2text converter>=${convertCmd}"
      printf "%s\n" "The default option values are presented above. Normally you will"
      printf "%s\n" " only use the -p -d options."
      printf "\n%s" "The filenames are not allowed to contain \"${fileSep}\", but" 
      printf "%s\n" "this can be changed by the -s option. Blanks in "
      printf "%s\n\n" "file names are translated into _"
      exit 0;;
   
   s) fileSep=$OPTARG;;

   d) searchDir=$OPTARG;;

   c) convertCmd=$OPTARG;;

   p) searchPattern=$OPTARG;;

  esac

done


libUtil="libutil.sh"
#include libutil.sh if present
if [ -f $libUtil ]; then
  source $libUtil
else
  printf "%s\n" "Error: I can't find file ${libUtil}."
  exit 1;
fi



tempDir=${TEMP}/tmp_pdf2text_${date_ymd}_${date_HMS}

#find pdf files and substitute
#spaces with underscores 
pdfs=$(find ${searchDir} -iname "$searchPattern" -printf "%p${fileSep}")

# pattern replace {// */_} does not work
# hence we remove blanks in a loop
pdfs2=${pdfs// /_}
pdfs3=${pdfs2// /_}

while [ "$pdfs2" != "$pdfs3" ]
do
pdfs3=${pdfs3// /_}
done

pdfs3=${pdfs2//${fileSep}/ }

mkdir $tempDir
cd $tempDir
printf "\n%s\n" "Creating files in ${tempDir}"
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
