# file: gui/makefile
#
# last changed : 05-2003  

include ../makefile.inc

.PHONY: all
all: SecondoObject.class ViewerControl.class ServerDialog.class \
     ResultProcessor.class ObjectList.class CommandPanel.class \
     MainWindow.class

SecondoObject.class: SecondoObject.java
	$(JAVAC) -classpath $(CLASSPATH) SecondoObject.java

ViewerControl.class: ViewerControl.java
	$(JAVAC) -classpath $(CLASSPATH) ViewerControl.java        

ServerDialog.class: ServerDialog.java
	$(JAVAC) -classpath $(CLASSPATH) ServerDialog.java

ResultProcessor.class: ResultProcessor.java
	$(JAVAC) -classpath $(CLASSPATH) ResultProcessor.java

CommandPanel.class: CommandPanel.java
	$(JAVAC) -classpath $(CLASSPATH) CommandPanel.java

ObjectList.class: ObjectList.java
	$(JAVAC) -classpath $(CLASSPATH) ObjectList.java

MainWindow.class: MainWindow.java
	$(JAVAC) -classpath $(CLASSPATH) MainWindow.java



.PHONY: clean
clean:
	rm -f *.class


t: addclasses showclasses

.PHONY: addclasses
addclasses: ALLCLASSFILES= $(shell find *.class) 


showclasses:
	@echo $(ALLCLASSFILES)