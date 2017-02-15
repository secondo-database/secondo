#!/bin/bash

echo $PWD

#Bash-Script to generate PD of the NR2A

cat NestedRelation2Algebra.h Include.h NestedRelation2Algebra.cpp Nr2aException.h Nr2aException.cpp Nr2aHelper.h Nr2aHelper.cpp NRel.h NRel.cpp ARel.h ARel.cpp ProgressEstimator.h Nr2aLocalInfo.h BlockingProgressEstimator.h BlockingProgressEstimator.cpp LinearProgressEstimator.h Count.h Count.cpp Rename.h Rename.cpp Extract.h Extract.cpp Feed.h Feed.cpp Consume.h Consume.cpp Aconsume.h Aconsume.cpp GetTuples.h GetTuples.cpp Nest.h Nest.cpp Unnest.h Unnest.cpp DblpImport.h DblpImport.cpp DblpParser.h DblpParser.cpp XmlParserInterface.h XmlParserInterface.cpp Element.h Element.cpp XmlFileReader.h XmlFileReader.cpp GenRel.h GenRel.cpp TypeOf.h TypeOf.cpp > DocumentationNR2A;
pd2pdf DocumentationNR2A




