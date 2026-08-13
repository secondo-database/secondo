include ./Makefile.inc

DIRS = dfs shared commlayer indexnode datanode qunit qunitrunner cli 

HDFSDIRS = dfshdfs qunitrunner-hdfs

# "|| exit 1" in each loop: without it the recipe's exit status is only that of
# the last directory, so a subdirectory that failed to build was reported as
# success.
.PHONY: all compile hdfs clean prune deploy

all: compile

compile:
	for dir in $(DIRS); do $(MAKE) -C $$dir || exit 1; done

hdfs:
	for dir in $(HDFSDIRS); do $(MAKE) -C $$dir || exit 1; done

clean:
	for dir in $(DIRS); do $(MAKE) clean -C $$dir || exit 1; done
	rm -rf _package

prune: clean
	rm -rf datanode/data
	rm -rf indexnode/data

deploy:
	rm -rf package package.tgz
	mkdir package
	cp indexnode/indexnode package
	cp datanode/datanode package
	cp cli/cli package
	tar czf package.tgz package

