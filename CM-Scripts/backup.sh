# !/bin/bash
#
# backup.sh: Creates (incremental) backups 
#
# 14.01.05 M. Spiekermann 

cmd_rm="rm"
cmd_cp="cp -v"

# check arguments
if [ "$1" != "-i" -a "$1" != "-f" -o $# -ne 3 ]; then

  printf "%s\n" "Usage: $0 [-i (=incremental) | -f (=full backup)] <SourceDir> <BackupDir>"
  printf "%s\n" "  SourceDir must at least one /"

  exit 1

fi

SourceDir="$2"
BackupDir="$3"
LastName=${2##*/}

printf "%s\n" "Backing up $SourceDir -> ${BackupDir}/$LastName ..."

if [ $1 == "-f" ]; then

  printf "%s\n" "Starting full backup of $SourceDir ..."
  if ( tar -cjf $BackupDir/FULL_BACKUP_OF_${LastName}_${date_ymd}.tar.bz2 $SourceDir )
  then
    $cmd_rm -rf $BackupDir/$LastName
  fi
  find $SourceDir ! -perm "+u=w" -print -exec chmod u+w '{}' ';'

else

  printf "%s\n" "Starting incremental backup of $SourceDir ..."

fi

printf "%s\n" "Copying new files ..."

$cmd_cp -ru $SourceDir $BackupDir;


