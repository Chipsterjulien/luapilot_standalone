#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <lua.hpp>

/**
 * @brief Binding TCP sockets : babet.socket.{connect,listen}
 *
 * Décisions actées (toutes observables) :
 *
 *   SOCK-A  Bloquant pur avec timeout (v1). Non-bloquant reporté.
 *   SOCK-B  TCP seul (v1). UDP reporté.
 *   SOCK-C  Pas de TLS (v1). Reporté ; OpenSSL déjà lié, ajoutable.
 *   SOCK-D  POSIX direct, zéro dépendance externe.
 *   SOCK-1  Sous-table `babet.socket` (singulier).
 *   SOCK-2  Client + serveur en v1, API haute : connect() / listen()
 *           font tout d'un coup (pas de bind() exposé).
 *   SOCK-3  Userdata avec métatable + __gc -> jamais de fuite de FD
 *           même si le script oublie :close().
 *   SOCK-4  Réception : recv(n) = "au plus n octets", recv_line(),
 *           recv_all(). recv_exact() est trivialement implémentable
 *           par l'utilisateur via boucle, donc non fourni.
 *   SOCK-5  EOF -> (nil, "closed"). Chaîne typée pour distinguer
 *           EOF / timeout / vraie erreur sans deviner.
 *           recv_line() avec EOF en plein milieu d'une ligne :
 *           (nil, "closed", partial_data) -- les 3 valeurs sont
 *           justifiées ici (ne pas perdre les octets déjà lus).
 *   SOCK-6  set_timeout(s) unique, applicable à toutes les opérations.
 *           timeout=0 désactive le timeout. Dépassement -> (nil, "timeout").
 *
 * Détail d'implémentation important :
 *   listen() active SO_REUSEADDR en interne via setsockopt() entre
 *   socket() et bind(). NON EXPOSÉ dans l'API publique. Raison :
 *   sans ça, un serveur tué et relancé immédiatement (cas typique
 *   des tests et des petits démons) butte sur EADDRINUSE pendant
 *   ~60s à cause de TIME_WAIT. Sous Linux, SO_REUSEADDR seul suffit.
 *
 * Mauvais usage (mauvais TYPE d'argument) : luaL_error.
 * Mauvaises VALEURS (port négatif, host vide, etc.) : (nil, err).
 *   -> miroir de http, argparse, toml.
 *
 * Hors v1 (additif plus tard sous SemVer) : UDP, TLS, mode
 * non-bloquant, select/poll multi-sockets, options socket avancées
 * (keepalive, SO_REUSEPORT, etc.).
 */

int lua_socket_connect(lua_State *L);
int lua_socket_listen(lua_State *L);

/**
 * @brief Construit la sous-table `socket` et l'attache à babet.
 *
 * Précondition : la table babet est au sommet de la pile (-1),
 * comme register_json / register_http / register_toml. La pile est
 * inchangée après l'appel.
 *
 * Effet secondaire : la métatable "LuapilotSocket" est aussi posée
 * dans le registry Lua (via luaL_newmetatable). C'est la métatable
 * partagée par tous les userdata socket retournés par connect() /
 * listen() / accept(). Contient les méthodes (send, recv, ...) et
 * le __gc pour ne jamais fuir de FD.
 */
void register_socket(lua_State *L);

#endif // SOCKET_HPP