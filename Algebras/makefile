########################################################################
#
# SECONDO makefile for algebra manager and algebra modules
#
########################################################################

include ../makefile.env

.PHONY: all
all: makedirs specs

specs:  Standard-C++/*.spec Relation-C++/*.spec PointRectangle/*.spec StreamExample/*.spec BTree/*.spec Range/*.spec Spatial/*.spec FText/*.spec
	cat $^ | sort -b | ./specs.awk
	cat $^ | ./rm_comment_spec.awk > specs

.PHONY: makedirs
makedirs:
	$(MAKE) -C Standard-C++
	$(MAKE) -C Function-C++
	$(MAKE) -C PointRectangle
	$(MAKE) -C StreamExample
	$(MAKE) -C Relation-C++
	$(MAKE) -C Date
	$(MAKE) -C Polygon
	$(MAKE) -C BTree
	$(MAKE) -C Range
	$(MAKE) -C Spatial
	$(MAKE) -C FText

buildlibs:
	$(MAKE) -C Standard-C++   buildlibs
	$(MAKE) -C Function-C++   buildlibs
	$(MAKE) -C PointRectangle buildlibs
	$(MAKE) -C StreamExample  buildlibs
	$(MAKE) -C Relation-C++   buildlibs
	$(MAKE) -C Date           buildlibs
	$(MAKE) -C Polygon	  buildlibs
	$(MAKE) -C BTree	  buildlibs
	$(MAKE) -C Range          buildlibs
	$(MAKE) -C Spatial        buildlibs
	$(MAKE) -C FText 	  buildlibs

.PHONY: clean
clean:
	$(RM) specs
	$(MAKE) -C Management     clean
	$(MAKE) -C Standard-C++   clean
	$(MAKE) -C Function-C++   clean
	$(MAKE) -C PointRectangle clean
	$(MAKE) -C StreamExample  clean
	$(MAKE) -C Relation-C++   clean
	$(MAKE) -C Date           clean
	$(MAKE) -C Polygon        clean
	$(MAKE) -C BTree          clean
	$(MAKE) -C Range          clean
	$(MAKE) -C Spatial        clean
	$(MAKE) -C FText          clean

