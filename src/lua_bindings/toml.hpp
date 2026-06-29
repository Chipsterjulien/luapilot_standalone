#ifndef TOML_HPP
#define TOML_HPP

#include <lua.hpp>

/**
 * @brief Binding TOML : babet.toml.decode(s) -> (table, nil) | (nil, err)
 *
 * Décisions actées avec le mainteneur :
 *
 *   - Périmètre v1 : decode SEUL. Pas d'encode, pas de decode_file,
 *     pas de sentinelles, pas d'options de parsing. Tout ajoutable
 *     plus tard sous SemVer sans casse.
 *
 *   - Contrat de retour, strict miroir de babet.json.decode :
 *       * succès        : (table, nil)
 *       * TOML invalide : (nil, "toml: <description> (line L, col C)")
 *       * mauvais usage (arg absent / non-string) : luaL_error
 *
 *   - Mapping des types temporels TOML -> Lua : strings ISO 8601.
 *     TOML a quatre types temporels distincts ; Lua n'a aucune notion
 *     de date. On les sérialise donc en strings (lossy mais
 *     prévisible) : le script re-parse avec son outil préféré.
 *       local-date       : "YYYY-MM-DD"
 *       local-time       : "HH:MM:SS[.fffffffff]"
 *       local-date-time  : "YYYY-MM-DDTHH:MM:SS[.fffffffff]"
 *       offset-date-time : "YYYY-MM-DDTHH:MM:SS[.fffffffff]<+|-HH:MM|Z>"
 *
 *   - Tables vs arrays côté Lua :
 *       * TOML table     -> Lua table à clés string
 *       * TOML array     -> Lua table à clés 1..n (séquence)
 *     Cohérent avec ce que fait babet.json.decode.
 *
 *   - Aucune exception C++ ne traverse vers Lua : invariant de
 *     correction (try/catch global même si toml::parse ne lève pas
 *     en mode noexcept, par défense au cas où la conversion lève).
 */

int lua_toml_decode(lua_State *L);

/**
 * @brief Construit la sous-table `toml` et l'attache à babet.
 *
 * Précondition : la table babet est au sommet de la pile (-1),
 * exactement comme register_json / register_http. La pile est
 * inchangée après l'appel.
 */
void register_toml(lua_State *L);

#endif // TOML_HPP