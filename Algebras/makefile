########################################################################
#
# SECONDO makefile for algebra manager and algebra modules
#
########################################################################

include ../makefile.env

.PHONY: all
all: makedirs specs

specs:  Standard-C++/*.spec Relation-C++/*.spec PointRectangle/*.spec StreamExample/*.spec MovingObjectsInNetworks/*.spec 	
	cat $^ | sort -b -u | ./specs.awk 
	cat $^ > specs

.PHONY: makedirs
makedirs:
	$(MAKE) -C Standard-C++
	$(MAKE) -C Function-C++
	$(MAKE) -C PointRectangle
	$(MAKE) -C StreamExample
	$(MAKE) -C Relation-C++
	$(MAKE) -C Polygon
#	$(MAKE) -C StDb

buildlibs:
	$(MAKE) -C Standard-C++ buildlibs
	$(MAKE) -C Function-C++ buildlibs
	$(MAKE) -C PointRectangle buildlibs
	$(MAKE) -C StreamExample buildlibs
	$(MAKE) -C Relation-C++ buildlibs
	$(MAKE) -C Polygon	buildlibs
#	$(MAKE) -C StDb         buildlibs

.PHONY: clean
clean:
	$(RM) specs
	$(MAKE) -C Management clean
	$(MAKE) -C Standard-C++ clean
	$(MAKE) -C Function-C++ clean
	$(MAKE) -C PointRectangle clean
	$(MAKE) -C StreamExample clean
	$(MAKE) -C Relation-C++ clean
	$(MAKE) -C Polygon clean

