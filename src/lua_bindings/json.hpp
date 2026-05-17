#ifndef JSON_HPP
#define JSON_HPP

#include <lua.hpp>

/**
 * @brief Lua binding: str, err = luapilot.json.encode(value [, opts])
 *
 * Mapping Lua -> JSON :
 *   - nil (niveau racine seulement) -> null
 *   - luapilot.json.null            -> null
 *   - luapilot.json.empty_array     -> []
 *   - boolean                       -> true/false
 *   - integer (math.type "integer") -> entier JSON (pas de ".0")
 *   - float   (math.type "float")   -> flottant JSON
 *   - string                        -> string JSON (UTF-8 valide attendu)
 *   - table clés 1..n               -> array JSON
 *   - table clés string             -> object JSON
 *   - table vide {}                 -> object JSON {}
 *
 * Erreurs renvoyées en (nil, "json: ...") — JAMAIS d'exception Lua :
 *   clés mixtes, array à trous, clé non représentable, NaN/Infinity,
 *   type non encodable (function/userdata/thread), imbrication trop
 *   profonde (capte aussi les cycles).
 *
 * opts.indent (entier) : si présent et >= 0, pretty-print avec cette
 * indentation. Absent / nil / négatif : sortie compacte.
 *
 * @return 2 valeurs : la chaîne JSON ou nil, et un message d'erreur ou nil.
 */
int lua_json_encode(lua_State *L);

/**
 * @brief Lua binding: value, err = luapilot.json.decode(str)
 *
 * Mapping JSON -> Lua :
 *   - null    -> luapilot.json.null (sentinel, PAS nil : sinon collision
 *                avec (val, err) et perte de clé dans les tables)
 *   - true/false -> boolean
 *   - integer -> integer (sous-type préservé)
 *   - float   -> float
 *   - string  -> string
 *   - array   -> table séquentielle (clés 1..n) ; un array VIDE est une
 *                table neuve marquée pour rester [] au re-encode
 *   - object  -> table à clés string
 *
 * Un entier non signé dépassant un lua_Integer est rendu en float
 * plutôt que de boucler en négatif silencieusement.
 *
 * @return 2 valeurs : la valeur Lua ou nil, et un message d'erreur ou nil.
 */
int lua_json_decode(lua_State *L);

/**
 * @brief Lua binding: t = luapilot.json.as_array(t)
 *
 * Marque la table `t` pour qu'elle soit sérialisée comme un tableau
 * JSON par encode(), même vide. Résout le cas du tableau construit
 * dynamiquement qui peut finir vide :
 *
 *   local tags = {}
 *   for ... do table.insert(tags, x) end
 *   encode({ tags = luapilot.json.as_array(tags) })
 *   -- 0 itération -> {"tags":[]} et non {"tags":{}}
 *
 * Renvoie la table elle-même (chaînable, idempotent).
 *
 * IMPORTANT : le marquage EST la métatable de la table. as_array()
 * écrase donc une métatable préexistante sur `t`. Sans conséquence
 * pour des tables de données pures (le seul cas où l'on sérialise du
 * JSON), mais à connaître. Même contrainte que dkjson / lua-cjson.
 *
 * Erreurs (luaL_error — usage incorrect, pas une condition runtime) :
 *   - argument absent ou non-table
 *   - argument = un sentinel json (null / empty_array) : interdit, car
 *     poser une métatable contournerait leur protection.
 *
 * La sémantique de sérialisation est celle, déjà testée, des tables
 * marquées : vide -> [], séquence 1..n -> array, clés string -> erreur
 * "array-tagged table has non-array keys" au moment du encode().
 *
 * @param L L'état Lua.
 * @return 1 valeur : la table passée en argument.
 */
int lua_json_as_array(lua_State *L);

/**
 * @brief Construit la sous-table `json` et l'attache à la table luapilot.
 *
 * Précondition : la table luapilot doit être au sommet de la pile (-1).
 * Après l'appel, la pile est inchangée (la table luapilot est toujours
 * au sommet) et luapilot.json contient : encode, decode, as_array,
 * null, empty_array.
 *
 * Les deux sentinels (null, empty_array) sont aussi stockés dans le
 * registry Lua pour que encode/decode puissent les identifier par
 * identité (lua_rawequal), indépendamment de toute modification que le
 * script ferait sur luapilot.json.
 *
 * @param L L'état Lua.
 */
void register_json(lua_State *L);

#endif // JSON_HPP
