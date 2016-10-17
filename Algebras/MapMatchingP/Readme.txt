

1. Relation SedentalerStr

1.1 Erzeugung der Relation SedentalerStr

SedentalerStr basiert auf Relation Ways (erstellt durch "OrderedRelationGraphFromFullOSMImport-D.SEC"),
gefiltert nach WayTagValue "Sedentaler Strasse" und WayTagKey "highway".
Erzeugung der Relationen durch:

let SedentalerWays = Ways feed filter[.WayInfo afeed filter[.WayTagValue = "Sedentaler Straße"] count > 0] consume;
let SedentalerStr = SedentalerWays feed filter[.WayInfo afeed filter[.WayTagKey = "highway"] count > 0] consume

1.2 Aufbau der Relation SedentalerStr

Ist eine Relation mit Aufbau:
(rel(tuple ((WayId longint) (NodeList (arel (tuple 
       ((NodeId longint) (Pos point) (NodeIdNew int) (NodeCounter int)
        (NodeRef longint))))) (Curve line) (WayIdInTag longint) 
       (WayInfo (arel (tuple ((WayTagKey text) (WayTagValue text))))))
       ))

Übersicht:
Die Relation besteht aus Tupeln:
(WayId NodeListARel Curve WayIdInTag WayInfoARel)
wobei NodeListARel und WayInfoARel eingebettete Relationen bzw. Tupel sind mit:

- NodeList besteht aus Tupeln:
  (NodeId Pos NodeIdNew NodeCounter NodeRef)
- WayInfo besteht aus Tupeln:
  (WayTagKey WayTagValue)


	longint
	    [NodeList]
                longint
                point
                int
                int
                longint
        line
        longint
            [WayInfo]
                text
                text



**********


2. Weitere Relationen

Weitere Relationen, die durch das Script "OrderedRelationGraphFromFullOSMImport-D.SEC" erstellt wurden:
- CityNodesNew
(rel(tuple ((NodeId longint) (Pos point) (NodeIdNew int))))

- Roads (wie Ways, gefiltert nach WayTagKey "highway")
(rel (tuple ((WayId longint) (NodeList (arel (tuple 
       ((NodeId longint) (Pos point) (NodeIdNew int) (NodeCounter int)
        (NodeRef longint))))) (Curve line) (WayIdInTag longint) 
       (WayInfo (arel (tuple ((WayTagKey text) (WayTagValue text))))))
       ))


- Nodes
(rel (tuple ((WayId longint) (NodeCounter int) (NodeIdNew int) (Pos point))))


- EdgesUp
(rel (tuple ((WayId longint) (Source int) (Target int) 
       (SourcePos point) (TargetPos point) (SourceNodeCounter int) 
       (TargetNodeCounter int) (Curve sline) (RoadName text) 
       (RoadType text))))


- EdgesDown
(rel (tuple ((WayId longint) (Source int) (Target int) 
       (SourcePos point) (TargetPos point) (SourceNodeCounter int) 
       (TargetNodeCounter int) (Curve sline) (RoadName text) 
       (RoadType text))))


- Edges
(rel (tuple ((Source int) (Target int) (SourcePos point) 
       (TargetPos point) (SourceNodeCounter int) (TargetNodeCounter 
       int) (Curve sline) (RoadName text) (RoadType text) (WayId 
       longint))))


- EdgeIndex
(rel (tuple ((Source int) (Target int) (Curve sline) (Box 
       rect))))




3. RTrees
Ebenfalls durch das Script "OrderedRelatio     WayId : 254881822
  NodeList : 
    NodeId      : 240181037
    Pos         : point: (6.96655,51.2075)
    NodeIdNew   : 7298053
    NodeCounter : 0
    NodeRef     : 240181037

    NodeId      : 2606702642
    Pos         : point: (6.96702,51.2075)
    NodeIdNew   : 7298064
    NodeCounter : 1
    NodeRef     : 2606702642

    NodeId      : 3787358578
    Pos         : point: (6.96781,51.2075)
    NodeIdNew   : 7298913
    NodeCounter : 2
    NodeRef     : 3787358578

    NodeId      : 240147936
    Pos         : point: (6.96804,51.2075)
    NodeIdNew   : 7298915
    NodeCounter : 3
    NodeRef     : 240147936

     Curve : Generic display function used!
Type : line
Value: 
(
    (6.9665451 51.2074576 6.9670241 51.2074876) 
    (6.9670241 51.2074876 6.9678133 51.2074928) 
    (6.9678133 51.2074928 6.9680353 51.2074943))
WayIdInTag : 254881822
   WayInfo : 
    WayTagKey   : name
    WayTagValue : Sedentaler Straße

    WayTagKey   : lanes
    WayTagValue : 3

    WayTagKey   : oneway
    WayTagValue : yes

    WayTagKey   : source
    WayTagValue : survey

    WayTagKey   : highway
    WayTagValue : tertiary

    WayTagKey   : maxspeed
    WayTagValue : 50

    WayTagKey   : placement
    WayTagValue : left_of:3

    WayTagKey   : turn:lanes
    WayTagValue : left|through|right

    WayTagKey   : postal_code
    WayTagValue : 40699

    WayTagKey   : source:lanes
    WayTagValue : Bing
nGraphFromFullOSMImport-D.SEC" erstellt:
- Ways_Curve_rtree
((rtree (tuple ((Box rect))) rect FALSE))

- Ways_WayTag
((rtree (tuple ((Box rect))) rect FALSE))


********************
Beispiel für einen Straßenabschnitt:
query SedentalerStr feed filter [.WayId = 254881822] consume

     WayId : 254881822
  NodeList : 
    NodeId      : 240181037
    Pos         : point: (6.96655,51.2075)
    NodeIdNew   : 7298053
    NodeCounter : 0
    NodeRef     : 240181037

    NodeId      : 2606702642
    Pos         : point: (6.96702,51.2075)
    NodeIdNew   : 7298064
    NodeCounter : 1
    NodeRef     : 2606702642

    NodeId      : 3787358578
    Pos         : point: (6.96781,51.2075)
    NodeIdNew   : 7298913
    NodeCounter : 2
    NodeRef     : 3787358578

    NodeId      : 240147936
    Pos         : point: (6.96804,51.2075)
    NodeIdNew   : 7298915
    NodeCounter : 3
    NodeRef     : 240147936

     Curve : Generic display function used!
Type : line
Value: 
(
    (6.9665451 51.2074576 6.9670241 51.2074876) 
    (6.9670241 51.2074876 6.9678133 51.2074928) 
    (6.9678133 51.2074928 6.9680353 51.2074943))
WayIdInTag : 254881822
   WayInfo : 
    WayTagKey   : name
    WayTagValue : Sedentaler Straße

    WayTagKey   : lanes
    WayTagValue : 3

    WayTagKey   : oneway
    WayTagValue : yes

    WayTagKey   : source
    WayTagValue : survey

    WayTagKey   : highway
    WayTagValue : tertiary

    WayTagKey   : maxspeed
    WayTagValue : 50

    WayTagKey   : placement
    WayTagValue : left_of:3

    WayTagKey   : turn:lanes
    WayTagValue : left|through|right

    WayTagKey   : postal_code
    WayTagValue : 40699

    WayTagKey   : source:lanes
    WayTagValue : Bing


****************
Anwendung von operator "line2region" aus "SpatialAlgebra":

1. Erstelle line (geschlossene Linie)
let SedenDreieck = [const line value ((6.9665451 51.2074576 6.9670241 51.2074876) (6.9670241 51.2074876 6.9678133 51.2074928) (6.9678133 51.2074928 6.9680353 51.2074943) (6.9680353 51.2074943 6.9665451 51.2074576))]

2. Erstelle region
let s3region = SedenDreieck line2region

3. Query region 
query s3region
Type : region
Value: 
(
    (
        (
            (6.9665451 51.2074576) 
            (6.9680353 51.2074944) 
            (6.9678133 51.2074928) 
            (6.9670241 51.2074876))))


***************
WayIds (auf Sedentaler Strasse)
254881822
375343324
375363885
375363886

Filter for two wayIds:
query SedentalerStr feed filter[(.WayId = 254881822) or (.WayId = 375343324)] consume

***************

Half segment 0: (6.9665451, 51.2074576) - (6.9670241, 51.2074876) (dominant point:(6.96654510, 51.20745760)


    (6.9665451 51.2074576 6.9670241 51.2074876) 
    (6.9670241 51.2074876 6.9678133 51.2074928) 
    (6.9678133 51.2074928 6.9680353 51.2074943))




