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
