/*
*/

#include "interpolate.h"

#include <vector>
#include <lua5.2/lua.hpp>

static lua_State *L = NULL;
static void setfield(const char *key, int value);

static int lua_distance(lua_State *L);
static int lua_getbb(lua_State *L);
static int lua_getbboverlap(lua_State *L);

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

static Pt modPt (Pt pt, bool isdst) {
    Pt off = isdst ? lua_getPt("dstoff") : lua_getPt("srcoff");
    Pt scale = isdst ? lua_getPt("dstscale") : lua_getPt("srcscale");    
    
    return (pt+off)*scale;
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
    lua_pushcfunction(L, lua_getbb);
    lua_setglobal(L, "bb");
    lua_pushcfunction(L, lua_getbboverlap);
    lua_setglobal(L, "bboverlap");

    return 0;
}

void setupParams (vector<Reg> *regs, const char *prefix, int depth) {
    char offstr[10];
    char scalestr[12];
    
    snprintf(offstr, sizeof(offstr), "%soff", prefix);
    snprintf(scalestr, sizeof(scalestr), "%sscale", prefix);
    
    if (regs->size() > 0) {
        Reg *parent = (*regs)[0].parent;
        pair<Pt,Pt> bbox;
        if (parent)
            bbox = parent->GetBoundingBox();
        else
            bbox = Reg::GetBoundingBox(*regs);
        lua_setPt(offstr, bbox.first);
        if ((bbox.second.x > bbox.first.x) &&
                (bbox.second.y > bbox.first.y) && depth > 0) {
            Pt scale(1000 / (bbox.second.x - bbox.first.x),
                    1000 / (bbox.second.y - bbox.first.y));
            lua_setPt(scalestr, scale);
        } else {
            lua_setPt(scalestr, Pt(1,1));
        }
    } else {
        lua_setPt(scalestr, Pt(1,1));
        lua_setPt(offstr, Pt(0,0));
    }
    
    lua_newtable(L);
    for (unsigned int i = 0; i < regs->size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_pushlightuserdata(L, &(*regs)[i]);
        lua_rawset(L, -3);
    }
}

vector<pair<Reg *, Reg *> > _matchFacesLua(vector<Reg> *src, vector<Reg> *dst,
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

    cerr << "Calling LUA START\n";
    int st = lua_pcall(L, 3, 1, 0);
    if (st) {
        cerr << "Error calling matchFaces: " << lua_tostring(L, -1) << "\n";
        return ret;
    }
    cerr << "Calling LUA END\n";

    if (lua_istable(L, -1)) {
        lua_pushnil(L);
        int i = 0;
        while (lua_next(L, -2) != 0) {
            cerr << "Ret value " << i++ << "!\n";
            pair<Reg *, Reg *> p;
            if (lua_istable(L, -1)) {
                lua_getfield(L, -1, "src");
                if (lua_islightuserdata(L, -1)) {
                    p.first = (Reg *) lua_touserdata(L, -1);
                } else {
                    p.first = NULL;
                }
                lua_pop(L, 1);

                lua_getfield(L, -1, "dst");
                if (lua_islightuserdata(L, -1)) {
                    p.second = (Reg *) lua_touserdata(L, -1);
                } else {
                    p.second = NULL;
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


static int lua_distance(lua_State *L) {
    if (!lua_islightuserdata(L, -1) || !lua_islightuserdata(L, -2))
        return 0;

    Reg *src = (Reg*) lua_touserdata(L, -1);
    Reg *dst = (Reg*) lua_touserdata(L, -2);
    double dist = modPt(src->GetMiddle(),src->isdst)
                  .distance(modPt(dst->GetMiddle(),dst->isdst));
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
    lua_pushPt(reg->bbox.first);
    lua_pushPt(reg->bbox.second);
    
    return 2;
}

static int lua_getbboverlap(lua_State *L) {
    if ((lua_gettop(L) != 2) || 
            !lua_islightuserdata(L, -1) || !lua_islightuserdata(L, -2))
        return 0;

    Reg *src = (Reg*) lua_touserdata(L, -1);
    Reg *dst = (Reg*) lua_touserdata(L, -2);
    
    pair<Pt,Pt> sbb = src->GetBoundingBox();
    pair<Pt,Pt> dbb = dst->GetBoundingBox();
    
    sbb.first = modPt(sbb.first, 0);
    sbb.second = modPt(sbb.second, 0);
    dbb.first = modPt(dbb.first, 1);
    dbb.second = modPt(dbb.second, 1);
    
    pair<Pt,Pt> box;
    
    if ((sbb.first.x > dbb.second.x) || (dbb.first.x > sbb.second.x) ||
        (sbb.first.y > dbb.second.y) || (dbb.first.y > sbb.second.y)) {
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
    
    lua_pushnumber(L, (box.second.x-box.first.x)*(box.second.y-box.first.y));
    
    return 1;
}
