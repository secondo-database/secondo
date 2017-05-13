ImageSimilarityAlgebra
========

Written by Michael Loris, February-May 2017, 
licensed under the GNU license

An algebra for SECONDO to measure the distance of feature signatures of
jpeg images.
This algebra also makes use of the MTree algebra and the data type
"imagesignature", which represents an image's feature signature needs
to be registered in the GeneralTree algebra.

The work flow for this algebra is as follows:
1. place desired jpeg images in a directory
2. import them with the ImEx algebra: getDirectory
3. extend the stream of tuples (file names in this case) by running the
import operator "readSignatureFromImage" for each tuple.
4. pipe the tuple stream to the "createmtree" operator of the MTtree
algebra
5. run nearest neighbor of range queries against the new mtree


There are also three standalone programs supplied:
1. PicSim.cpp: will extract feature signatures from an image, it calls
the same methods as the "readSignatureFromImage" operator and uses
the same parameters.
2. EMD.cpp: will accept two files containing signatures and calculate 
the Earth Mover's Distance using the North-West version of the Simplex
algorithm.
3. SQFD.cpp: will accept two files containing signatures and calculate
the Signature Quadratic Form Distance.

All of the above files don't use threads or other methods to parallelize
the algorithms, as SECONDO doesn't like that. Anyhow, calculating the
texture features and the SQFD could be parallelized relatively easy. 
