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
#include "DistfunReg.h"
#include "Algebras/Picture/PictureAlgebra.h"
#include "PictureFuns.h"

using namespace gta;
using namespace std;


Lab::Lab (unsigned char r_, unsigned char g_, unsigned char b_)
{
    double R, G, B;
    double rd = (double) r_ / 255;
    double gd = (double) g_ / 255;
    double bd = (double) b_ / 255;

    if (rd > 0.04045)
        R = std::pow((rd + 0.055) / 1.055, 2.2);
    else
        R = rd / 12.92;

    if (gd > 0.04045)
        G = std::pow ((gd + 0.055) / 1.055, 2.2);
    else
        G = gd / 12.92;

    if (bd > 0.04045)
        B = std::pow ((bd + 0.055) / 1.055, 2.2);
    else
        B = bd / 12.92;

    // compute X,Y,Z coordinates of r,g,b
    double X = 0.4124 * R + 0.3576 * G + 0.1805 * B;
    double Y = 0.2127 * R + 0.7152 * G + 0.0722 * B;
    double Z = 0.0193 * R + 0.1192 * G + 0.9500 * B;

    /* used chromacity coordinates of whitepoint D65:
    x = 0.312713, y = 0.329016

    the respective XYZ coordinates are
    Y = 1,
    X = Y * x / y       = 0.9504492183, and
    Z = Y * (1-x-y) / y = 1.0889166480
    */

    double eps = 0.008856; // = 216 / 24389
    double x = X / 0.95045;
    double y = Y;
    double z = Z / 1.08892;
    long double fx, fy, fz;

    if (x > eps)
        fx = std::pow (x, 0.333333);
    else
        fx = 7.787 * x + 0.137931;

    if (y > eps)
        fy = std::pow (y, 0.333333);
    else
        fy = 7.787 * y + 0.137931;

    if (z > eps)
        fz = std::pow (z, 0.333333);
    else
        fz = 7.787 * z + 0.137931;

    // compute Lab coordinates
    double Lab_Ld = ((116  * fy) - 16);
    double Lab_ad = (500 * (fx - fy));
    double Lab_bd = (200 * (fy - fz));

    L = (signed char) Lab_Ld;
    a = (signed char) Lab_ad;
    b = (signed char) Lab_bd;
}


HSV::HSV (unsigned char r, unsigned char g, unsigned char b)
{
    unsigned char rgbMin = std::min (std::min (r, g), b);
    unsigned char rgbMax = std::max (std::max (r, g), b);
    unsigned char delta = rgbMax - rgbMin;

    // compute h
    if (delta == 0)
    {
        h = 0;
    }
    else
    {
        if (rgbMax == r)
        {
            h = 60 * (g - b) / delta;
        }
        else if (rgbMax == g)
        {
            h = 120 * (g - b) / delta;
        }
        else // rgbMax == b
        {
            h = 240 * (g - b) / delta;
        }
    }

    if (h < 0)
        h += 360;

    // compute s
    if (rgbMax == 0)
        s = 0;
    else
        s = 255 * delta / rgbMax;

    // compute v
    v = rgbMax;
}



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
        double d1 = std::pow(0.25 * (v1 - v2), 2);
        double d2 = std::pow((0.125 + (s1 * 0.25)) * std::cos((h1 * 90.0)) -
                        (0.125 + (s2 * 0.25)) * std::cos((h2 * 90.0)), 2);
        double d3 = std::pow((0.125 + (s1 * 0.25)) * std::sin((h1 * 90.0)) -
                        (0.125 + (s2 * 0.25)) * std::sin((h2 * 90.0)), 2);

        int pos1 = (16 * h1) + (4 * s1) + v1;
        int pos2 = (16 * h2) + (4 * s2) + v2;
        simMatrix[pos1*64 + pos2] = std::exp((-2) * std::sqrt(d1 + d2 + d3));
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
        double d1 = std::pow(0.25 * (v1 - v2), 2);
        double d2 = std::pow((0.125 + (s1 * 0.25)) * std::cos((h1 * 45.0)) -
                        (0.125 + (s2 * 0.25)) * std::cos((h2 * 45.0)), 2);
        double d3 = std::pow((0.125 + (s1 * 0.25)) * std::sin((h1 * 45.0)) -
                        (0.125 + (s2 * 0.25)) * std::sin((h2 * 45.0)), 2);

        int pos1 = (16 * h1) + (4 * s1) + v1;
        int pos2 = (16 * h2) + (4 * s2) + v2;
        simMatrix[pos1*128 + pos2] = std::exp((-2) * std::sqrt(d1 + d2 + d3));
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
        double d1 = std::pow(0.25 * (v1 - v2), 2);
        double d2 = std::pow((0.125 + (s1 * 0.25)) * std::cos((h1 * 22.5)) -
                        (0.125 + (s2 * 0.25)) * std::cos((h2 * 22.5)), 2);
        double d3 = std::pow((0.125 + (s1 * 0.25)) * std::sin((h1 * 22.5)) -
                        (0.125 + (s2 * 0.25)) * std::sin((h2 * 22.5)), 2);

        int pos1 = (16 * h1) + (4 * s1) + v1;
        int pos2 = (16 * h2) + (4 * s2) + v2;
        simMatrix[pos1*256 + pos2] = std::exp((-2) * std::sqrt(d1 + d2 + d3));
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
                            double d = std::pow(25.0 * (L1 - L2), 2) +
                                       std::pow(23.125 * (a1 - a2), 2) +
                                       std::pow(25.250 * (b1 - b2), 2);

                            int pos1 = (64 * L1) + (8 * a1) + b1;
                            int pos2 = (64 * L2) + (8 * a2) + b2;
                            simMatrix[(pos1*256) + pos2] =
                                std::exp((-2) * std::sqrt(d));
                        }
}

/*
Method ~getHPoint[_]hsv8~:

*/
HPoint* PictureFuns::getHPoint_hsv8 (const void* attr)
{
    DistData* data = getData_hsv8<false>(attr);
    histDomain hist[8];
    decodeHistogram<histDomain> (hist, data, 8, false);
    histDomain* h = new GTA_SPATIAL_DOM[8];
    for (unsigned i = 0; i < 8; ++i)
        h[i] = hist[i];

    return new HPoint(8, h);
}

/*
Method ~getHPoint[_]hsv16~:

*/
HPoint* PictureFuns::getHPoint_hsv16 (const void* attr)
{
    DistData* data = getData_hsv16<false>(attr);
    histDomain hist[16];
    decodeHistogram<histDomain> (hist, data, 16, false);
    histDomain* h = new GTA_SPATIAL_DOM[16];
    for (unsigned i = 0; i < 16; ++i)
        h[i] = hist[i];

    return new HPoint(16, h);
}

/*
Method ~getHPoint[_]hsv32~:

*/
HPoint* PictureFuns::getHPoint_hsv32 (const void* attr)
{
    DistData* data = getData_hsv32<false>(attr);
    histDomain hist[32];
    decodeHistogram<histDomain> (hist, data, 32, false);
    histDomain* h = new GTA_SPATIAL_DOM[32];
    for (unsigned i = 0; i < 32; ++i)
        h[i] = hist[i];

    return new HPoint(32, h);
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
        HSV8, HSV8_DESCR, DDATA_HSV8_ID,
        Picture::BasicType(), &getData_hsv8<true>));

    DistDataReg::addInfo(DistDataInfo(
        HSV16, HSV16_DESCR, DDATA_HSV16_ID,
        Picture::BasicType(), &getData_hsv16<true>));

    DistDataReg::addInfo(DistDataInfo(
        HSV32, HSV32_DESCR, DDATA_HSV32_ID,
        Picture::BasicType(), &getData_hsv32<true>));

    DistDataReg::addInfo(DistDataInfo(
        HSV64, HSV64_DESCR, DDATA_HSV64_ID,
        Picture::BasicType(), &getData_hsv64<true>));

    DistDataReg::addInfo(DistDataInfo(
        HSV128, HSV128_DESCR, DDATA_HSV128_ID,
        Picture::BasicType(), &getData_hsv128<true>));

    DistDataReg::addInfo(DistDataInfo(
        HSV256, HSV256_DESCR, DDATA_HSV256_ID,
        Picture::BasicType(), &getData_hsv256<true>));

    DistDataReg::addInfo(DistDataInfo(
        LAB256, LAB256_DESCR, DDATA_LAB256_ID,
        Picture::BasicType(), &getData_lab256<true>));

    DistDataReg::addInfo(DistDataInfo(
        HSV8_NCOMPR, HSV8_NCOMPR_DESCR,
        DDATA_HSV8_NCOMPR_ID, Picture::BasicType(), &getData_hsv8<false>));

    DistDataReg::addInfo(DistDataInfo(
        HSV16_NCOMPR, HSV16_NCOMPR_DESCR,
        DDATA_HSV16_NCOMPR_ID, Picture::BasicType(), &getData_hsv16<false>));

    DistDataReg::addInfo(DistDataInfo(
        HSV32_NCOMPR, HSV32_NCOMPR_DESCR,
        DDATA_HSV32_NCOMPR_ID, Picture::BasicType(), &getData_hsv32<false>));

    DistDataReg::addInfo(DistDataInfo(
        HSV64_NCOMPR, HSV64_NCOMPR_DESCR,
        DDATA_HSV64_NCOMPR_ID, Picture::BasicType(), &getData_hsv64<false>));

    DistDataReg::addInfo(DistDataInfo(
        HSV128_NCOMPR, HSV128_NCOMPR_DESCR,
        DDATA_HSV128_NCOMPR_ID, Picture::BasicType(), &getData_hsv128<false>));

    DistDataReg::addInfo(DistDataInfo(
        HSV256_NCOMPR, HSV256_NCOMPR_DESCR,
        DDATA_HSV256_NCOMPR_ID, Picture::BasicType(), &getData_hsv256<false>));

    DistDataReg::addInfo(DistDataInfo(
        LAB256_NCOMPR, LAB256_NCOMPR_DESCR,
        DDATA_LAB256_NCOMPR_ID, Picture::BasicType(), &getData_lab256<false>));
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
        DistDataReg::getInfo(Picture::BasicType(), HSV64),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv128<true>,
        DistDataReg::getInfo(Picture::BasicType(), HSV128),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv256<true>,
        DistDataReg::getInfo(Picture::BasicType(), HSV256),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_lab256<true>,
        DistDataReg::getInfo(Picture::BasicType(), LAB256),
        DFUN_IS_METRIC));

    // euclidean distance functions for compressed data
    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<8, true>,
        DistDataReg::getInfo(Picture::BasicType(), HSV8),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<16, true>,
        DistDataReg::getInfo(Picture::BasicType(), HSV16),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<32, true>,
        DistDataReg::getInfo(Picture::BasicType(), HSV32),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<64, true>,
        DistDataReg::getInfo(Picture::BasicType(), HSV64),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<128, true>,
        DistDataReg::getInfo(Picture::BasicType(), HSV128),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<256, true>,
        DistDataReg::getInfo(Picture::BasicType(), HSV256),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<256, true>,
        DistDataReg::getInfo(Picture::BasicType(), LAB256),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    // quadratic distance functions for uncompressed data
    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv64<false>,
        DistDataReg::getInfo(Picture::BasicType(), HSV64_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv128<false>,
        DistDataReg::getInfo(Picture::BasicType(), HSV128_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_hsv256<false>,
        DistDataReg::getInfo(Picture::BasicType(), HSV256_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_QUADRATIC, DFUN_QUADRATIC_DESCR,
        quadr_lab256<false>,
        DistDataReg::getInfo(Picture::BasicType(), LAB256_NCOMPR),
        DFUN_IS_METRIC));

    // euclidean distance functions for uncompressed data
    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<8, false>,
        DistDataReg::getInfo(Picture::BasicType(), HSV8_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<16, false>,
        DistDataReg::getInfo(Picture::BasicType(), HSV16_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<32, false>,
        DistDataReg::getInfo(Picture::BasicType(), HSV32_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<64, false>,
        DistDataReg::getInfo(Picture::BasicType(), HSV64_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<128, false>,
        DistDataReg::getInfo(Picture::BasicType(), HSV128_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<256, false>,
        DistDataReg::getInfo(Picture::BasicType(), HSV256_NCOMPR),
        DFUN_IS_METRIC));

    DistfunReg::addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        euclidean<256, false>,
        DistDataReg::getInfo(Picture::BasicType(), LAB256_NCOMPR),
        DFUN_IS_METRIC));
}

/*
Method ~initHPointReg~

*/
void PictureFuns::initHPointReg()
{
    HPointReg::addInfo(HPointInfo(
        HSV8, Picture::BasicType(), &getHPoint_hsv8));

    HPointReg::addInfo(HPointInfo(
        HSV16, Picture::BasicType(), &getHPoint_hsv16));

    HPointReg::addInfo(HPointInfo(
        HSV32, Picture::BasicType(), &getHPoint_hsv32));

    HPointReg::addInfo(HPointInfo(
        HSV64, Picture::BasicType(), &getHPoint_hsv64));

    HPointReg::addInfo(HPointInfo(
        HSV128, Picture::BasicType(), &getHPoint_hsv128));

    HPointReg::addInfo(HPointInfo(
        HSV256, Picture::BasicType(), &getHPoint_hsv256));

    HPointReg::addInfo(HPointInfo(
        LAB256, Picture::BasicType(), &getHPoint_lab256, HPOINT_IS_DEFAULT));
}
