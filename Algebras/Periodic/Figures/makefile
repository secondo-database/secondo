

FILES=ObviousSolution.eps UnFoldedSolution.eps RepTreeWithAdditionalLevel.eps UniLogo.eps MRealFormula.eps SecondoLogo.eps 


.PHONY:all  clean
all: $(FILES)


UniLogo.eps: logo.eps1
	cp $< $@

%.eps: %.dvi
	dvips -E -o $@ $<

%.dvi: %.tex
	latex $<


clean:
	rm -f *.ps *.eps *.dvi *.log *.aux 



