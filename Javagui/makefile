#This file is part of SECONDO.
#
#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.
#
#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.
#
#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# filename Javagui.makefile
#
# last change 05-2003; Thomas Behr
#
#             10-2003; M. Spiekermann, automatic creation of configuration files added.
# 2004-01 stefan.wich@fernuni-hagen.de: doc sj.lang added

include ./makefile.inc

.PHONY: all #creates the gui and all known viewers
all: makedirs
	@chmod ugo+x sgui


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

