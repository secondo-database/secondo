# filename fuzzyobjects/makefile
#
# last change: 05-2003; Th. Behr

include ../makefile.inc
CLASSFILES = \
AdditionalOps.class \
Algebra.class \
Application.class \
Circle.class \
ColPoint.class \
ConnectedComponentsPair.class \
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
NumberTest.class \
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
RPGConv.class \
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
Starter.class \
StarterPolTest.class \
SweepStElem.class \
TriList.class \
TriPair.class \
TriTri_Ops.class \
Triangle.class \
TriangleLists.class \
WrongTypeException.class \
ll_minus.class \
lr_inside.class \
lr_intersection.class \
pp_equal.class \
pp_plus.class \
pr_inside.class \
rr_common_border.class \
rr_intersection.class \
rr_minus.class 

.PHONY:all
all: $(CLASSFILES)

%.class:%.java
	$(JAVAC) -classpath $(CLASSPATH) $<   

.PHONY: clean
clean:
	rm -f *.class
	rm -f *~