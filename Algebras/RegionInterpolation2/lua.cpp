/*
*/

#include "interpolate.h"

#include <vector>
#include <lua5.1/lua.hpp>

static lua_State *l = NULL;

void luaInit (void) {
    if (l != NULL)
        lua_close(l);
    l = lua_open();
    luaopen_base(l);
    luaL_loadfile(l, "/tmp/mf.lua");
}

vector<pair<Reg *, Reg *> > _matchFacesLua(vector<Reg> *src, vector<Reg> *dst) {
    vector<pair<Reg *, Reg *> > ret;
    
    lua_getglobal(l, "matchFaces");
    lua_pushinteger(l, src->size());
    lua_pushinteger(l, dst->size());
    lua_call(l, 2, 1);
    
    return ret;
}
