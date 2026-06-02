// =====================================================================
// signal.hpp — bindings Lua pour les signaux POSIX
// =====================================================================
//
// Expose la sous-table luapilot.signal avec trois fonctions :
//
//   luapilot.signal.handle(name, fn_or_nil) -> true | (nil, err)
//     Installe un handler Lua pour le signal donné, OU le désinstalle
//     si on passe nil (auquel cas le comportement par défaut du
//     système reprend).
//
//   luapilot.signal.ignore(name) -> true | (nil, err)
//     Marque le signal comme ignoré (SIG_IGN). Le process ne réagit
//     plus du tout à ce signal.
//
//   luapilot.signal.default(name) -> true | (nil, err)
//     Restaure le comportement système par défaut (SIG_DFL) pour ce
//     signal. Pour TERM/INT/HUP cela tue le process.
//
// Signaux supportés (liste blanche) :
//   TERM, INT, HUP, USR1, USR2, PIPE
//
// Volontairement exclus :
//   - CHLD : pas d'exec async en v1, donc pas de raison de l'exposer.
//   - SEGV, BUS, FPE, ILL : interception dangereuse depuis Lua.
//   - ALRM : conflit potentiel avec sleep() interne.
//   - KILL, STOP : non interceptables par définition POSIX.
//
// ---------------------------------------------------------------------
// Sécurité : handler C minimal + dispatch différé
// ---------------------------------------------------------------------
//
// Les handlers POSIX s'exécutent en contexte async-signal-safe : aucune
// allocation mémoire, aucun appel non-réentrant, aucune interaction
// avec la VM Lua sous peine de crash ou corruption immédiate. C'est
// pourquoi notre handler C ne fait QU'UNE chose : positionner un flag
// `volatile sig_atomic_t pending[signum] = 1`.
//
// Le callback Lua de l'utilisateur est invoqué plus tard, en contexte
// normal, via signal_dispatch_pending(L). Cette dispatch a lieu :
//
//   - Automatiquement, depuis un debug hook Lua "count" installé dès
//     le premier appel à handle(). Le hook se déclenche toutes les
//     ~10000 instructions Lua, soit quelques ms de latence max sur
//     du code Lua typique.
//
//   - Explicitement (phase B, à venir), depuis les fonctions
//     bloquantes de LuaPilot (socket:recv, sleep, etc.) quand un
//     EINTR survient. C'est ce qui permettra à un SIGTERM
//     d'interrompre un recv_line() bloquant et de renvoyer
//     (nil, "interrupted").
//
// ---------------------------------------------------------------------
// Comportement avec les workers
// ---------------------------------------------------------------------
//
// Dans un programme multithreadé, les signaux sont délivrés à
// n'importe quel thread du process par le kernel — comportement
// imprévisible. Pour garantir que les handlers Lua ne s'exécutent
// QUE dans le thread principal, workers.cpp bloque tous les signaux
// supportés au démarrage de chaque worker via pthread_sigmask().
// L'utilisateur n'a rien à savoir.
//
// ---------------------------------------------------------------------
// Erreurs et propagation
// ---------------------------------------------------------------------
//
// Si le callback Lua d'un signal lève une erreur, elle est SILENCIEUSEMENT
// avalée par signal_dispatch_pending(). Raison : on est appelé depuis
// un hook ou depuis du code C profond, et propager l'erreur risque de
// crasher le programme à un endroit non lié au handler. C'est cohérent
// avec la sémantique POSIX : un handler de signal ne propage pas d'erreur.
// Le user qui veut une logique d'erreur la met dans le callback.

#ifndef LUA_BINDINGS_SIGNAL_HPP
#define LUA_BINDINGS_SIGNAL_HPP

struct lua_State;

// Crée la sous-table luapilot.signal avec les fonctions handle, ignore,
// default. Précondition de pile : la table luapilot est au sommet.
// Postcondition : la pile est inchangée (la sous-table est posée
// comme champ "signal" de luapilot).
void register_signal(lua_State *L);

// Capture le pthread_t du thread courant comme "main thread". Doit
// être appelée une fois, depuis main(), AVANT register_signal et
// AVANT tout spawn de worker. Permet ensuite à handle/ignore/default
// de refuser les appels depuis un autre thread (les signaux POSIX
// sont process-wide, donc configurer un handler depuis un worker
// produirait un bug latent : le signal serait routé vers le main
// thread, qui n'a pas le callback Lua du worker).
void register_main_thread(void);

// Vérifie les flags pending et invoque les callbacks Lua correspondants.
// Appelée automatiquement depuis le debug hook count installé par
// handle(). Peut aussi être appelée explicitement par d'autres modules
// (phase B) pour propager les interruptions en sortie de syscall.
void signal_dispatch_pending(lua_State *L);

// Indique s'il existe un signal supporté actuellement pending sans
// déclencher de callback. Utilisé par les fonctions bloquantes pour
// décider si un EINTR doit être traduit en (nil, "interrupted")
// (phase B).
bool signal_any_handled_pending();

#endif // LUA_BINDINGS_SIGNAL_HPP