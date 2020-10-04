# filename Javagui.makefile
#
# last change 05-2003; Thomas Behr
#
#             10-2003; M. Spiekermann, automatic creation of configuration files added.
#             10-2014: A. Castor, mmdb-module included


include ./makefile.inc

.PHONY: all #creates the gui and all known viewers
all: check makedirs 
	@chmod ugo+x sgui

.PHONY: check
check:
	$(checkjavac)

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
	$(JAVADOC)  -d $(DOCPATH) -sourcepath $(CLASSPATH):$(JARS):secondoInterface  -classpath $(CLASSPATH):$(JARS) $(shell find -type d -not -name "*CVS" -not -name tmp -not -iname "[0-9]*" -not -ipath "*/[0-9]*" -not -iname "*-*"  -printf '%P\n' | sed "s#/#.#g" | sed "s#secondoInterface.##g" )   


.PHONY: demo
demo:
	@echo "Building Javagui-Demo-Version, please be patient"
	@make -s
	@rm -rf Demo Demotmp
	@mkdir Demotmp
	@find -maxdepth 1 -not -name . -not -name Demotmp -not -name tmp -not -name osmDowloads -exec cp -r {} Demotmp \;
	@find Demotmp -type d -name CVS -exec rm -rf {} \; 2>/dev/null || echo ""
	@find Demotmp -name "*java" -exec rm -f {} \;
	@echo "extract third party jar files and SecondoInterface"
	@rm -rf Demotmp/lib
	@cd Demotmp;find ../lib -name "*jar" -exec jar -xf {} \; ; cd ..
	@cd Demotmp;find ../secondoInterface -name "*jar" -exec jar -xf {} \; ; cd ..
	@cp res/*.gif Demotmp/res
	@echo "Main-Class: gui.MainWindow" > Manifest.txt
	@rm Demotmp/GBS* 
	@rm Demotmp/gui.cfg*
	@echo "Building jar archive"
	@(cd Demotmp; jar -cmf ../Manifest.txt ../Javagui.jar .)
	@rm -rf Demotmp
	@rm Manifest.txt
	@mkdir Demo
	@mv Javagui.jar Demo
	@cp gui.cfg Demo || cp gui.cfg.example Demo/gui.cfg 
	@cp GBS.cfg Demo || cp GBS.cfg.sample Demo/GBS.cfg
	@echo 'java -Xmx512M -jar Javagui.jar $$*' > Demo/sgui
	@chmod +x Demo/sgui
	@cp -r data Demo
	@echo 'java -Xmx512M -jar Javagui.jar %*' > Demo/sgui.bat
	@java tools/CheckViewer >Demo/ViewerList.txt
	@echo "Building Demo has finished successfully, you can find it in the Demo directory"

#	@echo "Javagui recompiled, extract the files needed by Demo"
#	@find . -type d -printf "%P\n" | grep -v '^CVS' | grep -v '^Demotmp' | grep -v '^doc'| sed -e "s#.*#mkdir -p Demotmp/&#g " | sh
#	@find . -name "*.class" -printf "%P\n" | sed -e "s#.*#cp '&' 'Demotmp/&' #g" | sh
#	@(cd Demotmp; jar -xf ../lib/$(MP3); jar -xf ../lib/$(CUPRUNTIME); find -name "*.java" -exec rm {} \; )
#	@echo "Building jar-archive and removing temporaly files"


.PHONY: removedoc # removes the documentation
removedoc:
	(rm -fr $(DOCPATH))


.PHONY: makedirs
makedirs:
	rm -rf sj
	$(MAKE) -C secondoInterface SecondoInterface.jar
	$(MAKE) -C communication all
	$(MAKE) -C extern all
	$(MAKE) -C components all
	$(MAKE) -C viewer all
	$(MAKE) -C project all
	$(MAKE) -C gui all
	$(MAKE) -C viewer viewers
	$(MAKE) -C tools all
	$(MAKE) -C stlib all
	$(MAKE) -C mmdb all


.PHONY: clean # removes the documentation and all .class files
clean: removedoc
	$(MAKE) -C secondoInterface clean
	$(MAKE) -C communication clean
	$(MAKE) -C components clean
	$(MAKE) -C extern clean
	$(MAKE) -C gui clean
	$(MAKE) -C project clean
	$(MAKE) -C tools clean
	$(MAKE) -C viewer clean
	$(MAKE) -C stlib clean
	$(MAKE) -C mmdb clean
	rm -rf Demo
	rm -rf sj
