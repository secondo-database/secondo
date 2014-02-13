/*
 1 Lua-interface to matchFaces-strategies.

 This file implements the Lua-interface for the matchFaces-strategies. Lua can
 be activated and deactivated in interpolate.h (comment out the define of the
 correct lua-version there. If both are commented, the lua-interface is not
 compiled).
 
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
static void setfield(const char *key, int value);
static void add_custom_functions();

// The set of faces is scaled to the boundary-box (0/0) - (SCALESIZE/SCALESIZE)
static int SCALESIZE = 1000000;


static void setfield(const char *key, double value) {
    lua_pushstring(L, key);
    lua_pushnumber(L, value);
    lua_settable(L, -3);
}

static void lua_pushPt(Pt pt) {
    lua_newtable(L);
    setfield("x", pt.x);
    setfield("y", pt.y);
}

static void lua_setPt(const char *name, Pt pt) {
    lua_pushPt(pt);
    lua_setglobal(L, name);
}

static Pt lua_getPt(const char *name) {
    double x = 0, y = 0;

    lua_getglobal(L, name);
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "x");
        lua_getfield(L, -2, "y");
        if (lua_isnumber(L, -1) && lua_isnumber(L, -2)) {
            x = lua_tonumber(L, -2);
            y = lua_tonumber(L, -1);
        }
        lua_pop(L, 2);
    }
    lua_pop(L, 1);

    return Pt(x, y);
}

static pair<Pt, Pt> getOffAndScale(bool isdst) {
    Pt off = isdst ? lua_getPt("dstoff") : lua_getPt("srcoff");
    Pt scale = isdst ? lua_getPt("dstscale") : lua_getPt("srcscale");

    return pair<Pt, Pt>(off, scale);
}

static Pt modPt(Pt pt, bool isdst) {
    pair<Pt, Pt> offandscale = getOffAndScale(isdst);
    Pt off = offandscale.first;
    Pt scale = offandscale.second;

    return (pt - off)*scale;
}

int luaInit(void) {
    if (L != NULL)
        lua_close(L);
    L = luaL_newstate();
    luaL_openlibs(L);
    int st = luaL_loadfile(L, "matchFaces.luac");
    if (st) {
        st = luaL_loadfile(L, "matchFaces.lua");
        if (st) {
            cerr << "Error parsing LUA-file: " << lua_tostring(L, -1) << "\n";
            return -1;
        }
    }

    st = lua_pcall(L, 0, 0, 0);
    if (st) {
        cerr << "Error running LUA-file: " << lua_tostring(L, -1) << "\n";
        return -1;
    }

    add_custom_functions();

    return 0;
}

void setupParams(set<Face*> *regs, const char *prefix, int depth) {
    char offstr[10];
    char scalestr[12];

    snprintf(offstr, sizeof (offstr), "%soff", prefix);
    snprintf(scalestr, sizeof (scalestr), "%sscale", prefix);

    if (!regs->empty()) {
        Face *r = *(regs->begin());
        Face *parent = r->parent;
        pair<Pt, Pt> bbox;
        if (parent) {
            cout << "Parent is " << bbox.first.ToString() << "\n";
            bbox = parent->GetBoundingBox();
        } else {
            cout << "Bbox is " << bbox.first.ToString() << "\n";
            bbox = Face::GetBoundingBox(*regs);
        }
        lua_setPt(offstr, bbox.first);
        if ((bbox.second.x > bbox.first.x) &&
                (bbox.second.y > bbox.first.y)) {
            Pt scale(SCALESIZE / (bbox.second.x - bbox.first.x),
                    SCALESIZE / (bbox.second.y - bbox.first.y));
            lua_setPt(scalestr, scale);
        } else {
            lua_setPt(scalestr, Pt(1, 1));
        }
    } else {
        lua_setPt(scalestr, Pt(1, 1));
        lua_setPt(offstr, Pt(0, 0));
    }

    lua_newtable(L);
    int i = 1;
    for (std::set<Face*>::iterator it = regs->begin(); it != regs->end(); ++it){
        lua_pushnumber(L, i++);
        lua_pushlightuserdata(L, const_cast<Face*> (*it));
        lua_rawset(L, -3);
    }
}

vector<pair<Face *, Face *> > __matchFacesLua(set<Face*> *src, set<Face*> *dst,
        int depth, string args) {
    vector<pair<Face *, Face *> > ret;

    if (!L || depth == 0) {
        int st = luaInit();
        if (st < 0)
            return ret;
    }


    lua_getglobal(L, "matchFaces");

    setupParams(src, "src", depth);
    setupParams(dst, "dst", depth);
    lua_pushinteger(L, depth);
    lua_pushstring(L, args.c_str());

    int st = lua_pcall(L, 4, 1, 0);
    if (st) {
        cerr << "Error calling matchFaces: " << lua_tostring(L, -1) << "\n";
        return ret;
    }

    if (lua_istable(L, -1)) {
        lua_pushnil(L);
        int i = 0;
        while (lua_next(L, -2) != 0) {
            pair<Face *, Face *> p(NULL, NULL);
            if (lua_istable(L, -1)) {
                lua_getfield(L, -1, "src");
                if (lua_islightuserdata(L, -1)) {
                    Face *r = (Face *) lua_touserdata(L, -1);
                    if (src->erase(r) != 0)
                        p.first = r;
                } else if (lua_istable(L, -1)) {
                    lua_pushnil(L);
                    if (lua_next(L, -2) && lua_islightuserdata(L, -1)) {
                        Face *r1 = (Face *) lua_touserdata(L, -1);
                        lua_pop(L, 1);
                        if (lua_next(L, -2) && lua_islightuserdata(L, -1)) {
                            Face *r2 = (Face *) lua_touserdata(L, -1);
                            Face *r3 = new Face();
                            //                            *r3 = r2->Merge(*r1);
                            p.first = r2;
                            lua_pop(L, 1);
                        }
                    }
                    lua_pop(L, 1);
                }
                lua_pop(L, 1);

                lua_getfield(L, -1, "dst");
                if (lua_islightuserdata(L, -1)) {
                    Face *r = (Face *) lua_touserdata(L, -1);
                    if (dst->erase(r) != 0)
                        p.second = r;
                }
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
            if (p.first || p.second) {
                ret.push_back(p);
            }
        }
    } else {
        cerr << "Return-Value is not a table!\n";
    }

    return ret;
}


/*
  3 C-Functions for Lua-scripts
 
 Custom functions to be called from Lua-scripts should be defined here.
 A prototype 
 
*/



static int lua_distance(lua_State *L) {
    if (!lua_islightuserdata(L, -1) || !lua_islightuserdata(L, -2))
        return 0;

    Face *src = (Face*) lua_touserdata(L, -1);
    Face *dst = (Face*) lua_touserdata(L, -2);
    double dist = modPt(src->GetMiddle(), src->isdst)
            .distance(modPt(dst->GetMiddle(), dst->isdst));
    lua_pushnumber(L, dist);
    return 1;
}

static int lua_middle(lua_State *L) {
    if ((lua_gettop(L) != 1) || !lua_islightuserdata(L, -1))
        return 0;

    Face *r = (Face*) lua_touserdata(L, -1);
    Pt pt = modPt(r->GetMiddle(), r->isdst);

    lua_newtable(L);
    setfield("x", pt.x);
    setfield("y", pt.y);

    return 1;
}

static int lua_boundingbox(lua_State *L) {
    if ((lua_gettop(L) != 1) || !lua_islightuserdata(L, -1)) {
        return 0;
    }
    Face *reg = (Face*) lua_touserdata(L, -1);
    lua_pushPt(modPt(reg->bbox.first, reg->isdst));
    lua_pushPt(modPt(reg->bbox.second, reg->isdst));

    return 2;
}

static int lua_bboverlap(lua_State *L) {
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

    if ((sbb.first.x > dbb.second.x) || (dbb.first.x > sbb.second.x) ||
            (sbb.first.y > dbb.second.y) || (dbb.first.y > sbb.second.y)) {
        //        cerr
        //                << sbb.first.ToString() << " "
        //                << sbb.second.ToString() << " "
        //                << dbb.first.ToString() << " "
        //                << dbb.second.ToString() << "\n";
        lua_pushnumber(L, 0);
        return 1;
    }

    box.first.x = (sbb.first.x < dbb.first.x) ? dbb.first.x : sbb.first.x;
    box.first.y = (sbb.first.y < dbb.first.y) ? dbb.first.y : sbb.first.y;

    box.second.x = (sbb.second.x > dbb.second.x) ? dbb.second.x : sbb.second.x;
    box.second.y = (sbb.second.y > dbb.second.y) ? dbb.second.y : sbb.second.y;

    if ((box.first.x >= box.second.x) || (box.first.y >= box.second.y)) {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, (box.second.x - box.first.x)*
            (box.second.y - box.first.y));

    lua_pushnumber(L, (sbb.second.x - sbb.first.x)*
            (sbb.second.y - sbb.first.y));

    lua_pushnumber(L, (dbb.second.x - dbb.first.x)*
            (dbb.second.y - dbb.first.y));

    return 3;
}

static int lua_overlap(lua_State *L) {
    if ((lua_gettop(L) != 2) ||
            !lua_islightuserdata(L, 1) || !lua_islightuserdata(L, 2))
        return 0;

    Face *src = (Face*) lua_touserdata(L, 1);
    Face *dst = (Face*) lua_touserdata(L, 2);

    pair<Pt, Pt> oassrc = getOffAndScale(src->isdst);
    pair<Pt, Pt> oasdst = getOffAndScale(dst->isdst);

    Region r1 = src->MakeRegion(oassrc.first.x, oassrc.first.y,
            oassrc.second.x, oassrc.second.y, false);
    Region r2 = dst->MakeRegion(oasdst.first.x, oasdst.first.y,
            oasdst.second.x, oasdst.second.y, false);

    Region intersect(0);

    r1.Intersection(r2, intersect, NULL);
    double r1area = r1.Area(NULL);
    double r2area = r2.Area(NULL);
    double intersectarea = intersect.Area(NULL);

    lua_pushnumber(L, intersectarea);
    lua_pushnumber(L, r1area);
    lua_pushnumber(L, r2area);

    return 3;
}

static int lua_points(lua_State *L) {
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

static int lua_centroid(lua_State *L) {
    if ((lua_gettop(L) != 1) || !lua_islightuserdata(L, 1))
        return 0;
    Face *f = (Face*) lua_touserdata(L, 1);

    lua_pushPt(modPt(f->GetCentroid(), f->isdst));

    return 1;
}

static int lua_area(lua_State *L) {
    if ((lua_gettop(L) != 1) || !lua_islightuserdata(L, 1))
        return 0;
    Face *f = (Face*) lua_touserdata(L, 1);

    lua_pushnumber(L, f->GetArea());

    return 1;
}

vector<pair<Face *, Face *> > _matchFacesLua(vector<Face> *src,
        vector<Face> *dst, int depth, string args) {
    set<Face*> srcset, dstset;
    for (unsigned int i = 0; i < src->size(); i++)
        srcset.insert(&((*src)[i]));
    for (unsigned int i = 0; i < dst->size(); i++)
        dstset.insert(&((*dst)[i]));

    return __matchFacesLua(&srcset, &dstset, depth, args);
}

static void add_custom_functions () {
    LUA_ADD_FUNCTION(distance);
    LUA_ADD_FUNCTION(middle);
    LUA_ADD_FUNCTION(boundingbox);
    LUA_ADD_FUNCTION(bboverlap);
    LUA_ADD_FUNCTION(centroid);
    LUA_ADD_FUNCTION(points);
    LUA_ADD_FUNCTION(area);
}

#endif
