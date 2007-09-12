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
DIRECTORIES="/cvsroot"          

# define where to store the backups
BACKUPDIR=/www/CVS_Backup


# Abbreviation of the day for a full backup
# Note: This depends on language settings, 
# for Example (LANG=en) Sun Mon Tue Wed Thu Fri Sat
FULLDAY="Tue"

###
###  Rarely used options
###

# name and locaction of tar
TAR=/bin/tar                    

TIMEDIR=$BACKUPDIR/last-full    

###
### IMPLEMENTATION PART
###
### You should not have to change anything below here
###

PATH=/usr/local/bin:/usr/bin:/bin
DOW=`date +%a`                        # Day of the week e.g. Mon
DOM=`date +%d`                        # Date of the Month e.g. 27
DM=`date +%d%b`                       # Date and Month e.g. 27Sep

# On the 1st of the month a permanet full backup is made
# At FULLDAY a full backup is made - If this task is successfull, the
# last full backup will be removed.
# The rest of the time an incremental backup is made. Each incremental
# backup overwrites last weeks incremental backup of the same day.
#
# if NEWER = "", then tar backs up all files in the directories
# otherwise it backs up files newer than the NEWER date. NEWER
# gets it date from the file written every Sunday.

incFile="$TIMEDIR/$PREFIX-full-date"
# Monthly full backup, replaces the backup of the last month.
if [ "$DOM" = "01" ]; then
  NEWER=""
  $TAR $NEWER -cpf $BACKUPDIR/tmp-$PREFIX-$DM-full-monthly.tar $DIRECTORIES
  rc=$?
  if [ $rc -ne 0 ]; then 
    echo -e "Error during creation of the monthly full backup."
  else
    rm -f $BACKUPDIR/$PREFIX-*-full-monthly.tar
    mv $BACKUPDIR/tmp-$PREFIX-$DM-full-monthly.tar $BACKUPDIR/$PREFIX-$DM-full-monthly.tar
  fi	  
fi

# Weekly full backup
if [ "$DOW" = "$FULLDAY" ]; then

  # Update full backup date
  $TAR -g $incFile -cpf $BACKUPDIR/tmp-$PREFIX-$DOW-full-weekly.tar $DIRECTORIES
  rc=$?
  if [ $rc -ne 0 ]; then 
    echo -e "Error during creation of the weekly full backup."
  else
    rm -f $BACKUPDIR/tmp-$PREFIX-$DOW-full-weekly.tar
    mv $BACKUPDIR/tmp-$PREFIX-$DOW-full-weekly.tar $BACKUPDIR/$PREFIX-$DOW-full-weekly.tar
  fi	  

  # Make incremental backup - overwrite the last weeks one
else

  $TAR -g $incFile -cpf $BACKUPDIR/$PREFIX-$DOW.tar $DIRECTORIES
  rc=$?
  if [ $rc -ne 0 ]; then 
    echo -e "Error during creation of a daily incremental backup."
  fi
fi

exit $rc
