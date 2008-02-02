input metauml;

	Class.CHTNO("ConvexHullTreeNode")()();
	
	Class.CHTN("ConvexHullTreeNode")
	("-level: int",
   	"-linelist: vector<CHLine>",
    	"-smallestpoint: int",
    	"-smallestx: double",
    	"-smallesty: double",
    	"-hole: bool",
    	"-myParent: RegionTreeNode*")
    	("+ConvexHullTreeNode()",
    	"+ConvexHullTreeNode(ll: LineWA[], llength: int, myParent: RegionTreeNode*)",
	"+ConvexHullTreeNode(lines: LineWA[], llength: int, mP: RTN*, level: int)",
	"+ConvexHullTreeNode(ll:LineWA[],lll: int,mP: RTN*,lvl: int,isHole: bool)",	
	"+~ConvexHullTreeNode()",
	"+getLevel(): int",
	"+isHole(): bool",
	"+getNrLines(): int",
	"+getCHLine(i: int): CHLine",
	"+getCenter(): LineWA*",
	"+getSteinerPoint(): LineWA*",
	"+getLines(): vector<CHLine>",
	"+getOutLine(): vector<LineWA>",
	"+getChildren(): vector<ConvexHullTreeNode>*",
	"+setParent(myParent: RegionTreeNode*)",
	"+setLevel(lev: int)",
	"+setHole(isHole: bool)".	
	"+hashCode(): int",
	"+equals(other: RegionTreeNode*): bool",					
	"-insertChild(lineindex: int, child: ConvexHullTreeNode*)",
	"-insertLine(line: LineWA*): int",
	"-getLine(index: int): LineWA*");
	
	Class FO("Face")()();
	Class F("Face")
	("-Cycle: ConvexHullTreeNode*",
    	"-Holes: vector<ConvexHullTreeNode>",
    	"-parent: Region*")
	("+Face()",
	"+Face(linelist: LineWA[], linelistlength: int, parent: Region*)",
	"+~Face()",
	"+getParent(): Region*",
	"+getNrOfHoles(): int",
	"+getCycle(): ConvexHullTreeNode*",
    	"+getHole(index: int): ConvexHullTreeNode*",
    	"+setParent(parent: Region*)",    	
        "+removeHole(toDelete: ConvexHullTreeNode*)",
        "+addHole(newHole: ConvexHullTreeNode*)",
     	"+addHole(linelist: LineWA[], linelistlength: int)",
	"+hashCode(): int",
	"+equals(other: RegionTreeNode*): bool");
        
        Class RO("Region")()();
        Class R("Region")
	("-Faces: vector<Face>")
	("+Region()",
	"+~Region()",	
	 "+getNrOfFaces(): int", 
	 "+getFace(index: int): Face*",
	 "+getFaces(): vector<Face>*",
	 "+addFace(newFace: Face*)",
	 "+removeFace(index: int)");	
	    
	 
	 Interface.RTNO("RegionTreeNode")();
	 Interface.RTN("RegionTreeNode")
	("+~RegionTreeNode()",
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
	("+static getArea(linelist: LineWA[],linelistlength: int): double ",
	"+static convexHull(lt: LineWA[],ltlength: int): vector<LineWA>",
	"+static compareLineWA(line1: LineWA, line2: LineWA): int", 
	 "+static sameSide(line1: LineWA*, line2: LineWA*, p1: LineWA*): int",	 	
	 "+static indexOf(array: LineWA*, obj: LineWA): int",
	"+static getAngleRad(x: double, y: double, preX: double, preY: double, ",
	"+                             folX: double, folY: double): double");
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
	

beginfig(1);
leftToRight(60)(CHTNO,FO,RO);
topToBottom(60)(SMO,RTNO,FO,LWAO,CHLO);

leftToRight(60)(UTIO, RTNO);
topToBottom(60)(UTIO,CHTNO,LDO);
leftToRight(60)(LDO,LWAO);
drawObjects(RO,FO,RTNO,CHTNO,LWAO,CHLO,LDO,UTIO,SMO);

link(realization)(RO.n -- RTNO.s);
link(realization)(FO.n -- RTNO.s);
link(realization)(CHTNO.n -- RTNO.s);
link(inheritance)(CHLO.n -- LWAO.s);
link(composition)(FO.e -- RO.w);
link(composition)(CHTNO.e -- FO.w);
link(composition)(CHTNO.e -- FO.w);
link(aggregation)(CHLO.n -- CHTNO.s);
link(association)(CHTNO.s -- LWAO.n);
link(association)(FO.s -- LWAO.n);
link(association)(RO.s -- LWAO.n);
link(association)(CHTNO.s -- LDO.n);
link(dependency)(CHTNO.n -- UTIO.s);
link(composition)(RTNO.n -- SMO.s);
item.aName(iAssoc)("1...*")(aName.sw = FO.e);

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
end