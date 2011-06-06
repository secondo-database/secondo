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

This document is used to explain the file-related operators of 
the HadoopParallelAlgebra: ~fconsume~, ~ffeed~ and ~fdistribute~, 
especially their fault-tolerance feature inside a computer cluster 
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

2.1 fconsume and fdistribute Operators

The ~fconsume~ and ~fdistribute~ operators export a stream tuples 
data into the file system. The syntax of ~fconsume~ operator is: 

----
(stream(tuple(...))
  x fileName x filePath x [fileSuffix]
  x [typeLoc1] x [typeLoc2]
  x [targetLoc x dupTimes])
-> bool
----

and the syntax of ~fdistribute~ operator is: 

----
stream(tuple(...))
x fileName x path x attrName
x [nBuckets] x [KPA]
x [typeNodeIndex1] x [typeNodeIndex2]
x [targetIndex x dupTimes ]
-> stream(tuple(fileSufix, value))
---- 

Both operators export a stream tuples data into the file system. 
Therefore the first three parameters of both operators are same: 
a tuple stream, fileName and filePath. The filePath can be set as 
empty, if so then a default file path will be used. 

After the exportation, there are two kinds of files are produced. 
One is called as type file, which is a text file, and use nested-list 
format to keep the schema of the input tuple stream. The name of the 
type file is set as *fileName\_type*. Both operators only produce one 
type file. 

The other kind of files is called as data file, 
which is a binary file, and keeps all binary data of the tuples. 
The ~fconsume~ operator produces one data file, 
while the ~fdistribute~ may produces several data files. 
All these files names start by the given fileName parameter, 
and may end with some integer suffices. 
In ~fconsume~ operator, this integer suffix is an optional value, 
if it's given, then the data file name will be *fileName\_fileSuffix*.
However, in ~fdistribute~ operator, files suffices are inevitable. 
Tuples are classified according to a specific attribute value, 
and all tuples written into a same data file can calculate a same 
integer value by using the attribute's HashValue method, 
which is set to be the file's suffix. 
If the optional nBuckets parameter is given, then these tuples will 
be re-hashed based on the attribute value and the bucket number, 
to achieve an even partition.

The ~fconsume~ operator export every attribute value of each tuple 
into the data file, but the ~fdistribute~ operator can choose 
whether keeping the partition attribute or not by setting the 
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

2.3 Default File Path

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


3 Fault-tolerance in cluster



*/
