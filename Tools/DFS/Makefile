include ./Makefile.inc

DIRS = dfs dfshdfs shared commlayer indexnode datanode qunit qunitrunner cli qunitrunner-hdfs

all: compile

compile:
	for dir in $(DIRS); do $(MAKE) -C $$dir; done

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
	scp package.tgz kingster@kingster.de:/home/kingster/public_html/masterarbeit/package.tgz

hdfs:
	$(MAKE) -C dfshdfs
	$(MAKE) -C clihdfs
