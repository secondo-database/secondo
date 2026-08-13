

PDDIR := $(SECONDO_BUILD_DIR)/Tools/pd

.PHONY: all
all: SDK_SETUP.pdf

# pd2pdf is called by its full path rather than relying on Tools/pd being on
# PATH; it puts its own directory on PATH for the tools it calls in turn.
# PD_HEADER is still set here because this document wants the "listing" header,
# not the default one.
SDK_SETUP.pdf: Linux-Installation-Guide.txt
	rm -f $<.{aux,tex}
	PD_HEADER=$(PDDIR)/pd_header_listing $(PDDIR)/pd2pdf $<
	mv $<.pdf $@
