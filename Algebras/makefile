########################################################################
#
# SECONDO makefile for algebra manager and algebra modules
#
########################################################################

include ../makefile.env

.PHONY: all
all: makedirs specs

specs:  Standard-C++/*.spec Relation-C++/*.spec PointRectangle/*.spec StreamExample/*.spec BTree/*.spec Range/*.spec Spatial/*.spec
	cat $^ | sort -b | ./specs.awk
	cat $^ | ./rm_comment_spec.awk > specs

.PHONY: makedirs
makedirs:
	$(MAKE) -C Standard-C++
	$(MAKE) -C Function-C++
	$(MAKE) -C PointRectangle
	$(MAKE) -C StreamExample
	$(MAKE) -C Relation-C++
	$(MAKE) -C ExtRelation-C++
	$(MAKE) -C Date
	$(MAKE) -C Polygon

buildlibs:
	$(MAKE) -C Standard-C++  	  buildlibs
	$(MAKE) -C Function-C++   	buildlibs
	$(MAKE) -C PointRectangle 	buildlibs
	$(MAKE) -C StreamExample 	  buildlibs
	$(MAKE) -C Relation-C++   	buildlibs
	$(MAKE) -C ExtRelation-C++ 	buildlibs
	$(MAKE) -C Date           	buildlibs
	$(MAKE) -C Polygon	  	    buildlibs

.PHONY: clean
clean:
	$(RM) specs
	$(MAKE) -C Management     	clean
	$(MAKE) -C Standard-C++   	clean
	$(MAKE) -C Function-C++   	clean
	$(MAKE) -C PointRectangle 	clean
	$(MAKE) -C StreamExample  	clean
	$(MAKE) -C Relation-C++   	clean
	$(MAKE) -C ExtRelation-C++ 	clean
	$(MAKE) -C Date           	clean
	$(MAKE) -C Polygon        	clean
	
