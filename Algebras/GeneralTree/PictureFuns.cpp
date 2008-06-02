/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

1 Implementation file "PictureFuns.cpp"[4]

January-May 2008, Mirko Dibbert

*/
#include "PictureFuns.h"
#include "DistfunReg.h"

using namespace gta;
using namespace symbols;

/*
initialize static members:

*/
histDomain* PictureFuns::simMatrix = 0;
SIM_MATRIX_ID PictureFuns::simMatrixId = SIM_MATRIX_NONE;
unsigned* PictureFuns::pictureLabOffsetTable = 0;

/*
Method ~computeHsv64Matrix~:

*/
void PictureFuns::computeHsv64SimMatrix()
{
    if (simMatrixId != SIM_MATRIX_NONE)
        delete[] simMatrix;
    simMatrixId = SIM_MATRIX_HSV64;
    simMatrix = new histDomain[64*64];

    for (int h1 = 0; h1 < 4; ++h1)
        for (int s1 = 0; s1 < 4; ++s1)
            for (int v1 = 0; v1 < 4; ++v1)
                for (int h2 = 0; h2 < 4; ++h2)
                    for (int s2 = 0; s2 < 4; ++s2)
                        for (int v2 = 0; v2 < 4; ++v2)
    {
        double d1 = pow(0.25 * (v1 - v2), 2);
        double d2 = pow((0.125 + (s1 * 0.25)) * cos((h1 * 90.0)) -
                        (0.125 + (s2 * 0.25)) * cos((h2 * 90.0)), 2);
        double d3 = pow((0.125 + (s1 * 0.25)) * sin((h1 * 90.0)) -
                        (0.125 + (s2 * 0.25)) * sin((h2 * 90.0)), 2);

        int pos1 = (16 * h1) + (4 * s1) + v1;
        int pos2 = (16 * h2) + (4 * s2) + v2;
        simMatrix[pos1*128 + pos2] = exp((-2) * sqrt(d1 + d2 + d3));
    }
}

/*
Method ~computeHsv128Matrix~:

*/
void PictureFuns::computeHsv128SimMatrix()
{
    if (simMatrixId != SIM_MATRIX_NONE)
        delete[] simMatrix;
    simMatrixId = SIM_MATRIX_HSV128;
    simMatrix = new histDomain[128*128];

    for (int h1 = 0; h1 < 8; ++h1)
        for (int s1 = 0; s1 < 4; ++s1)
            for (int v1 = 0; v1 < 4; ++v1)
                for (int h2 = 0; h2 < 8; ++h2)
                    for (int s2 = 0; s2 < 4; ++s2)
                        for (int v2 = 0; v2 < 4; ++v2)
    {
        double d1 = pow(0.25 * (v1 - v2), 2);
        double d2 = pow((0.125 + (s1 * 0.25)) * cos((h1 * 45.0)) -
                        (0.125 + (s2 * 0.25)) * cos((h2 * 45.0)), 2);
        double d3 = pow((0.125 + (s1 * 0.25)) * sin((h1 * 45.0)) -
                        (0.125 + (s2 * 0.25)) * sin((h2 * 45.0)), 2);

        int pos1 = (16 * h1) + (4 * s1) + v1;
        int pos2 = (16 * h2) + (4 * s2) + v2;
        simMatrix[pos1*128 + pos2] = exp((-2) * sqrt(d1 + d2 + d3));
    }
}

/*
Method ~computeHsv256Matrix~:

*/
void PictureFuns::computeHsv256SimMatrix()
{
    if (simMatrixId != SIM_MATRIX_NONE)
        delete[] simMatrix;
    simMatrixId = SIM_MATRIX_HSV256;
    simMatrix = new histDomain[256*256];

    for (int h1 = 0; h1 < 16; ++h1)
        for (int s1 = 0; s1 < 4; ++s1)
            for (int v1 = 0; v1 < 4; ++v1)
                for (int h2 = 0; h2 < 16; ++h2)
                    for (int s2 = 0; s2 < 4; ++s2)
                        for (int v2 = 0; v2 < 4; ++v2)
    {
        double d1 = pow(0.25 * (v1 - v2), 2);
        double d2 = pow((0.125 + (s1 * 0.25)) * cos((h1 * 22.5)) -
                        (0.125 + (s2 * 0.25)) * cos((h2 * 22.5)), 2);
        double d3 = pow((0.125 + (s1 * 0.25)) * sin((h1 * 22.5)) -
                        (0.125 + (s2 * 0.25)) * sin((h2 * 22.5)), 2);

        int pos1 = (16 * h1) + (4 * s1) + v1;
        int pos2 = (16 * h2) + (4 * s2) + v2;
        simMatrix[pos1*256 + pos2] = exp((-2) * sqrt(d1 + d2 + d3));
    }
}

/*
Method ~computeLab256Matrix~:

*/
void PictureFuns::computeLab256SimMatrix()
{
    if (simMatrixId != SIM_MATRIX_NONE)
        delete[] simMatrix;
    simMatrixId = SIM_MATRIX_LAB256;
    simMatrix = new histDomain[256*256];

    for (int L1 = 0; L1 < 4; ++L1)
        for (int a1 = 0; a1 < 8; ++a1)
            for (int b1 = 0; b1 < 8; ++b1)
                for (int L2 = 0; L2 < 4; ++L2)
                    for (int a2 = 0; a2 < 8; ++a2)
                        for (int b2 = 0; b2 < 8; ++b2)
                        {
                            double d = pow(25.0 * (L1 - L2), 2) +
                                       pow(23.125 * (a1 - a2), 2) +
                                       pow(25.250 * (b1 - b2), 2);

                            int pos1 = (64 * L1) + (8 * a1) + b1;
                            int pos2 = (64 * L2) + (8 * a2) + b2;
                            simMatrix[(pos1*256) + pos2] =
                                exp((-2) * sqrt(d));
                        }
}

/*
Method ~getHPoint[_]hsv64~:

*/
HPoint* PictureFuns::getHPoint_hsv64 (const void* attr)
{
    DistData* data = getData_hsv64<false>(attr);
    histDomain hist[64];
    decodeHistogram<histDomain> (hist, data, 64, false);
    histDomain* h = new GTA_SPATIAL_DOM[64];
    for (unsigned i = 0; i < 64; ++i)
        h[i] = hist[i];

    return new HPoint(64, h);
}

/*
Method ~getHPoint[_]hsv128~:

*/
HPoint* PictureFuns::getHPoint_hsv128 (const void* attr)
{
    DistData* data = getData_hsv128<false>(attr);
    histDomain hist[128];
    decodeHistogram<histDomain> (hist, data, 128, false);
    histDomain* h = new GTA_SPATIAL_DOM[128];
    for (unsigned i = 0; i < 128; ++i)
        h[i] = hist[i];

    return new HPoint(128, h);
}

/*
Method ~getHPoint[_]hsv256~:

*/
HPoint* PictureFuns::getHPoint_hsv256 (const void* attr)
{
    DistData* data = getData_hsv256<false>(attr);
    histDomain hist[256];
    decodeHistogram<histDomain> (hist, data, 256, false);
    histDomain* h = new GTA_SPATIAL_DOM[256];
    for (unsigned i = 0; i < 256; ++i)
        h[i] = hist[i];

    return new HPoint(256, h);
}

/*
Method ~getHPoint[_]lab256~:

*/
HPoint* PictureFuns::getHPoint_lab256 (const void* attr)
{
    DistData* data = getData_lab256<false>(attr);
    histDomain hist[256];
    decodeHistogram<histDomain> (hist, data, 256, false);
    histDomain* h = new GTA_SPATIAL_DOM[256];
    for (unsigned i = 0; i < 256; ++i)
        h[i] = hist[i];

    return new HPoint(256, h);
}

/*
Method ~initDistDataReg~

*/
void PictureFuns::initDistData()
{
    DistDataReg::addInfo(DistDataInfo(
        HSV64, HSV64_DESCR, DDATA_HSV64_ID,
        PICTURE, &getData_hsv64<true>));

    DistDataReg::addInfo(DistDataInfo(
        HSV128, HSV128_DESCR, DDATA_HSV128_ID,
        PICTURE, &getData_hsv128<true>));

    DistDataReg::addInfo(DistDataInfo(
        HSV256, HSV256_DESCR, DDATA_HSV256_ID,
        PICTURE, &getData_hsv256<true>));

    DistDataReg::addInfo(DistDataInfo(
        LAB256, LAB256_DESCR, DDATA_LAB256_ID,
        PICTURE, &getData_lab256<true>));

    DistDataReg::addInfo(DistDataInfo(
        HSV64_NCOMPR, HSV64_NCOMPR_DESCR,
        DDATA_HSV64_NCOMPR_ID, PICTURE, &getData_hsv64<false>));

    DistDataReg::addInfo(DistDataInfo(
        HSV128_NCOMPR, HSV128_NCOMPR_DESCR,
        DDATA_HSV128_NCOMPR_ID, PICTURE, &getData_hsv128<false>));

    DistDataReg::addInfo(DistDataInfo(
        HSV256_NCOMPR, HSV256_NCOMPR_DESCR,
        DDATA_HSV256_NCOMPR_ID, PICTURE, &getData_hsv256<false>));

    DistDataReg::addInfo(DistDataInfo(
        LAB256_NCOMPR, LAB256_NCOMPR_DESCR,
        DDATA_LAB256_NCOMPR_ID, PICTURE, &getData_lab256<false>));
}

/*
Method ~initDistfunReg~

*/
void PictureFuns::initDistfuns()
{
    // quadratic distance functions for compressed data
    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv64<true>,
        DistDataReg::getInfo(PICTURE, HSV64),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv128<true>,
        DistDataReg::getInfo(PICTURE, HSV128),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv256<true>,
        DistDataReg::getInfo(PICTURE, HSV256),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_lab256<true>,
        DistDataReg::getInfo(PICTURE, LAB256),
        DFUN_IS_METRIC));

    // euclidean distance functions for compressed data
    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<64, true>,
        DistDataReg::getInfo(PICTURE, HSV64),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<128, true>,
        DistDataReg::getInfo(PICTURE, HSV128),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<256, true>,
        DistDataReg::getInfo(PICTURE, HSV256),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<256, true>,
        DistDataReg::getInfo(PICTURE, LAB256),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    // quadratic distance functions for uncompressed data
    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv64<false>,
        DistDataReg::getInfo(PICTURE, HSV64_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv128<false>,
        DistDataReg::getInfo(PICTURE, HSV128_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv256<false>,
        DistDataReg::getInfo(PICTURE, HSV256_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_lab256<false>,
        DistDataReg::getInfo(PICTURE, LAB256_NCOMPR),
        DFUN_IS_METRIC));

    // euclidean distance functions for uncompressed data
    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<64, false>,
        DistDataReg::getInfo(PICTURE, HSV64_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<128, false>,
        DistDataReg::getInfo(PICTURE, HSV128_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<256, false>,
        DistDataReg::getInfo(PICTURE, HSV256_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<256, false>,
        DistDataReg::getInfo(PICTURE, LAB256_NCOMPR),
        DFUN_IS_METRIC));
}

/*
Method ~initHPointReg~

*/
void PictureFuns::initHPointReg()
{
    HPointReg::addInfo(HPointInfo(
        HSV64, PICTURE, &getHPoint_hsv64));

    HPointReg::addInfo(HPointInfo(
        HSV128, PICTURE, &getHPoint_hsv128));

    HPointReg::addInfo(HPointInfo(
        HSV256, PICTURE, &getHPoint_hsv256));

    HPointReg::addInfo(HPointInfo(
        LAB256, PICTURE, &getHPoint_lab256, HPOINT_IS_DEFAULT));
}
