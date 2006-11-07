

all: SDK_SETUP.pdf

SDK_SETUP.pdf: Linux-Installation-Guide.txt
	rm -f $<.{aux,tex}
	export PD_HEADER=$(SECONDO_BUILD_DIR)/Tools/pd/pd_header_listing; \
	pd2pdf $<
	mv $<.pdf $@
