let OrteP = plz feed sortby[Ort asc] groupby[Ort; Anzahl: . count] consume
let OrteP2 = plz feed project[Ort] consume
let OrteP2_Ort = OrteP2 createbtree[Ort] 

sql select count(*)
from [ortep as o1, plz as p, ortep2 as o2]
where [o1:anzahl = 5, o1:ort = p:ort, p:ort = o2:ort]


sql select count(*)
from [ortep as o1, plz as p, ortep2 as o2]
where [o1:anzahl = 823, o1:ort = p:ort, p:ort = o2:ort]


sql select count(*)
from [ortep as o1, plz as p, ortep2 as o2]
where [o1:anzahl > 100, o1:ort = p:ort, p:ort = o2:ort]


sql select count(*)
from [ortep as o1, plz as p, ortep2 as o2]
where [o1:anzahl < 10, o1:ort = p:ort, p:ort = o2:ort]

