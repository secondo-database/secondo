installation instructions:

1. Call installPrerequisites.sh with root permissions. This file will install necessary libraries for Intel Threading Building Blocks
2. Execute copyFilesOutsideParThread.sh from this folder to update the query processor related files and other files outside the ParThreadAlgebra folder.
3. Rebuild the secondo installation

Run secondo with SecondoTTYNT and one of the configs in Algebras/ParThread/Tests/TestConfigs directory. Example
queries are located in Algebras/ParThread/Tests/TestQueries
