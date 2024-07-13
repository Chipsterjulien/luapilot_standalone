#include "helloThere.hpp"
#include <iostream>

int lua_helloThere(lua_State *L) {
    std::cout << "General Kenobi: 'Hello there!'" << std::endl;
    return 0;
}
