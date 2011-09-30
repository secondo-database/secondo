

.PHONY: all

all: 
	make -C lib all
	make -C tools all
	make -C sj all

.PHONY:SecondoInterface.jar
SecondoInterface.jar: all 
	rm -rf exporttmp
	mkdir exporttmp
	@find -maxdepth 1 -not -name . -not -name exporttmp -exec cp -r {} exporttmp \;
	@find exporttmp -type d -name CVS -exec rm -rf {} \; 2>/dev/null || echo ""
	@find exporttmp -name "*java" -exec rm -f {} \;
	@echo "extract third party jar files"
	@rm -rf exporttmp/lib
	@cd exporttmp;find ../lib -name "*jar" -exec jar -xf {} \; ; cd ..
	@echo "Building jar archive"
	@(cd exporttmp; jar -cf  ../SecondoInterface.jar .)	
	rm -rf exporttmp






.PHONY: clean

clean: 
	make -C sj clean
	make -C tools clean
	make -C lib clean
	rm -f SecondoInterface.jar
