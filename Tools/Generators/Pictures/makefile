# March 2005, M. Spiekermann
#
# Create a relation object called paintings
# containing about 4100 JPEG objects.

.PHONY all:
all: paintings

paintings:
	find . -iname "*.jpg" | artpics2rel.sh > $@ 
