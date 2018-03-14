include ./Makefile.inc

DIRS = dfs shared commlayer indexnode datanode qunit qunitrunner cli 

HDFSDIRS = dfshdfs qunitrunner-hdfs

all: compile

compile:
	for dir in $(DIRS); do $(MAKE) -C $$dir; done

hdfs:
	for dir in $(HDFSDIRS); do $(MAKE) -C $$dir; done

clean:
	for dir in $(DIRS); do $(MAKE) clean -C $$dir; done
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

