operator bbox alias BBOX pattern op(_,_)
operator bbox2d alias BBOX2D pattern op(_,_)
# to avoid contradictory definitions, two parameters must be specified for 
# bbox and bbox2d; however, as in other algebras, only the first parameter is 
# actually used.  (e.g. RTreeAlgebra.cpp: RTree2RectTypeMap; RTree.examples).
# Precise2D.examples suggests that the second parameter is an optional [geoid]
#   Operator : bbox
#   Signature: (pointp||pointsp||linep) [x geoid] -> rect
# however, in Precise2DAlgebraa.cpp: Spatial2Rect_TypeMap, again only the first
# parameter is used. 
operator size alias SIZE pattern op(_)
operator minZ alias MINZ pattern op(_)
operator maxZ alias MAXZ pattern op(_)

operator merge alias MERGE pattern op(_,_)

operator importxyz alias IMPORTXYZ pattern op(_,_,_,_)
operator importpc2fromlas alias IMPORTPC2FROMLAS pattern op(_)
operator importPc2FromStl alias IMPORTPC2FROMSTL pattern op(_,_,_,_,_)

operator feed alias FEED pattern _ op
operator consume alias CONSUME pattern _ op
operator collectPc2 alias COLLECTPC2 pattern _ op [_,_,_;list]

operator restrictPc2 alias RESTRICTPC2 pattern _ op[_]
operator restrictXY alias RESTRICTXY pattern _ op[_]
operator restrictZ alias RESTRICTXY pattern _ op[_]
operator restrictAttr alias RESTRICTATTR pattern _ op [_]
operator restrictRnd alias RESTRICTRND pattern _ op [_]

operator pc2SetParam alias PC2SETPARAM pattern op(_,_)
operator pc2GetParams alias PC2GETPARAMS pattern op()

operator analyzeRaster alias ANALYZERASTER pattern _ op
operator rasterTestPc2 alias RASTERTESTPC2 pattern op(_,_)

operator projectUTM alias PROJECTUTM pattern _ op
operator projectWGS84 alias PROJECTWGS84 pattern _ op[_,_]
operator UTMZone alias UTMZONE pattern _ op
operator UTMSouth alias UTMSOUTH pattern _ op

operator clusterPc2 alias CLUSTERPC2 pattern _ op[_, _]
operator removeNoise alias REMOVENOISE pattern _ op[_,_]
operator createPc2Shapes alias CREATEPC2SHAPES pattern op(_,_,_,_,_,_,_,_,_,_,_)
operator analyzeGeom alias ANALYZEGEOM pattern _ op
