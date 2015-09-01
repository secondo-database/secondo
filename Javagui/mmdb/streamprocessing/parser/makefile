CLASSPATH:=../../../../secondoInterface/lib/JLex_v1.2.6.jar:../../../../secondoInterface/lib/java_cup_v10k.jar

parser.java sym.java: parser.cup parser.lex
	java -classpath $(CLASSPATH) JLex.Main parser.lex
	mv parser.lex.java Yylex.java
	java -classpath $(CLASSPATH) java_cup10.Main parser.cup
#	javac -classpath $(CLASSPATH) -d . parser.java sym.java Yylex.java