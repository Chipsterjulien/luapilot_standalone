#ifndef HTTP_HPP
#define HTTP_HPP

#include <lua.hpp>

/**
 * @brief Binding HTTP client : babet.http.{request,get,post}
 *
 * Philosophie d'erreur calquée sur babet.exec :
 *
 *   - Échec transport (DNS, connexion refusée, TLS, timeout, ...) :
 *       (nil, "http: <libellé httplib brut>")
 *     Le libellé vient de httplib::to_string(Error). Depuis
 *     cpp-httplib v0.45.0, l'enum inclut Timeout / ConnectionTimeout
 *     distincts ; combiné à set_max_timeout (cf. opts.timeout), un
 *     dépassement remontera typiquement "http: Timeout". Aucune
 *     heuristique temporelle de renommage côté binding (décision i).
 *
 *   - Réponse HTTP reçue, QUEL QUE SOIT le statut (y compris 4xx/5xx) :
 *       (result, nil)  avec  result = { status, body, headers }
 *     Un 404 ou un 500 n'est PAS une erreur (miroir exact de
 *     « code de sortie != 0 n'est pas une erreur d'exec »).
 *
 *   - Mauvais usage (opts absent/non-table, url absente/non-string,
 *     champ au mauvais TYPE) : luaL_error (lève), comme les autres
 *     bindings. Les mauvaises VALEURS d'option (timeout <= 0, url
 *     malformée, méthode inconnue, body sur GET/HEAD/OPTIONS, clés
 *     ou valeurs query non string/number) renvoient (nil, err) —
 *     même découpage que exec.
 *
 * result.body est binaire-safe (lua_pushlstring avec longueur) : un
 * corps contenant des octets nuls (image, gzip) passe intact.
 *
 * result.headers : clés normalisées en MINUSCULES (HTTP est
 * insensible à la casse, Lua non) ; sur doublon, la dernière valeur
 * rencontrée gagne.
 *
 * opts (table) :
 *   url              string  (requis) — http:// ou https://
 *   method           string  (défaut "GET")
 *   headers          table   { ["Name"] = "value", ... }
 *   body             string  (méthodes à corps uniquement ; un body
 *                            sur GET/HEAD/OPTIONS renvoie (nil, err)
 *                            — pas de comportement muet)
 *   query            table   { k = v, ... } -> ?k=v&... (percent-encodé)
 *   timeout          number  secondes > 0, plafond GLOBAL bout-en-bout
 *                            (équivalent --max-time de curl) appliqué
 *                            via cpp-httplib set_max_timeout.
 *   verify           bool    (défaut true) — vérif. cert. serveur TLS
 *   ca_cert          string  chemin d'un bundle CA (optionnel)
 *   follow_redirects bool    (défaut false) — pas de magie silencieuse
 *
 * Périmètre v1 : pas de streaming, multipart, cookies, session,
 * keep-alive exposé. Ajoutable plus tard sous SemVer sans casse.
 *
 * get(url [, opts])            -> request{ url=url, method="GET",  ...opts }
 * post(url [, body] [, opts])  -> request{ url=url, method="POST", ... }
 * Ce sont de purs wrappers : un seul chemin de code (http_perform).
 */
int lua_http_request(lua_State *L);
int lua_http_get(lua_State *L);
int lua_http_post(lua_State *L);

/**
 * @brief Construit la sous-table `http` et l'attache à babet.
 *
 * Précondition : la table babet est au sommet de la pile (-1),
 * exactement comme register_json. La pile est inchangée après l'appel.
 */
void register_http(lua_State *L);

#endif // HTTP_HPP
