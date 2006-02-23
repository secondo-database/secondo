TEXTMODULES = \
	Intro.tex
GRAPHIC = \
	uregion-degenerated.pdf
SCREENSHOTS = \
	screenshot01.png \
	screenshot02.png

Using.pdf: $(TEXTMODULES) $(GRAPHIC) $(SCREENSHOTS)
	pdflatex Intro
	pdflatex Intro

%.pdf: %.eps
	epstopdf $<

clean:
	rm -f *.pdf *.aux *~ *.log *.toc *.flc *.lot *.lof 