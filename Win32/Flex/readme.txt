# ./Win32/Flex/readme.txt
#
# 27.01.2004 M. Spiekermann

the file 
  
  tables.c 

is a replacement for the same file in the flex-2.5.27
distribution. Some changes were made to drop the dependency
to winsock32.dll. The original tar.gz sources will not compile
you have to revise the makefile in order to link with winsock32.dll
or to replace tables.c.

Note: The flex release 2.5.27 is an inoffical version located
at http://site.n.ml.org/info/flex/ the gnu ftp server an mirrors
provide version 2.5.4a.
