include ../../../makefile.env

.PHONY: all
all:  $(JPL_SRC)/jpl_fli_Prolog.h

$(JPL_SRC)/jpl_fli_Prolog.h:
	javah -jni -d $(JPL_SRC) jpl.fli.Prolog


.PHONY: clean
clean:
	make -C jpl clean

