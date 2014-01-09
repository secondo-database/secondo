JarFile = GenMOD.jar
SEC_P = ${SECONDO_BUILD_DIR}/Javagui/secondoInterface/SecondoInterface.jar
HDC_P = ${HADOOP_HOME}/hadoop-0.20.2-core.jar

all: checkbin
	javac src/Constant.java -d bin/
	javac -classpath ${SEC_P} src/QuerySecondo.java -d bin/
	javac src/PSNode.java -d bin/
	javac -classpath ./bin:${SEC_P}:${HDC_P} src/MapRunScripts.java -d bin/
	javac -classpath ./bin:${SEC_P}:${HDC_P} src/ReduceAggre.java -d bin/
	javac -classpath ./bin:${SEC_P}:${HDC_P} src/GenMOD.java -d bin/
	cp -r ../../../../Javagui/secondoInterface/sj bin/
	cp -r ../../../../Javagui/secondoInterface/lib bin/
	cp -r ../../../../Javagui/secondoInterface/tools bin/
	jar -cf ${JarFile} -C bin/ .
	cp ${JarFile} ../../Parallel_BerlinMOD/generator

clean:
	rm ${JarFile}
	rm ./bin/*.class

checkbin:
	if [ ! -d ./bin ];then mkdir ./bin; fi
