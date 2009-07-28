#!/bin/sh
# full and incremental backup script
#
# created 07 February 2000
# Based on a script by Daniel O'Callaghan <danny@freebsd.org>
# and modified by Gerhard Mourani <gmourani@videotron.ca>
# 
# 2007, M. Spiekermann: Support for tar's -g option and error handling
# added

###
### CONFIGURATION PART
###
### Change the five variables below to fit the script
### to your backup needs.
###

# common prefix for all backup files
PREFIX=cvs                       

# define a list of directories to backup
# Note: don't start with "/"
DIRECTORIES="cvsroot"          

# define where to store the backups
BACKUPDIR=/home/spieker/CVS_Backup


# Abbreviation of the day for a full backup
# Note: This depends on language settings, 
# for Example (LANG=en) Sun Mon Tue Wed Thu Fri Sat
FULLDAY="Fri"

###
###  Rarely used options
###

# name and locaction of tar. It must be a gnu tar which supports
# incremental backups by the switch -g.
TAR=/bin/tar                    

###
### IMPLEMENTATION PART
###
### You should not have to change anything below here
###

PATH=/usr/local/bin:/usr/bin:/bin
DOW=`date +%a`                        # Day of the week e.g. Mon
DOM=`date +%d`                        # Date of the Month e.g. 27
DM=`date +%d%b`                       # Date and Month e.g. 27Sep

# At FULLDAY a full backup is made - If this task is successfull, the
# last full backup will be replaced.
#
# The rest of the time an incremental backup is made. Each incremental
# backup overwrites last weeks incremental backup of the same day.
#
# To do: Avoid that other processes access the file system subtrees which
#        are visited by tar.
#
#        Multiple tar volumes
#
#        Other backup schemes than 7/1

incFile="$BACKUPDIR/$PREFIX-snapshot.info"
overWriteFlag="$BACKUPDIR/$PREFIX-no-inc-overwrite"

# Weekly full backup
if [ "$DOW" = "$FULLDAY" ]; then

  # Create a new full backup
  # Delete $incFile and create a new full backup 
  # If this is successfull, replace the old full backup
  # by the new one otherwise prevent to overwrite the daily incremental
  # backups
  fullFile=$BACKUPDIR/$PREFIX-$DOW-full-weekly.tar
  tmpFile=$fullFile-tmp$$
  tmpInc=$incFile-tmp$$
  $TAR -g $tmpInc -cpf $tmpFile -C / $DIRECTORIES
  rc=$?
  if [ $rc -ne 0 ]; then 
    echo -e "Error during creation of the weekly full backup. Please check $BACKUPDIR"
    touch $overWriteFlag
  else
    mv $tmpFile $fullFile
    mv $tmpInc $incFile
    rm $overWriteFlag
  fi	  

  # Make incremental backup - overwrite the last weeks one
else

  dailyFile=$BACKUPDIR/$PREFIX-$DOW
  if [ -e  $overWriteFlag ]; then
    dailyFile=$dailyFile-$DM 
  fi 
  dailyFile=$dailyFile.tar	  

  $TAR -g $incFile -cpf $dailyFile -C / $DIRECTORIES
  rc=$?
  if [ $rc -ne 0 ]; then 
    echo -e "Error during creation of a daily incremental backup. Please check $BACKUPDIR"
  fi
fi

exit $rc
