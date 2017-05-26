/*
----
This file is NOT part of SECONDO.
* Authors: Greg Hamerly and Jonathan Drake
 * Feedback: hamerly@cs.baylor.edu
 * See: http://cs.baylor.edu/~hamerly/software/kmeans.php
 * Copyright 2014
 *
 * A kernelized version of Elkan's algorithm. It would be nice to make this
 * inherit from both KernelKmeans and ElkanKmeans, but for now we basically
 * replicate code.
----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//[TOC] [\tableofcontents]

[1] Declarations for the Eklan's' kmeans algorithm


1 Declarations for the Elkan's kmeans algorithm

*/

#ifndef ELKAN_KMEANS_H
#define ELKAN_KMEANS_H

/* Authors: Greg Hamerly and Jonathan Drake
 * Feedback: hamerly@cs.baylor.edu
 * See: http://cs.baylor.edu/~hamerly/software/kmeans.php
 * Copyright 2014
 *
 * Elkan's k-means algorithm that uses k lower bounds per point to prune
 * distance calculations.
 */


#include "triangle_inequality_base_kmeans.h"

class ElkanKmeans : public TriangleInequalityBaseKmeans {
    public:
        ElkanKmeans() : centerCenterDistDiv2(NULL) {}
        virtual ~ElkanKmeans() { free(); }
        virtual void free();
        virtual void initialize(Dataset const *aX, unsigned short aK, 
        unsigned short *initialAssignment, int aNumThreads);
        virtual std::string getName() const { return "elkan"; }

    protected:
        virtual int runThread(int threadId, int maxIterations);

        // Update the distances between each pair of centers.
        void update_center_dists(int threadId);

        // Update the upper and lower bounds for the range of points given.
        void update_bounds(int startNdx, int endNdx);

        // Keep track of the distance (divided by 2) between each pair of
        // points.
        double *centerCenterDistDiv2;
};

#endif

