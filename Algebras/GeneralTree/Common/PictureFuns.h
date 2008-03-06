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

1 Headerfile "PictureGuns.h"[4]

January-February 2008, Mirko Dibbert

1.1 Overview

This headerfile contains the "PictureFuns"[4] class, which implements all functions for the "DistDataReg"[4], the "DistfunReg"[4] and the "BBoxReg"[4] class for the picture type constructor.

*/

#ifndef __PICTURE_DISTFUNS_H
#define __PICTURE_DISTFUNS_H

#include "DistfunReg.h"
#include "BBoxReg.h"
#include "PictureAlgebra.h"
#include "JPEGPicture.h"

using namespace generalTree;

namespace picture_funs
{

typedef float histDomain;

/********************************************************************
1.1 Struct Lab

Transforms a rgb color value into an Lab color value.

********************************************************************/
struct Lab
{
    signed char L, a, b;

    Lab (unsigned char r_, unsigned char g_, unsigned char b_)
    {
        double R, G, B;
        double rd = (double) r_ / 255;
        double gd = (double) g_ / 255;
        double bd = (double) b_ / 255;

        if (rd > 0.04045)
            R = pow ((rd + 0.055) / 1.055, 2.2);
        else
            R = rd / 12.92;

        if (gd > 0.04045)
            G = pow ((gd + 0.055) / 1.055, 2.2);
        else
            G = gd / 12.92;

        if (bd > 0.04045)
            B = pow ((bd + 0.055) / 1.055, 2.2);
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
            fx = pow (x, 0.333333);
        else
            fx = 7.787 * x + 0.137931;

        if (y > eps)
            fy = pow (y, 0.333333);
        else
            fy = 7.787 * y + 0.137931;

        if (z > eps)
            fz = pow (z, 0.333333);
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
}; // struct Lab

/********************************************************************
1.1 Struct HSV

Transforms a rgb color value into an HSV color value.

********************************************************************/
struct HSV
{
    int h, s, v;

    HSV (unsigned char r, unsigned char g, unsigned char b)
    {
        unsigned char rgbMin = min (min (r, g), b);
        unsigned char rgbMax = max (max (r, g), b);
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
}; // struct HSV

/********************************************************************
1.1 Class PictureFuns

Contains all functions for the DistDataReg, DistfunReg and BBoxReg classes for the picture type constructor.

********************************************************************/
class PictureFuns
{
public:
/*
These methods are called from the "initialise"[4] method of the respective registration classes (DistDataReg, DistfunReg and BBoxReg).

*/
    static void initDistData();
    static void initDistfuns();
    static void initBBoxes();

/********************************************************************
The following methods implement the getdata functions for the respective distdata type:

********************************************************************/
    template<bool compressData>
    static DistData* getData_hsv128 (const void* attr);

    template<bool compressData>
    static DistData* getData_hsv256 (const void* attr);

    template<bool compressData>
    static DistData* getData_lab256 (const void* attr);

/********************************************************************
The following methods implement the defined distance functions for the picture type constructor:

********************************************************************/
    template<bool compressedData>
    static void eucl_hsv128 (
            const DistData* dd1, const DistData* dd2,
            DFUN_RESULT& result);

    template<bool compressedData>
    static void eucl_hsv256 (
            const DistData* dd1, const DistData* dd2,
            DFUN_RESULT& result);

    template<bool compressedData>
    static void eucl_lab256 (
            const DistData* dd1, const DistData* dd2,
            DFUN_RESULT& result);

    template<bool compressedData>
    static void quadr_hsv128 (
            const DistData* dd1, const DistData* dd2,
            DFUN_RESULT& result);

    template<bool compressedData>
    static void quadr_hsv256 (
            const DistData* dd1, const DistData* dd2,
            DFUN_RESULT& result);

    template<bool compressedData>
    static void quadr_lab256 (
            const DistData* dd1, const DistData* dd2,
            DFUN_RESULT& result);

/********************************************************************
The following methods implement the GetBBox and CreateBBox functions for the defined distdata types:

********************************************************************/
    // get bounding-region function
    // (creates a bounding box for the respective histogram)
    template<unsigned dim, bool compressedData>
    static BBox* getBBox (const DistData* data);

    template<unsigned dim, bool compressedData>
    static BBox* createBBox ();

private:
    static void computeHsv128SimMatrix();
    static void computeHsv256SimMatrix();
    static void computeLab256SimMatrix();

/********************************************************************
Computes the Minkowski-distance (and in particular the euclidean distance, if n=2)

********************************************************************/
    template<unsigned dim, int n>
    inline static void computeMinkowskiDist (
            histDomain* v1, histDomain* v2, DFUN_RESULT& result);

/********************************************************************
Computes the quadratic distance, using the given similarity matrix.

********************************************************************/
    template<unsigned dim>
    static inline void computeQuadraticDist (
            histDomain* v1, histDomain* v2, histDomain* simmatrix,
            DFUN_RESULT& result);

/********************************************************************
Encodes the histogram into a distdata object.

********************************************************************/
    template<class TFloat>
    static DistData* encodeHistogram (
            unsigned long* hist_abs, unsigned numOfPixels,
            unsigned size, bool compressData, TFloat threshold = 5e-5);

/********************************************************************
Decodes the histogram distdata object and writes the result to "hist"[4].

********************************************************************/
    template<class TFloat>
    static void decodeHistogram (
            TFloat* hist, const DistData* dd, unsigned size,
            bool compressedData);

/********************************************************************
Deletes the auxiliary arrays to free the reserved memory (currently not used)

********************************************************************/
    static void removeAuxiliaryArrays();

/********************************************************************
Auxiliary arrays for picture distfun:

********************************************************************/
    static histDomain* hsv128SimMatrix;
    static histDomain* hsv256SimMatrix;
    static histDomain* lab256SimMatrix;
    static unsigned* pictureLabOffsetTable;
}; // class PictureFuns



/********************************************************************
1.1 Implementation of inline and template methods

********************************************************************/
/*
Method ~getData[_]hsv128~:

*/
template<bool compressData> DistData*
PictureFuns::getData_hsv128 (const void* attr)
{
    unsigned long size;
    const char* imgdata = static_cast<const Picture*> (attr)->
                                                  GetJPEGData (size);

    JPEGPicture rgb ((unsigned char *) imgdata, size);

    unsigned long int rgbSize;
    unsigned char* rgbData = rgb.GetImageData (rgbSize);

    const unsigned int numOfPixels = rgbSize / 3;

    unsigned long hist_abs[128];
    memset (hist_abs, 0, 128*sizeof (unsigned long));

    for (int i = 0; i < 128; ++i)
        hist_abs[i] = 0;

    for (unsigned long pos = 0; pos < (numOfPixels); ++pos)
    {
        unsigned char r = rgbData[ (3*pos) ];
        unsigned char g = rgbData[ (3*pos) +1];
        unsigned char b = rgbData[ (3*pos) +2];

        HSV hsv (r, g, b);

        int h_offset = hsv.h / 45;  // 8 parts
        int s_offset = hsv.s / 64;  // 4 parts
        int v_offset = hsv.v / 128; // 4 parts
        ++hist_abs[16*h_offset + 4*s_offset + v_offset];
    }

    return encodeHistogram<histDomain> (
            hist_abs, numOfPixels, 128, compressData);
}

/*
Method ~getData[_]hsv256~:

*/
template<bool compressData> DistData*
PictureFuns::getData_hsv256 (const void* attr)
{
    unsigned long size;
    const char* imgdata = static_cast<const Picture*> (attr)->
                            GetJPEGData (size);

    JPEGPicture rgb ((unsigned char *) imgdata, size);

    unsigned long int rgbSize;
    unsigned char* rgbData = rgb.GetImageData (rgbSize);

    const unsigned int numOfPixels = rgbSize / 3;

    unsigned long hist_abs[256];
    memset (hist_abs, 0, 256*sizeof (unsigned long));

    for (unsigned long pos = 0; pos < (numOfPixels); ++pos)
    {
        unsigned char r = rgbData[ (3*pos) ];
        unsigned char g = rgbData[ (3*pos) +1];
        unsigned char b = rgbData[ (3*pos) +2];

        HSV hsv (r, g, b);

        int h_offset = (int) (hsv.h / 22.5); // 16 parts
        int s_offset = hsv.s / 64;        // 4 parts
        int v_offset = hsv.v / 128;       // 4 parts
        ++hist_abs[16*h_offset + 4*s_offset + v_offset];
    }

    return encodeHistogram<histDomain> (
            hist_abs, numOfPixels, 256, compressData);
}

/*
Method ~getData[_]lab256~:

*/
template<bool compressData> DistData*
PictureFuns::getData_lab256 (const void* attr)
{
    if (!pictureLabOffsetTable)
    {
        pictureLabOffsetTable = new unsigned[64*64*64];

        for (signed char r = 0; r < 64; r++)
        for (signed char g = 0; g < 64; g++)
            for (signed char b = 0; b < 64; b++)
            {
            Lab lab (2 + (r*4), 2 + (g*4), 2 + (b*4));

            // map values [0, 99] x [-86, 98] x [-107,94] to
            // [0, 3] x [0, 7] x [0, 7] (6 x 8 x 8 = 384 bins)
            int L_offset = (int) (lab.L / 25);
            int a_offset = (int) ((lab.a + 86) / 23.125);
            int b_offset = (int) ((lab.b + 107) / 25.1);
            pictureLabOffsetTable[r*4096 + g*64 + b] =
                64 * L_offset + 8 * a_offset + b_offset;
            }
    }

    unsigned long size;
    const char* imgdata = static_cast<const Picture*> (attr)->
                            GetJPEGData (size);

    JPEGPicture rgb ((unsigned char *) imgdata, size);

    unsigned long int rgbSize;
    unsigned char* rgbData = rgb.GetImageData (rgbSize);

    const unsigned int numOfPixels = rgbSize / 3;

    unsigned long hist_abs[256];
    memset (hist_abs, 0, 256*sizeof (unsigned long));

    for (unsigned long pos = 0; pos < numOfPixels; ++pos)
    {
        unsigned char r = static_cast<unsigned char>
                        (rgbData[ (3*pos) ] / 4);

        unsigned char g = static_cast<unsigned char>
                        (rgbData[ (3*pos) +1] / 4);

        unsigned char b = static_cast<unsigned char>
                        (rgbData[ (3*pos) +2] / 4);

        ++hist_abs[pictureLabOffsetTable[ (r*4096) + (g*64) + b] ];
    }

    return encodeHistogram<histDomain> (
            hist_abs, numOfPixels, 256, compressData);
}

/*
Method ~eucl[_]hsv128~:

*/
// distance functions
template<bool compressedData> void
PictureFuns::eucl_hsv128 (
const DistData* dd1, const DistData* dd2, DFUN_RESULT& result)
{
    histDomain hist1[128];
    histDomain hist2[128];
    decodeHistogram<histDomain> (hist1, dd1, 128, compressedData);
    decodeHistogram<histDomain> (hist2, dd2, 128, compressedData);
    computeMinkowskiDist<128, 2>(hist1, hist2, result);
}

/*
Method ~eucl[_]hsv256~:

*/
template<bool compressedData> void
PictureFuns::eucl_hsv256 (
const DistData* dd1, const DistData* dd2, DFUN_RESULT& result)
{
    histDomain hist1[256];
    histDomain hist2[256];
    decodeHistogram<histDomain> (hist1, dd1, 256, compressedData);
    decodeHistogram<histDomain> (hist2, dd2, 256, compressedData);
    computeMinkowskiDist<256, 2>(hist1, hist2, result);
}

/*
Method ~eucl[_]lab256~:

*/
template<bool compressedData> void
PictureFuns::eucl_lab256 (
const DistData* dd1, const DistData* dd2, DFUN_RESULT& result)
{
    histDomain hist1[256];
    histDomain hist2[256];
    decodeHistogram<histDomain> (hist1, dd1, 256, compressedData);
    decodeHistogram<histDomain> (hist2, dd2, 256, compressedData);
    computeMinkowskiDist<256, 2>(hist1, hist2, result);
}

/*
Method ~quadr[_]hsv128~:

*/
template<bool compressedData> void
PictureFuns::quadr_hsv128 (
const DistData* dd1, const DistData* dd2, DFUN_RESULT& result)
{
    if (!hsv128SimMatrix)
        computeHsv128SimMatrix();

    histDomain hist1[128];
    histDomain hist2[128];
    decodeHistogram<histDomain> (hist1, dd1, 128, compressedData);
    decodeHistogram<histDomain> (hist2, dd2, 128, compressedData);
    computeQuadraticDist<128>(hist1, hist2, hsv128SimMatrix, result);
}

/*
Method ~quadr[_]hsv256~:

*/
template<bool compressedData> void
PictureFuns::quadr_hsv256 (
const DistData* dd1, const DistData* dd2, DFUN_RESULT& result)
{
    if (!hsv256SimMatrix)
        computeHsv256SimMatrix();

    histDomain hist1[256];
    histDomain hist2[256];
    decodeHistogram<histDomain> (hist1, dd1, 256, compressedData);
    decodeHistogram<histDomain> (hist2, dd2, 256, compressedData);
    computeQuadraticDist<256>( hist1, hist2, hsv256SimMatrix, result);
}

/*
Method ~quadr[_]lab256~:

*/
template<bool compressedData> void
PictureFuns::quadr_lab256 (
const DistData* dd1, const DistData* dd2, DFUN_RESULT& result)
{
    if (!lab256SimMatrix)
        computeLab256SimMatrix();

    histDomain hist1[256];
    histDomain hist2[256];
    decodeHistogram<histDomain> (hist1, dd1, 256, compressedData);
    decodeHistogram<histDomain> (hist2, dd2, 256, compressedData);
    computeQuadraticDist<256>( hist1, hist2, lab256SimMatrix, result);
}

/*
Method ~getBBox~:

*/
template<unsigned dim, bool compressedData> BBox*
PictureFuns::getBBox (const DistData* data)
{
    histDomain hist[dim];
    decodeHistogram<histDomain> (hist, data, dim, true);
    histDomain* lb = new histDomain[dim];
    histDomain* ub = new histDomain[dim];
    memcpy (lb, hist, dim*sizeof (histDomain));
    memcpy (ub, hist, dim*sizeof (histDomain));

//     if (compressedData)
//         return new HistBBox<histDomain, dim>(lb, ub);
//     else
        return new GenericBBox<histDomain, dim>(lb, ub);
}

/*
Method ~CreateBBox~:

*/
template<unsigned dim, bool compressedData> BBox*
PictureFuns::createBBox ()
{
//     if (compressedData)
//         return new HistBBox<histDomain, dim>;
//     else
        return new GenericBBox<histDomain, dim>;
}

/*
Method ~computeMinkowskiDist~:

*/
template<unsigned dim, int n> void
PictureFuns::computeMinkowskiDist (
        histDomain* v1, histDomain* v2, DFUN_RESULT& result)
{
    result = 0;
    for (unsigned pos = 0; pos < dim; ++pos)
        result  += pow (abs (v1[pos] - v2[pos]), n);
}

/*
Method ~computeQuadraticDist~:

*/
template<unsigned dim> void
PictureFuns::computeQuadraticDist (
        histDomain* v1, histDomain* v2, histDomain* simMatrix,
        DFUN_RESULT& result)
{
    histDomain diff[dim];
    unsigned first = 0;
    unsigned last = dim - 1;

    for (unsigned pos = 0; pos < dim; ++pos)
    {
        diff[pos] = v1[pos] - v2[pos];
    }

    while (last > 0 && !diff[last])
        --last;

    while (first < last && !diff[first])
        ++first;

    result = 0;

    for (unsigned pos1 = first; pos1 < last; ++pos1)
        for (unsigned pos2 = first; pos2 < last; ++pos2)
        result += diff[pos1] * diff[pos2] *
                    simMatrix[ (pos1*dim) +pos2];

    result = sqrt (result);
}

/*
Method ~encodeHistogram~:

*/
template<class TFloat> DistData*
PictureFuns::encodeHistogram (
        unsigned long* hist_abs, unsigned numOfPixels,
        unsigned size, bool compressData, TFloat threshold)
{
    TFloat hist[size];
    list<unsigned char> indizes;
    indizes.push_back (0);
    bool isZeroValue = true;

    for (unsigned i = 0; i < size; ++i)
    {
        hist[i] = (TFloat) hist_abs[i] / numOfPixels;

        if (hist[i] < threshold)
            hist[i] = 0;

        if (isZeroValue != (hist[i] == 0))
        {
            isZeroValue = !isZeroValue;
            indizes.push_back (0);
        }

        if (indizes.back() < numeric_limits<unsigned char>::max())
        {
            ++indizes.back();
        }
        else
        {
            indizes.push_back (0);
            indizes.push_back (1);
        }
    }

    if (!compressData)
        return new DistData (size*sizeof (TFloat), &hist);

    // count sum of histogram entries
    unsigned sum = 0;  // sum of non zero entries
    unsigned sum2 = 0; // sum of all entries
    bool evenValue = false;

    for (list<unsigned char>::iterator it = indizes.begin();
            it != indizes.end(); ++it)
    {
        if (evenValue)
        sum += *it;
        sum2 += *it;
        evenValue = !evenValue;
    }

    assert (sum2 == size);

    char result[sizeof (int) + indizes.size() *sizeof (char) +
                sum*sizeof (TFloat) ];

    int indexCount = indizes.size();
    memcpy (result, &indexCount, sizeof (int));
    int offset = sizeof (int);

    for (list<unsigned char>::iterator it = indizes.begin();
            it != indizes.end(); ++it)
    {
        unsigned char index = *it;
        memcpy (result + offset, &index, sizeof (char));
        offset += sizeof (char);
    }

    // copy non zero histogram values to result buffer
    unsigned histPos = 0;

    list<unsigned char>::iterator it = indizes.begin();

    while (histPos < size)
    {
        histPos += *it;
        ++it;

        if (it != indizes.end())
        {
            unsigned j = histPos + *it;
            ++it;

            for (;histPos < j; ++histPos)
            {
                memcpy (result + offset,
                        &hist[histPos], sizeof (TFloat));
                offset += sizeof (TFloat);
            }
        }
    }
    return new DistData (offset, result);
}

/*
Method ~decodeHistogram~:

*/
template<class TFloat> void
PictureFuns::decodeHistogram (
TFloat* hist, const DistData* dd, unsigned size,
bool compressedData)
{
    const char* data = static_cast<const char*> (dd->value());

    if (!compressedData)
    {
        memcpy (hist, data, size*sizeof (TFloat));
        return;
    }

    memset (hist, 0, size*sizeof (TFloat));

    int cnt;
    memcpy (&cnt, data, sizeof (int));
    int offset = sizeof (int);

    unsigned char indizes[cnt];
    memcpy (indizes, data + offset, cnt*sizeof (char));
    offset += cnt * sizeof (char);

    unsigned histPos = 0;
    int i = -1;

    while (histPos < size)
    {
        histPos += indizes[++i];

        if (++i < cnt)
        {
            unsigned j = histPos + indizes[i];

            for (;histPos < j; ++histPos)
            {
                memcpy (&hist[histPos], data + offset, sizeof (TFloat));
                offset += sizeof (TFloat);
            }
        }
    }
}

} // namespace picture_distfuns
#endif // #ifndef __PICTURE_DISTFUNS_H
