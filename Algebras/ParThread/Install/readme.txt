Installation instructions:

  1 Call installPrerequisites.sh with root permissions. This file will install necessary libraries for Intel Threading Building Blocks.

  2 Execute copyFilesOutsideParThread.sh from this folder to update the query processor related files and other files outside the ParThreadAlgebra folder.

  3 Copy entries for ParThreadAlgebra from makefile.algebras.sample to your file makefile.algebras.

  4 Rebuild the Secondo installation (make).

Run Secondo with SecondoTTYNT and one of the configs in Algebras/ParThread/Tests/TestConfigs directory. Example queries are located in Algebras/ParThread/Tests/TestQueries
