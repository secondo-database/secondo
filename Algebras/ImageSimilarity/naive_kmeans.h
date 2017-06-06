/*
----
This file is NOT part of SECONDO.
* Authors: Greg Hamerly and Jonathan Drake
 * Feedback: hamerly@cs.baylor.edu
 * See: http://cs.baylor.edu/~hamerly/software/kmeans.php
 * Copyright 2014
 *
 * NaiveKmeans is the standard k-means algorithm that has no acceleration
 * applied. Also known as Lloyd's algorithm.

----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//[TOC] [\tableofcontents]

[1] Declarations for the Lloyd's  algorithm


1 Declarations for the Lloyd's  algorithm

*/

#ifndef NAIVE_KMEANS_H
#define NAIVE_KMEANS_H

/* Authors: Greg Hamerly and Jonathan Drake
 * Feedback: hamerly@cs.baylor.edu
 * See: http://cs.baylor.edu/~hamerly/software/kmeans.php
 * Copyright 2014
 *
 * NaiveKmeans is the standard k-means algorithm that has no acceleration
 * applied. Also known as Lloyd's algorithm.

 */

#include "original_space_kmeans.h"

class NaiveKmeans : public OriginalSpaceKmeans {
    public:
        virtual std::string getName() const { return "naive"; }
    protected:
        virtual ~NaiveKmeans() { free(); }
        virtual int runThread(int threadId, int maxIterations);
};

#endif

