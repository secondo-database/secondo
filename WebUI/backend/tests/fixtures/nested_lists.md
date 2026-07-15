# Captured nested-list samples (berlintest)

Real `NestedList::ToString` output captured from a live SecondoMonitor via the
`secondo_native` binding. Used as fixtures for the nested-list parser and the
GeoJSON converter (Milestone 2+).

## point (`query mehringdamm`)
```
(point (9396.0 9871.0))
```

## region (`query thecenter`) — truncated
```
(region ((((4751.258065743784 11098.44273821416) (5174.969135626282 10226.09641786784) ...) )))
```
Structure: `(region ( face* ))`, `face = ( cycle* )`, `cycle = ( (x y)* )`.
First cycle of a face is the outer boundary, following cycles are holes.

## line (`query BGrenzenLine`) — truncated
```
(line ((-10849.0 1142.0 -10720.0 454.0) (-10849.0 1142.0 -10349.0 1402.0) ...))
```
Structure: `(line ( segment* ))`, `segment = (x1 y1 x2 y2)`.

## relation (`query Trains feed head[1] project[Id, Line] consume`)
```
((rel (tuple ((Id int) (Line int)))) ((1 1)))
```
Structure: `((rel (tuple ( (attrname attrtype)* ))) ( tuple* ))`,
`tuple = ( attrvalue* )`.

## scalars
```
(int 13)
(inquiry (databases (BERLINTEST OPT SYMTRAJSMALL)))
```
