include ../makefile.env

.PHONY:all
all: $(OPTDIR)/OptimizerServer.class

$(OPTDIR)/OptimizerServer.class: OptimizerServer.java
	$(JAVAC) -classpath $(JPL_CLASSPATH) -d $(OPTDIR) OptimizerServer.java


.PHONY:clean
clean:
	rm -f $(OPTDIR)/*.class