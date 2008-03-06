/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute iter and/or modify
iter under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that iter will be useful,
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

January-February 2008, Mirko Dibbert
\\[3ex]
This file implements the "PictureFuns"[4] class.

*/
#include "PictureFuns.h"
#include "DistfunReg.h"

using namespace picture_funs;
using namespace generalTree;

/*
initialize static members:

*/
histDomain* PictureFuns::hsv128SimMatrix = 0;
histDomain* PictureFuns::hsv256SimMatrix = 0;
histDomain* PictureFuns::lab256SimMatrix = 0;
unsigned* PictureFuns::pictureLabOffsetTable = 0;

/*
Method ~computeHsv128Matrix~:

*/
void
PictureFuns::computeHsv128SimMatrix()
{
    hsv128SimMatrix = new histDomain[128*128];

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
        hsv128SimMatrix[pos1*128 + pos2]
        = exp((-2) * sqrt(d1 + d2 + d3));
    }
}

/*
Method ~computeHsv256Matrix~:

*/
void
PictureFuns::computeHsv256SimMatrix()
{
    hsv256SimMatrix = new histDomain[256*256];

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
        hsv256SimMatrix[pos1*256 + pos2]
        = exp((-2) * sqrt(d1 + d2 + d3));
    }
}

/*
Method ~computeLab256Matrix~:

*/
void
PictureFuns::computeLab256SimMatrix()
{
    lab256SimMatrix = new histDomain[256*256];

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
                            lab256SimMatrix[(pos1*256) + pos2] =
                                exp((-2) * sqrt(d));
                        }
}

/*
Method ~removeAuxiliaryArrays~:

*/
void
PictureFuns::removeAuxiliaryArrays()
{
    if (hsv128SimMatrix)
    {
        delete[] hsv128SimMatrix;
        hsv128SimMatrix = 0;
    }

    if (hsv256SimMatrix)
    {
        delete[] hsv256SimMatrix;
        hsv256SimMatrix = 0;
    }

    if (lab256SimMatrix)
    {
        delete[] lab256SimMatrix;
        lab256SimMatrix = 0;
    }

    if (pictureLabOffsetTable)
    {
        delete[] pictureLabOffsetTable;
        pictureLabOffsetTable = 0;
    }
}

/*
Method ~initDistDataReg~

*/
void PictureFuns::initDistData()
{
    DistDataReg::addInfo(DistDataInfo(
        DDATA_HSV128, DDATA_HSV128_DESCR, DDATA_HSV128_ID,
        PICTURE, &getData_hsv128<true>));

    DistDataReg::addInfo(DistDataInfo(
        DDATA_HSV256, DDATA_HSV256_DESCR, DDATA_HSV256_ID,
        PICTURE, &getData_hsv256<true>));

    DistDataReg::addInfo(DistDataInfo(
        DDATA_LAB256, DDATA_LAB256_DESCR, DDATA_LAB256_ID,
        PICTURE, &getData_lab256<true>));

    DistDataReg::addInfo(DistDataInfo(
        DDATA_HSV128_NCOMPR,
        DDATA_HSV128_NCOMPR_DESCR,
        DDATA_HSV128_NCOMPR_ID,
        PICTURE, &getData_hsv128<false>));

    DistDataReg::addInfo(DistDataInfo(
        DDATA_HSV256_NCOMPR,
        DDATA_HSV256_NCOMPR_DESCR,
        DDATA_HSV256_NCOMPR_ID,
        PICTURE, &getData_hsv256<false>));

    DistDataReg::addInfo(DistDataInfo(
        DDATA_LAB256_NCOMPR,
        DDATA_LAB256_NCOMPR_DESCR,
        DDATA_LAB256_NCOMPR_ID,
        PICTURE, &getData_lab256<false>));
}

/*
Method ~initDistfunReg~

*/
void PictureFuns::initDistfuns()
{
/*
Distance function using compressed histograms:

*/
    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv128<true>,
        DistDataReg::getDataId(PICTURE, DDATA_HSV128),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv256<true>,
        DistDataReg::getDataId(PICTURE, DDATA_HSV256),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_lab256<true>,
        DistDataReg::getDataId(PICTURE, DDATA_LAB256),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        eucl_hsv128<true>,
        DistDataReg::getDataId(PICTURE, DDATA_HSV128),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        eucl_hsv256<true>,
        DistDataReg::getDataId(PICTURE, DDATA_HSV256),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        eucl_lab256<true>,
        DistDataReg::getDataId(PICTURE, DDATA_LAB256),
        DFUN_IS_METRIC));

/*
Distance function using uncompressed histograms:

*/
    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv128<false>,
        DistDataReg::getDataId(PICTURE, DDATA_HSV128_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv256<false>,
        DistDataReg::getDataId(PICTURE, DDATA_HSV256_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_lab256<false>,
        DistDataReg::getDataId(PICTURE, DDATA_LAB256_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        eucl_hsv128<false>,
        DistDataReg::getDataId(PICTURE, DDATA_HSV128_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        eucl_hsv256<false>,
        DistDataReg::getDataId(PICTURE, DDATA_HSV256_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        eucl_lab256<false>,
        DistDataReg::getDataId(PICTURE, DDATA_LAB256_NCOMPR),
        DFUN_IS_METRIC));
}

/*
Method ~initBBRegionReg~

*/
void PictureFuns::initBBoxes()
{
    BBoxReg::addInfo(BBoxInfo(
        getBBox<128, true>, createBBox<128, true>,
        DistDataReg::getDataId(PICTURE, DDATA_HSV128)));

    BBoxReg::addInfo(BBoxInfo(
        getBBox<256, true>, createBBox<256, true>,
        DistDataReg::getDataId(PICTURE, DDATA_HSV256)));

    BBoxReg::addInfo(BBoxInfo(
        getBBox<256, true>, createBBox<256, true>,
        DistDataReg::getDataId(PICTURE, DDATA_LAB256)));

    BBoxReg::addInfo(BBoxInfo(
        getBBox<128, true>, createBBox<128, true>,
        DistDataReg::getDataId(PICTURE, DDATA_HSV128_NCOMPR)));

    BBoxReg::addInfo(BBoxInfo(
        getBBox<256, true>, createBBox<256, true>,
        DistDataReg::getDataId(PICTURE, DDATA_HSV256_NCOMPR)));

    BBoxReg::addInfo(BBoxInfo(
        getBBox<256, true>, createBBox<256, true>,
        DistDataReg::getDataId(PICTURE, DDATA_LAB256_NCOMPR)));
}
