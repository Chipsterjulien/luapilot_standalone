#include "lua_bindings/split.hpp"
#include <sstream>
#include <vector>
#include <string>

// Fonction pour diviser une chaîne en utilisant un délimiteur avec une limite sur le nombre de divisions
std::vector<std::string> split(const std::string &str, char delimiter, int max_splits) {
    std::vector<std::string> tokens; // Vecteur pour stocker les morceaux de la chaîne
    std::string token;
    std::istringstream tokenStream(str); // Flux de chaîne pour parcourir la chaîne d'entrée
    int splits = 0;

    // Parcourt la chaîne et divise selon le délimiteur
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token); // Ajoute le morceau actuel au vecteur
        if (max_splits != -1 && ++splits >= max_splits) {
            // Ajoute la partie restante de la chaîne comme dernier morceau si la limite de divisions est atteinte
            std::string remaining;
            std::getline(tokenStream, remaining);
            tokens.push_back(remaining);
            break;
        }
    }

    return tokens; // Retourne le vecteur de morceaux
}

// Interface Lua pour la fonction split
int split(lua_State *L) {
    // Vérifie le nombre d'arguments
    int argc = lua_gettop(L);
    if (argc < 2 || argc > 3) {
        return luaL_error(L, "Expected 2 or 3 arguments: string, delimiter, and optional max_splits");
    }

    // Vérifie et obtient le premier argument
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Expected a string as the first argument");
    }
    const char *str = luaL_checkstring(L, 1);

    // Vérifie et obtient le second argument
    if (!lua_isstring(L, 2)) {
        return luaL_error(L, "Expected a string as the second argument");
    }
    std::string delim = luaL_checkstring(L, 2);
    if (delim.length() != 1) {
        return luaL_error(L, "Delimiter should be a single character");
    }

    // Troisième argument optionnel : max_splits
    int max_splits = -1; // Valeur par défaut
    if (argc == 3) {
        if (!lua_isinteger(L, 3)) {
            return luaL_error(L, "Expected an integer as the third argument");
        }
        max_splits = luaL_checkinteger(L, 3);
    }

    // Effectue l'opération de division (en supposant qu'une fonction split est définie ailleurs)
    std::vector<std::string> tokens = split(str, delim[0], max_splits);

    // Crée une nouvelle table sur la pile Lua
    lua_newtable(L);

    // Ajoute les morceaux dans la table
    int index = 1;
    for (const std::string &token : tokens) {
        lua_pushnumber(L, index++); // Pousse l'index sur la pile Lua
        lua_pushstring(L, token.c_str()); // Pousse le morceau de chaîne sur la pile Lua
        lua_settable(L, -3); // Définit l'entrée de la table
    }

    return 1;
}
