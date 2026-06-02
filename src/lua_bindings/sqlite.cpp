// =====================================================================
// sqlite.cpp — implémentation des bindings SQLite
// =====================================================================
// Session 1 : open / close / exec SANS paramètres.
// Sessions à venir : bind de paramètres, query, types, doc.
//
// Voir sqlite.hpp pour le design global.

#include "sqlite.hpp"
#include "lua_utils.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

#include "sqlite3.h"

#include <cstring>
#include <string>

namespace
{

    // ============================================================
    // Userdata Db : handle vers une connexion SQLite
    // ============================================================
    //
    // Le userdata Lua contient un Db par valeur (pas un pointeur),
    // créé via lua_newuserdata + placement new.
    //
    // Quand le user appelle db:close(), on ferme sqlite3 et on
    // positionne handle = nullptr. Toute opération ultérieure
    // retourne (nil, "sqlite: connection closed").
    //
    // À la fin, le __gc Lua appelle Db::~Db qui ferme handle s'il
    // n'a pas été closé explicitement. Cohérent avec le pattern Sock
    // dans socket.cpp.

    struct Db
    {
        sqlite3 *handle;

        Db() : handle(nullptr) {}
        ~Db()
        {
            if (handle)
            {
                // sqlite3_close_v2 est la variante "tolérante" : ferme
                // proprement même si des statements préparées sont
                // encore vivantes. Comme on ne stocke pas de stmts
                // entre les appels (chaque exec/query prepare et
                // finalize en interne), close_v2 fera juste le même
                // travail que close() en pratique — mais c'est plus
                // sûr en cas de bug ou d'évolution future.
                sqlite3_close_v2(handle);
                handle = nullptr;
            }
        }
    };

    // Métaclé du userdata Db. L'adresse de cette variable sert d'ID
    // unique dans le registre Lua (idiome standard).
    const char *DB_MT = "luapilot.sqlite.db";

    Db *check_db(lua_State *L, int idx)
    {
        return static_cast<Db *>(luaL_checkudata(L, idx, DB_MT));
    }

    // ============================================================
    // Helpers d'erreur
    // ============================================================

    // Pose (nil, "sqlite: <msg>") sur la pile et renvoie 2 (nombre
    // de valeurs Lua à retourner). Cohérent avec le contrat LuaPilot.
    int push_sqlite_fail(lua_State *L, const std::string &msg)
    {
        std::string full = "sqlite: ";
        full += msg;
        return push_fail(L, full);
    }

    // Pose (nil, "sqlite: <msg from sqlite3>") d'après l'erreur la
    // plus récente du handle. Si handle est nullptr, fallback sur un
    // message générique.
    int push_sqlite_db_error(lua_State *L, sqlite3 *handle,
                             const char *context)
    {
        std::string msg;
        if (context)
        {
            msg += context;
            msg += ": ";
        }
        if (handle)
        {
            msg += sqlite3_errmsg(handle);
        }
        else
        {
            msg += "no database handle";
        }
        return push_sqlite_fail(L, msg);
    }

    // ============================================================
    // Parsing des opts pour open
    // ============================================================

    struct OpenOpts
    {
        bool wal;
        int busy_timeout_ms;

        OpenOpts() : wal(false), busy_timeout_ms(0) {}
    };

    // Lit opts depuis la pile (table à idx, ou nil/absent → defaults).
    // En cas d'option invalide, lance une erreur Lua (luaL_error).
    OpenOpts parse_open_opts(lua_State *L, int idx)
    {
        OpenOpts opts;
        int t = lua_type(L, idx);
        if (t == LUA_TNONE || t == LUA_TNIL)
        {
            return opts;
        }
        if (t != LUA_TTABLE)
        {
            luaL_error(L, "sqlite.open: opts must be a table or nil, got %s",
                       lua_typename(L, t));
        }

        // wal
        lua_getfield(L, idx, "wal");
        if (!lua_isnil(L, -1))
        {
            if (!lua_isboolean(L, -1))
            {
                lua_pop(L, 1);
                luaL_error(L, "sqlite.open: opts.wal must be a boolean");
            }
            opts.wal = lua_toboolean(L, -1);
        }
        lua_pop(L, 1);

        // busy_timeout
        lua_getfield(L, idx, "busy_timeout");
        if (!lua_isnil(L, -1))
        {
            if (!lua_isinteger(L, -1))
            {
                lua_pop(L, 1);
                luaL_error(L, "sqlite.open: opts.busy_timeout must be an integer (ms)");
            }
            lua_Integer v = lua_tointeger(L, -1);
            if (v < 0)
            {
                lua_pop(L, 1);
                luaL_error(L, "sqlite.open: opts.busy_timeout must be >= 0");
            }
            if (v > 60 * 60 * 1000) // 1h max, valeur sanity
            {
                lua_pop(L, 1);
                luaL_error(L, "sqlite.open: opts.busy_timeout too large (max 3600000 ms)");
            }
            opts.busy_timeout_ms = static_cast<int>(v);
        }
        lua_pop(L, 1);

        return opts;
    }

    // ============================================================
    // Méthodes du userdata Db
    // ============================================================

    // db:close() → (true, nil) | (nil, err)
    //
    // Idempotent : un second close() retourne (true, nil) sans rien
    // faire. Cohérent avec sock:close().
    int db_close(lua_State *L)
    {
        Db *db = check_db(L, 1);
        if (db->handle)
        {
            int rc = sqlite3_close_v2(db->handle);
            db->handle = nullptr;
            if (rc != SQLITE_OK)
            {
                // close_v2 ne devrait jamais échouer en pratique, mais
                // on retourne quand même l'info.
                return push_sqlite_fail(L, sqlite3_errstr(rc));
            }
        }
        return push_ok(L);
    }

    // db:exec(sql) → (true, nil) | (nil, err)
    //
    // V1 session 1 : SANS paramètres (les params arriveront session 2).
    // Utilise sqlite3_exec qui peut traiter plusieurs statements
    // séparés par ';' (utile pour CREATE TABLE ... ; CREATE INDEX ...
    // d'un coup).
    int db_exec(lua_State *L)
    {
        Db *db = check_db(L, 1);
        if (!db->handle)
        {
            return push_sqlite_fail(L, "connection closed");
        }

        // Strict : pas de coercion number→string. Cohérent avec
        // sqlite.open et toml.decode.
        luaL_checktype(L, 2, LUA_TSTRING);
        const char *sql = lua_tostring(L, 2);

        // sqlite3_exec accepte un callback pour les SELECT, mais
        // comme on n'expose pas SELECT via exec (c'est query() qui
        // s'en chargera), on passe nullptr. Si le user appelle
        // exec("SELECT ..."), ça marchera mais les résultats seront
        // ignorés. Documenté dans le README.
        char *errmsg = nullptr;
        int rc = sqlite3_exec(db->handle, sql, nullptr, nullptr, &errmsg);
        if (rc != SQLITE_OK)
        {
            std::string msg = errmsg ? errmsg : sqlite3_errmsg(db->handle);
            sqlite3_free(errmsg);
            return push_sqlite_fail(L, msg);
        }
        return push_ok(L);
    }

    // __gc : ferme automatiquement la DB si pas déjà close().
    int db_gc(lua_State *L)
    {
        Db *db = check_db(L, 1);
        db->~Db();
        return 0;
    }

    // __tostring : utile pour le debug. Affiche l'état de la
    // connexion.
    int db_tostring(lua_State *L)
    {
        Db *db = check_db(L, 1);
        if (db->handle)
        {
            lua_pushfstring(L, "luapilot.sqlite.db (open, %p)", db->handle);
        }
        else
        {
            lua_pushliteral(L, "luapilot.sqlite.db (closed)");
        }
        return 1;
    }

    // ============================================================
    // API du module : luapilot.sqlite.open
    // ============================================================

    // luapilot.sqlite.open(path, opts?) → db | (nil, err)
    //
    // path : ":memory:" pour une DB en RAM (jetable),
    //        sinon un chemin de fichier (créé s'il n'existe pas).
    //
    // opts : { wal = bool, busy_timeout = ms } — tous optionnels.
    int sqlite_open(lua_State *L)
    {
        // luaL_checkstring convertit silencieusement les nombres en
        // strings (sémantique Lua par défaut). On veut rejeter
        // open(42) explicitement : c'est probablement un bug côté
        // appelant, pas une intention d'ouvrir un fichier nommé "42".
        // Pattern aligné sur toml.decode et workers.spawn.
        luaL_checktype(L, 1, LUA_TSTRING);
        const char *path = lua_tostring(L, 1);

        // Parse opts ; lance une erreur Lua si malformés.
        OpenOpts opts = parse_open_opts(L, 2);

        // Ouverture avec les flags par défaut équivalents à sqlite3_open :
        //   CREATE | READWRITE.
        // sqlite3_open_v2 permettrait de durcir avec NOMUTEX par exemple,
        // mais on garde le défaut pour cohérence avec SQLITE_THREADSAFE=1.
        sqlite3 *handle = nullptr;
        int rc = sqlite3_open(path, &handle);
        if (rc != SQLITE_OK)
        {
            std::string msg = handle ? sqlite3_errmsg(handle) : sqlite3_errstr(rc);
            if (handle)
            {
                sqlite3_close_v2(handle);
            }
            return push_sqlite_fail(L, msg);
        }

        // Appliquer busy_timeout AVANT WAL : si WAL bloque sur lock,
        // on veut le retry automatique.
        if (opts.busy_timeout_ms > 0)
        {
            rc = sqlite3_busy_timeout(handle, opts.busy_timeout_ms);
            if (rc != SQLITE_OK)
            {
                std::string msg = sqlite3_errmsg(handle);
                sqlite3_close_v2(handle);
                return push_sqlite_fail(L, "busy_timeout: " + msg);
            }
        }

        // Activer WAL si demandé. PRAGMA journal_mode renvoie le mode
        // effectif (peut être "memory" pour :memory:, "wal" pour fichier).
        // On accepte tout retour non-erreur — un mode différent n'est
        // pas une erreur, juste un fallback géré par SQLite lui-même.
        if (opts.wal)
        {
            char *errmsg = nullptr;
            rc = sqlite3_exec(handle, "PRAGMA journal_mode=WAL;",
                              nullptr, nullptr, &errmsg);
            if (rc != SQLITE_OK)
            {
                std::string msg = errmsg ? errmsg : sqlite3_errmsg(handle);
                sqlite3_free(errmsg);
                sqlite3_close_v2(handle);
                return push_sqlite_fail(L, "enabling WAL: " + msg);
            }
            sqlite3_free(errmsg);
        }

        // Allouer le userdata Db et y poser handle.
        // Placement new pour initialiser correctement le struct
        // (cohérent avec push_new_sock dans socket.cpp).
        Db *db = static_cast<Db *>(lua_newuserdata(L, sizeof(Db)));
        new (db) Db();
        db->handle = handle;

        // Attacher la métatable (créée à register_sqlite).
        luaL_getmetatable(L, DB_MT);
        lua_setmetatable(L, -2);

        return 1;
    }

    // ============================================================
    // Construction de la métatable + sous-table luapilot.sqlite
    // ============================================================

    void create_db_metatable(lua_State *L)
    {
        // Crée la métatable et l'enregistre dans le registry sous
        // la clé DB_MT.
        luaL_newmetatable(L, DB_MT);

        // __index = self (les méthodes sont des champs directs de
        // la métatable).
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");

        // __gc : appelé par Lua quand le userdata est collecté.
        lua_pushcfunction(L, db_gc);
        lua_setfield(L, -2, "__gc");

        // __tostring : pour print(db).
        lua_pushcfunction(L, db_tostring);
        lua_setfield(L, -2, "__tostring");

        // Méthodes : close, exec.
        // (query arrivera session 3.)
        lua_pushcfunction(L, db_close);
        lua_setfield(L, -2, "close");

        lua_pushcfunction(L, db_exec);
        lua_setfield(L, -2, "exec");

        // On dépile la métatable, elle reste en registry.
        lua_pop(L, 1);
    }

} // namespace anonyme

// ============================================================
// Fonction exportée (déclarée dans sqlite.hpp)
// ============================================================

void register_sqlite(lua_State *L)
{
    // Précondition : la table luapilot est au sommet.

    // Créer la métatable du userdata Db (en registry).
    create_db_metatable(L);

    // Sous-table luapilot.sqlite avec la fonction open.
    lua_newtable(L);

    lua_pushcfunction(L, sqlite_open);
    lua_setfield(L, -2, "open");

    lua_setfield(L, -2, "sqlite");
}
