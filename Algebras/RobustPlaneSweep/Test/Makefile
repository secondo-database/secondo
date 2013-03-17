CC=gcc
LD=g++
CFLAGS=-c -Wall -std=c++0x -O3 -DRPS_TEST
# CC=clang++
# CFLAGS=-c -Weverything -std=c++0x -O3 -Wno-padded -Wno-unused-parameter -Wno-newline-eof -Wno-weak-vtables -Wno-float-equal
LDFLAGS=-lstdc++
SOURCES=Test.cpp ../Algorithm/BentleyOttmann.cpp ../Algorithm/Hobby.cpp ../Algorithm/HobbyNaiveIntersectionAlgorithm.cpp ../Algorithm/IntersectionAlgorithm.cpp ../Algorithm/NaiveIntersectionAlgorithm.cpp ../Algorithm/SimpleIntersectionAlgorithm.cpp ../Algorithm/SimpleSweepIntersectionAlgorithm.cpp ../Helper/InternalGeomtries.cpp ../Helper/LineSegmentComparer.cpp ../Helper/SpatialAlgebraStubs.cpp ../Helper/TestDataGenerator.cpp ../Helper/Utility.cpp

OBJECTS=$(SOURCES:.cpp=.obj)
EXECUTABLE=RobustPlaneSweep

build: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

%.obj: %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS)

rebuild: clean build
