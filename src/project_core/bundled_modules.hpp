#ifndef BUNDLED_MODULES_HPP
#define BUNDLED_MODULES_HPP

#include <lua.hpp>

/**
 * @brief Enregistre les modules Lua bundlés dans package.preload.
 *
 * Après cet appel, un script peut faire require("inspect") (et d'autres
 * modules bundlés à venir) sans qu'aucun fichier .lua ne soit présent
 * sur le disque : le source est compilé en dur dans le binaire.
 *
 * À appeler juste après luaL_openlibs(L), avant tout register_babet
 * ou exécution de script utilisateur.
 *
 * Note : si l'utilisateur a un fichier inspect.lua à côté de son main.lua,
 * package.preload est testé AVANT package.path par require, donc le
 * bundlé gagne — ce qui est volontaire (impossible de casser inspect en
 * mettant un mauvais fichier à côté).
 */
void register_bundled_modules(lua_State *L);

#endif // BUNDLED_MODULES_HPP