# file viewer/makefile
#
# last change 05-2003; Thomas Behr

include ../makefile.inc


.PHONY: all  # compile Menuvector and SecondoViewer
all: MenuVector.class SecondoViewer.class MessageBox.class


# compile all viewers

.PHONY: viewers
viewers : StandardViewer.class RelViewer.class FormattedViewer.class relsplit hoese viewer3d QueryViewer.class fuzzy2d

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
viewer3d: subviewer3d Viewer3D.class

.PHONY: subviewer3d
subviewer3d:
	(cd viewer3d;make all);

# ----- viewer3d end --------



#### the 2D-Viewer for fuzzy objects
.PHONY: fuzzy2d
fuzzy2d: subfuzzy2d Fuzzy2D.class

.PHONY: subfuzzy2d
subfuzzy2d:
	(cd fuzzy2d;make all);

# ----- fuzzy2d end --------




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
	(cd fuzzy2d;make clean)
	rm -f *.class


%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<

