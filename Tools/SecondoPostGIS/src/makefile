java2classfiles: main/*.java gui/*.java secondo/*.java util/*.java utilgui/*.java postgres/*.java SecondoInterface.jar  jgraphx.jar postgresql-9.2-1002.jdbc4.jar
	javac -cp SecondoInterface.jar:jgraphx.jar:postgresql-9.2-1002.jdbc4.jar main/*.java gui/*.java secondo/*.java util/*.java utilgui/*.java postgres/*.java
	jar xvf jgraphx.jar
	jar xvf postgresql-9.2-1002.jdbc4.jar
	jar -cvfm meinejar.jar manifest.txt SecondoInterface.jar jgraphx.jar postgresql-9.2-1002.jdbc4.jar HelpHTML.txt Loading.gif main/*.class gui/*.class secondo/*.class util/*.class utilgui/*.class postgres/*.class org/ com/
	jar uf meinejar.jar META-INF/services/*
	rm main/*.class gui/*.class secondo/*.class util/*.class utilgui/*.class postgres/*.class
	rm -r META-INF/
	rm -r org/
	rm -r com/




