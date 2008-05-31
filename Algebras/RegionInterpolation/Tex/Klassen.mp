input metauml;

	Class.CHTNO("ConvexHullTreeNode")()();
	
	Class.CHTN("ConvexHullTreeNode")
	("-level: int",
   	"-linelist: vector<CHLine>",
    	"-hole: bool",
    	"-dirtyHash bool ",
	"-m_HashCode int",
    	"-myParent: RegionTreeNode*")
    	("+ConvexHullTreeNode()",
    	"+ConvexHullTreeNode(ll: LineWA[], llength: int, myParent: RegionTreeNode*)",
	"+ConvexHullTreeNode(lines: LineWA[], llength: int, mP: RTN*, level: int)",
	"+ConvexHullTreeNode(ll:LineWA[],lll: int,mP: RTN*,lvl: int,isHole: bool)",	
	"+~ConvexHullTreeNode()",
	"+getLevel(): int",
	"+isHole(): bool",
	"+getNrLines(): int",
	"+getParentNode(): RegionTreeNode *",			
	"+getCHLine(i: int): CHLine",
	"+getCenter(): LineWA*",
	"+getSteinerPoint(): LineWA*",
	"+getLines(): vector<CHLine>",
	"+getOutLine(): vector<LineWA>",
	"+getChildren(): vector<ConvexHullTreeNode>*",
	"+getLineForChild(child: ConvexHullTreeNode*): vector<LineWA*>",
	"+isDirtyHash(): bool",
	"+setParent(myParent: RegionTreeNode*)",
	"+setLevel(lev: int)",
	"+setHole(isHole: bool)".	
	"+setDirtyHash(): void",
	"+hashCode(): int",
	"+equals(other: RegionTreeNode*): bool",					
	"+getSplitLine(ref1: ConvexHullTreeNode*, ref2: ConvexHullTreeNode*): vector<LineWA*>*",
	"+getSplitNodes(splitLine: vector<LineWA*>*): vector<vector<LineWA>>*",
	"-insertChild(lineindex: int, child: ConvexHullTreeNode*)",
	"-insertLine(line: LineWA*): int",
	"-getLine(index: int): LineWA*",
	"-calculateHashCode(): void ");			
		
	Class FO("Face")()();
	Class F("Face")
	("-Cycle: ConvexHullTreeNode*",
    	"-Holes: vector<ConvexHullTreeNode>",
    	"-parent: Region*",
    	"-dirtyHash: bool",
	"-m_HashCode: int ")
	("+Face()",
	"+Face(linelist: LineWA[], linelistlength: int, parent: Region*)",
	"+~Face()",
	"+getParent(): Region*",
	"+getNrOfHoles(): int",
	"+getCycle(): ConvexHullTreeNode*",
    	"+getHole(index: int): ConvexHullTreeNode*",
    	"+getHolesAndConcavities(): vector<ConvexHullTreeNode*>*",
    	"+isDirtyHash(): bool",
    	"+clone(): Face*",
    	"+setParent(parent: Region*)",    	
        "+removeHole(toDelete: ConvexHullTreeNode*)",
        "+addHole(newHole: ConvexHullTreeNode*)",
     	"+addHole(linelist: LineWA[], linelistlength: int)",
     	"+splitOnLine(splitLine: vector<LineWA*>*): Face*",
     	"+setDirtyHash(): void",
	"+hashCode(): int",
	"+equals(other: RegionTreeNode*): bool",
	"-calculateHashCode(): void");
	     			
        Class RO("Region")()();
        Class R("Region")
	("-Faces: vector<Face>"
	 "-dirtyHash: bool",
	 "-m_HashCode: int")
	("+Region()",
	"+Region(newRegion: Region*);
	"+~Region()",	
	 "+getNrOfFaces(): int", 
	 "+getFace(index: int): Face*",
	 "+getFaces(): vector<Face>*",	 
	 "+clone(): Region*",
	 "+isDirtyHash(): bool",
	 "+addFace(newFace: Face*)",
	 "+removeFace(index: int)",
	 "+setDirtyHash(): void ",
	 "+splitOnLine(splitLine: vector<LineWA*>*): vector<Face*>*",
	 "+hashCode(): int",
	 "+equals(other: RegionTreeNode*): bool",
	 "-calculateHashCode(): void ");	
	 
	 Interface.RTNO("RegionTreeNode")();
	 Interface.RTN("RegionTreeNode")
	("+~RegionTreeNode()",
	"+setDirtyHash(): void",	
	"+isDirtyHash(): bool",	
	"+hashCode(): int",
	"+equals(other: RegionTreeNode*): bool");
	
	
	classStereotypes.RTN("<<interface>>");
	classStereotypes.RTNO("<<interface>>");
	
	Class.LWAO("LineWA")()();
	Class.LWA("LineWA")
	("-x: double",
        "-y: double",
        "-angle: double")
	("+LineWA()",
  	"+LineWA(x: double, y: double)",    
  	"+LineWA(x: double, y: double, angle: double)",
  	"+LineWA(original: LineWA*)",
  	"+LineWA(line: CHLine*)",
        "+~LineWA()",
        "+getX(): double",
        "+getY(): double",
        "+getAngle(): double",
        "+setX(x: double)",
        "+setY(y: double)",
        "+setAngle(angle: double)",
         "+compareTo(line: LineWA*): int",
         "+equals(line: LineWA*): bool");
         
         Class.LDO("LineDist")()();
         Class.LD("LineDist")
         ("-x: double",
        "-y: double",
        "-distance: double")
	("+LineDist()",
  	"+LineDist(x: double, y: double)", 	
  	"+LineDist(p: LineWA*, distance: double)",
        "+~LineDist()",
        "+getX(): double",
        "+getY(): double",
        "+getDistance(): double",
        "+setX(x: double)",
        "+setY(y: double)",
        "+setDistance(angle: double)",
        "+compareTo(line: LineDist*): int");
        
        Class.PWNLO("PointWNL")()();
        Class.PWNL("PointWNL")
        ("-T: int")
        ("+PointWNL()",
        "+PointWNL(x: double, y: doubel, t : int)",
        "+PointWNL(line: LineWA*, t : int)",
        "+getT(): int"
	"+equals(other: PointWNL*): bool");
        
        
        Class.CHLO("CHLine")()();
        Class.CHL("CHLine")
        ("-child: ConvexHullTreeNode*")
        ("+CHLine()",
	"+CHLine(line: LineWA*)",
	"+getChild(): ConvexHullTreeNode*",
	"+setChild(child: ConvexHullTreeNode*)");
	
	Class.UTIO("Utils")
	()
	();
	Class.UTI("Utils")
	()
	("+getArea(linelist: LineWA[],linelistlength: int): double",
	 "+getArea(linelist: vector<LineWA*>): doubel",
	 "+getArea(linelist: vector<CHLine>): double",
	 "+getArea(linelist: vector<LineWA>*): double",
	 "+convexHull(lt: LineWA[],ltlength: int): vector<LineWA>",
	 "+compareLineWA(line1: LineWA, line2: LineWA): int", 
	 "+sameSide(line1: LineWA*, line2: LineWA*, p1: LineWA*): int",	 	
	 "+indexOf(array: LineWA*, obj: LineWA): int",
	 "+indexOf(array: vector<LineWA*>, obj: LineWA ): int",	 	
	 "+getAngleRad(x: double, y: double, preX: double, preY: double, ",
	 "+                             folX: double, folY: double): double", 
	 "+reverseVector(vec: vector<LineWA>*): void",
	 "+getHausdorfDistance(obj1: vector<CHLine>, obj2: vector<CHLine>): double",
    	 "+getDiameter(Poly: vector<LineWA*>): double",
    	 "+getDiameter(Poly: vector<CHLine>): double",
    	 "+getMaxDistance(Polys: vector<vector<LineWA*>>): double",	
    	 "+convertCHLine2LineWA(vec: vector<CHLine>): vector<LineWA*>",
    	 "+convertLineWA2CHLine(vec: vector<LineWA>*): vector<CHLine>*",
    	 "+getSquareDistance(p1: LineWA*,p2: LineWA*): double",
    	 "+computeLineAngles(lines: vector<LineWA>*): void",
    	 "+getOverlap(l1: vector<CHLine>*,l2: vector<CHLine>*): double",
    	 "+convert2Region(l1: vector<CHLine>*): Region*'",
    	 "+getRectangularDistance(lineA:LineWA*,lineB:LineWA*,point:LineWA*):double",
    	 "+getIntersections(lineA: LineWA*,lineB: LineWA*,",
	  "                             poly: vector<CHLine>*): vector<LineWA*>*",
    	 "+getIntersection(line1A: LineWA*, line1B: LineWA*, line2A: LineWA*,",
 	 "+                             line2B: LineWA*): LineWA*",
    	 "+joinLinelists(first: vector<LineWA>*,",
	 "+                             second: vector<LineWA>*): vector<LineWA>*",
    	 "+getClosestBoundaryPoint(lineA: LineWA*, lineB: LineWA*,", 
	 "+                             poly: vector<LineWA>*): LineWA*",
    	 "+PointsOnLine(points: vector<LineWA>*,lineA:LineWA*,lineB:LineWA*):bool",	
    	 "-getSingleHausdorffDistance(obj1: LineWA*, obj2: vector<CHLine>): double",
    	 "-getLongestDistFromPoint(from: LineWA*,list:vector<LineWA*>,pos:int):int");
	classStereotypes.UTIO("<<static>>");
	classStereotypes.UTI("<<static>>");

	Class.SMO("SingleMatch")
	()
	();
	
	Class.SM("SingleMatch")
	("-source: RegionTreeNode*",
	"-targets: vector<RegionTreeNode*>")
	("+SingleMatch(source: RegionTreeNode*, target: RegionTreeNode*)",
	"+getSource(): RegionTreeNode*",
	"+getNrTargets(): int",
	"+getTargetAt(i: int): RegionTreeNode *",
	"+addTarget(target: RegionTreeNode*)",
	"+removeTarget(index: index)",
	"+removeTargets()",
	"+removeNulls()",
	"+hashCode(): int",
	"+equals(o: SingleMatch*): bool");	
	
			
	Class.MAO("Match")
	()
	();
	
	Class.MA("Match")
	("-name: string",
    	 "-description: string",
    	 "-Hausdorffrating: double",
    	 "-Ovelaprating: double",
    	 "-Arearating: double",
    	 "-linearRating: double",
    	 "-NrOfRatings: int",
         "-greatestDist: double",
         "-source: Region*",
    	 "-target: Region*",
    	 "-maps: map<int,SingleMatch*>")
	("+Match(source: Region*,target: Region*,name: string,description: string)",
	"+getName(): string ",
	"+getDescription(): string ",
	"+getAreaRating(): double ",
    	"+getOverlapRating(): double ",
    	"+getHausdorffRating(): double ",
    	"+getLinarRating(): double ",
    	"+getRating(AreaW: dbl, OverlapW:dbl,HausdorffW:dbl,LinearW:dbl):dbl",
    	"+getSource(): Region*",
    	"+getTarget(): Region*",
    	"+getMatches(source: RegionTreeNode*): vector<RegionTreeNode*>",
    	"+getMaps(): map<int, SingleMatch*> ",
    	"+getTargetChildren(source: RegionTreeNode*): vector<ConvexHullTreeNode*>*",
    	"+addMatch(source: RegionTreeNode*, target: RegionTreeNode*): void ",
    	"+addName(newName: string): void",
    	"+removeMatches(toDelete: RegionTreeNode*): void ",
    	"+finalize(): void",
    	"+getBestMatch(source: CHTN*,targets: vector<CHTN*>*): CHTN*",
    	"+getBestMatch(source: Face*, targets: vector<Face*>*): Face*",
	"~generateRatings(): void",
    	"~rateFace(source: Face*, targets: vector<RegionTreeNode*>): void",
    	"~rateCHTN(source: CHTN*,targets vector<RTN*>*): void",
    	"-findIndex(source: RegionTreeNode*):int,
    	"-removeSingleMatch(toDelete: RegionTreeNode*): void");
    	
    	classStereotypes.MA("<<abstract>>");
    	classStereotypes.MAO("<<abstract>>");
    	
    	Class.CMO("CentroidMatch")
    	()
    	();
    	
    	Class.CM("CentroidMatch")
    	("-threshold: double")
    	("+CentroidMatch(source: Region*, target: Region*, threshold: double)",
    	"+getBestMatch(source: CHTN *, targets: vector<CHTN*>*): CHTN*",
    	"+getBestMatch(source: Face*,targets: vector<Face*>*): Face*",
    	"+matchCHTNs(chtn1: vector<CHTN*> *,chtn2: vector<CHTN*>*): void ",
	"+matchFaces(faces1: vector<Face*>*, faces2: vector<Face*>*): void",
	"-getDistance(chtn1: CHTN*,chtn2: CHTN*): double ");

	Class.StMO("SteinerPointMatch")
    	()
    	();
    	
    	Class.StM("SteinerPointMatch")
    	("-threshold: double")
    	("+SteinerPointMatch(source: Region*, target: Region*, threshold: double)",
    	"+getBestMatch(source: CHTN *, targets: vector<CHTN*>*): CHTN*",
    	"+getBestMatch(source: Face*,targets: vector<Face*>*): Face*",
    	"+matchCHTNs(chtn1: vector<CHTN*> *,chtn2: vector<CHTN*>*): void ",
	"+matchFaces(faces1: vector<Face*>*, faces2: vector<Face*>*): void",
	"-getDistance(chtn1: CHTN*,chtn2: CHTN*): double ");
	
	Class.OMO("OverlapingMatch")
    	()
    	();
    	
    	Class.OM("OverlapingMatch")
    	("-threshold: double")
    	("+OverlapingMatch(source: Region*, target: Region*, threshold: double)",
    	"+getBestMatch(source: CHTN *, targets: vector<CHTN*>*): CHTN*",
    	"+getBestMatch(source: Face*,targets: vector<Face*>*): Face*",
    	"+matchCHTNs(chtn1: vector<CHTN*> *,chtn2: vector<CHTN*>*): void ",
	"+matchFaces(faces1: vector<Face*>*, faces2: vector<Face*>*): void");
	
	Class.MLO("mLineRep")()();
	
	Class.ML("mLineRep")
	("-triangles: vector<MSegmentData>",
	"-myMatch: Match*")
	("+mLineRep()",
	 "+mLineRep(myMatch: Match*)",
	 "+~mLineRep()",
	 "+getTriangles(): vector<MSegmentData>",
	 "-addTrianglesFromFaceFace(face1: Face*, face2: Face*, time1: int,",
 	 "-       time2: int, time: int, facenr: int): void",
	"-addTrianglesFromCHTCHT(cht1: CHTN*, cht2:CHTN*, time1:int, time2: int,",
 	"-facenr: int, cyclenr: int): void ",
	"-addTrianglesFromFaceNull(face: Face*, time: int, facenr: int): void ",
	"-addTrianglesFromCHTNull(cht:CHTN*,time:int,facenr:int,cyclenr:int):void",
	"-addTrianglesFromCHTPoint(ConvexHullTreeNode *chtn, time: int,",
	"-        x double, y: double, t: int, facenr: int, cyclenr: int): void",
	"-addTrianglesFromCHTPoint(chtn: ConvexHullTreeNode*, time: int,", 
	"-        p3: PointWNL*, facenr: int, cyclenr: int): void",
	"-addTriangle(p1: PointWNL*, p2: PointWNL*, p3: PointWNL*, facenr: int,",
 	"-        cyclenr: int): void",
	"-getCorrespondingPoint(p1: PointWNL*, p2: PointWNL*, facenr: int, ",
	"-        cyclenr: int): PointWNL*",
	"-findIndex(p1: PointWNL*, p2: PointWNL*, facenr: int, cyclenr: int): int",
	"-removeTriangle(p1:PointWNL*,p2: PointWNL*,facenr: int,cyclenr:int): void",
	"-removeTrapezoid(p1: PointWNL*, p2: PointWNL*, p3: PointWNL*,",
 	"-p4: PointWNL*, facenr int, cyclenr int): void",
	"-rotaring_pane(chtn1: ConvexHullTreeNode*, chtn2: ConvexHullTreeNode*,",
 	"-time1: int, time2: int, facenr: int, cyclenr: int): void",
	"-findMatchingIndex(s vector<LineWA>*, j:int, angle: double, ka:bool): int");
	
	Class.OpMO("OptimalMatch")()();
	Class.OpM("OptimalMatch")
	()
	("+OptimalMatch(source: Region*, target: Region*, weights: vector<double>)");	

	


beginfig(1);
leftToRight(35)(CHTNO,FO,RO,MAO,MLO);
topToBottom(60)(RTNO,FO,LWAO,CHLO);
topToBottom(60)(SMO,MA);
topToBottom(60)(OpMO,CMO);
leftToRight(80)(RTNO,SMO);

topToBottom(60)(UTIO,CHTNO,LDO);
leftToRight(35)(LDO,LWAO,OpMO);
leftToRight(35)(CHLO,OMO,CMO,StMO);
drawObjects(RO,FO,RTNO,CHTNO,LWAO,CHLO,LDO,OMO,CMO,StMO,MAO,OpMO,MLO,SMO );

link(realization)(RO.n -- RTNO.s);
link(realization)(FO.n -- RTNO.s);
link(realization)(CHTNO.n -- RTNO.s);
link(inheritance)(CHLO.n -- LWAO.s);
link(inheritance)(OMO.n -- MAO.s);
link(inheritance)(StMO.n -- MAO.s);
link(inheritance)(CMO.n -- MAO.s);
link(inheritance)(OpMO.n -- MAO.s);
link(composition)(FO.e -- RO.w);

link(composition)(SMO.s -- MAO.n);
link(composition)(RTNO.e -- SMO.w);
link(association)(MAO.e -- MLO.w);

link(composition)(CHTNO.e -- FO.w);
link(composition)(CHTNO.e -- FO.w);
link(aggregation)(CHLO.n -- CHTNO.s);
link(association)(CHTNO.s -- LWAO.n);
link(association)(FO.s -- LWAO.n);
link(association)(RO.s -- LWAO.n);
link(association)(CHTNO.s -- LDO.n);

endfig;
beginfig(2);
drawObjects(CHTN);
endfig;

beginfig(3);
drawObjects(RTN);
endfig;

beginfig(4);
drawObjects(F);
endfig;

beginfig(5);
drawObjects(R);
endfig;
beginfig(6);
drawObjects(LWA);
endfig;
beginfig(7);
drawObjects(CHL);
endfig;

beginfig(8);
drawObjects(LD);
endfig;

beginfig(9);
drawObjects(UTI);
endfig;

beginfig(10);
drawObjects(SM);
endfig;

beginfig(11);
drawObjects(MA);
endfig;

beginfig(12);
drawObjects(CM);
endfig;

beginfig(13);
drawObjects(PWNL);
endfig;

beginfig(14);
drawObjects(StM);
endfig;

beginfig(15);
drawObjects(OM);
endfig;

beginfig(16);
drawObjects(ML);
endfig;

beginfig(17);
drawObjects(OpM);
endfig;
end