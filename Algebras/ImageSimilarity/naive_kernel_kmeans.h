/*
----
This file is NOT part of SECONDO.
* Authors: Greg Hamerly and Jonathan Drake
 * Feedback: hamerly@cs.baylor.edu
 * See: http://cs.baylor.edu/~hamerly/software/kmeans.php
 * Copyright 2014
 *
 * NaiveKernelKmeans is a kernelized k-means algorithm that has no acceleration
 * applied.

----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//[TOC] [\tableofcontents]

[1] Declarations for the NaiveKernelKmeans algorithm


1 Declarations for the NaiveKernelKmeans algorithm

*/

#ifndef NAIVE_KERNEL_KMEANS_H
#define NAIVE_KERNEL_KMEANS_H

/* Authors: Greg Hamerly and Jonathan Drake
 * Feedback: hamerly@cs.baylor.edu
 * See: http://cs.baylor.edu/~hamerly/software/kmeans.php
 * Copyright 2014
 *
 * NaiveKernelKmeans is a kernelized k-means algorithm that has no acceleration
 * applied.

 */

#include "kernel_kmeans.h"

class NaiveKernelKmeans : public KernelKmeans {
    public:
        NaiveKernelKmeans(Kernel const *k) : KernelKmeans(k) {}
        virtual ~NaiveKernelKmeans() { free(); }
        virtual std::string getName() const {
            std::ostringstream out;
            out << "naive_kernel(" << kernel.getName() << ")";
            return out.str();
        }

    protected:
        int runThread(int threadId, int maxIterations);
};

#endif

