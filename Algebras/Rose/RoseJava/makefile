# filename fuzzyobjects/makefile
#
# last change: 05-2003; Th. Behr

include ../makefile.inc
CLASSFILES = \
AdditionalOps.class \
Algebra.class \
Circle.class \
ConnectedComponentsPair.class \
DivisionByZeroException.java \
ElemList.class \
ElemListList.class \
ElemListListList.class \
ElemPair.class \
Element.class \
GFXout.class \
Graph.class \
InfiniteGradientException.class \
Interval.class \
IvlList.class \
LeftJoinPair.class \
LeftJoinPairList.class \
Lines.class \
Mathset.class \
NoAdjacentSegmentException.class \
NoOverlapException.class \
RationalOverflowException.class \
PairList.class \
PairListList.class \
Point.class \
PointList.class \
PointPoint_Ops.class \
PointPol_Ops.class \
PointSeg_Ops.class \
PointTri_Ops.class \
Points.class \
PolList.class \
Polygons.class \
ROSEAlgebra.class \
Rational.class \
Rect.class \
Regions.class \
ResultList.class \
RoseImExport.class \
SPoint.class \
SegList.class \
SegPol_Ops.class \
SegSeg_Ops.class \
SegTri_Ops.class \
Segment.class \
SetOps.class \
SweepStElem.class \
TriList.class \
TriPair.class \
TriTri_Ops.class \
Triangle.class \
TriangleLists.class \
WrongTypeException.class \

.PHONY:all
all: $(CLASSFILES)

%.class:%.java
	$(JAVAC) -classpath $(CLASSPATH) $<   

.PHONY: clean
clean:
	rm -f *.class
	rm -f *~