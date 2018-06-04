simon jacobi, 2018-04-29

current status of Temporal2 algebra:

*extendwith operator*
prototypical, naive implementation for mpoint x stream(ipoint) -> mpoint is working

sample query:
let t7c = [const mpoint value undef];
query t7c extendwith intstream (1,10) use [ fun (i:int) final ([const mpoint value ((("2000-01-01-00:00" "2000-01-01-00:01" TRUE FALSE) (0.0 0.0 1.0 1.0)) )] translate [[const duration value (0 60000)] * i, 1.0 * i , 1.0* i ]) ];


*Tools for testing*
Test operators for remote stream control implemented:
streamvalve / streamnext


*Open topics*
Operator to handle items in stream in separate transactions (c.f. ContinuousUpdate -> owntransactioninsert, but for StreamAlgebra -> use)
Interface for using Index-based access to Moving Types:
- How to update Index (if any):
-> within extendwith operator (register index at mpoint and trigger index update on change)
-> separate exendwithandupdateindex operator



---
2018-05-14

open:
- how to detect closing of database?
-> would be nice to persist mem before close
-> if second thread timer triggered writeback -> how to ensure we remain in consistent state?
--> does transaction prevent closing?

shared memory should be per database...
how to handle correct cleanup?
sequence 1 open db a, close a, open b, close b
sequence 2 open db a, ....... .........          close a
                                             ^ destroy mem b
                                                      ^destroy mem a

Probably during Create we can check which (if any) database is open.
Then we may open the log... (-> can we detect, if it is already open?)

related:
how to export?
-> nl-rep: should not have internal id
-> nl->rep mem/log nono
but database:
- log + mpoint2 have id

object lifecycle:
-> Close: if last ref closed -> persist!? At least log?
-> Delete: mark deleted in log, persist log?

MPoint semantic -> no units =>? undefined