include ../makefile.env

.PHONY:all
all: $(OPTDIR)/OptimizerServer.class $(OPTDIR)/PrologParser.class $(OPTDIR)/NamedVariable.class

$(OPTDIR)/OptimizerServer.class: OptimizerServer.java $(OPTDIR)/PrologParser.class $(OPTDIR)/NamedVariable.class
	$(JAVAC) -classpath $(JPL_CLASSPATH) -d $(OPTDIR) OptimizerServer.java


$(OPTDIR)/PrologParser.class: PrologParser.java $(OPTDIR)/NamedVariable.class
	$(JAVAC) -classpath $(JPL_CLASSPATH) -d $(OPTDIR) PrologParser.java

$(OPTDIR)/NamedVariable.class: NamedVariable.java
	$(JAVAC) -classpath $(JPL_CLASSPATH) -d $(OPTDIR) NamedVariable.java


.PHONY:clean
clean:
	rm -f $(OPTDIR)/*.class
