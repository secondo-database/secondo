The functionality of the algebra requires adjustments to the code of the Sec kernel.
In addition to the standard activities described in the "Programmers guid" section 1.2.3,
the following steps are necessary:

The following 6 files must be exchanged.

 - $Secondo_Home/include/QueryProcessor.h
 - $Secondo_Home/include/AlgebraManager.h
 - $Secondo_Home/Algebras/Management/AlgebraManager.cpp
 - $Secondo_Home/QueryProcessor/SecondoInterfaceTTY.cpp
 - $Secondo_Home/QueryProcessor/QueryProcessor.cpp
 - $Secondo_Home/makefile
 
 
Activate Algebra:
=================
Back up the original sourcefiles and replace them with the modified sourcefiles contained
in the tar file "ReplaceSecondoKernel4CEA.tgz" in this folder or
call the Shell-Script "PrepareCEA4UseSecondo.sh" in this folder. 

Disable algebra:
================
There are two possibilities:
1.)
By de-activating the algebra by default, the changed code in the sourcefiles is deactivated
via the preprocessor directive.
2.)
If the code is to be completely removed, the above saved files, which were saved when activated,
must be restored or call the Shell-Script "RestoreOrigSecondo.sh" in this folder.
CAUTION: Works only if these files have not been changed in the meantime.
