JarFile = ParallelSecondo.jar
SEC_P = ${SECONDO_BUILD_DIR}/Javagui/secondoInterface/SecondoInterface.jar
HDC_P = ${HADOOP_HOME}/hadoop-0.20.2-core.jar
PKGNM = ParallelSecondo
BIN_P = bin
CLS_P = ${BIN_P}/${PKGNM}
SRC_P = ./${PKGNM}
SECLIB = ${BIN_P}/lib/SecondoInterface.jar

all: checkenv checkbin checklib editor mod ${CLS_P}/PS_HadoopMap.class ${CLS_P}/PS_HadoopReduce.class ${CLS_P}/PS_HadoopReduce2.class ${CLS_P}/PS_HadoopMapAll.class
	jar -cf ${JarFile} -C ${BIN_P} .
	cp ${JarFile} ../../../bin/

${CLS_P}/PS_HadoopMapAll.class: ${SRC_P}/PS_HadoopMapAll.java ${CLS_P}/QuerySecondo.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PS_HadoopMapAll_Map.class \
${CLS_P}/PS_HadoopMapAll_Reduce.class
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopMapAll.java -d ${BIN_P}

${CLS_P}/PS_HadoopMapAll_Map.class: ${SRC_P}/PS_HadoopMapAll_Map.java ${CLS_P}/QuerySecondo.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/HPA_AuxFunctions.class 
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopMapAll_Map.java -d ${BIN_P}

${CLS_P}/PS_HadoopMapAll_Reduce.class: ${SRC_P}/PS_HadoopMapAll_Reduce.java ${CLS_P}/QuerySecondo.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/HPA_AuxFunctions.class 
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopMapAll_Reduce.java -d ${BIN_P}

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
${CLS_P}/PS_HadoopReduce_QMap.class ${SRC_P}/PS_HadoopReduce.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce.java -d ${BIN_P}

${CLS_P}/PS_HadoopReduce_Map.class: ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class  \
${SRC_P}/PS_HadoopReduce_Map.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce_Map.java -d ${BIN_P}	

${CLS_P}/PS_HadoopReduce_QMap.class: ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class  \
${SRC_P}/PS_HadoopReduce_QMap.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce_QMap.java -d ${BIN_P}	

${CLS_P}/PS_HadoopReduce_Reduce.class: ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class  \
${SRC_P}/PS_HadoopReduce_Reduce.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce_Reduce.java -d ${BIN_P}	

${CLS_P}/PS_HadoopReduce2.class: ${CLS_P}/PS_HadoopReduce2_Map.class ${CLS_P}/PS_HadoopReduce2_Reduce.class \
${CLS_P}/PS_HadoopReduce2_QMap.class ${CLS_P}/PS_HadoopReduce2_HDJ_QMap.class ${CLS_P}/PS_HadoopReduce2_HDJ_Reduce.class \
${CLS_P}/HDJ_GetZTuple.class ${SRC_P}/PS_HadoopReduce2.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce2.java -d ${BIN_P}

${CLS_P}/PS_HadoopReduce2_Map.class: ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class  \
${SRC_P}/PS_HadoopReduce2_Map.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce2_Map.java -d ${BIN_P}	

${CLS_P}/PS_HadoopReduce2_QMap.class: ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class  \
${SRC_P}/PS_HadoopReduce2_QMap.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce2_QMap.java -d ${BIN_P}	

${CLS_P}/PS_HadoopReduce2_HDJ_QMap.class: ${SRC_P}/PS_HadoopReduce2_HDJ_QMap.java \
${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class  \
${CLS_P}/SecExRunnable.class ${CLS_P}/RemoteStream.class ${CLS_P}/RMDataOutputStream.class ${CLS_P}/RMDataInputStream.class 
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce2_HDJ_QMap.java -d ${BIN_P}	

${CLS_P}/PS_HadoopReduce2_HDJ_Reduce.class: ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class  \
${CLS_P}/SecExRunnable.class ${CLS_P}/RemoteStream.class ${CLS_P}/RMDataOutputStream.class ${CLS_P}/RMDataInputStream.class \
${SRC_P}/PS_HadoopReduce2_HDJ_Reduce.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce2_HDJ_Reduce.java -d ${BIN_P}	


${CLS_P}/PS_HadoopReduce2_Reduce.class: ${CLS_P}/HPA_AuxFunctions.class ${CLS_P}/PSNode.class \
${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${CLS_P}/QuerySecondo.class  \
${SRC_P}/PS_HadoopReduce2_Reduce.java
	javac -classpath ${BIN_P}:${SEC_P}:${HDC_P} ${SRC_P}/PS_HadoopReduce2_Reduce.java -d ${BIN_P}	

${CLS_P}/HPA_AuxFunctions.class: ${CLS_P}/Constant.class ${CLS_P}/ExtListExpr.class ${SRC_P}/HPA_AuxFunctions.java
	javac -classpath ${BIN_P}:${SEC_P} ${SRC_P}/HPA_AuxFunctions.java -d ${BIN_P}

${CLS_P}/RemoteStream.class: ${SRC_P}/RemoteStream.java ${CLS_P}/RMDataInputStream.class ${CLS_P}/RMDataOutputStream.class
	javac -classpath ${BIN_P}:${SEC_P} ${SRC_P}/RemoteStream.java -d ${BIN_P}
${CLS_P}/SecExRunnable.class: ${SRC_P}/SecExRunnable.java ${CLS_P}/QuerySecondo.class
	javac -classpath ${BIN_P}:${SEC_P} ${SRC_P}/SecExRunnable.java -d ${BIN_P}

${CLS_P}/HDJ_GetZTuple.class: ${SRC_P}/HDJ_GetZTuple.java ${CLS_P}/RMDataInputStream.class \
${CLS_P}/RMDataOutputStream.class ${CLS_P}/PSNode.class
	javac -classpath ${BIN_P}:${SEC_P} ${SRC_P}/HDJ_GetZTuple.java -d ${BIN_P}

${CLS_P}/PSNode.class: ${SRC_P}/PSNode.java
	javac -classpath ${BIN_P}:${SEC_P} ${SRC_P}/PSNode.java -d ${BIN_P}

${CLS_P}/RMDataInputStream.class: ${SRC_P}/RMDataInputStream.java
	javac -classpath ${SEC_P} $? -d ${BIN_P}
${CLS_P}/RMDataOutputStream.class: ${SRC_P}/RMDataOutputStream.java
	javac -classpath ${SEC_P} $? -d ${BIN_P}


${CLS_P}/Constant.class: ${SRC_P}/Constant.java
	javac $? -d ${BIN_P}

${CLS_P}/ExtListExpr.class: ${SRC_P}/ExtListExpr.java 
	javac -classpath ${SEC_P} $? -d ${BIN_P}
	
${CLS_P}/QuerySecondo.class: ${SRC_P}/QuerySecondo.java
	javac -classpath ${SEC_P} $? -d ${BIN_P}


run:

clean:
	if [ -f ${JarFile} ];then rm ${JarFile}; fi
	rm -rf ${BIN_P}
	rm -f PSEditor.jar
output:

doc:
	

checkenv:
ifndef HADOOP_HOME
	@ echo Environment variable HADOOP_HOME is not set
	exit 1
endif

checkbin:
	if [ ! -d ${BIN_P} ];then mkdir -p ${BIN_P}/lib; fi

checklib:
	if [ ! -f ${SECLIB} ]; then cp ${SEC_P} ${SECLIB}; fi

editor:
	(cd PSEditor; javac *.java)
	jar -cfm PSEditor.jar PSEditor/META-INF/MANIFEST.MF PSEditor *.xml

mod:
	(cd genMod; make all)

help:
	@echo "Usage:"
	@echo " make jar     - Build Jar File."
	@echo " make clean   - Clean up Output directory on HDFS."
	@echo " make run     - Run your MapReduce code on Hadoop."
	@echo " make output  - Download and show output file"
	@echo " make editor  - Compile the graphical configure editor."
	@echo " make help    - Show Makefile options."
	@echo " "
	@echo "Example:"
	@echo " make jar; make run; make output; make clean"

