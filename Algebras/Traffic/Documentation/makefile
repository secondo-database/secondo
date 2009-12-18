# makefile for creating pdf files from tex sources
#
# This makefile creates a pdf in different steps
# First, a dvi file is created possible some bibliography is included.
# From this dvi file a PostScript file is created. Afterwards the 
# Postscript file is converted to pdf. To changes this makefile 
# just adjust the variable FILES. 
#
# Last Change Dec. 2004, T.Behr
#





FILES = TrafficAlgebra

DEPFILES = makefile

TARGETFILES = $(addsuffix .pdf,$(FILES)) $(addsuffix .ps,$(FILES))

DELETESUFFIXES = pdf blg bbl dvi ps log aux


.PHONY:all
all: $(TARGETFILES)

%.pdf: %.ps 
	ps2pdf -dPDFSettings=/prepress -dEmbedAllFonts=true -dColorImageFilter=/FlateEncode -dAutoFilterColorImages=false $<

%.ps: %.dvi
	dvips  $<

%.dvi: %.tex $(DEPFILES)
	latex $< 
	bibtex $(subst .tex,,$<); echo""
	latex $< 
	latex $< 
	latex $<

.PHONY:clean
clean:
	$(foreach Suff,$(DELETESUFFIXES),@rm -f $(addsuffix .$(Suff),$(FILES)))
	$(foreach Suff,$(DELETESUFFIXES),@rm -f $(addsuffix .$(Suff),$(DEPFILES)))
