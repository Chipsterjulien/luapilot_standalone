// =====================================================================
// sqlite.hpp — bindings Lua pour SQLite 3 (amalgamation embarquée)
// =====================================================================
//
// Expose la sous-table luapilot.sqlite avec une API haut niveau :
//
//   db, err = luapilot.sqlite.open(path, opts?)
//   ok, err = db:exec(sql, params?)
//   for row in db:query(sql, params?) do ... end        -- (à venir, session 3)
//   ok, err = db:close()
//
// Le userdata "db" est un handle vers une connexion SQLite. Il est
// automatiquement fermé par __gc s'il n'est pas explicitement close().
//
// ---------------------------------------------------------------------
// Design V1 (figé pour cette release)
// ---------------------------------------------------------------------
//
// Style :  haut niveau seulement (pas de prepare/bind/step/finalize
//          exposés en V1). Les prepared statements sont créés et
//          libérés en interne à chaque exec/query.
//
// Types :  NULL ↔ nil
//          INTEGER ↔ integer Lua
//          REAL ↔ number Lua (float)
//          TEXT ↔ string Lua
//          BLOB ↔ string Lua (binary-safe)
//          BOOL Lua ↔ INTEGER 0/1 (en bind seulement, lecture = integer)
//
// Erreurs : contrat LuaPilot standard : (nil, "sqlite: <msg>") ou
//           (true, nil). Pas de raise sur erreur SQL ; raise seulement
//           sur erreur de type d'argument (programmation Lua incorrecte).
//
// Options open :
//   wal           : bool (par défaut false) — active PRAGMA journal_mode=WAL
//   busy_timeout  : int  (par défaut 0)     — sqlite3_busy_timeout en ms
//
// Concurrence : aucun lock LuaPilot global. SQLite gère ses propres
//   verrous fichier. Mode WAL recommandé pour multi-readers + 1 writer.
//   Pour multi-workers qui écrivent : pattern "1 worker = DB-owner".
//   Voir README pour les détails.

#ifndef LUA_BINDINGS_SQLITE_HPP
#define LUA_BINDINGS_SQLITE_HPP

struct lua_State;

// Crée la sous-table luapilot.sqlite avec les fonctions exposées.
// Précondition de pile : la table luapilot est au sommet.
// Postcondition : pile inchangée (la sous-table est posée comme
// champ "sqlite" de luapilot).
void register_sqlite(lua_State *L);

#endif // LUA_BINDINGS_SQLITE_HPP
