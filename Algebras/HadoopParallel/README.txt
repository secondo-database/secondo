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

[1] File-Related Operators in HadoopParallelAlgebra 

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
divide tuples into different files based on one partition 
attribute's values. These binary files are called as data files. 
At the same time, the type list of the input tuple stream is kept 
in a text file, which are called as type file. After the exportation, 
these files can be imported into the Secondo database efficiently 
by using the ~ffeed~ operator.

Inside a computer cluster, above type file and data files can be 
duplicated on several computers, in case some of them may corrupt 
during the parallel processing. At present, we pick up a simple 
chained-declustering mechanism[YYTM10] to manage these duplicated files.
All computers involved in keeping these duplicated files build up 
a simple distributed file system of the parallel Secondo. 

2 Basic functions

2.1 File Names

These file-related operators exchange data between Secondo database 
and file systems, and we use file names to identify different files. 
Hence the string name must be given by the user, and it will be 
used as the prefix name of all involving files. 

As we explained in the last section, there are two kinds of files 
are processed by these operators, a text type file and binary data files. 
The type file's name is fixed by connecting the given file name
and a string of "type", with an underscore. 
E.g., if we set the name as FILENAME, then the type file's name is 
FILENAME\_type.

There are totally three kinds of scenarios when using these 
file-related operators. First, a Secondo relation may be exported 
into one data file completely. Secondo, a Secondo relation may be 
exported into several data files for parallel processing. 
At last, a Secondo relation may be divided by several computers, 
and is partitioned into a matrix file at last. 

At the first situation, the binary data file's name will be the same 
as the given name. But in the other two situations, these data files 
share the given name as the prefix string of their names 
to denote they are belong to a same Secondo relation, 
and use integer numbers as the postfix string to distinguish 
them from each other. The prefix name and postfix numbers are 
connected by underscores too. 
E.g., the file name of FILENAME\_0\_1 means 
this is a data file with the given name of FILENAME,
and it locates in the 1th row, 2th column of a file matrix.


2.2 File Path

All three file-related operators set the file path as indispensable 
parameters, which decides a local directory that kept all these 
type and data files. If the file path is used, then it must be 
an absolute path. But it can also be set as an empty text string,
and then the default file path will be used. 

The default path is set inside the SecondoConfig.ini file, 
which normally is kept in \$SECONDO\_BUILD\_DIR/bin. 
Inside the file, a parameter called as *SecondoFilePath*, 
belong to the *ParallelSecondo* environment, denotes the file path.
If this parameter is not defined, 
or it's not exist and cannot be created, then the directory in 
\$SECONDO\_BUILD\_DIR/bin/parallel will be used as the default file path.
If this path is still unavailable, then the operation fail.


2.3 fconsume and fdistribute Operators

~fconsume~ exports a stream of tuples into a data file, 
it's syntax is:  

----
(stream(tuple(...))
  x fileName x filePath x [rowNum] x [colNum]
  x [typeLoc1] x [typeLoc2]
  x [targetIndex x dupTimes])
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
Therefore they set a stream of tuples, a fileName and
the file path as indispensable parameters.

Besides these parameters, ~fconsume~ also accepts 
the rowNum and colNum as optional parameters. 
If they are not defined, then the data file will be named 
with the given fileName directly. 
The rowNum and colNum are not dependent from each other. 
If only one number is given, then this number can be viewed as 
a row number or a column number, and it doesn't affect the exported
file's name.

The ~fdistribute~ requires another indispensable parameter partAttr, 
which divides tuples into different data files based its value.
The type of the partAttr must provides an available HashValue function, 
since we reply on this function to classify the tuples. 
In some cases, directly dividing the tuples based on partAttr's 
hash values is not a good idea, since it may produce extreme uneven 
partition. The user can set the optional parameter nBucket 
which is used to re-hash these tuples, and get a new hash value
to achieve an even partition.
These hash values are viewed as column numbers of the data files, 
during the matrix file distribution, the optional integer parameter 
rowNum can also be set, and it will be set in data files' names 
between the fileName and the column number. 

The ~fconsume~ operator export tuples' complete values into the data file, 
but the ~fdistribute~ operator can choose whether keeping 
the partition attribute or not, by setting the 
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


2.4 ffeed Operator

The ~ffeed~ operator is used to import the files produced by 
~fconsume~ and ~fdistribute~ operators, back into the Secondo 
database again. The syntax of the ~ffeed~ is: 

----
fileName 
x filePath x [rowNum] x [colNum]                ;
x [typeNodeIndex]                               ;
x [producerIndex x targetIndex x attemptTimes]  ;
->stream(tuple(...))
---- 

The type of the output tuple stream is set by reading the nested-list 
in the type file, which is named as *fileName\_type*. 
The data of the tuples are read from the binary data file, 
and it feed in only one data file each time. 
If the optional parameters rowNum or colNum is defined, 
then the data file's name is composed by these parameters.
If the filePath is empty, Secondo will search both type and 
data files in the default file path. 

Similar as the above operators, ~ffeed~ also offers some other 
groups optional parameter for feeding files from other machines, 
parameters belong to different groups are divided by semicolons, 
if the user doesn't need the remote-feed functions, then he don't 
have to set any other parameters, but still need to write 
two semicolons to denote the empty parameters. E.g.m a query like: 

----
  query "PLZFILE" ffeed['';;] count
----

can feed in the files from the local computer that we created 
in the last example.



3 Fault-tolerance in cluster

During the parallel processing, data are kept as files on disks 
of different computers. 
Sometimes, especially within a cluster of thousands computers, 
it's common that several computers may crush during the processing, 
and all the data kept in these computers become inavailable. 
In case of this kind of problem, data must be duplicated on several computers, 
so to keep the reliability of them.

In Parallel Secondo, we pick up the simple chained-declustering 
mechanism to duplicate these files. I.e., Nodes are listed one 
by one into a slaveList, and each file can be duplicated into several successive 
~n~ nodes within the slaveList, where the ~n~ is denoted by the user himself. 

The slaveList follows the schema: 

----
IPAddress:FilePath:MonitorPort
----

The IPAdrress denotes files' locations inside the computer cluster, 
then the filePath denotes the locations inside nodes' disks.
Then the last defines a port number through which a Secondo Monitor 
listens the requirements from different nodes. 

As in HadoopParallel algebra, data are kept into two kinds of files, 
the type file and the data files, we duplicate these files separately.
In each file-related operators, besides above parameters that are 
used for local mode, there are two left groups of parameters are 
prepared for the remote modes, i.e. the fault-tolerance feature 
of Parallel Secondo. 

The first group is used to process the type file on remote machines, 
hence we call it as type remote mode parameters. 
The parameters are integer numbers, indicate target machines' indices 
inside the slaveList. For ~fconsume~ and ~fdistribute~ operators, 
at most two remote nodes' inidices can be set as parameters, 
i.e., the type file produced by these two operators can be duplicated 
into at most two other nodes. And for ~ffeed~ operator, 
only one node index can be set as type remote index. 

The second group is used to process data files on remote machines, 
hence we call it as data remote mode parameters. 
In ~fconsume~ and ~fdistribute~ operators, the data remote parameters
are same, and are composed by two integers, one defines the first 
target node, and the other defines the duplicate times.
Sometimes, if the local node is not included inside the duplication 
nodes, then the created data file will be removed after the duplication. 
If one node is defined twice during the duplication, the copy only 
happens once. 
It's common that two nodes produce files with a same name, 
and both attempt to duplicate the file into another node's disk. 
To distinguish the files that are created by the current node, 
and the files that are copied from other nodes, 
all files created by other nodes, must use the producer's IP address 
as the postfix of the file name. 

In ~ffeed~, there are totally three parameters are required for 
the data remote mode: producerIndex, targetIndex and attemptTimes.
The producerIndex identify which node created the data file. 
Then it use the targetIndex to locate the node that the operator 
starts looking for the data file from. At last, the attemptTimes 
denotes the maximum attempt times on differnt machines for searching 
the file. If the operator cannot find the data file after attemptTimes
search, then nothing will be returned, or else it will copy the data 
from the remote machine to the local disk, then read the tuples 
from the data file. 


*/
