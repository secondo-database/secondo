# file viewer/makefile
#
# changes:
# 05-2003; Thomas Behr
# 01-2004: stefan.wich@fernuni-hagen.de: jpeg added, sorted viewers alnum, one line per subtask
# 02-2004: Markus Spiekermann: The makfile was revised to make it more generic.
include ../makefile.inc


.PHONY: all  # compile Menuvector and SecondoViewer
all: MenuVector.class SecondoViewer.class MessageBox.class


# add below new Viewer classes or subdirectories to compile
VIEWER_CLASSES := \
		FormattedViewer.class \
		InquiryViewer.class \
		QueryViewer.class \
		RelViewer.class \
		StandardViewer.class \
		Viewer3D.class \
		Fuzzy2D.class \
		RelSplit.class \
		HoeseViewer.class \
		MP3V.class \
		MidiViewer.class

VIEWER_DIRS := fuzzy2d jpeg midi relsplit viewer3d hoese


# compile all viewers
.PHONY: viewers
viewers: $(VIEWER_CLASSES) makedirs


.PHONY: makedirs
makedirs:
	for dir in $(VIEWER_DIRS); do \
          if !($(MAKE) -C $$dir) then \
            exit 1; \
          fi \
        done


.PHONY: clean
clean:
	rm -f *.class
	for dir in $(VIEWER_DIRS); do \
          if !($(MAKE) -C $$dir clean) then \
            exit 1; \
          fi \
        done

# default rule for creating java classes
%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH):$(MP3CLASSESPATH) $<

