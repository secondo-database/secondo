
FILES=createText createTextRelationsWithSel  createRandomTextRelation

all: $(FILES)


createText: createText.cpp
	g++ -o $@ $< 

createRandomTextRelation: createRandomTextRelation.cpp
	g++ -o $@ $< 

createTextRelationsWithSel: createTextRelationsWithSel.cpp
	g++ -o $@ $< 



.PHONY:clean
clean:
	rm -f $(FILES)
