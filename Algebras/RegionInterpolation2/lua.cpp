/*
*/

#include "interpolate.h"

#include <vector>
#include <set>
#include <lua5.2/lua.hpp>

static int SCALESIZE = 1000000000;

static lua_State *L = NULL;
static void setfield(const char *key, int value);

static int lua_distance(lua_State *L);
static int lua_getbb(lua_State *L);
static int lua_getbboverlap(lua_State *L);
static int lua_getmiddle(lua_State *L);

static void setfield(const char *key, int value) {
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

static Pt modPt(Pt pt, bool isdst) {
    Pt off = isdst ? lua_getPt("dstoff") : lua_getPt("srcoff");
    Pt scale = isdst ? lua_getPt("dstscale") : lua_getPt("srcscale");

    return (pt - off)*scale;
}

int luaInit(void) {
    if (L != NULL)
        lua_close(L);
    L = luaL_newstate();
    luaL_openlibs(L);
    int st = luaL_loadfile(L, "mf.lua");
    if (st) {
        cerr << "Error parsing LUA-file: " << lua_tostring(L, -1) << "\n";
        return -1;
    }

    st = lua_pcall(L, 0, 0, 0);
    if (st) {
        cerr << "Error running LUA-file: " << lua_tostring(L, -1) << "\n";
        return -1;
    }

    lua_pushcfunction(L, lua_distance);
    lua_setglobal(L, "distance");
    lua_pushcfunction(L, lua_getmiddle);
    lua_setglobal(L, "getmiddle");
    lua_pushcfunction(L, lua_getbb);
    lua_setglobal(L, "bb");
    lua_pushcfunction(L, lua_getbboverlap);
    lua_setglobal(L, "bboverlap");

    return 0;
}

void setupParams(set<Reg*> *regs, const char *prefix, int depth) {
    char offstr[10];
    char scalestr[12];

    snprintf(offstr, sizeof (offstr), "%soff", prefix);
    snprintf(scalestr, sizeof (scalestr), "%sscale", prefix);

    if (!regs->empty()) {
        Reg *r = *(regs->begin());
        Reg *parent = r->parent;
        pair<Pt, Pt> bbox;
        if (parent)
            bbox = parent->GetBoundingBox();
        else
            bbox = Reg::GetBoundingBox(*regs);
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
    for (std::set<Reg*>::iterator it = regs->begin(); it != regs->end(); ++it) {
        lua_pushnumber(L, i++);
        lua_pushlightuserdata(L, const_cast<Reg*> (*it));
        lua_rawset(L, -3);
    }
}

vector<pair<Reg *, Reg *> > __matchFacesLua(set<Reg*> *src, set<Reg*> *dst,
        int depth) {
    vector<pair<Reg *, Reg *> > ret;

    if (!L || depth == 0) {
        int st = luaInit();
        if (st < 0)
            return ret;
    }

    lua_getglobal(L, "matchFaces");

    setupParams(src, "src", depth);
    setupParams(dst, "dst", depth);
    lua_pushinteger(L, depth);

    int st = lua_pcall(L, 3, 1, 0);
    if (st) {
        cerr << "Error calling matchFaces: " << lua_tostring(L, -1) << "\n";
        return ret;
    }

    if (lua_istable(L, -1)) {
        lua_pushnil(L);
        int i = 0;
        while (lua_next(L, -2) != 0) {
            pair<Reg *, Reg *> p(NULL, NULL);
            if (lua_istable(L, -1)) {
                lua_getfield(L, -1, "src");
                if (lua_islightuserdata(L, -1)) {
                    Reg *r = (Reg *) lua_touserdata(L, -1);
                    if (src->erase(r) != 0)
                        p.first = r;
                } else if (lua_istable(L, -1)) {
                    lua_pushnil(L);
                    if (lua_next(L, -2) && lua_islightuserdata(L, -1)) {
                        Reg *r1 = (Reg *) lua_touserdata(L, -1);
                        lua_pop(L, 1);
                        if (lua_next(L, -2) && lua_islightuserdata(L, -1)) {
                            Reg *r2 = (Reg *) lua_touserdata(L, -1);
                            Reg *r3 = new Reg();
                            *r3 = r2->Merge(*r1);
                            p.first = r3;
                            lua_pop(L, 1);
                        }
                    }
                    lua_pop(L, 1);
                }
                lua_pop(L, 1);

                lua_getfield(L, -1, "dst");
                if (lua_islightuserdata(L, -1)) {
                    Reg *r = (Reg *) lua_touserdata(L, -1);
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

//    for (std::set<Reg*>::iterator it = src->begin(); it != src->end(); ++it) {
//        pair<Reg *, Reg *> p(*it, NULL);
//        ret.push_back(p);
//    }
//
//    for (std::set<Reg*>::iterator it = dst->begin(); it != dst->end(); ++it) {
//        pair<Reg *, Reg *> p(NULL, *it);
//        ret.push_back(p);
//    }


    return ret;
}

static int lua_distance(lua_State *L) {
    if (!lua_islightuserdata(L, -1) || !lua_islightuserdata(L, -2))
        return 0;

    Reg *src = (Reg*) lua_touserdata(L, -1);
    Reg *dst = (Reg*) lua_touserdata(L, -2);
    double dist = modPt(src->GetMiddle(), src->isdst)
            .distance(modPt(dst->GetMiddle(), dst->isdst));
    lua_pushnumber(L, dist);
    return 1;
}

static int lua_getmiddle(lua_State *L) {
    if ((lua_gettop(L) != 1) || !lua_islightuserdata(L, -1))
        return 0;

    Reg *r = (Reg*) lua_touserdata(L, -1);
    Pt pt = modPt(r->GetMiddle(), r->isdst);

    lua_newtable(L);
    setfield("x", pt.x);
    setfield("y", pt.y);

    return 1;
}

static int lua_getbb(lua_State *L) {
    if ((lua_gettop(L) != 1) || !lua_islightuserdata(L, -1)) {
        return 0;
    }
    Reg *reg = (Reg*) lua_touserdata(L, -1);
    lua_pushPt(modPt(reg->bbox.first, reg->isdst));
    lua_pushPt(modPt(reg->bbox.second, reg->isdst));

    return 2;
}

static int lua_getbboverlap(lua_State *L) {
    if ((lua_gettop(L) != 2) ||
            !lua_islightuserdata(L, -1) || !lua_islightuserdata(L, -2))
        return 0;

    Reg *src = (Reg*) lua_touserdata(L, -1);
    Reg *dst = (Reg*) lua_touserdata(L, -2);

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

    return 1;
}

vector<pair<Reg *, Reg *> > _matchFacesLua(vector<Reg> *src, vector<Reg> *dst,
        int depth) {
    set<Reg*> srcset, dstset;
    for (unsigned int i = 0; i < src->size(); i++)
        srcset.insert(&((*src)[i]));
    for (unsigned int i = 0; i < dst->size(); i++)
        dstset.insert(&((*dst)[i]));

    return __matchFacesLua(&srcset, &dstset, depth);
}