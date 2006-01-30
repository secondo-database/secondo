# filename Javagui.makefile
#
# last change 05-2003; Thomas Behr
#
#             10-2003; M. Spiekermann, automatic creation of configuration files added.


include ./makefile.inc

.PHONY: all #creates the gui and all known viewers
all: check makedirs 
	@chmod ugo+x sgui

.PHONY: check
check:
	$(checkjavac)

.PHONY:nltest
nltest:
	$(JAVA) -classpath $(CLASSPATH):$(JARS) NLTest


.PHONY: help
help:
	@echo "make all      -> compile all gui-components and know viewers"
	@echo "make doc      -> call javadoc"
	@echo "make clean    -> remove all files created by this makefile"
	@echo "make demo     -> creates a single jar archive from the class files and the needed resources"
	@echo "                 you can find it in the Demo directory with additionally configuration files"
	@echo "                 as well as a script for starting the demo"


.PHONY: doc # creates the documentation  via $(JAVADOC)
doc: removedoc 
	@make -s all
	@mkdir $(DOCPATH)
	@$(JAVADOC)  -d $(DOCPATH) -sourcepath $(CLASSPATH):$(JARS) -classpath $(CLASSPATH):$(JARS) $(shell find -type d -printf '%P\n'|  grep -v  "CVS" | sed -e 's#/#.#g')   


.PHONY: demo
demo:
	@echo "Building Javagui-Demo-Version, please be patient"
	@make -s
	@rm -rf Demotmp
	@rm -rf Demo
	@mkdir Demotmp
	@echo "Javagui recompiled, extract the files needed by Demo"
	@find . -type d -printf "%P\n" | grep -v '^CVS' | grep -v '^Demotmp' | grep -v '^doc'| sed -e "s#.*#mkdir -p Demotmp/&#g " | sh
	@find . -name "*.class" -printf "%P\n" | sed -e "s#.*#cp '&' 'Demotmp/&' #g" | sh
	@echo "Main-Class: gui.MainWindow" > Manifest.txt
	@(cd Demotmp; jar -xf ../lib/$(MP3); jar -xf ../lib/$(CUPRUNTIME); find -name "*.java" -exec rm {} \; )
	@cp res/*.gif Demotmp/res
	@echo "Building jar-archive and removing temporaly files"
	@(cd Demotmp; jar -cmf ../Manifest.txt ../Javagui.jar .)
	@rm -rf Demotmp
	@rm Manifest.txt
	@mkdir Demo
	@mv Javagui.jar Demo
	@cp gui.cfg Demo
	@cp GBS.cfg Demo
	@echo "java -Xmx80M -jar Javagui.jar" > Demo/sgui
	@chmod +x Demo/sgui
	@echo "Building Demo has finished successfully, you can find it in the Demo directory"


.PHONY: removedoc # removes the documentation
removedoc:
	(rm -fr $(DOCPATH))


.PHONY: makedirs
makedirs:
	make -C sj all
	make -C communication all
	make -C extern all
	make -C components all
	make -C viewer all
	make -C project all
	make -C gui all
	make -C viewer viewers
	make -C tools all

.PHONY: external # create a version for external use
external:
	rm -rf external
	mkdir external
	find -maxdepth 1 \! -name external \! -name '.' -exec cp -r {} external \;
	sed -i 's#GUIPATH *:=.*#GUIPATH := $$(SECONDO_BUILD_DIR)/Javagui/external#g' external/makefile.inc
	make -sC external clean
	find external -type d -name "CVS" | sed "s#.*#rm -r &#g" | sh
	rm -r external/viewer/jpeg
	rm -r external/viewer/midi
	rm external/viewer/MP3V.java
	rm external/viewer/hoese/algebras/Dsplpicture.java
	rm external/viewer/MidiViewer.java
	sed -i -e "s#midi# #g"  -e "s#jpeg# #g" -e "s#MP3V.class# #g" -e "s#MidiViewer.class# #g" external/viewer/makefile
	@#make -C external
	@#make -C external clean
	sed -i 's#GUIPATH *:=.*#GUIPATH := $$(SECONDO_BUILD_DIR)/Javagui#g' external/makefile.inc

	


.PHONY: clean # removes the documentation and all .class files
clean: removedoc
	make -C sj clean
	make -C communication clean
	make -C components clean
	make -C extern clean
	make -C gui clean
	make -C project clean
	make -C tools clean
	make -C viewer clean
	rm -rf Demo

