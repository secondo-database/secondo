# !/bin/bash
#
# cvs-backup.sh: Makes a (incremental) copy of the CVSROOT
#
# 14.01.05 M. Spiekermann 

CVSRootPath=/home
CVSRootName=cvsroot
CVSRootDir=$CVSRootPath/$CVSRootName
BackupPath=/www/spieker_downloads

cmd_rm="rm"
cmd_cp="cp -v"


# check argument
if [ $# -eq 0 ]
then

  echo "Usage: $0 mode = [ incremental | complete ]"
  exit

fi

if [ $1 == "complete" ]
then
  tar -cjf $BackupPath/CVS_BCK_`date +%y%m%d`
  $cmd_rm -rf $BackupPath/$CVSRootName;
  find $CVSRootDir ! -perm "+u=w" -print -exec chmod u+w '{}' ';'

fi

$cmd_cp -ru $CVSRootDir $BackupPath;


