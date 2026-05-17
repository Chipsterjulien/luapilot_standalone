-- =====================================================================
-- LuaPilot — self-test harness
-- Couvre l'intégralité de l'API uniformisée (convention result, err).
-- Lancé via : ./luapilot <dir>  (le binaire exécute ce main.lua)
-- =====================================================================

local inspect = require("inspect")

-- --- compteurs et helpers -------------------------------------------
local pass, fail = 0, 0

local function ok(name, cond, detail)
    if cond then
        pass = pass + 1
        print("[PASS] " .. name)
    else
        fail = fail + 1
        print("[FAIL] " .. name .. (detail and ("  -> " .. tostring(detail)) or ""))
    end
end

-- (value, err) attendu en succès : value non-nil, err nil
local function ok_val(name, val, err, validator)
    local good = (err == nil) and (val ~= nil)
    if good and validator then good = validator(val) end
    ok(name, good, "val=" .. tostring(val) .. " err=" .. tostring(err))
end

-- (true, nil) attendu en succès d'action
local function ok_act(name, res, err)
    ok(name, res == true and err == nil,
        "res=" .. tostring(res) .. " err=" .. tostring(err))
end

-- (nil, "msg") attendu en échec
local function ok_fail(name, val, err)
    ok(name, val == nil and type(err) == "string",
        "val=" .. tostring(val) .. " err=" .. tostring(err))
end

-- --- setup -----------------------------------------------------------
local startDir, sderr = luapilot.currentDir()
if not startDir then
    print("FATAL: currentDir() a échoué: " .. tostring(sderr))
    os.exit(1)
end

local SB = "_luapilot_selftest"
local function sb(name) return SB .. "/" .. name end

-- nettoyage préventif d'un éventuel run précédent
luapilot.rmdirAll(SB)

print("=== setup ===")
ok_act("mkdir(sandbox)", luapilot.mkdir(SB))

-- =====================================================================
print("")
print("=== predicates ===")

do
    luapilot.touch(sb("probe.txt"))

    local v, e = luapilot.fileExists(sb("probe.txt"))
    ok("fileExists(probe.txt) == true", v == true and e == nil)

    v, e = luapilot.fileExists(sb("nope.txt"))
    ok("fileExists(nope.txt) == false, err nil", v == false and e == nil)

    v, e = luapilot.isdir(SB)
    ok("isdir(sandbox) == true", v == true and e == nil)

    v, e = luapilot.isdir(sb("probe.txt"))
    ok("isdir(probe.txt) == false", v == false and e == nil)

    v, e = luapilot.isfile(sb("probe.txt"))
    ok("isfile(probe.txt) == true", v == true and e == nil)

    v, e = luapilot.isfile(SB)
    ok("isfile(sandbox) == false", v == false and e == nil)

    -- isdir / isfile sur chemin INEXISTANT : doit retourner (false, nil),
    -- pas (nil, err) — c'est une réponse légitime, pas une erreur.
    v, e = luapilot.isdir(sb("absent_dir"))
    ok("isdir(absent) == false, err nil", v == false and e == nil,
        "v=" .. tostring(v) .. " e=" .. tostring(e))

    v, e = luapilot.isfile(sb("absent_file"))
    ok("isfile(absent) == false, err nil", v == false and e == nil,
        "v=" .. tostring(v) .. " e=" .. tostring(e))
end

-- =====================================================================
print("")
print("=== value getters ===")

do
    local v, e = luapilot.currentDir()
    ok_val("currentDir()", v, e, function(x) return type(x) == "string" and #x > 0 end)

    v, e = luapilot.fileSize(sb("probe.txt"))
    ok_val("fileSize(probe.txt)", v, e, function(x) return type(x) == "number" end)

    v, e = luapilot.fileSize(sb("nope.txt"))
    ok_fail("fileSize(nope.txt) -> (nil, err)", v, e)

    v, e = luapilot.getBasename("/a/b/c.txt")
    ok("getBasename == 'c.txt'", v == "c.txt" and e == nil, "val=" .. tostring(v))

    v, e = luapilot.getExtension("/a/b/c.txt")
    ok("getExtension == '.txt'", v == ".txt" and e == nil, "val=" .. tostring(v))

    v, e = luapilot.getFilename("/a/b/c.txt")
    ok("getFilename == 'c'", v == "c" and e == nil, "val=" .. tostring(v))

    v, e = luapilot.getPath("/a/b/c.txt")
    ok("getPath == '/a/b'", v == "/a/b" and e == nil, "val=" .. tostring(v))

    v = luapilot.joinPath("a", "b", "c")
    ok("joinPath('a','b','c')", type(v) == "string" and #v > 0, "val=" .. tostring(v))

    v, e = luapilot.listFiles(SB)
    ok_val("listFiles(sandbox)", v, e, function(x) return type(x) == "table" end)

    v, e = luapilot.listFiles("/n/existe/pas")
    ok_fail("listFiles(bad) -> (nil, err)", v, e)

    v, e = luapilot.getMode(sb("probe.txt"))
    ok_val("getMode(probe.txt)", v, e, function(x) return type(x) == "number" end)

    v, e = luapilot.getMode("/n/existe/pas")
    ok_fail("getMode(bad) -> (nil, err)", v, e)

    v, e = luapilot.getAttributes(sb("probe.txt"))
    ok_val("getAttributes(probe.txt)", v, e, function(x)
        return type(x) == "table" and x.mode and x.owner and x.group
    end)

    v, e = luapilot.getAttributes("/n/existe/pas")
    ok_fail("getAttributes(bad) -> (nil, err)", v, e)
end

-- =====================================================================
print("=== hashes ===")

do
    local hashers = {
        "md5sum", "sha1sum", "sha256sum", "sha512sum",
        "sha384sum",
        "sha3_256sum", "sha3_384sum", "sha3_512sum",
        "blake2b512sum", "blake2s256sum",
    }
    for _, fn in ipairs(hashers) do
        local v, e = luapilot[fn](sb("probe.txt"))
        ok_val(fn .. "(probe.txt)", v, e,
            function(x) return type(x) == "string" and #x > 0 end)

        v, e = luapilot[fn]("/n/existe/pas")
        ok_fail(fn .. "(bad) -> (nil, err)", v, e)
    end
end

-- =====================================================================
print("")
print("=== json ===")

do
    local J = luapilot.json

    -- --- aller-retour des scalaires --------------------------------
    do
        local s, e = J.encode(42)
        ok("encode(42) == '42'", s == "42" and e == nil, "s=" .. tostring(s))

        s, e = J.encode(3.0)
        ok("encode(3.0) == '3.0' (float préservé)", s == "3.0" and e == nil,
            "s=" .. tostring(s))

        s, e = J.encode("héllo")
        ok("encode(string) ok", e == nil and type(s) == "string")

        s, e = J.encode(true)
        ok("encode(true) == 'true'", s == "true" and e == nil)
    end

    -- --- sous-type entier vs flottant à la décode -------------------
    do
        local v, e = J.decode("42")
        ok("decode('42') -> integer", e == nil and math.type(v) == "integer",
            "type=" .. tostring(math.type(v)))

        v, e = J.decode("42.0")
        ok("decode('42.0') -> float", e == nil and math.type(v) == "float",
            "type=" .. tostring(math.type(v)))

        v, e = J.decode("1e3")
        ok("decode('1e3') -> float", e == nil and math.type(v) == "float")
    end

    -- --- table vide vs empty_array ---------------------------------
    do
        local s, e = J.encode({})
        ok("encode({}) == '{}' (objet par défaut)", s == "{}" and e == nil,
            "s=" .. tostring(s))

        s, e = J.encode(J.empty_array)
        ok("encode(empty_array) == '[]'", s == "[]" and e == nil,
            "s=" .. tostring(s))

        s, e = J.encode({ items = J.empty_array })
        local back = J.decode(s)
        ok("{ items = empty_array } -> [] dans le JSON",
            e == nil and type(back) == "table"
            and type(back.items) == "table" and #back.items == 0,
            "s=" .. tostring(s))
    end

    -- --- array / object --------------------------------------------
    do
        local s, e = J.encode({ 10, 20, 30 })
        ok("encode séquence -> array", s == "[10,20,30]" and e == nil,
            "s=" .. tostring(s))

        local v
        v, e = J.decode("[1,2,3]")
        ok("decode array -> séquence",
            e == nil and v[1] == 1 and v[3] == 3 and #v == 3)

        s, e = J.encode({ a = 1 })
        ok("encode objet ok", e == nil and s:find('"a"', 1, true) ~= nil,
            "s=" .. tostring(s))
    end

    -- --- null : aller-retour sans perte ----------------------------
    do
        local v, e = J.decode("null")
        ok("decode('null') == json.null", v == J.null and e == nil)

        local s
        s, e = J.encode(J.null)
        ok("encode(json.null) == 'null'", s == "null" and e == nil,
            "s=" .. tostring(s))

        -- la clé doit survivre (un nil Lua l'effacerait)
        s, e = J.encode({ x = J.null })
        local back = J.decode(s)
        ok("{ x = json.null } : clé préservée à l'aller-retour",
            e == nil and back ~= nil and back.x == J.null,
            "s=" .. tostring(s))
    end

    -- --- imbrication + pretty-print --------------------------------
    do
        local doc = { user = { name = "ada", tags = { "x", "y" } }, ok = true }
        local s, e = J.encode(doc)
        local back = J.decode(s)
        ok("aller-retour imbriqué",
            e == nil and back.user.name == "ada"
            and back.user.tags[2] == "y" and back.ok == true)

        s, e = J.encode(doc, { indent = 2 })
        ok("encode(opts.indent=2) -> pretty (contient des sauts de ligne)",
            e == nil and type(s) == "string" and s:find("\n", 1, true) ~= nil)

        s, e = J.encode(doc, { indent = -1 })
        ok("indent négatif -> compact (pas de saut de ligne)",
            e == nil and s:find("\n", 1, true) == nil)
    end

    -- --- UTF-8 conservé --------------------------------------------
    do
        local original = "café ☕ — naïve"
        local s = J.encode(original)
        local v, e = J.decode(s)
        ok("aller-retour UTF-8 intact", e == nil and v == original,
            "v=" .. tostring(v))
    end

    -- --- erreurs : (nil, err), jamais d'exception Lua --------------
    do
        local v, e = J.encode({ [1] = "a", foo = "b" })
        ok_fail("encode clés mixtes -> (nil, err)", v, e)

        v, e = J.encode({ [1] = "a", [3] = "c" })
        ok_fail("encode array à trous -> (nil, err)", v, e)

        v, e = J.encode(0 / 0)
        ok_fail("encode(NaN) -> (nil, err)", v, e)

        v, e = J.encode(math.huge)
        ok_fail("encode(Infinity) -> (nil, err)", v, e)

        v, e = J.encode(print)
        ok_fail("encode(function) -> (nil, err)", v, e)

        v, e = J.decode("{ pas du json")
        ok_fail("decode(JSON invalide) -> (nil, err)", v, e)

        -- table cyclique : captée par le garde-fou de profondeur
        local cyc = {}
        cyc.me = cyc
        v, e = J.encode(cyc)
        ok_fail("encode(table cyclique) -> (nil, err)", v, e)
    end

    -- --- aller-retour d'un array vide ------------------------------
    do
        local t, e = J.decode("[]")
        ok("decode('[]') -> table", e == nil and type(t) == "table")

        local s, e2 = J.encode(t)
        ok("re-encode(decode('[]')) == '[]'", s == "[]" and e2 == nil,
            "s=" .. tostring(s))

        -- la table décodée reste mutable : on peut la remplir
        t[1] = "x"
        s = J.encode(t)
        ok("array vide décodé puis rempli -> array JSON",
            s == '["x"]', "s=" .. tostring(s))

        -- array vide imbriqué : aller-retour préservé
        local nested = J.decode('{"a":[]}')
        s = J.encode(nested)
        ok('aller-retour {"a":[]} préservé', s == '{"a":[]}',
            "s=" .. tostring(s))
    end

    -- --- sentinels en lecture seule --------------------------------
    do
        local mut_ok = pcall(function() J.null.foo = "bar" end)
        ok("json.null est en lecture seule", mut_ok == false)

        local mut_ok2 = pcall(function() J.empty_array.x = 1 end)
        ok("json.empty_array est en lecture seule", mut_ok2 == false)
    end

    -- --- as_array : forçage array pour tables dynamiques -----------
    do
        -- renvoie la table elle-même (chaînable / idempotent)
        local x = {}
        ok("as_array(t) renvoie la même table", J.as_array(x) == x)

        -- table vide marquée -> []
        local s, e = J.encode(J.as_array({}))
        ok("encode(as_array({})) == '[]'", s == "[]" and e == nil,
            "s=" .. tostring(s))

        -- le cas qui motive la feature : array dynamique resté vide
        local tags = {}
        s, e = J.encode({ tags = J.as_array(tags) })
        ok('as_array : array dynamique vide -> {"tags":[]}',
            s == '{"tags":[]}' and e == nil, "s=" .. tostring(s))

        -- puis rempli -> array normal
        tags[1] = "a"
        s = J.encode({ tags = tags })
        ok('as_array : puis rempli -> {"tags":["a"]}',
            s == '{"tags":["a"]}', "s=" .. tostring(s))

        -- idempotent : double appel sans effet de bord
        local d = J.as_array(J.as_array({}))
        ok("as_array idempotent", J.encode(d) == "[]")

        -- marquée mais avec clé string -> erreur au encode
        local bad = J.as_array({})
        bad.k = "v"
        local v2, e2 = J.encode(bad)
        ok_fail("as_array + clé string -> (nil, err) au encode", v2, e2)

        -- garde-fous : non-table et sentinel refusés
        ok("as_array(42) lève une erreur",
            pcall(function() J.as_array(42) end) == false)
        ok("as_array(json.null) lève une erreur",
            pcall(function() J.as_array(J.null) end) == false)
        ok("as_array(json.empty_array) lève une erreur",
            pcall(function() J.as_array(J.empty_array) end) == false)

        -- récupération : métatable strippée puis re-marquée
        local r = J.decode("[]")
        setmetatable(r, nil)
        ok("après setmetatable(nil), re-encode == '{}'",
            J.encode(r) == "{}")
        ok("as_array recupère le marquage -> '[]'",
            J.encode(J.as_array(r)) == "[]")
    end
end

-- =====================================================================
print("")
print("=== actions: filesystem ===")

do
    -- mkdir / rmdir
    ok_act("mkdir(sandbox/d1)", luapilot.mkdir(sb("d1")))
    ok_act("rmdir(sandbox/d1)", luapilot.rmdir(sb("d1")))

    local r, e = luapilot.rmdir(sb("d1")) -- déjà supprimé
    ok_fail("rmdir(already gone) -> (nil, err)", r, e)

    -- touch / remove
    ok_act("touch(sandbox/t1.txt)", luapilot.touch(sb("t1.txt")))
    ok_act("remove(sandbox/t1.txt)", luapilot.remove(sb("t1.txt")))

    r, e = luapilot.remove(sb("t1.txt")) -- déjà supprimé
    ok_fail("remove(already gone) -> (nil, err)", r, e)

    -- rename
    luapilot.touch(sb("old.txt"))
    ok_act("rename(old.txt -> new.txt)", luapilot.rename(sb("old.txt"), sb("new.txt")))

    r, e = luapilot.rename(sb("old.txt"), sb("x.txt")) -- source absente
    ok_fail("rename(missing source) -> (nil, err)", r, e)

    -- copy
    ok_act("copy(new.txt -> copy.txt)", luapilot.copy(sb("new.txt"), sb("copy.txt")))

    r, e = luapilot.copy(sb("nope.txt"), sb("x.txt"))
    ok_fail("copy(missing source) -> (nil, err)", r, e)

    -- link
    ok_act("link(new.txt -> link.txt)",
        luapilot.link(sb("new.txt"), sb("link.txt")))

    -- copyTree
    luapilot.mkdir(sb("treesrc"))
    luapilot.touch(sb("treesrc/a.txt"))
    ok_act("copyTree(treesrc -> treedst)",
        luapilot.copyTree(sb("treesrc"), sb("treedst")))

    -- moveTree
    luapilot.mkdir(sb("movesrc"))
    luapilot.touch(sb("movesrc/b.txt"))
    luapilot.mkdir(sb("movedst"))
    ok_act("moveTree(movesrc -> movedst)",
        luapilot.moveTree(sb("movesrc"), sb("movedst")))

    -- rmdirAll
    luapilot.mkdir(sb("deeptree"))
    luapilot.mkdir(sb("deeptree/sub"))
    luapilot.touch(sb("deeptree/sub/c.txt"))
    ok_act("rmdirAll(deeptree)", luapilot.rmdirAll(sb("deeptree")))

    -- moveTree fast path : destination INEXISTANTE, rename O(1) atomique
    luapilot.mkdir(sb("movefast"))
    luapilot.touch(sb("movefast/x.txt"))
    ok_act("moveTree(fast path : dest inexistante)",
        luapilot.moveTree(sb("movefast"), sb("movefast_new")))
    -- vérifie que le contenu est bien arrivé à destination
    local v_fast, _ = luapilot.fileExists(sb("movefast_new/x.txt"))
    ok("moveTree fast path : contenu déplacé", v_fast == true)
    -- et que la source a disparu
    local v_src, _ = luapilot.isdir(sb("movefast"))
    ok("moveTree fast path : source disparue", v_src == false)

    -- moveTree échec : source inexistante -> (nil, err)
    local r_mv, e_mv = luapilot.moveTree(sb("n_existe_pas"), sb("dst"))
    ok_fail("moveTree(source inexistante) -> (nil, err)", r_mv, e_mv)

    -- garde-fou : destination dans source doit être refusée
    luapilot.mkdir(sb("nested_src"))
    luapilot.touch(sb("nested_src/file.txt"))

    local r_nest_mv, e_nest_mv = luapilot.moveTree(sb("nested_src"), sb("nested_src/backup"))
    ok_fail("moveTree(destination dans source) -> (nil, err)", r_nest_mv, e_nest_mv)
    ok("  message mentionne 'inside'",
        type(e_nest_mv) == "string" and e_nest_mv:find("inside", 1, true) ~= nil,
        "err=" .. tostring(e_nest_mv))

    local r_nest_cp, e_nest_cp = luapilot.copyTree(sb("nested_src"), sb("nested_src/backup"), false)
    ok_fail("copyTree(destination dans source) -> (nil, err)", r_nest_cp, e_nest_cp)
    ok("  message mentionne 'inside'",
        type(e_nest_cp) == "string" and e_nest_cp:find("inside", 1, true) ~= nil,
        "err=" .. tostring(e_nest_cp))

    -- nettoyage
    luapilot.rmdirAll(sb("nested_src"))

    -- --- durcissement symlinks (résolution réelle des chemins) -----
    -- Tout est confiné sous sb("sym") et nettoyé par UN seul rmdirAll :
    -- remove_all ne suit pas les liens et ne dépend pas de l'ordre, donc
    -- aucun symlink ballant ne peut subsister dans le sandbox et fausser
    -- une section ultérieure (ex. createFileIterator).
    local function mklink(target, linkpath)
        return luapilot.exec("ln", { "-s", target, linkpath })
    end
    local function readlink(p)
        local r = luapilot.exec("readlink", { p })
        return type(r) == "table" and (r.stdout:gsub("%s+$", "")) or nil
    end
    local cwd = luapilot.currentDir()
    local function abs(p) return cwd .. "/" .. p end

    do
        -- racine isolée, repartie de zéro même si un run précédent a coupé
        luapilot.rmdirAll(sb("sym"))
        luapilot.mkdir(sb("sym"))

        -- A) destination atteignant source VIA un symlink -> refus.
        --    Avant durcissement (comparaison lexicale), ce cas passait.
        luapilot.mkdir(sb("sym/hl_src"))
        luapilot.touch(sb("sym/hl_src/f.txt"))
        mklink(abs(sb("sym/hl_src")), sb("sym/hl_link"))

        local r1, e1 = luapilot.copyTree(sb("sym/hl_src"), sb("sym/hl_link/backup"))
        ok_fail("copyTree: dest via symlink vers source -> refus", r1, e1)

        local r2, e2 = luapilot.moveTree(sb("sym/hl_src"), sb("sym/hl_link/backup"))
        ok_fail("moveTree: dest via symlink vers source -> refus", r2, e2)

        -- B) source donnée VIA un symlink + lien absolu intra-source :
        --    doit être retargeté dans la destination. Avant durcissement
        --    (source non résolue), il restait pointé vers la source.
        luapilot.mkdir(sb("sym/rs"))
        luapilot.touch(sb("sym/rs/data.txt"))
        mklink(abs(sb("sym/rs/data.txt")), sb("sym/rs/ptr"))
        mklink(abs(sb("sym/rs")), sb("sym/srcvia"))

        local rc = luapilot.copyTree(sb("sym/srcvia"), sb("sym/viad"))
        ok_act("copyTree(srcvia -> viad)", rc)
        local tgt = readlink(sb("sym/viad/ptr"))
        ok("lien absolu intra-source retargeté vers la destination",
            type(tgt) == "string"
            and tgt:find("viad/data.txt", 1, true) ~= nil
            and tgt:find("/rs/data.txt", 1, true) == nil,
            "tgt=" .. tostring(tgt))

        -- C) lien pointant HORS de source : conservé tel quel.
        luapilot.mkdir(sb("sym/os_src"))
        luapilot.touch(sb("sym/os_src/keep.txt"))
        mklink("/tmp", sb("sym/os_src/outside"))
        local rc2 = luapilot.copyTree(sb("sym/os_src"), sb("sym/os_dst"))
        ok_act("copyTree(os_src -> os_dst)", rc2)
        ok("lien hors-source conservé tel quel",
            readlink(sb("sym/os_dst/outside")) == "/tmp",
            "tgt=" .. tostring(readlink(sb("sym/os_dst/outside"))))

        -- D) lien CASSÉ (cible inexistante) : moveTree le conserve,
        --    sans erreur (décision actée n°2).
        luapilot.mkdir(sb("sym/bk_src"))
        mklink("/n/existe/pas/cible", sb("sym/bk_src/broken"))
        local rm = luapilot.moveTree(sb("sym/bk_src"), sb("sym/bk_dst"))
        ok_act("moveTree avec lien cassé : pas d'erreur", rm)
        ok("lien cassé conservé à l'identique",
            readlink(sb("sym/bk_dst/broken")) == "/n/existe/pas/cible",
            "tgt=" .. tostring(readlink(sb("sym/bk_dst/broken"))))

        -- E) piège du préfixe ".." : un composant "..data" commence par
        --    les caractères ".." sans être le parent. La destination est
        --    bien DANS source -> doit être refusée (avant le fix par
        --    composant, ce cas passait à tort = trou du garde).
        luapilot.mkdir(sb("sym/dd_src"))
        luapilot.touch(sb("sym/dd_src/f.txt"))
        local rdd, edd = luapilot.copyTree(sb("sym/dd_src"), sb("sym/dd_src/..data"))
        ok_fail("copyTree: dest 'src/..data' (dans source) -> refus", rdd, edd)
        local rdm, edm = luapilot.moveTree(sb("sym/dd_src"), sb("sym/dd_src/..data"))
        ok_fail("moveTree: dest 'src/..data' (dans source) -> refus", rdm, edm)

        -- nettoyage : un seul appel, sûr, indépendant de l'ordre
        luapilot.rmdirAll(sb("sym"))
    end
end

-- =====================================================================
print("")
print("=== actions: attributes ===")

do
    -- setMode / getMode : on teste les DEUX formes acceptées
    luapilot.touch(sb("perm.txt"))

    -- forme string (réflexe chmod)
    ok_act("setMode(perm.txt, '700') [string]",
        luapilot.setMode(sb("perm.txt"), "700"))
    local m, e = luapilot.getMode(sb("perm.txt"))
    ok("getMode reflète setMode('700')", m == tonumber("700", 8) and e == nil,
        "mode=" .. tostring(m))

    -- forme nombre
    ok_act("setMode(perm.txt, 0o644) [number]",
        luapilot.setMode(sb("perm.txt"), tonumber("644", 8)))
    m, e = luapilot.getMode(sb("perm.txt"))
    ok("getMode reflète setMode(644)", m == tonumber("644", 8) and e == nil,
        "mode=" .. tostring(m))

    -- string invalide -> (nil, err) propre
    local r, e2 = luapilot.setMode(sb("perm.txt"), "858")
    ok_fail("setMode(perm.txt, '858') -> (nil, err)", r, e2)

    -- setAttributes : chown vers son propre uid/gid (toujours autorisé)
    local attrs = luapilot.getAttributes(sb("perm.txt"))
    if attrs then
        ok_act("setAttributes(perm.txt, self uid/gid)",
            luapilot.setAttributes(sb("perm.txt"), attrs.owner, attrs.group))
    else
        ok("setAttributes (préparation)", false, "getAttributes a échoué")
    end

    local r2, e3 = luapilot.setAttributes("/n/existe/pas", 0, 0)
    ok_fail("setAttributes(bad path) -> (nil, err)", r2, e3)

    -- symlinkattr sur le lien créé dans la section filesystem
    if attrs then
        ok_act("symlinkattr(link.txt, self uid/gid)",
            luapilot.symlinkattr(sb("link.txt"), attrs.owner, attrs.group))
    end
end

-- =====================================================================
print("")
print("=== find ===")

do
    local results, e = luapilot.find(SB, { type = "f" })
    ok_val("find(sandbox, {type='f'})", results, e,
        function(x) return type(x) == "table" end)

    results, e = luapilot.find("/n/existe/pas", { type = "f" })
    ok_fail("find(bad path) -> (nil, err)", results, e)

    -- find avec regex ECMAScript : trouve tous les fichiers .txt
    -- du sandbox (on en a créé plusieurs au fil des tests précédents)
    local results_txt, e_txt = luapilot.find(SB, { type = "f", name = ".*\\.txt$" })
    ok_val("find avec regex ECMAScript (.*\\.txt$)", results_txt, e_txt,
        function(x) return type(x) == "table" and #x > 0 end)

    -- find avec iname (case-insensitive)
    local results_ci, e_ci = luapilot.find(SB, { type = "f", iname = ".*\\.TXT$" })
    ok_val("find iname case-insensitive", results_ci, e_ci,
        function(x) return type(x) == "table" and #x > 0 end)
end

-- =====================================================================
print("")
print("=== createFileIterator ===")

do
    local it, e = luapilot.createFileIterator(SB, true)
    ok_val("createFileIterator(sandbox)", it, e,
        function(x) return x ~= nil end)

    if it then
        local count = 0
        while true do
            local f = it:next()
            if not f then break end
            count = count + 1
        end
        ok("iterator parcourt des fichiers", count > 0, "count=" .. count)
    end

    local it2, e2 = luapilot.createFileIterator("/n/existe/pas")
    ok_fail("createFileIterator(bad) -> (nil, err)", it2, e2)

    -- :close() puis :next() doit lever une erreur Lua propre
    local it3 = luapilot.createFileIterator(SB)
    if it3 then
        it3:close()
        local raised = not pcall(function() it3:next() end)
        ok("iterator :next() après :close() lève une erreur", raised)
    end
end

-- =====================================================================
print("")
print("=== require() : module en sous-dossier ===")

do
    -- mymod/init.lua doit être trouvé via require("mymod"),
    -- aussi bien en mode dossier qu'en mode embarqué.
    local loaded, mod = pcall(require, "mymod")
    ok("require('mymod') ne lève pas d'erreur", loaded,
        loaded and "" or tostring(mod))

    if loaded then
        ok("mymod.hello() retourne la bonne valeur",
            type(mod) == "table" and mod.hello and mod.hello() == "init.lua chargé !",
            mod and mod.hello and tostring(mod.hello()) or "table/fonction absente")
    end

    -- inspect est bundlé en dur dans le binaire : doit marcher même
    -- sans inspect.lua sur le disque.
    -- Note : inspect est une TABLE appelable (métatable __call),
    -- pas une fonction — donc on teste son appelabilité, pas son type.
    local ok_callable = pcall(function() return inspect("test") end)
    ok("inspect (bundlé) est chargé et appelable", ok_callable)

    local repr = inspect({ a = 1, b = "deux" })
    ok("inspect({a=1, b='deux'}) renvoie une string non vide",
        type(repr) == "string" and #repr > 0,
        "len=" .. tostring(#repr))
end

-- =====================================================================
print("")
print("=== exec ===")

do
    -- commande simple qui réussit
    local r, e = luapilot.exec("echo", { "hello" })
    ok("exec('echo hello') -> (table, nil)",
        type(r) == "table" and e == nil,
        "r=" .. tostring(r) .. " e=" .. tostring(e))
    if type(r) == "table" then
        ok("  stdout contient 'hello'",
            type(r.stdout) == "string" and r.stdout:find("hello", 1, true) ~= nil,
            "stdout=" .. tostring(r.stdout))
        ok("  code == 0", r.code == 0, "code=" .. tostring(r.code))
        ok("  stderr vide", r.stderr == "", "stderr=" .. tostring(r.stderr))
    end

    -- code de sortie != 0 : ce n'est PAS une erreur d'exec
    local r2 = luapilot.exec("sh", { "-c", "exit 3" })
    ok("exec('sh -c \"exit 3\"'): code == 3, pas d'err",
        type(r2) == "table" and r2.code == 3,
        "code=" .. tostring(r2 and r2.code))

    -- stderr capturé séparément de stdout
    local r3 = luapilot.exec("sh", { "-c", "echo oops 1>&2" })
    ok("exec: stderr capturé séparément de stdout",
        type(r3) == "table"
        and r3.stderr:find("oops", 1, true) ~= nil
        and r3.stdout == "",
        "stdout=" .. tostring(r3 and r3.stdout) ..
        " stderr=" .. tostring(r3 and r3.stderr))

    -- binaire introuvable -> (nil, err)
    local r4, e4 = luapilot.exec("ce_binaire_nexiste_pas_12345")
    ok_fail("exec(binaire inexistant) -> (nil, err)", r4, e4)

    -- option cwd
    local r5 = luapilot.exec("pwd", {}, { cwd = "/tmp" })
    ok("exec('pwd', cwd=/tmp): stdout contient /tmp",
        type(r5) == "table" and r5.stdout:find("/tmp", 1, true) ~= nil,
        "stdout=" .. tostring(r5 and r5.stdout))

    -- cwd invalide -> (nil, err)
    local r6, e6 = luapilot.exec("pwd", {}, { cwd = "/n/existe/pas" })
    ok_fail("exec(cwd invalide) -> (nil, err)", r6, e6)

    -- option env (fusionnée avec l'environnement existant)
    local r7 = luapilot.exec("sh", { "-c", "echo $LUAPILOT_TEST_VAR" },
        { env = { LUAPILOT_TEST_VAR = "ok42" } })
    ok("exec: variable d'environnement transmise",
        type(r7) == "table" and r7.stdout:find("ok42", 1, true) ~= nil,
        "stdout=" .. tostring(r7 and r7.stdout))

    -- grosse sortie : vérifie l'absence de deadlock (drain avant waitpid)
    local r8 = luapilot.exec("sh",
        { "-c", "for i in $(seq 1 50000); do echo line$i; done" })
    ok("exec: grosse sortie sans deadlock",
        type(r8) == "table" and r8.code == 0 and #r8.stdout > 100000,
        "len=" .. tostring(r8 and r8.stdout and #r8.stdout))

    -- stdin : transmis au process
    local r9 = luapilot.exec("cat", {}, { stdin = "contenu via stdin" })
    ok("exec('cat', stdin=...): stdout == stdin",
        type(r9) == "table" and r9.stdout == "contenu via stdin",
        "stdout=" .. tostring(r9 and r9.stdout))

    -- stdin consommé par un filtre
    local r10 = luapilot.exec("grep", { "foo" },
        { stdin = "ligne1\nfoo bar\nligne3\nfoo encore\n" })
    ok("exec('grep foo', stdin=...): filtre 2 lignes",
        type(r10) == "table"
        and select(2, r10.stdout:gsub("\n", "\n")) == 2,
        "stdout=" .. tostring(r10 and r10.stdout))

    -- gros stdin : pas de deadlock (on écrit pendant qu'on lit)
    local big = string.rep("x", 500000)
    local r11 = luapilot.exec("cat", {}, { stdin = big })
    ok("exec: gros stdin sans deadlock",
        type(r11) == "table" and #r11.stdout == 500000,
        "len=" .. tostring(r11 and r11.stdout and #r11.stdout))

    -- stdin envoyé à un process qui ne le lit pas (EPIPE géré, pas de crash)
    local r12 = luapilot.exec("true", {}, { stdin = "ignoré" })
    ok("exec: stdin vers process qui l'ignore (EPIPE géré)",
        type(r12) == "table" and r12.code == 0,
        "code=" .. tostring(r12 and r12.code))

    -- timeout : un process qui dépasse est arrêté
    local r13 = luapilot.exec("sleep", { "10" }, { timeout = 0.5 })
    ok("exec('sleep 10', timeout=0.5): timed_out == true",
        type(r13) == "table" and r13.timed_out == true,
        "timed_out=" .. tostring(r13 and r13.timed_out) ..
        " code=" .. tostring(r13 and r13.code))

    -- un process rapide n'est PAS marqué timed_out
    local r14 = luapilot.exec("echo", { "vite" }, { timeout = 5 })
    ok("exec('echo', timeout=5): timed_out == false",
        type(r14) == "table" and r14.timed_out == false and r14.code == 0,
        "timed_out=" .. tostring(r14 and r14.timed_out))

    -- la sortie produite AVANT le timeout est bien récupérée
    local r15 = luapilot.exec("sh",
        { "-c", "echo avant_timeout; sleep 10" }, { timeout = 0.5 })
    ok("exec: stdout d'avant le timeout est conservé",
        type(r15) == "table"
        and r15.timed_out == true
        and r15.stdout:find("avant_timeout", 1, true) ~= nil,
        "stdout=" .. tostring(r15 and r15.stdout))

    -- timeout invalide -> (nil, err)
    local r16, e16 = luapilot.exec("echo", { "x" }, { timeout = -1 })
    ok_fail("exec(timeout négatif) -> (nil, err)", r16, e16)

    -- sans timeout, le champ timed_out existe et vaut false
    local r17 = luapilot.exec("echo", { "x" })
    ok("exec sans timeout: timed_out == false",
        type(r17) == "table" and r17.timed_out == false,
        "timed_out=" .. tostring(r17 and r17.timed_out))

    -- process tué par signal (le shell se kill lui-même avec SIGKILL)
    -- Convention shell : code de sortie = 128 + numéro du signal
    -- SIGKILL = 9 -> code attendu = 137
    local r_sig = luapilot.exec("sh", { "-c", "kill -9 $$" })
    ok("exec: process tué par signal -> code = 128 + signal",
        type(r_sig) == "table" and r_sig.code == 137,
        "code=" .. tostring(r_sig and r_sig.code))

    -- max_output : sortie coupée aux PREMIERS octets + flag séparé, le
    -- process est mené à terme (on draine le reste), pas de deadlock.
    local r_mo = luapilot.exec("sh",
        { "-c", "for i in $(seq 1 100000); do echo AAAAAAAA; done" },
        { max_output = 1024 })
    ok("exec max_output: stdout coupé à la limite",
        type(r_mo) == "table" and #r_mo.stdout == 1024,
        "len=" .. tostring(r_mo and r_mo.stdout and #r_mo.stdout))
    ok("exec max_output: stdout_truncated == true",
        type(r_mo) == "table" and r_mo.stdout_truncated == true)
    ok("exec max_output: stderr_truncated == false (stderr vide)",
        type(r_mo) == "table" and r_mo.stderr_truncated == false)
    ok("exec max_output: process termine normalement (code 0)",
        type(r_mo) == "table" and r_mo.code == 0,
        "code=" .. tostring(r_mo and r_mo.code))

    -- max_output invalide -> (nil, err)
    local r_mo_bad, e_mo_bad = luapilot.exec("echo", { "x" },
        { max_output = 0 })
    ok_fail("exec(max_output <= 0) -> (nil, err)", r_mo_bad, e_mo_bad)

    local r_mo_frac, e_mo_frac = luapilot.exec("echo", { "x" },
        { max_output = 3.7 })
    ok_fail("exec(max_output non entier) -> (nil, err)", r_mo_frac, e_mo_frac)

    local r_mo_huge, e_mo_huge = luapilot.exec("echo", { "x" },
        { max_output = 3 * 1024 * 1024 * 1024 })
    ok_fail("exec(max_output > 2 Gio) -> (nil, err)", r_mo_huge, e_mo_huge)

    -- sans max_output : flags de troncature présents et à false
    local r_mo_def = luapilot.exec("echo", { "court" })
    ok("exec sans max_output: flags de troncature à false",
        type(r_mo_def) == "table"
        and r_mo_def.stdout_truncated == false
        and r_mo_def.stderr_truncated == false)

    -- timeout : tout le GROUPE est tué, y compris un petit-enfant en
    -- arrière-plan qui garde le pipe ouvert. Sans groupe de process,
    -- exec resterait bloqué ~30s ; avec, il rend la main vite.
    local t0 = os.time()
    local r_pg = luapilot.exec("sh",
        { "-c", "sleep 30 & wait" }, { timeout = 0.5 })
    local dt = os.time() - t0
    ok("exec timeout tue tout le groupe (pas de hang petit-enfant)",
        type(r_pg) == "table" and r_pg.timed_out == true and dt < 10,
        "dt=" .. tostring(dt) .. "s timed_out=" ..
        tostring(r_pg and r_pg.timed_out))
end

-- =====================================================================
print("")
print("=== deepCopyTable ===")

do
    -- copie profonde de base : indépendance des sous-tables
    local original = { x = 1, nested = { y = 2 } }
    local copy = luapilot.deepCopyTable(original)
    ok("deepCopyTable : sous-table est une vraie copie",
        type(copy) == "table"
        and copy.nested ~= nil
        and copy.nested ~= original.nested
        and copy.nested.y == 2)

    -- indépendance : modifier la copie n'affecte pas l'original
    copy.nested.y = 999
    ok("deepCopyTable : modifier la copie n'affecte pas l'original",
        original.nested.y == 2,
        "original.nested.y = " .. tostring(original.nested.y))

    -- clés numériques TROUÉES : [10] ne doit pas être perdue
    local sparse = { [1] = "A", [10] = "B" }
    local sparse_copy = luapilot.deepCopyTable(sparse)
    ok("deepCopyTable : clé numérique trouée [10] préservée",
        sparse_copy[1] == "A" and sparse_copy[10] == "B",
        "[1]=" .. tostring(sparse_copy[1]) ..
        " [10]=" .. tostring(sparse_copy[10]))

    -- clés numériques NON basées sur 1 : pas de renumérotation
    local offset = { [5] = "x", [6] = "y" }
    local offset_copy = luapilot.deepCopyTable(offset)
    ok("deepCopyTable : clés numériques non renumérotées",
        offset_copy[5] == "x" and offset_copy[6] == "y"
        and offset_copy[1] == nil,
        "[1]=" .. tostring(offset_copy[1]) ..
        " [5]=" .. tostring(offset_copy[5]))

    -- clés mixtes (string + numérique) toutes préservées
    local mixed = { name = "test", [1] = "premier", [3] = "troisieme" }
    local mixed_copy = luapilot.deepCopyTable(mixed)
    ok("deepCopyTable : clés mixtes string+numériques préservées",
        mixed_copy.name == "test"
        and mixed_copy[1] == "premier"
        and mixed_copy[3] == "troisieme")

    -- cycle : table qui se référence elle-même
    local cyclic = {}
    cyclic.self = cyclic
    local cyclic_copy = luapilot.deepCopyTable(cyclic)
    ok("deepCopyTable : cycle auto-référent géré",
        type(cyclic_copy) == "table"
        and cyclic_copy.self == cyclic_copy -- pointe vers la COPIE
        and cyclic_copy.self ~= cyclic,     -- pas vers l'original
        "self == copy ? " .. tostring(cyclic_copy.self == cyclic_copy))

    -- sous-table partagée : doit être copiée UNE seule fois
    local shared = { value = 42 }
    local container = { a = shared, b = shared }
    local container_copy = luapilot.deepCopyTable(container)
    ok("deepCopyTable : sous-table partagée copiée une seule fois",
        container_copy.a == container_copy.b -- même copie réutilisée
        and container_copy.a ~= shared       -- mais pas l'original
        and container_copy.a.value == 42)

    -- cycle indirect : a -> b -> a
    local a = {}
    local b = { back = a }
    a.forward = b
    local a_copy = luapilot.deepCopyTable(a)
    ok("deepCopyTable : cycle indirect (a->b->a) géré",
        type(a_copy) == "table"
        and type(a_copy.forward) == "table"
        and a_copy.forward.back == a_copy, -- referme la boucle sur la copie
        "boucle refermée ? " ..
        tostring(a_copy.forward and a_copy.forward.back == a_copy))
end

-- =====================================================================
print("")
print("=== fonctions pures ===")

do
    -- split
    local parts = luapilot.split("Hello there !", " ")
    ok("split('Hello there !', ' ') -> 3 éléments",
        type(parts) == "table" and #parts == 3,
        parts and ("#=" .. #parts) or "nil")

    parts = luapilot.split("a,b,c", ",")
    ok("split('a,b,c', ',') -> 3 éléments",
        type(parts) == "table" and #parts == 3)

    -- mergeTables
    local merged = luapilot.mergeTables({ "a", "b" }, { "c", "d" })
    ok("mergeTables -> table", type(merged) == "table")

    -- getMemoryUsage
    local mem = luapilot.getMemoryUsage()
    ok("getMemoryUsage() -> nombre > 0", type(mem) == "number" and mem > 0,
        "mem=" .. tostring(mem))

    local used, total = luapilot.getDetailedMemoryUsage()
    ok("getDetailedMemoryUsage() -> 2 nombres",
        type(used) == "number" and type(total) == "number")

    -- helloThere : imprime, ne retourne rien, ne doit pas planter
    local hello_ok = pcall(luapilot.helloThere)
    ok("helloThere() ne plante pas", hello_ok)

    -- sleep : action courte
    ok_act("sleep(1, 'ms')", luapilot.sleep(1, "ms"))
end

-- =====================================================================
print("")
print("=== arg ===")

do
    ok("arg: table globale présente", type(arg) == "table")
    ok("arg[0] est une chaîne",
        type(arg) == "table" and type(arg[0]) == "string",
        "arg[0]=" .. tostring(arg and arg[0]))

    if arg and arg[1] == "__ARG__a" then
        -- Invocation via run_tests.sh (sentinelles connues) : on
        -- vérifie la convention complète, de façon déterministe et
        -- identique dans les 3 modes (les args utilisateur tombent
        -- toujours en 1..n, que le binaire soit packagé ou runner).
        ok("arg[1] == sentinelle 1", arg[1] == "__ARG__a")
        ok("arg[2] == sentinelle 2", arg[2] == "__ARG__b",
            "arg[2]=" .. tostring(arg[2]))
        ok("arg[3] == nil (rien au-delà des sentinelles)",
            arg[3] == nil, "arg[3]=" .. tostring(arg[3]))

        if arg[-1] ~= nil then
            -- Runner de dossier : arg[-1]=binaire luapilot, arg[0]=<dir>
            ok("mode dossier: arg[-1] (binaire) est une chaîne",
                type(arg[-1]) == "string",
                "arg[-1]=" .. tostring(arg[-1]))
            ok("mode dossier: arg[0] == '.' (dossier lancé)",
                arg[0] == ".", "arg[0]=" .. tostring(arg[0]))
        else
            -- Packagé : arg[0]=binaire, aucun indice négatif.
            ok("mode packagé: arg[0] (binaire) non vide",
                #arg[0] > 0, "arg[0]=" .. tostring(arg[0]))
            ok("mode packagé: aucun indice -1", arg[-1] == nil)
        end
    else
        -- Run manuel hors run_tests.sh : positions non connues, on
        -- s'en tient aux invariants de structure (déjà testés ci-dessus).
        print("[INFO] arg: run hors run_tests.sh, "
            .. "vérifications positionnelles ignorées")
    end
end

-- =====================================================================
print("")
print("=== chdir (en dernier : modifie le cwd) ===")

do
    local r, e = luapilot.chdir(SB)
    ok_act("chdir(sandbox)", r, e)

    local cwd = luapilot.currentDir()
    ok("currentDir() reflète le chdir",
        type(cwd) == "string" and cwd:find(SB, 1, true) ~= nil,
        "cwd=" .. tostring(cwd))

    r, e = luapilot.chdir("/n/existe/pas")
    ok_fail("chdir(bad path) -> (nil, err)", r, e)

    -- retour au répertoire de départ
    r, e = luapilot.chdir(startDir)
    ok_act("chdir(retour startDir)", r, e)
end

-- =====================================================================
print("")
print("=== teardown ===")
ok_act("rmdirAll(sandbox)", luapilot.rmdirAll(SB))

-- =====================================================================
print("")
print("==========================================")
print(string.format("Résultat : %d PASS / %d FAIL", pass, fail))
print("==========================================")

if fail > 0 then
    os.exit(1)
end
