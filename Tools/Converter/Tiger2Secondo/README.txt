

Tiger/Line 2 SECONDO Converter
==============================

The tools in this directory convert tiger/line files into a 
database readable by the secondo system. The version is from 1999.


How to get Tiger data
---------------------
The files can be downloaded from
   http://www.census.gov/geo/tiger99/

They are about 3.6 GB of zipped data. For downloading all the
files, you can use wget:

   wget -np -nc -r -x http://www.census.gov/geo/tiger99/


Preparing the tools
-------------------
The tgr2sec tool is available as source code. Before you can use it,
you have to compile the source. You need an installed 'gcc' compiler
and 'make' to do that. For compiling the tool, just go into its 
directory and type 'make'.

Using the tgr2sec tool
----------------------
All zip files from the tiger/line data contain a set of files with 
extensions .RT1 RT2 .... RTZ (with missing characters). You can convert 
one or more such sets vi the tgr2sec tool. This tool is called by:
  tgr2sec <options> <files>
Allowed options can be get by 
   tgr2sec -help 
Without any files, the filenames are read from the standard input. As filenames,
you have to enter the real filenames without any extension, that is if you want to
convert the TGR13153.* set, use the filename 'TGR13153'.

Using the tigerconv script
--------------------------
tigerconv is a script for comfortable converting of multiple tiger files. Theoretically it is
possible to convert all the files by a single call of this tool. But this is not a good idea.
The reason is the filesize which corresponds to the unzipped size of all data of the tiger files.
This will be ten or more GB.  
The first argument of this script may be '--oldstyle'. If this option is used, the produced 
database format  will contain the "DESCRIPTIVE ALGEBRA" part which is not longer allowed within
the current Secondo implementation.

The second option is "-single". All other arguments has to be directories containing
tiger data or the names of the zip files itself. If the -single option is used, all files in a
directory are summarized. Otherwise for each file a coresponding relation will be created.

To convert all the tiger data to a very big database use:

tigerconv -single <location of the tiger directory>/*

 



