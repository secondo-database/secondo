/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf] [}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[newpage] [\newpage]

[1] Fault-tolerance Feature in HadoopParallelAlgebra 

June 2011 Jiamin Lu

[TOC]

[newpage]

1 Abstract

This document explains the file-related operators of 
the HadoopParallelAlgebra: ~fconsume~, ~ffeed~ and ~fdistribute~, 
and their fault-tolerance feature inside a computer cluster 
environment. 

The ~fconsume~ and ~fdistribute~ operators export a stream tuples 
data into the file system. The ~fconsume~ writes 
the whole stream tuples into one binary file, and the ~fdistribute~ 
divide tuples into different files based on their partition 
attribute's values. These binary files are called as data files, 
while the type nested list of the input tuple stream is recorded 
in a text file, which are called as type file. After the exportation, 
these files can be imported into the Secondo database efficiently 
by using the ~ffeed~ operator.

Inside a computer cluster, above type file and data files can be 
duplicated on several computers, in case some of them may corrupt 
during the processing. At present, we pick up a simple 
chained-declustering mechanism[YYTM10] to manage these duplicated files.
All computers involved in keeping these duplicated files build up 
a simple file system of the parallel Secondo. 

2 Basic functions

2.1 File Names

These file-related operators exchange data between Secondo database 
and file systems, and we use the file names to identify these files. 
Hence a string name must be given by the user to be the prefix name 
of all these involving files. 

As we explained in the last section, there are two kinds of files 
are processed by these operators, a type file and data files. 
The type file's name is fixed by connecting the given file name
with a underscore and a string of "type". 
E.g., if we set the name as FILENAME, then the type file's name is 
FILENAME_type.

A data file can use the given name as the file name directly. 
However, in most cases, a Secondo relation may be distributed into 
several data files, and all these files sharing a same prefix name. 
To distinguish these data files, integer numbers 
are given by user too to be the postfixes of these file names. 
At most two integer numbers can be used for making the postfix of 
filenames, since sometimes, a Secondo relation may be distributed
into a matrix of files. E.g., a file name of FILENAME_0_1 means 
this is a data file with the given name of FILENAME,
and it locates in the 1th row, 2th column of a file matrix.


2.2 Default File Path

File paths of above three file-related operators are unavoidable. 
If they are set, then the file path must be an absolute path. 
However, we can also simple set it as a empty string, 
and then we can use the default file path. 

The default path can be set inside the SecondoConfig.ini file, 
which normally is kept in the path \$SECONDO\_BUILD\_DIR/bin. 
Inside the value, we can define a new parameter called as 
*SecondoFilePath*, belong to the *ParallelSecondo* environment.
The file path of this parameter also must be an absolute path. 
If this parameter is not defined, then we will create a directory
in \$SECONDO\_BUILD\_DIR/bin/parallel as the default file path.


2.3 fconsume and fdistribute Operators

~fconsume~ exports a stream of tuples into a data file, 
it's syntax is:  

----
(stream(tuple(...))
  x fileName x filePath x [rowNum] x [colNum]
  x [typeLoc1] x [typeLoc2]
  x [targetLoc x dupTimes])
-> bool
----

~fdistribute~ distribute a stream of tuples into several data files, 
and it's syntax is:  

----
stream(tuple(...))
x fileName x path x partAttr x [rowNum]  ;
x [nBuckets] x [KPA]                     ;
x [typeNodeIndex1] x [typeNodeIndex2]    ;
x [targetIndex x dupTimes ]              ;
-> stream(tuple(fileSufix, value))
---- 

Both operators export a stream tuples data into the file system. 
Therefore they both accept a stream of tuples, a fileName and
the file path. 

Besides these necessary parameters, ~fconsume~ also accepts 
the rowNum and fileSuffix as optional parameters. 
If they are not defined, then the data file will be named 
by the given fileName directly. Or else the data file's name is 
composed by fileName, rowNum and colNum together with underscores.
The rowNum and colNum are not dependent from each other. 
If only one integer is given, then this number can be viewed as 
a row number or a column number.

The ~fdistribute~ requires another necessary parameter called 
partAttr. Tuples are divided into different data files based 
on the values of this attribute. The type of the partAttr must 
provides an available HashValue function, since we reply on this 
function to calculate an integer number of the tuple. 
In some cases, dividing the tuples based on partAttr's hash values
directly is not a good idea, since it may produce extreme uneven 
partition of the tuples. Therefore, the user can set the optional 
parameter nBucket to re-hash these tuples, and get a new hash value. 
These hash values are used as the suffix numbers of the data files 
that produced by ~fdistribute~. 
In case the matrix file distribution, the optional integer parameter 
rowNum can also be set, and it will be set in data files' names 
between the fileName and the suffix number. 

The ~fconsume~ operator export the complete value of tuples 
into the data file, but the ~fdistribute~ operator can choose 
whether keeping the partition attribute or not, by setting the 
optional parameter KPA(Keep Partition Attribute). 
Since sometimes, the partition attribute is a temporary attribute. 
By default, the KPA is set as false.

Above functions are called as local mode of these two operators, 
users can use these two operators on a discrete computer too. 
Besides the local mode, there are other two groups of parameters 
are used in a cluster environment, to duplicate those produced type
and data files into some remote computers. Parameters of different 
modes are separated into groups by semicolons, 
and the other two groups parameters are all optional. 
Hence if you can don't need to duplicate these files, 
these parameters can be simply set as empty. E.g., a query like: 

----
  query plz feed fconsume["PLZFILE",'';;]
----    

will create two files, one is PLZFILE\_type that keeps the nested list 
of the relation plz. The other file is PLZFILE, which keeps all the 
binary data of the tuples inside the replation plz. 
Both files will be stored in default paths of the computer.  
Since the fileSuffix is set as empty, the data file doesn't have 
any suffix number. At the same time, all the parameters for 
file duplicating are set as empty, therefore these files won't be 
duplicated to any other machines. 



2.2 ffeed Operator

The ~ffeed~ operator is used to import the files produced by 
~fconsume~ and ~fdistribute~ operators, back into the Secondo 
database again. The syntax of the ~ffeed~ is: 

----
fileName x filePath x [fileSuffix] x [typeNodeIndex]
x [targetNodeIndex x attemptTimes]
->stream(tuple(...))
---- 

The type of the output tuple stream is set by reading the nested-list 
of the text type file, which is named as *fileName\_type*. 
The data of the tuples are read from the binary data file, 
and it can only feed in one data file each time. 
If the optional parameter is defined, then the data file's name 
is *fileName\_fileSuffix*. If the filePath is empty, Secondo will 
search both type and data files in the default file path. 
If these files are retrieved from some remote machines, 
these files are also copied into the local file path first.

Similar as the above operators, ~ffeed~'s other parameters 
involving the fault-tolerance feature are optional, 
and are divided by semicolons. A query like: 

----
  query "PLZFILE" ffeed['';;] count
----

can feed in the files that we created in the last example.



3 Fault-tolerance in cluster



*/
