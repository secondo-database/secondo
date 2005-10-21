#
# makefile, 2005-10-21
#
# Dirk Ansorge, FernUniversitaet Hagen
#

include ../../../../makefile.inc

platform := $(SECONDO_PLATFORM)

.PHONY: all

ifeq ($(platform),win32)
MYLIBS=NGlibs/nglib.o NGlibs/stlgeom.o NGlibs/stlgeomchart.o NGlibs/stlline.o NGlibs/stltopology.o NGlibs/stltool.o NGlibs/meshstlsurface.o NGlibs/gprim.o NGlibs/geomtest3d.o NGlibs/meshing2.o NGlibs/meshsurf.o NGlibs/netrule2.o NGlibs/stlgeommesh.o NGlibs/meshclass.o NGlibs/clusters.o NGlibs/topology.o NGlibs/improve2.o NGlibs/meshtype.o NGlibs/meshtool.o NGlibs/msghandler.o NGlibs/ngexception.o NGlibs/geom3d.o NGlibs/surface.o NGlibs/parser2.o NGlibs/improve2gen.o NGlibs/mystring.o NGlibs/densemat.o NGlibs/hashtabl.o NGlibs/moveablemem.o NGlibs/localh.o NGlibs/brick.o NGlibs/triapprox.o NGlibs/polynomial.o NGlibs/adfront2.o NGlibs/splinegeometry2.o NGlibs/meshfunc2d.o NGlibs/table.o NGlibs/flags.o NGlibs/symbolta.o NGlibs/bitarray.o NGlibs/optmem.o NGlibs/triarls.o NGlibs/quadrls.o NGlibs/adfront3.o NGlibs/geom2dmesh.o NGlibs/ruler2.o NGlibs/spline2d.o NGlibs/curvedelems2.o NGlibs/geomsearch.o NGlibs/curvedelems.o NGlibs/algprim.o NGlibs/meshfunc.o NGlibs/improve3.o NGlibs/meshing3.o NGlibs/delaunay.o NGlibs/pyramidrls.o NGlibs/pyramid2rls.o NGlibs/seti.o NGlibs/prism2rls.o NGlibs/netrule3.o NGlibs/global.o NGlibs/bfgs.o NGlibs/smoothing3.o NGlibs/linsearch.o NGlibs/smoothing2.o NGlibs/hprefinement.o NGlibs/tetrarls.o NGlibs/bisect.o NGlibs/secondorder.o NGlibs/sort.o NGlibs/parser3.o NGlibs/geom2d.o NGlibs/ruler3.o NGlibs/genmesh2d.o NGlibs/geomfuncs.o NGlibs/VCmtex.o NGlibs/VCmtcore.o NGlibs/VCobjcore.o NGlibs/VCexcept.o ../MeshGeneratorNetGen.o

else
MYLIBS=NGlibs/nglib.o NGlibs/stlgeom.o NGlibs/stlgeomchart.o NGlibs/stlline.o NGlibs/stltopology.o NGlibs/stltool.o NGlibs/meshstlsurface.o NGlibs/gprim.o NGlibs/geomtest3d.o NGlibs/meshing2.o NGlibs/meshsurf.o NGlibs/netrule2.o NGlibs/stlgeommesh.o NGlibs/meshclass.o NGlibs/clusters.o NGlibs/topology.o NGlibs/improve2.o NGlibs/meshtype.o NGlibs/meshtool.o NGlibs/msghandler.o NGlibs/ngexception.o NGlibs/geom3d.o NGlibs/surface.o NGlibs/parser2.o NGlibs/improve2gen.o NGlibs/mystring.o NGlibs/densemat.o NGlibs/hashtabl.o NGlibs/moveablemem.o NGlibs/localh.o NGlibs/brick.o NGlibs/triapprox.o NGlibs/polynomial.o NGlibs/adfront2.o NGlibs/splinegeometry2.o NGlibs/meshfunc2d.o NGlibs/table.o NGlibs/flags.o NGlibs/symbolta.o NGlibs/bitarray.o NGlibs/optmem.o NGlibs/triarls.o NGlibs/quadrls.o NGlibs/adfront3.o NGlibs/geom2dmesh.o NGlibs/ruler2.o NGlibs/spline2d.o NGlibs/curvedelems2.o NGlibs/geomsearch.o NGlibs/curvedelems.o NGlibs/algprim.o NGlibs/meshfunc.o NGlibs/improve3.o NGlibs/meshing3.o NGlibs/delaunay.o NGlibs/pyramidrls.o NGlibs/pyramid2rls.o NGlibs/seti.o NGlibs/prism2rls.o NGlibs/netrule3.o NGlibs/global.o NGlibs/bfgs.o NGlibs/smoothing3.o NGlibs/linsearch.o NGlibs/smoothing2.o NGlibs/hprefinement.o NGlibs/tetrarls.o NGlibs/bisect.o NGlibs/secondorder.o NGlibs/sort.o NGlibs/parser3.o NGlibs/geom2d.o NGlibs/ruler3.o NGlibs/genmesh2d.o NGlibs/geomfuncs.o  ../MeshGeneratorNetGen.o
endif

all: libMeshGeneratorNetGen.so

libMeshGeneratorNetGen.so: $(MYLIBS)
	gcc -shared -o libMeshGeneratorNetGen.so $(MYLIBS) -lstdc++

NGlibs/nglib.o: libsrc/interface/nglib.cpp
	gcc -c -o NGlibs/nglib.o libsrc/interface/nglib.cpp -Ilibsrc/interface -Ilibsrc/include

NGlibs/geom2d.o: libsrc/gprim/geom2d.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/genmesh2d.o: libsrc/geom2d/genmesh2d.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/stlgeom.o: libsrc/stlgeom/stlgeom.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/stlgeomchart.o: libsrc/stlgeom/stlgeomchart.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/stlline.o: libsrc/stlgeom/stlline.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/stltopology.o: libsrc/stlgeom/stltopology.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/stltool.o: libsrc/stlgeom/stltool.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/meshstlsurface.o: libsrc/stlgeom/meshstlsurface.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/gprim.o: libsrc/gprim/adtree.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/geomtest3d.o: libsrc/gprim/geomtest3d.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/meshing2.o: libsrc/meshing/meshing2.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/meshsurf.o: libsrc/csg/meshsurf.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/netrule2.o: libsrc/meshing/netrule2.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/stlgeommesh.o: libsrc/stlgeom/stlgeommesh.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/meshclass.o: libsrc/meshing/meshclass.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/clusters.o: libsrc/meshing/clusters.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/topology.o: libsrc/meshing/topology.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/improve2.o: libsrc/meshing/improve2.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/meshtype.o: libsrc/meshing/meshtype.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/meshtool.o: libsrc/meshing/meshtool.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/msghandler.o: libsrc/meshing/msghandler.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/ngexception.o: libsrc/general/ngexception.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/geom3d.o: libsrc/gprim/geom3d.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/surface.o: libsrc/csg/surface.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/parser2.o: libsrc/meshing/parser2.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/improve2gen.o: libsrc/meshing/improve2gen.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/mystring.o: libsrc/general/mystring.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/densemat.o: libsrc/linalg/densemat.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/hashtabl.o: libsrc/general/hashtabl.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/moveablemem.o: libsrc/general/moveablemem.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/localh.o: libsrc/meshing/localh.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/brick.o: libsrc/csg/brick.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/triapprox.o: libsrc/csg/triapprox.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/polynomial.o: libsrc/linalg/polynomial.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/adfront2.o: libsrc/meshing/adfront2.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/splinegeometry2.o: libsrc/geom2d/splinegeometry2.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/meshfunc2d.o: libsrc/meshing/meshfunc2d.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/table.o: libsrc/general/table.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/flags.o: libsrc/general/flags.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/symbolta.o: libsrc/general/symbolta.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/bitarray.o: libsrc/general/bitarray.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/optmem.o: libsrc/general/optmem.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/triarls.o: libsrc/meshing/triarls.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/quadrls.o: libsrc/meshing/quadrls.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/adfront3.o: libsrc/meshing/adfront3.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/geom2dmesh.o: libsrc/geom2d/geom2dmesh.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/ruler2.o: libsrc/meshing/ruler2.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/spline2d.o: libsrc/geom2d/spline2d.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/curvedelems2.o: libsrc/meshing/curvedelems2.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/geomsearch.o: libsrc/meshing/geomsearch.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/curvedelems.o: libsrc/meshing/curvedelems.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/algprim.o: libsrc/csg/algprim.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/meshfunc.o: libsrc/meshing/meshfunc.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/improve3.o: libsrc/meshing/improve3.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/meshing3.o: libsrc/meshing/meshing3.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/delaunay.o: libsrc/meshing/delaunay.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/pyramidrls.o: libsrc/meshing/pyramidrls.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/pyramid2rls.o: libsrc/meshing/pyramid2rls.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/seti.o: libsrc/general/seti.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/prism2rls.o: libsrc/meshing/prism2rls.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/netrule3.o: libsrc/meshing/netrule3.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/global.o: libsrc/meshing/global.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/bfgs.o: libsrc/opti/bfgs.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/smoothing3.o: libsrc/meshing/smoothing3.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/linsearch.o: libsrc/opti/linsearch.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/smoothing2.o: libsrc/meshing/smoothing2.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/hprefinement.o: libsrc/meshing/hprefinement.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/tetrarls.o: libsrc/meshing/tetrarls.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/bisect.o: libsrc/meshing/bisect.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/secondorder.o: libsrc/meshing/secondorder.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/sort.o: libsrc/general/sort.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/parser3.o: libsrc/meshing/parser3.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/ruler3.o: libsrc/meshing/ruler3.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/geomfuncs.o: libsrc/gprim/geomfuncs.cpp
	gcc -c -o $@ $< -Ilibsrc/interface -Ilibsrc/include

NGlibs/VCmtex.o: libsrc/include/winlibsVC/mtex.cpp
	gcc -c -o $@ $< -Ilibsrc/winlibsVC

NGlibs/VCmtcore.o: libsrc/include/winlibsVC/mtcore.cpp
	gcc -c -o $@ $< -Ilibsrc/winlibsVC

NGlibs/VCobjcore.o: libsrc/include/winlibsVC/objcore.cpp
	gcc -c -o $@ $< -Ilibsrc/winlibsVC

NGlibs/VCexcept.o: libsrc/include/winlibsVC/except.cpp
	gcc -c -o $@ $< -Ilibsrc/winlibsVC

.PHONY: clean

clean:
	cd NGlibs; rm *.o
