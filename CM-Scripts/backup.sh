# !/bin/bash
#
# backup.sh: Creates (incremental) backups 
#
# 14.01.05 M. Spiekermann 


# include function definitions
# libutil.sh must be in a directory specified in $PATH 
source ${0%/*}/libutil.sh

declare -i err=0

function checkErr {

  if [ $? -ne 0 ]; then
    err=$[$err + 1]
  fi

}

# check arguments
if [ "$1" != "-i" -a "$1" != "-f" -o $# -ne 3 ]; then

  printf "%s\n" "Usage: $0 [-i (=incremental) | -f (=full backup)] <SourceDir> <BackupDir>"
  printf "%s\n" "  SourceDir must have at least one /"

  exit 1

fi

SourceDir="$2"
BackupDir="$3"
LastName=${2##*/}

printf "%s\n" "Backing up $SourceDir -> ${BackupDir}/$LastName ..."

if [ $1 == "-f" ]; then

  printSep "Starting full backup of $SourceDir ..."
  nice -n19 tar -cjf ${BackupDir}/${date_ymd}_FULL_BACKUP_OF_${LastName}.tar.bz2 --exclude secondo-data $SourceDir
  checkErr $?
  
fi

printSep "Mirroring zeppelin:/home/cvsroot to vieta:CVS_Backup ..."

rsync -vuac --exclude secondo-data /home/cvsroot /www/CVS_Backup/
rsync -vua /home/cvsroot/secondo-data /www/CVS_Backup/cvsroot
checkErr $?

exit $err
