TarFile="ParallelSecondo.jar"
SEC_P="../../../Javagui/secondoInterface/SecondoInterface.jar"
HDC_P="${HADOOP_HOME}/hadoop-0.20.2-core.jar"
SRC_P="ParallelSecondo/"
BIN_P="bin"


all: checkenv
	if [ ! -d ${BIN_P} ];then mkdir -p ${BIN_P}/lib; fi
	javac -classpath ${SEC_P} ${SRC_P}/QuerySecondo.java -d ${BIN_P}
	javac -classpath ${SEC_P} ${SRC_P}/ExtListExpr.java -d ${BIN_P}
	javac ${SRC_P}/Constant.java -d ${BIN_P}
	javac ${SRC_P}/PSNode.java -d ${BIN_P}
	javac -classpath ./bin:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopMap_Map.java -d ${BIN_P}
	javac -classpath ./bin:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopMap_Reduce.java -d ${BIN_P}
	javac -classpath ./bin:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopMap.java -d ${BIN_P}
	javac -classpath ./bin:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce_Map.java -d ${BIN_P}
	javac -classpath ./bin:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce_Reduce.java -d ${BIN_P}
	javac -classpath ./bin:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce.java -d ${BIN_P}
	javac -classpath ./bin:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce2_Map.java -d ${BIN_P}
	javac -classpath ./bin:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce2_Reduce.java -d ${BIN_P}
	javac -classpath ./bin:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce2.java -d ${BIN_P}
	cp ${SEC_P} ${BIN_P}/lib/
	jar -cf ${TarFile} -C ${BIN_P} .
	cp ${TarFile} ../../../bin/

run:

clean:
	rm ParallelSecondo.jar 
output:

doc:
	cd src/org/myorg/ && cat MapSec.java MapMixDivider.java ReducePartJoin.java SecExRunnable.java QuerySecondo.java > MSDoc
	cd src/org/myorg/ && pdview MSDoc
	cd ~/betSec/Algebras/HadoopParallel/ && cat HadoopParallelAlgebra.cpp HadoopParallelAlgebra.h > MSDoc_2
	cd ~/betSec/Algebras/HadoopParallel/ && pdview MSDoc_2
	

checkenv:
ifndef HADOOP_HOME
	@ echo Environment variable HADOOP_HOME is not set
	exit 1
endif	

help:
	@echo "Usage:"
	@echo " make jar     - Build Jar File."
	@echo " make clean   - Clean up Output directory on HDFS."
	@echo " make run     - Run your MapReduce code on Hadoop."
	@echo " make output  - Download and show output file"
	@echo " make help    - Show Makefile options."
	@echo " "
	@echo "Example:"
	@echo " make jar; make run; make output; make clean"

