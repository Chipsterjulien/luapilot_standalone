-- logging.lua — logger pour LuaPilot, Lua pur, bundlé via
-- tools/embed_lua_module.sh (require("logging")).
--
-- Contrat observable (décisions actées avec le mainteneur) :
--
--   local log = require("logging")
--   log.info("user=", uid, " status=", status)    -- variadique style print
--   log.set_level("debug")                         -- ou log.DEBUG
--   log.set_output(io.open("app.log", "a"))        -- tout objet :write()-able
--   log.set_color(true)                            -- ANSI OFF par défaut, opt-in
--
--   * Niveaux : trace=10, debug=20, info=30, warn=40, error=50
--     Seuil par défaut : info. Un message dont le niveau est >= seuil
--     est émis ; les autres sont silencieusement coupés (le SEUL
--     silence légitime, car explicitement demandé par set_level).
--
--   * Destination par défaut : stderr (convention Unix, ne pollue pas
--     stdout). set_output accepte tout objet qui répond à :write.
--
--   * Format figé en v1 (pas de formatter custom) :
--       "YYYY-MM-DD HH:MM:SS [LEVEL] message"
--     Étiquettes paddées à 5 caractères : [TRACE] [DEBUG] [INFO ]
--     [WARN ] [ERROR]. Pas de millisecondes ni UTC en v1.
--
--   * Couleurs ANSI (quand set_color(true)) :
--       trace = gris dim, debug = cyan, info = SANS COULEUR (neutre),
--       warn = jaune, error = rouge.
--     Couleurs OFF par défaut ; pas de détection auto isatty en v1.
--
--   * Contrat d'erreur :
--       - Les appels log.trace/debug/info/warn/error ne lèvent JAMAIS
--         et ne retournent rien. Une erreur d'écriture (disque plein,
--         handle fermé, ...) est silencieusement avalée — règle
--         standard pour un logger : ne jamais transformer un log
--         raté en plantage métier.
--       - Les setters (set_level, set_output, set_color) lèvent via
--         error() en cas de mauvais usage du programmeur (niveau
--         inconnu, objet non :write()-able, type incorrect).
--         Cohérent avec luaL_error côté bindings C.
--
-- Périmètre v1 (volontairement limité ; tout ajoutable plus tard sans
-- casse, Lua pur additif) : pas de rotation de fichiers, pas de
-- loggers nommés / hiérarchie / propagation, pas de filtres custom,
-- pas de format personnalisé, pas de millisecondes / UTC dans
-- l'horodatage, pas de détection isatty.

local logging        = {
    _VERSION = "luapilot logging 1.0.0",
    _DESCRIPTION = "leveled logger (pure Lua, bundled)",
}

-- Niveaux exposés sous forme de constantes numériques. Le filtrage
-- compare >= ; le saut de 10 laisse de la place pour des niveaux
-- intermédiaires ajoutés plus tard sans changer le contrat numérique.
logging.TRACE        = 10
logging.DEBUG        = 20
logging.INFO         = 30
logging.WARN         = 40
logging.ERROR        = 50

local LEVEL_BY_NAME  = {
    trace = logging.TRACE,
    debug = logging.DEBUG,
    info  = logging.INFO,
    warn  = logging.WARN,
    error = logging.ERROR,
}

-- Étiquettes paddées à 5 caractères pour que les colonnes restent
-- droites. info et warn ont un espace de padding final.
local LABEL          = {
    [logging.TRACE] = "TRACE",
    [logging.DEBUG] = "DEBUG",
    [logging.INFO]  = "INFO ",
    [logging.WARN]  = "WARN ",
    [logging.ERROR] = "ERROR",
}

-- Codes ANSI ; info volontairement vide (neutre, par décision).
local COLOR_ON       = {
    [logging.TRACE] = "\27[2m",  -- dim (gris)
    [logging.DEBUG] = "\27[36m", -- cyan
    [logging.INFO]  = "",        -- pas de couleur (décision G)
    [logging.WARN]  = "\27[33m", -- jaune
    [logging.ERROR] = "\27[31m", -- rouge
}
local COLOR_RESET    = "\27[0m"

-- État du module (variables locales au require, donc privées).
local current_level  = logging.INFO -- seuil par défaut
local current_output = io.stderr    -- destination par défaut
local color_enabled  = false        -- OFF par défaut, opt-in

-- --- helpers internes ------------------------------------------------

-- Normalise un argument de niveau : accepte la string ("info") ou le
-- nombre (logging.INFO). Renvoie (nbr, nil) ou (nil, err_msg).
local function normalize_level(lvl)
    local t = type(lvl)
    if t == "number" then
        -- accepte n'importe quel entier ; on n'impose pas que ce soit
        -- l'un des cinq constants (utile si on étend dans le futur)
        return lvl, nil
    end
    if t == "string" then
        local n = LEVEL_BY_NAME[lvl:lower()]
        if n then
            return n, nil
        end
        return nil, "logging: unknown level name '" .. lvl .. "'"
    end
    return nil, "logging: level must be a number or string, got "
        .. t
end

-- Concatène les arguments variadiques comme `print` : séparateur
-- espace, conversion via tostring. select('#', ...) pour gérer les
-- nil au milieu sans les perdre.
local function format_message(...)
    local n = select("#", ...)
    if n == 0 then
        return ""
    end
    local parts = {}
    for i = 1, n do
        parts[i] = tostring(select(i, ...))
    end
    return table.concat(parts, " ")
end

-- Construit la ligne complète et l'écrit. Toute exception d'écriture
-- est silencieusement avalée (cf. contrat d'erreur).
local function emit(level, msg)
    local prefix = os.date("%Y-%m-%d %H:%M:%S") .. " ["
        .. LABEL[level] .. "] "
    local line
    if color_enabled then
        local c = COLOR_ON[level]
        if c and c ~= "" then
            line = c .. prefix .. msg .. COLOR_RESET .. "\n"
        else
            line = prefix .. msg .. "\n"
        end
    else
        line = prefix .. msg .. "\n"
    end
    -- pcall : ni une exception d'écriture (disque plein, handle
    -- fermé) ni une convert tostring qui aurait pu raté plus haut ne
    -- doit traverser ; le log ne plante jamais le programme.
    pcall(function() current_output:write(line) end)
end

-- --- API publique ----------------------------------------------------

-- Crée chaque méthode de niveau via une fabrique pour ne pas dupliquer
-- la mécanique (filtrage + format + emit).
local function make_log_fn(level)
    return function(...)
        if level < current_level then
            return
        end
        local msg = format_message(...)
        emit(level, msg)
    end
end

logging.trace = make_log_fn(logging.TRACE)
logging.debug = make_log_fn(logging.DEBUG)
logging.info  = make_log_fn(logging.INFO)
logging.warn  = make_log_fn(logging.WARN)
logging.error = make_log_fn(logging.ERROR)

-- set_level : accepte "info" ou logging.INFO. error() sur entrée
-- invalide (mauvais usage du programmeur).
function logging.set_level(lvl)
    local n, err = normalize_level(lvl)
    if not n then
        error(err, 2)
    end
    current_level = n
end

-- set_output : accepte tout objet répondant à :write. On valide
-- structurellement la présence de la méthode plutôt que de tester un
-- type strict (Lua = duck-typing).
function logging.set_output(out)
    if type(out) ~= "table" and type(out) ~= "userdata" then
        error("logging: output must be a writable object (table or "
            .. "userdata with :write), got " .. type(out), 2)
    end
    -- Pour table : doit avoir un champ write callable. Pour userdata
    -- (typiquement io.stderr) : on suppose qu'il a :write — un objet
    -- de fichier Lua standard l'a toujours, et tester via pcall
    -- consommerait un write factice. On accepte donc tout userdata.
    if type(out) == "table" and type(out.write) ~= "function" then
        error("logging: output table must have a :write method", 2)
    end
    current_output = out
end

-- set_color : OFF par défaut, opt-in explicite. error() sur type
-- incorrect (les valeurs "truthy" Lua acceptées au-delà de bool
-- masqueraient des bugs).
function logging.set_color(on)
    if type(on) ~= "boolean" then
        error("logging: set_color expects a boolean, got " .. type(on), 2)
    end
    color_enabled = on
end

-- Getters utiles (additifs, faible coût, aident les tests et le
-- diagnostic d'un script qui veut savoir l'état courant).
function logging.get_level() return current_level end

function logging.get_output() return current_output end

function logging.get_color() return color_enabled end

return logging
