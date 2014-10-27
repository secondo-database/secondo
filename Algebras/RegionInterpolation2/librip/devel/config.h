/*
 1 config.h
 
 This file defines some values used to configure the
 RegionInterpolation library

*/

#ifndef _INTERPOLATE_CONFIG_H
#define _INTERPOLATE_CONFIG_H

// For matching faces, each set of faces will be translated and scaled to the
// boundary-box (0/0) - (SCALESIZE/SCALESIZE). Used by matchFacesCriterion and
// matchFacesLua
#define SCALESIZE 1000000

// This is the duration of one moment in ms, used for the borderregions.
#define MOMENTMS 100

// Choose the installed Lua-version here
#define LUA5_1 // Lua5.1
//#define LUA5_2 // Lua5.2

// The main Lua Scriptname (without suffix .lua or .luac)
// This can be a relative or an absolute pathname
#define LUASCRIPTNAME "matchFaces"

// Values for testing and developing
#define DEBUGLEVEL 4 // The current debug level (valid: 1-4, 0 means off)

// scale-factor for region-import and (m)region-export
#define SCALEIN  1   // Imported values are multiplied by SCALEIN
#define SCALEOUT 1   // Exported values are divided    by SCALEOUT

// function name of trapezium-intersects-check.
// Must have the prototype:
// bool TRAPEZIUMINTERSECTS(MSeg m, MSeg a, unsigned int &detailedResult);
// currently valid values:
// trapeziumIntersects : function defined in ointersect.cpp
#define TRAPEZIUMINTERSECTS trapeziumIntersects
#define STRICT 0

#endif /* _INTERPOLATE_CONFIG_H */
