# !/bin/bash
#
# backup.sh: Creates (incremental) backups 
#
# 14.01.05 M. Spiekermann 


# include function definitions
# libutil.sh must be in a directory specified in $PATH 
source libutil.sh

cmd_rm="rm"
cmd_cp="cp -v"

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

printSep "Changing permissons in the repository directory ..."
find ${BackupDir}/$LastName ! -perm "+u=w" -print -exec chmod u+w '{}' ';'

if [ $1 == "-f" ]; then

  printSep "Starting full backup of $SourceDir ..."
  if ( tar -cjf ${BackupDir}/${date_ymd}_FULL_BACKUP_OF_${LastName}.tar.bz2 $SourceDir )
  then
    $cmd_rm -rf ${BackupDir}/$LastName
  fi
  

else

  printSep "Starting incremental backup of ${SourceDir} ..."

fi

printSep "Copying new or modified files ..."

$cmd_cp -ru $SourceDir $BackupDir;


