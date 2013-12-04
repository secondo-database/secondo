/*
*/

#include "interpolate.h"

#include <vector>
#include <lua5.2/lua.hpp>

static lua_State *L = NULL;
static void setfield(const char *key, int value);

static int lua_distance(lua_State *L);

int luaInit(void) {
    if (L != NULL)
        lua_close(L);
    L = luaL_newstate();
    luaL_openlibs(L);
    int st = luaL_loadfile(L, "/tmp/mf.lua");
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

    return 0;
}

vector<pair<Reg *, Reg *> > _matchFacesLua(vector<Reg> *src, vector<Reg> *dst,
        int level) {
    vector<pair<Reg *, Reg *> > ret;

    if (!L) {
        int st = luaInit();
        if (st < 0)
            return ret;
    }

    Pt srcoff = Reg::GetMinXY(*src);
    Pt dstoff = Reg::GetMinXY(*dst);
    Pt off = dstoff - srcoff;

    lua_newtable(L);
    setfield("x", off.x);
    setfield("y", off.y);
    lua_setglobal(L, "offset");

    //    lua_newtable(L);
    //    setfield("x", dstoff.x);
    //    setfield("y", dstoff.y);
    //    lua_setglobal(L, "dstoff");

    lua_getglobal(L, "matchFaces");

    lua_newtable(L);
    for (unsigned int i = 0; i < src->size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_pushlightuserdata(L, &(*src)[i]);
        lua_rawset(L, -3);
    }

    lua_newtable(L);
    for (unsigned int i = 0; i < dst->size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_pushlightuserdata(L, &(*dst)[i]);
        lua_rawset(L, -3);
    }

    lua_pushinteger(L, level);

    cerr << "Calling LUA START\n";
    lua_call(L, 3, 1);
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

static void setfield(const char *key, int value) {
    lua_pushstring(L, key);
    lua_pushnumber(L, value);
    lua_settable(L, -3);
}

static Pt getoffset() {
    double xoff = 0, yoff = 0;

    lua_getglobal(L, "offset");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "x");
        lua_getfield(L, -2, "y");
        if (lua_isnumber(L, -1) && lua_isnumber(L, -2)) {
            xoff = lua_tonumber(L, -2);
            yoff = lua_tonumber(L, -1);
        }
        lua_pop(L, 2);
    }
    lua_pop(L, 1);
    
    return Pt(xoff, yoff);
}

static int lua_distance(lua_State *L) {
    if (!lua_islightuserdata(L, -1) || !lua_islightuserdata(L, -2))
        return 0;

    Pt offset = getoffset();

    Reg *src = (Reg*) lua_touserdata(L, -1);
    Reg *dst = (Reg*) lua_touserdata(L, -2);
    double dist = src->GetMiddle().distance(dst->GetMiddle() - offset);
    lua_pushnumber(L, dist);
    return 1;
}