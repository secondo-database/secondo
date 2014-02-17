/*
 1 Lua-interface to matchFaces-strategies.

 This file implements the Lua-interface for the matchFaces-strategies. Lua can
 be activated and deactivated in interpolate.h
 Comment out the define of the correct lua-version there. If both are
 commented, then the lua-interface is not compiled.
 
 Please note, that the correct lua-library has to be specified as dependency
 in the makefile.algebras, for example:
 
 ALGEBRA\_DEPS += lua5.1
 or
 ALGEBRA\_DEPS += lua5.2
 
*/

#include "interpolate.h"

#ifdef USE_LUA

#include <vector>
#include <set>
#if defined(LUA5_2)
#include <lua5.2/lua.hpp>
#elif defined(LUA5_1)
#include <lua5.1/lua.hpp>
#else
#error "Unknown Lua-Version"
#endif

static lua_State *L = NULL;
static void add_custom_functions();

/*
 2 Helper-functions which make the code below more clear.
 
*/

// Function to set a field in the table on top of the stack
static void setfield(const char *key, double value) {
    lua_pushstring(L, key);
    lua_pushnumber(L, value);
    lua_settable(L, -3);
}

// Function to push a point on the stack (a table with fields x and y)
static void lua_pushPt(Pt pt) {
    lua_newtable(L);
    setfield("x", pt.x);
    setfield("y", pt.y);
}

// Function to set a global variable ~name~ containing the point ~pt~
static void lua_setPt(const char *name, Pt pt) {
    lua_pushPt(pt);
    lua_setglobal(L, name);
}

// Function to retrieve a point from the Lua-environment, which is
// stored in the global variable ~name~
static Pt lua_getPt(const char *name) {
    Pt ret(0, 0); // Default-value if the variable is invalid

    lua_getglobal(L, name);
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "x");
        lua_getfield(L, -2, "y");
        if (lua_isnumber(L, -1) && lua_isnumber(L, -2)) {
            ret.x = lua_tonumber(L, -2);
            ret.y = lua_tonumber(L, -1);
            ret.valid = 1;
        }
        lua_pop(L, 2);
    }
    lua_pop(L, 1);

    return ret;
}

// Retrieves the offset and scale-factors for the source- or destination-faces
// (depends on ~isdst~)
static pair<Pt, Pt> getOffAndScale(bool isdst) {
    Pt off = isdst ? lua_getPt("dstoff") : lua_getPt("srcoff");
    Pt scale = isdst ? lua_getPt("dstscale") : lua_getPt("srcscale");

    return pair<Pt, Pt>(off, scale);
}

// Transforms a point with the offset and scale-factors of the source- or
// destination-faces (depends on ~isdst~)
static Pt modPt(Pt pt, bool isdst) {
    pair<Pt, Pt> offandscale = getOffAndScale(isdst);
    Pt off = offandscale.first;
    Pt scale = offandscale.second;

    return (pt - off)*scale;
}

/*
 2.1 luaInit prepares the Lua-environment:
 
 * Create a fresh Lua-context
 * Add the custom functions defined in section 3
 * Load the script matchFaces.luac (bytecode) or matchFaces.lua (source)
 * Run the script (this publishes the functions in the script etc.)

*/
int luaInit(void) {
    // Close an existing old state if any
    if (L != NULL)
        lua_close(L);
    L = luaL_newstate(); // and create a fresh one
    luaL_openlibs(L);    // load standard libraries
    
    add_custom_functions(); // and add our own functions

    // First try to load the compiled version of the matchFaces-script
    int st = luaL_loadfile(L, "matchFaces.luac");
    if (st) {
        // That failed, so we try to open the source-file
        st = luaL_loadfile(L, "matchFaces.lua");
        if (st) {
            // Either it wasn't present or a parse-error occurred
            cerr << "Error parsing LUA-file: " << lua_tostring(L, -1) << "\n";
            return -1;
        }
    }

    // Run the script to publish the functions into the global table etc.
    st = lua_pcall(L, 0, 0, 0);
    if (st) {
        // Runtime error
        cerr << "Error running LUA-file: " << lua_tostring(L, -1) << "\n";
        return -1;
    }

    return 0; // Everything went ok
}

/*
 2.2 setupParams sets up the params of the Lua-function matchFaces, which is
 to be called afterwards. ~faces~ is the set of faces, ~prefix~ should be the
 string "src" or "dst", depending if the source-set or destination-set of faces
 is to be prepared. ~depth~ is the recursion depth in the interpolation
 algorithm, which is also passed to the function.
 
*/
void setupParams(set<Face*> *faces, const char *prefix, int depth) {
    char offstr[10];
    char scalestr[12];

    // Prepare the variable names for the offset (srcoff/dstoff) and scale-
    // factors (srcscale/dstscale)
    snprintf(offstr, sizeof (offstr), "%soff", prefix);
    snprintf(scalestr, sizeof (scalestr), "%sscale", prefix);

    if (!faces->empty()) {
        // Calculate the bounding boxes
        Face *r = *(faces->begin());
        Face *parent = r->parent;
        pair<Pt, Pt> bbox;
        if (parent) {
            // If we have a parent face, take the bounding box of it
            bbox = parent->GetBoundingBox();
        } else {
            // otherwise get the common bounding box of all faces in this set
            bbox = Face::GetBoundingBox(*faces);
        }
        
        // The offset is the upper left corner of the bounding box
        lua_setPt(offstr, bbox.first);
        
        if ((bbox.second.x > bbox.first.x) &&
                (bbox.second.y > bbox.first.y)) {
            // Calculate the factor which scales the bounding box to SCALESIZE
            // in each direction.
            Pt scale(SCALESIZE / (bbox.second.x - bbox.first.x),
                    SCALESIZE / (bbox.second.y - bbox.first.y));
            lua_setPt(scalestr, scale);
        } else {
            // The factor would be invalid (FPE), so just use 1
            lua_setPt(scalestr, Pt(1, 1));
        }
    } else {
        // If the set is empty, use offset 0 and scalefactor 1
        lua_setPt(offstr, Pt(0, 0));
        lua_setPt(scalestr, Pt(1, 1));
    }

    // Now create a new table and push the pointer to the faces. Note, that
    // lua-tables by convention start with index 1 instead of 0.
    lua_newtable(L);
    int i = 1;
    for (std::set<Face*>::iterator it=faces->begin(); it != faces->end();++it) {
        lua_pushnumber(L, i++);
        lua_pushlightuserdata(L, const_cast<Face*> (*it));
        lua_rawset(L, -3);
    }
}

/*
 2.3 matchFacesLua is the primary interface between the interpolation and the
 matching strategies implemented in lua.
 It initializes the Lua-context, converts the parameters to Lua-values and
 converts the return value back again.
 
*/
vector<pair<Face *, Face *> > _matchFacesLua(set<Face*> *src, set<Face*> *dst,
        int depth, string args) {
    vector<pair<Face *, Face *> > ret;

    // Create a fresh Lua-context if none exists or if we are in recursion
    // depth 0 (which means, that a new interpolation started)
    if (!L || depth == 0) {
        int st = luaInit();
        if (st < 0)
            return ret;
    }

    // Lookup the matchFaces-Lua-function in the global table
    lua_getglobal(L, "matchFaces");

    // Convert the sets to tables and push it on the Lua-stack
    setupParams(src, "src", depth);
    setupParams(dst, "dst", depth);
    lua_pushinteger(L, depth); // Push the depth as third argument
    lua_pushstring(L, args.c_str()); // The optional parameterstring goes last

    int st = lua_pcall(L, 4, 1, 0); // The actual Lua-functioncall happens here
    if (st) {
        cerr << "Error calling matchFaces: " << lua_tostring(L, -1) << "\n";
        return ret;
    }

    // Now convert the return value
    if (lua_istable(L, -1)) {
        lua_pushnil(L);
        // Traverse the table
        while (lua_next(L, -2) != 0) {
            pair<Face *, Face *> p(NULL, NULL);
            // Each entry of the table should be another table with the fields
            // "src" and "dst", which should contain one face each.
            if (lua_istable(L, -1)) {
                lua_getfield(L, -1, "src");
                if (lua_islightuserdata(L, -1)) {
                    p.first = (Face *) lua_touserdata(L, -1);
                }
                lua_pop(L, 1);

                lua_getfield(L, -1, "dst");
                if (lua_islightuserdata(L, -1)) {
                    p.second = (Face *) lua_touserdata(L, -1);
                }
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
            // Push back the new pair. If there are double values, empty
            // pairings or missing faces, this is fixed by the caller, so
            // there's no need to check that here.
            ret.push_back(p);
        }
    }

    return ret;
}

// Helper function which converts the lists in sets
vector<pair<Face *, Face *> > _matchFacesLua(vector<Face> *src,
        vector<Face> *dst, int depth, string args) {
    set<Face*> srcset, dstset;
    for (unsigned int i = 0; i < src->size(); i++)
        srcset.insert(&((*src)[i]));
    for (unsigned int i = 0; i < dst->size(); i++)
        dstset.insert(&((*dst)[i]));

    return _matchFacesLua(&srcset, &dstset, depth, args);
}

/*
  3 C-Functions for Lua-scripts
 
 Custom functions to be called from Lua-scripts can be defined here.
 
 First, define the function with
 
 LUA\_FUNCTION(name) {
 }
 
 Inside the function body, L is defined as the current Lua-State.
 
 Then, add the function to the global environment by inserting a call to
 LUA\_ADD\_FUNCTION(name)
 into the function add\_custom\_functions() below.
 
 The following conventions should be followed:
 
 * L is the current Lua-state
 * Always check the number of parameters (lua\_gettop(L) returns the number of
   actual parameters
 * Always check the type of the parameters with
   lua\_is<type>(L, n) for the nth arg. The most important types are:
   lightuserdata (for regions), number (for double-values), string and table.
 * lightuserdata is always a region-pointer, other usertypes are not used
 * Before calculations are performed, always scale and translate coordinates
   with modPt(pt, isdst). pt is the point to be transformed and isdst tells
   modPt if a source- or destination-region-transformation should be performed.

*/

/*
  3.1 distance returns the distance of the center points of the bounding-boxes
  of the two regions given.
 
*/
LUA_FUNCTION(distance) {
    // First, check the arguments. This function
    if ((lua_gettop(L) != 2) || // expects two arguments of type lightuserdata
        !lua_islightuserdata(L, 1) || !lua_islightuserdata(L, 2))
        return 0; // Otherwise, no return value (equivalent to nil in lua)

    Face *src = (Face*) lua_touserdata(L, 1); // First argument
    Face *dst = (Face*) lua_touserdata(L, 2); // Second argument
    Pt smiddle = modPt(src->GetMiddle(), src->isdst); // Modify points
    Pt dmiddle = modPt(dst->GetMiddle(), dst->isdst); // (scale and translate)
    double dist = smiddle.distance(dmiddle); // calculate the distance
    
    lua_pushnumber(L, dist); // Push the return value on the stack
    return 1; // Return the number of return-values
}

/*
  3.2 middle returns the center of the bounding-box of the given region.
 
*/
LUA_FUNCTION(middle) {
    if ((lua_gettop(L) != 1) || !lua_islightuserdata(L, 1))
        return 0;

    Face *r = (Face*) lua_touserdata(L, 1);
    Pt pt = modPt(r->GetMiddle(), r->isdst);

    lua_newtable(L);
    setfield("x", pt.x);
    setfield("y", pt.y);

    return 1;
}

/*
 3.3 boundingbox returns the upper-left and lower-right corner of the
 bounding-box of the given region.
 
 This function returns two values, a Lua call-example:
 
 ul, lr = boundingbox(face)
  
*/
LUA_FUNCTION(boundingbox) {
    if ((lua_gettop(L) != 1) || !lua_islightuserdata(L, 1)) {
        return 0;
    }
    Face *reg = (Face*) lua_touserdata(L, 1);
    lua_pushPt(modPt(reg->bbox.first, reg->isdst));
    lua_pushPt(modPt(reg->bbox.second, reg->isdst));

    return 2;
}

/*
 3.4 bboverlap returns the overlapping area of the bounding-boxes of two
 faces. To be able to calculate the relative overlap easily, it also returns
 the area of the bounding-boxes of the two faces as second and third argument.
 
*/
LUA_FUNCTION(bboverlap) {
    if ((lua_gettop(L) != 2) ||
            !lua_islightuserdata(L, 1) || !lua_islightuserdata(L, 2))
        return 0;

    Face *src = (Face*) lua_touserdata(L, 1);
    Face *dst = (Face*) lua_touserdata(L, 2);

    pair<Pt, Pt> sbb = src->GetBoundingBox();
    pair<Pt, Pt> dbb = dst->GetBoundingBox();

    sbb.first = modPt(sbb.first, src->isdst);
    sbb.second = modPt(sbb.second, src->isdst);
    dbb.first = modPt(dbb.first, dst->isdst);
    dbb.second = modPt(dbb.second, dst->isdst);

    pair<Pt, Pt> box;
    
    box.first.x = (sbb.first.x < dbb.first.x) ? dbb.first.x : sbb.first.x;
    box.first.y = (sbb.first.y < dbb.first.y) ? dbb.first.y : sbb.first.y;

    box.second.x = (sbb.second.x > dbb.second.x) ? dbb.second.x : sbb.second.x;
    box.second.y = (sbb.second.y > dbb.second.y) ? dbb.second.y : sbb.second.y;

    double intersectionarea;
    if ((sbb.first.x > dbb.second.x) || (dbb.first.x > sbb.second.x) ||
            (sbb.first.y > dbb.second.y) || (dbb.first.y > sbb.second.y)) {
        // The boundingboxes are disjunct
        intersectionarea = 0;
    } else {
        // The boundingboxes overlap
        intersectionarea = (box.second.x - box.first.x)*
                (box.second.y - box.first.y);
    }

    lua_pushnumber(L, intersectionarea);

    lua_pushnumber(L, (sbb.second.x - sbb.first.x)*
            (sbb.second.y - sbb.first.y));

    lua_pushnumber(L, (dbb.second.x - dbb.first.x)*
            (dbb.second.y - dbb.first.y));

    return 3;
}

/*
 3.5 Overlap returns the overlapping area of the two faces given (ignoring
 holes) and additionally reports the area of the two faces as second and third
 return value.
 
*/
LUA_FUNCTION(overlap) {
    if ((lua_gettop(L) != 2) ||
            !lua_islightuserdata(L, 1) || !lua_islightuserdata(L, 2))
        return 0;

    Face *src = (Face*) lua_touserdata(L, 1);
    Face *dst = (Face*) lua_touserdata(L, 2);

    // First point in the pair is the offset, the second second is the scale
    pair<Pt, Pt> srctransform = getOffAndScale(src->isdst);
    pair<Pt, Pt> dsttransform = getOffAndScale(dst->isdst);

    Region r1 = src->MakeRegion(srctransform.first.x, srctransform.first.y,
            srctransform.second.x, srctransform.second.y, false);
    Region r2 = dst->MakeRegion(dsttransform.first.x, dsttransform.first.y,
            dsttransform.second.x, dsttransform.second.y, false);


    // Calculate the intersection of the regions in "intersect"
    Region intersect(0);
    r1.Intersection(r2, intersect, NULL);
    double intersectarea = intersect.Area(NULL); // Calculate the area
    
    // Calculate the areas of the two faces
    double r1area = r1.Area(NULL);
    double r2area = r2.Area(NULL);

    lua_pushnumber(L, intersectarea);
    lua_pushnumber(L, r1area);
    lua_pushnumber(L, r2area);

    return 3;
}

/*
 3.6 Points returns the list of cornerpoints of this face in a table. Holes are
 not included.
 
*/
LUA_FUNCTION(points) {
    if ((lua_gettop(L) != 1) || !lua_islightuserdata(L, 1))
        return 0;
    Face *f = (Face*) lua_touserdata(L, 1);

    lua_newtable(L);
    for (unsigned int i = 0; i < f->v.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_pushPt(modPt(f->v[i].s, f->isdst));
        lua_rawset(L, -3);
    }

    return 1;
}

/*
 3.7 Centroid returns the centroid of the given face (holes are not taken in
 account).
 
*/
LUA_FUNCTION(centroid) {
    if ((lua_gettop(L) != 1) || !lua_islightuserdata(L, 1))
        return 0;
    Face *f = (Face*) lua_touserdata(L, 1);

    lua_pushPt(modPt(f->GetCentroid(), f->isdst));

    return 1;
}

/*
 3.8 Area returns the area of the given face.
 
*/
LUA_FUNCTION(area) {
    if ((lua_gettop(L) != 1) || !lua_islightuserdata(L, 1))
        return 0;
    Face *f = (Face*) lua_touserdata(L, 1);

    pair<Pt, Pt> transform = getOffAndScale(f->isdst);
    // Get the area and correct it by the scaling factors
    double area = f->GetArea() * (transform.second.x*transform.second.y);
    lua_pushnumber(L, area);

    return 1;
}

/*
 3.9 add\_custom\_functions inserts the functions defined above into the
 global list of functions in the Lua-environment.
 If you added a function above, then you also have to add a corresponding line
 in this function.
 
*/
static void add_custom_functions () {
    LUA_ADD_FUNCTION(distance);
    LUA_ADD_FUNCTION(middle);
    LUA_ADD_FUNCTION(boundingbox);
    LUA_ADD_FUNCTION(bboverlap);
    LUA_ADD_FUNCTION(overlap);
    LUA_ADD_FUNCTION(centroid);
    LUA_ADD_FUNCTION(points);
    LUA_ADD_FUNCTION(area);
}

#endif
