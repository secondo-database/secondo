/*
*/

#include "interpolate.h"

#include <vector>
#include <lua5.1/lua.hpp>

static lua_State *L = NULL;

int luaInit (void) {
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
    
    return 0;
}

vector<pair<Reg *, Reg *> > _matchFacesLua(vector<Reg> *src, vector<Reg> *dst) {
    vector<pair<Reg *, Reg *> > ret;
    
    if (!L) {
        int st = luaInit();
        if (st < 0)
            return ret;
    }
    
    lua_getglobal(L, "matchFaces");
    lua_pushinteger(L, src->size());
    lua_pushinteger(L, dst->size());
    cerr << "Calling LUA START\n";
    lua_call(L, 2, 1);
    cerr << "Calling LUA END\n";
    
    return ret;
}
