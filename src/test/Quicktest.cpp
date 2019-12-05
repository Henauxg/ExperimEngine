#include "ExperimEngineConfig.h"

#include <lua/lua.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "Quicktest.hpp"

namespace expengine {
namespace quicktest {

void testSol()
{
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::io);

	lua["foo"] = lua.create_table_with(1, 24, 2, 25, 3, 26);

	lua.script_file("scripts/BasicScript.lua");
}

void testCMake()
{
	std::cout << "Version " << ExperimEngine_VERSION_MAJOR << "." << ExperimEngine_VERSION_MINOR
			  << "." << ExperimEngine_VERSION_PATCH << std::endl;
}

void testLuaJit()
{
	int status, result, i;
	double sum;
	lua_State* L;

	/*
	 * All Lua contexts are held in this structure. We work with it almost
	 * all the time.
	 */
	L = luaL_newstate();

	luaL_openlibs(L); /* Load Lua libraries */

	/* Load the file containing the script we are going to run */
	status = luaL_loadfile(L, "scripts/BasicScript.lua");
	if (status) {
		/* If something went wrong, error message is at the top of */
		/* the stack */
		fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
		exit(1);
	}

	/*
	 * Ok, now here we go: We pass data to the lua script on the stack.
	 * That is, we first have to prepare Lua's virtual stack the way we
	 * want the script to receive it, then ask Lua to run it.
	 */
	lua_newtable(L); /* We will pass a table */

	/*
	 * To put values into the table, we first push the index, then the
	 * value, and then call lua_rawset() with the index of the table in the
	 * stack. Let's see why it's -3: In Lua, the value -1 always refers to
	 * the top of the stack. When you create the table with lua_newtable(),
	 * the table gets pushed into the top of the stack. When you push the
	 * index and then the cell value, the stack looks like:
	 *
	 * <- [stack bottom] -- table, index, value [top]
	 *
	 * So the -1 will refer to the cell value, thus -3 is used to refer to
	 * the table itself. Note that lua_rawset() pops the two last elements
	 * of the stack, so that after it has been called, the table is at the
	 * top of the stack.
	 */
	for (i = 1; i <= 5; i++) {
		lua_pushnumber(L, i); /* Push the table index */
		lua_pushnumber(L, i * 2); /* Push the cell value */
		lua_rawset(L, -3); /* Stores the pair in the table */
	}

	/* By what name is the script going to reference our table? */
	lua_setglobal(L, "foo");

	/* Ask Lua to run our little script */
	result = lua_pcall(L, 0, LUA_MULTRET, 0);
	if (result) {
		fprintf(stderr, "Failed to run script: %s\n", lua_tostring(L, -1));
		exit(1);
	}

	/* Get the returned value at the top of the stack (index -1) */
	sum = lua_tonumber(L, -1);

	printf("Script returned: %.0f\n", sum);

	lua_pop(L, 1); /* Take the returned value out of the stack */
	lua_close(L); /* Cya, Lua */
}
} // namespace quicktest
} // namespace expengine