.PHONY:all
all: BBox.class


BBox.class: Point.class BBox.java
	javac -classpath $(SECONDO_BUILD_DIR)/Algebras/JBBox BBox.java

Point.class: Point.java
	javac -classpath $(SECONDO_BUILD_DIR)/Algebras/JBBox Point.java

.PHONY:clean
clean:
	rm -f *.class
