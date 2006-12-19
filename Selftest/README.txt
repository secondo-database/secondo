If results defined in the .examples file are too big, they can be stored as
nested list (in object format as written out by save obj to file) in a file
named by the following convention:

result<No>_<Opname>_<Algebra>

where <Algebra> is the first part of the .example file. Hence
the short version (without suffix Algebra), e.g.

result1_feed_Relation
result3_PLUS_Standard

<No> is the number of the example, <Opname> is the operator name or if it is a
symbol like + the alias Name written in capital letters (like in the spec
file). In the latter case the alias name must be specified in the .examples
file as shown in test.examples

Regards

Markus
