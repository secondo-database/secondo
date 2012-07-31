JarFile = ParallelSecondo.jar
SEC_P = ${SECONDO_BUILD_DIR}/Javagui/secondoInterface/SecondoInterface.jar
HDC_P = ${HADOOP_HOME}/hadoop-0.20.2-core.jar
PKGNM = ParallelSecondo
BIN_P = bin
CLS_P = ${BIN_P}/${PKGNM}
SRC_P = ./${PKGNM}

all: checkenv checkbin ${CLS_P}/PS_HadoopMap.class ${CLS_P}/PS_HadoopReduce.class ${CLS_P}/PS_HadoopReduce2.class 
	cp ${SEC_P} ${BIN_P}/lib/
	jar -cf ${JarFile} -C ${BIN_P} .
	cp ${JarFile} ../../../bin/

${CLS_P}/PS_HadoopMap.class: ${CLS_P}/PS_HadoopMap_Map.class ${CLS_P}/PS_HadoopMap_Reduce.class \
${SRC_P}/PS_HadoopMap.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopMap.java -d ${BIN_P}

${CLS_P}/PS_HadoopMap_Map.class: ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class \
${SRC_P}/PS_HadoopMap_Map.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopMap_Map.java -d ${BIN_P}	

${CLS_P}/PS_HadoopMap_Reduce.class: ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class \
${SRC_P}/PS_HadoopMap_Reduce.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopMap_Reduce.java -d ${BIN_P}	

${CLS_P}/PS_HadoopReduce.class: ${CLS_P}/PS_HadoopReduce_Map.class ${CLS_P}/PS_HadoopReduce_Reduce.class \
${SRC_P}/PS_HadoopReduce.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce.java -d ${BIN_P}

${CLS_P}/PS_HadoopReduce_Map.class: ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class  \
${SRC_P}/PS_HadoopReduce_Map.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce_Map.java -d ${BIN_P}	

${CLS_P}/PS_HadoopReduce_Reduce.class: ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class  \
${SRC_P}/PS_HadoopReduce_Reduce.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce_Reduce.java -d ${BIN_P}	

${CLS_P}/PS_HadoopReduce2.class: ${CLS_P}/PS_HadoopReduce2_Map.class ${CLS_P}/PS_HadoopReduce2_Reduce.class \
${SRC_P}/PS_HadoopReduce2.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce2.java -d ${BIN_P}

${CLS_P}/PS_HadoopReduce2_Map.class: ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class  \
${SRC_P}/PS_HadoopReduce2_Map.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce2_Map.java -d ${BIN_P}	

${CLS_P}/PS_HadoopReduce2_Reduce.class: ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class  \
${SRC_P}/PS_HadoopReduce2_Reduce.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce2_Reduce.java -d ${BIN_P}	

${CLS_P}/HPA_AuxFunctions.class: ${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${SRC_P}/HPA_AuxFunctions.java
	javac -classpath ${BIN_P}:${SEC_P} ${SRC_P}/HPA_AuxFunctions.java -d ${BIN_P}
	
${CLS_P}/Constant.class: ${SRC_P}/Constant.java
	javac $? -d ${BIN_P}

${CLS_P}/PSNode.class: ${SRC_P}/PSNode.java
	javac $? -d ${BIN_P}

${CLS_P}/ExtListExpr.class: ${SRC_P}/ExtListExpr.java 
	javac -classpath ${SEC_P} $? -d ${BIN_P}
	
${CLS_P}/QuerySecondo.class: ${SRC_P}/QuerySecondo.java
	javac -classpath ${SEC_P} $? -d ${BIN_P}


run:

clean:
	rm ParallelSecondo.jar
	rm -rf ${BIN_P}
output:

doc:
	

checkenv:
ifndef HADOOP_HOME
	@ echo Environment variable HADOOP_HOME is not set
	exit 1
endif	

checkbin:
	if [ ! -d ${BIN_P} ];then mkdir -p ${BIN_P}/lib; fi

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

