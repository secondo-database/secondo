# file viewer/makefile
#
# last change 05-2003; Thomas Behr

include ../makefile.inc


.PHONY: all  # compile Menuvector and SecondoViewer
all: MenuVector.class SecondoViewer.class


# compile all viewers
.PHONY: viewers 
viewers : StandardViewer.class RelViewer.class relsplit hoese viewer3d QueryViewer.class

#
# the viewer from Thomas Hoese (a gui in the past)
#
.PHONY: hoese
hoese: subhoese HoeseViewer.class
.PHONY: subhoese
subhoese:
	(cd hoese;make all)


# ----- hoese end --------


#### the 3D-Viewer for fuzzy objects
.PHONY: viewer3d
viewer3d: subviewer3d viewer3dhere

.PHONY: subviewer3d
subviewer3d:
	(cd viewer3d;make all);

.PHONY: viewer3dhere
viewer3dhere: Viewer3D.java
	$(JAVAC) -classpath $(CLASSPATH) Viewer3D.java


# ----- viewer3d end --------


###### a viewer to split relations #######
.PHONY: relsplit
relsplit: RelSub RelSplit.class

RelSplit.class: RelSplit.java
	$(JAVAC) -classpath $(CLASSPATH) RelSplit.java

RelSub:
	(cd relsplit; make all);

#  ---- end relsplit ----




.PHONY: clean
clean: 
	(cd hoese;make clean)
	(cd relsplit;make clean)
	(cd viewer3d;make clean)
	rm -f *.class


%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<
