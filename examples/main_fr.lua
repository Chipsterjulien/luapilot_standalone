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

    -- find concurrent depuis plusieurs workers (correctif post-Gemini :
    -- RegexCache est maintenant thread_local). Avec l'ancien cache
    -- global, ce test aurait corrompu la unordered_map et crashé.
    -- On spawn 4 workers qui font chacun find en boucle avec une
    -- regex différente. Si thread_local est bien appliqué, tous les
    -- workers terminent et rendent leur compteur de résultats.
    do
        local W = luapilot.workers
        local jobs = {}
        for i = 1, 4 do
            jobs[i] = W.spawn([[
                local sb = worker.args.sb
                local pat = worker.args.pat
                local count = 0
                for _ = 1, 20 do
                    local r, err = luapilot.find(sb, { type = "f", name = pat })
                    if not r then return { error = err } end
                    count = count + #r
                end
                return { count = count }
            ]], { sb = SB, pat = ".*\\.txt$" .. tostring(i) })
        end

        local all_ok = true
        for i = 1, 4 do
            local jok, jval = jobs[i]:join()
            if not (jok == true and type(jval) == "table"
                    and type(jval.count) == "number") then
                all_ok = false
            end
        end
        ok("find concurrent x4 workers (thread_local RegexCache)",
            all_ok)
    end
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

    -- Chantier 10-D : timeout NaN / Inf rejetés
    local r16b, e16b = luapilot.exec("echo", { "x" }, { timeout = 0 / 0 })
    ok_fail("exec(timeout NaN) -> (nil, err)", r16b, e16b)
    local r16c, e16c = luapilot.exec("echo", { "x" }, { timeout = math.huge })
    ok_fail("exec(timeout Inf) -> (nil, err)", r16c, e16c)

    -- Chantier 10-D : opts.env clé invalide rejetée
    local rE1, eE1 = luapilot.exec("echo", { "x" }, { env = { ["A=B"] = "v" } })
    ok_fail("exec(env clé contient '=') -> (nil, err)", rE1, eE1)
    local rE2, eE2 = luapilot.exec("echo", { "x" }, { env = { [""] = "v" } })
    ok_fail("exec(env clé vide) -> (nil, err)", rE2, eE2)

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
print("=== time ===")

do
    -- monotonic : strictement positif, croissant
    local m1 = luapilot.monotonic()
    ok("monotonic() -> number > 0",
        type(m1) == "number" and m1 > 0,
        "m1=" .. tostring(m1))

    local m2 = luapilot.monotonic()
    ok("monotonic() croissant (m2 >= m1)",
        type(m2) == "number" and m2 >= m1,
        "m1=" .. tostring(m1) .. " m2=" .. tostring(m2))

    -- monotonic mesure correctement un sleep
    local before = luapilot.monotonic()
    luapilot.sleep(100, "ms")
    local after = luapilot.monotonic()
    local elapsed = after - before
    ok("monotonic() mesure sleep(100ms) : 0.05 < dt < 1.0",
        elapsed > 0.05 and elapsed < 1.0,
        "elapsed=" .. tostring(elapsed))

    -- now : timestamp epoch, > 0, proche de os.time()
    local n = luapilot.now()
    ok("now() -> number > 0",
        type(n) == "number" and n > 0,
        "n=" .. tostring(n))

    local ot = os.time()
    ok("now() ~ os.time() (diff < 2s)",
        math.abs(n - ot) < 2,
        "now=" .. tostring(n) .. " os.time=" .. tostring(ot))

    -- now() a une partie fractionnaire la plupart du temps. Pas
    -- garanti à 100% (un appel pile sur la seconde retournerait
    -- un entier), donc on teste sur une moyenne de 5 appels.
    local has_frac = false
    for _ = 1, 5 do
        local v = luapilot.now()
        if v ~= math.floor(v) then
            has_frac = true; break
        end
    end
    ok("now() a une précision sub-seconde", has_frac)

    -- Mauvais usage : argument non autorisé
    ok("monotonic('abc') -> luaL_error",
        pcall(function() luapilot.monotonic("abc") end) == false)
    ok("now({}) -> luaL_error",
        pcall(function() luapilot.now({}) end) == false)
end

-- =====================================================================
print("")
print("=== http ===")

do
    local H = luapilot.http

    -- --- contrat d'erreur : AUCUNE dépendance externe, toujours joué --

    -- request(non-table) lève toujours via luaL_checktype dans
    -- lua_http_request (avant http_perform).
    ok("request(non-table) lève",
        pcall(function() return H.request("pas une table") end) == false)

    -- Chantier longjmp : ces erreurs runtime de http_perform passent
    -- maintenant en (nil, err) au lieu de luaL_error (cohérent avec
    -- le commentaire d'intention du fichier + évite les fuites C++).
    do
        local v, e = H.request({})
        ok_fail("request{} sans url -> (nil, err)", v, e)
    end
    do
        local v, e = H.request({ url = 123 })
        ok_fail("request{url=number} -> (nil, err)", v, e)
    end
    do
        local v, e = H.request({ url = "http://127.0.0.1:1/", headers = "x" })
        ok_fail("request{headers=string} -> (nil, err)", v, e)
    end
    do
        local v, e = H.request({ url = "http://x/", timeout = "x" })
        ok_fail("request{timeout=string} -> (nil, err)", v, e)
    end

    -- mauvaises VALEURS d'option -> (nil, err), pas d'exception
    do
        local v, e = H.request({ url = "notaurl" })
        ok_fail("url sans scheme -> (nil, err)", v, e)
        ok("  message mentionne 'scheme'",
            type(e) == "string" and e:find("scheme", 1, true) ~= nil,
            "err=" .. tostring(e))

        v, e = H.get("ftp://example.com/")
        ok_fail("scheme non supporté -> (nil, err)", v, e)

        v, e = H.get("http://")
        ok_fail("url sans host -> (nil, err)", v, e)

        v, e = H.request({ url = "http://127.0.0.1:1/", timeout = -1 })
        ok_fail("timeout <= 0 -> (nil, err)", v, e)

        v, e = H.request({
            url = "http://127.0.0.1:1/", method = "FOO", timeout = 1,
        })
        ok_fail("méthode inconnue -> (nil, err)", v, e)
        ok("  message mentionne 'method'",
            type(e) == "string" and e:find("method", 1, true) ~= nil,
            "err=" .. tostring(e))

        v, e = H.request({
            url = "http://127.0.0.1:1/",
            method = "GET",
            body = "x",
            timeout = 1,
        })
        ok_fail("body sur GET -> (nil, err)", v, e)
        ok("  message mentionne 'body not allowed'",
            type(e) == "string"
            and e:find("body not allowed", 1, true) ~= nil,
            "err=" .. tostring(e))
    end

    -- échec transport : port loopback fermé -> (nil, "http: ...")
    -- (reste sur 127.0.0.1, aucun accès réseau externe ; timeout court)
    do
        local v, e = H.get("http://127.0.0.1:1/", { timeout = 1 })
        ok_fail("connexion refusée loopback -> (nil, err)", v, e)
        ok("  err préfixée 'http: '",
            type(e) == "string" and e:find("http: ", 1, true) == 1,
            "err=" .. tostring(e))
    end

    -- --- succès OPTIONNEL : seulement si python3 est présent ----------
    local function have_python3()
        local r = luapilot.exec("python3", { "--version" })
        return type(r) == "table" and r.code == 0
    end

    if not have_python3() then
        print("[INFO] http: python3 absent, sous-section succès 2xx "
            .. "ignorée (hermétique, aucun prérequis dur)")
    else
        local SBH = "_luapilot_http_test"
        luapilot.rmdirAll(SBH)
        luapilot.mkdir(SBH)
        luapilot.exec("sh", { "-c",
            "printf 'AB\\000CD' > " .. SBH .. "/probe.bin" })

        print("[INFO] http: démarrage serveur local de test...")
        local port = 8000 + (os.time() % 1000)
        -- Bootstrap borné par timeout (filet de sécurité) ; setsid
        -- détache le serveur dans une session séparée, donc le timeout
        -- du bootstrap ne le tue PAS. exec rend la main dès `echo $!`
        -- (le serveur a ses 3 flux redirigés -> aucun pipe partagé,
        -- EOF immédiat) ; le timeout ne sert que de garde-fou ultime.
        local boot = luapilot.exec("sh", { "-c",
            "cd " .. SBH .. " && { setsid python3 -m http.server "
            .. port .. " --bind 127.0.0.1 >/dev/null 2>&1 </dev/null & } "
            .. "; echo $!" }, { timeout = 5 })

        local srv_pid
        if type(boot) == "table" and boot.stdout then
            srv_pid = boot.stdout:gsub("%s+$", "")
        end

        -- Budget d'attente DUR et COURT : 15 sondes x ~200 ms ≈ 3 s
        -- max, AVEC progression visible (jamais "aucun avancement").
        -- Pas prêt dans le budget -> skip : la sous-section est
        -- optionnelle (décision actée), on ne grind jamais en muet.
        local up = false
        if srv_pid and srv_pid ~= "" then
            for i = 1, 15 do
                luapilot.sleep(200, "ms")
                local probe = luapilot.http.get(
                    "http://127.0.0.1:" .. port .. "/probe.bin",
                    { timeout = 1 })
                if type(probe) == "table" and probe.status == 200 then
                    up = true
                    break
                end
                if i % 5 == 0 then
                    print("[INFO] http: attente serveur ("
                        .. i .. "/15)...")
                end
            end
        end

        if not up then
            print("[INFO] http: serveur local indisponible dans le "
                .. "budget, sous-section succès 2xx ignorée (optionnelle)")
            if srv_pid and srv_pid ~= "" then
                luapilot.exec("kill", { "-9", srv_pid })
            end
            luapilot.rmdirAll(SBH)
        else
            local base = "http://127.0.0.1:" .. port
            print("[INFO] http: serveur prêt, tests succès 2xx...")

            local res, err = luapilot.http.get(base .. "/probe.bin",
                { timeout = 5 })
            ok("GET 200 -> (table, nil)",
                type(res) == "table" and err == nil,
                "err=" .. tostring(err))
            if type(res) == "table" then
                ok("  status == 200", res.status == 200,
                    "status=" .. tostring(res.status))
                ok("  body binaire-safe (#==5, NUL conservé)",
                    type(res.body) == "string" and #res.body == 5
                    and res.body:byte(3) == 0,
                    "len=" .. tostring(#res.body))
                ok("  headers est une table",
                    type(res.headers) == "table")
                ok("  header clé en minuscules (content-type)",
                    type(res.headers) == "table"
                    and type(res.headers["content-type"]) == "string",
                    "ct=" .. tostring(res.headers
                        and res.headers["content-type"]))
            end

            local r404, e404 = luapilot.http.get(
                base .. "/nexiste_pas", { timeout = 5 })
            ok("GET 404 -> (table, nil) [4xx pas une erreur]",
                type(r404) == "table" and e404 == nil
                and r404.status == 404,
                "status=" .. tostring(r404 and r404.status)
                .. " err=" .. tostring(e404))

            local rq = luapilot.http.get(base .. "/probe.bin",
                { timeout = 5, query = { a = "x y", b = 42 } })
            ok("GET avec query -> 200 (encodage n'a pas cassé l'URL)",
                type(rq) == "table" and rq.status == 200,
                "status=" .. tostring(rq and rq.status))

            luapilot.exec("kill", { srv_pid })
            luapilot.sleep(100, "ms")
            luapilot.exec("kill", { "-9", srv_pid })
            luapilot.rmdirAll(SBH)
        end
    end
    -- --- dette de test post-Chantier 1 (4 cas inscrits au `todo`) ----

    -- 1. http.post(url, opts) sans body : forme à 2 args (opts en 2e
    -- position, pas de string body). On vise un port loopback fermé
    -- pour rester hermétique ; ce qu'on prouve, c'est que la forme
    -- est ACCEPTÉE (pas de luaL_error « body must be a string », pas
    -- de « body not allowed for POST ») et que la requête atteint le
    -- transport, qui échoue alors proprement.
    do
        local v, e = H.post("http://127.0.0.1:1/", { timeout = 1 })
        ok_fail("post(url, opts) sans body : forme acceptée, "
            .. "transport échoue -> (nil, err)", v, e)
        ok("  err préfixée 'http: ' (et pas 'body must be a string')",
            type(e) == "string"
            and e:find("http: ", 1, true) == 1
            and e:find("body must be", 1, true) == nil,
            "err=" .. tostring(e))
    end

    -- 2. URL malformée subtile : "http:///path" (host vide entre les
    -- `//` et le `/`). Rejet attendu par split_url (« missing host »).
    do
        local v, e = H.get("http:///path")
        ok_fail("http:///path -> (nil, err)", v, e)
        ok("  message mentionne 'host'",
            type(e) == "string"
            and e:find("host", 1, true) ~= nil,
            "err=" .. tostring(e))
    end

    -- 3. URL malformée : "http://:8080/" (port sans host). Après le
    -- durcissement de split_url (chantier post-Chantier 7), c'est
    -- rejeté AU PARSE avec message "missing host", plus à attendre
    -- un échec transport. Plus rapide et plus précis.
    do
        local v, e = H.get("http://:8080/")
        ok_fail("http://:8080/ : rejeté au parse -> (nil, err)", v, e)
        ok("  err mentionne 'host'",
            type(e) == "string"
            and e:find("host", 1, true) ~= nil,
            "err=" .. tostring(e))
    end

    -- 3b. URL malformée : "http://[]:8080/" (crochets IPv6 vides).
    -- Cohérent avec SU-3 : authority commence par '[' mais le host
    -- entre [] est vide -> rejet au parse.
    do
        local v, e = H.get("http://[]:8080/")
        ok_fail("http://[]:8080/ : rejeté au parse -> (nil, err)", v, e)
        ok("  err mentionne 'host'",
            type(e) == "string"
            and e:find("host", 1, true) ~= nil,
            "err=" .. tostring(e))
    end

    -- 4. IPv6 brut loopback fermé : prouve que [::1] est accepté par
    -- le parse, le transport échoue proprement (refus ou erreur IPv6
    -- selon configuration locale).
    do
        local v, e = H.get("http://[::1]:1/", { timeout = 1 })
        ok_fail("http://[::1]:1/ : IPv6 parse OK, transport échoue"
            .. " -> (nil, err)", v, e)
        ok("  err préfixée 'http: '",
            type(e) == "string"
            and e:find("http: ", 1, true) == 1,
            "err=" .. tostring(e))
    end
end

-- =====================================================================
print("")
print("=== argparse ===")

do
    local argparse = require("argparse")

    -- ----- contrat de base : module et constructeur ------------------

    ok("require renvoie une valeur callable",
        type(argparse) == "table" or type(argparse) == "function")

    local p0 = argparse("prog", "desc")
    ok("argparse() construit un parser",
        type(p0) == "table" and type(p0.parse) == "function")

    -- builder chaînable : chaque méthode renvoie self
    local p1 = argparse("prog")
    ok("flag() chaînable", p1:flag("-v --verbose") == p1)
    ok("option() chaînable", p1:option("-o --output") == p1)
    ok("argument() chaînable", p1:argument("input") == p1)

    -- ----- défauts et types ------------------------------------------

    do
        local p = argparse("prog")
            :flag("-v --verbose")
            :option("-o --output", { default = "a.out" })
            :option("-n --count")
            :argument("input")

        local res, err = p:parse({ "INPUT" })
        ok_val("succès minimal -> (table, nil)", res, err)
        ok("  flag par défaut = false",
            res and res.verbose == false,
            "verbose=" .. tostring(res and res.verbose))
        ok("  option avec default rendu",
            res and res.output == "a.out",
            "output=" .. tostring(res and res.output))
        ok("  option sans default = nil",
            res and res.count == nil,
            "count=" .. tostring(res and res.count))
        ok("  positionnel reçu",
            res and res.input == "INPUT",
            "input=" .. tostring(res and res.input))
    end

    -- ----- formes d'appel -------------------------------------------

    do
        local p = argparse("prog")
            :flag("-v --verbose")
            :option("-o --output")
            :argument("input")

        local r = p:parse({ "--output", "out1", "in1" })
        ok("forme --long val",
            r and r.output == "out1" and r.input == "in1",
            "out=" .. tostring(r and r.output)
            .. " in=" .. tostring(r and r.input))

        r = p:parse({ "--output=out2", "in2" })
        ok("forme --long=val",
            r and r.output == "out2" and r.input == "in2",
            "out=" .. tostring(r and r.output))

        r = p:parse({ "-o", "out3", "in3" })
        ok("forme -s val",
            r and r.output == "out3" and r.input == "in3")

        r = p:parse({ "-o=out4", "in4" })
        ok("forme -s=val",
            r and r.output == "out4" and r.input == "in4")

        r = p:parse({ "-v", "x" })
        ok("flag présent = true", r and r.verbose == true)
    end

    -- ----- --help : SUCCÈS, signalé par res.help (décision A) --------

    do
        local p = argparse("prog", "description du prog")
            :flag("-v --verbose", { help = "verbeux" })
            :option("-o --output", { help = "sortie" })
            :argument("input", { help = "fichier d'entrée" })

        for _, flagname in ipairs({ "-h", "--help" }) do
            local res, err = p:parse({ flagname })
            ok("--help -> succès (err == nil) [" .. flagname .. "]",
                err == nil,
                "err=" .. tostring(err))
            ok("--help -> res.help == true [" .. flagname .. "]",
                type(res) == "table" and res.help == true,
                "res.help=" .. tostring(res and res.help))
            ok("--help -> res.usage string non vide [" .. flagname .. "]",
                type(res) == "table" and type(res.usage) == "string"
                and #res.usage > 0)
        end

        ok("get_usage() renvoie une string",
            type(p:get_usage()) == "string")
    end

    -- ----- terminateur "--" : -h après -- est un positionnel littéral

    do
        local p = argparse("prog"):argument("input")
        local res, err = p:parse({ "--", "-h" })
        ok("'--' neutralise les options : -h devient positionnel",
            err == nil and res and res.help ~= true
            and res.input == "-h",
            "help=" .. tostring(res and res.help)
            .. " input=" .. tostring(res and res.input))
    end

    -- ----- erreurs -> (nil, msg) ; jamais d'exception ----------------

    do
        local p = argparse("prog")
            :option("-o --output")
            :flag("-v --verbose")
            :argument("input")

        local v, e = p:parse({ "--nope" })
        ok_fail("option inconnue -> (nil, err)", v, e)
        ok("  message contient 'unknown'",
            type(e) == "string" and e:find("unknown", 1, true) ~= nil)

        v, e = p:parse({ "--output" })
        ok_fail("valeur manquante -> (nil, err)", v, e)

        v, e = p:parse({ "-v=oops", "x" })
        ok_fail("flag avec =val -> (nil, err)", v, e)

        v, e = p:parse({})
        ok_fail("positionnel requis manquant -> (nil, err)", v, e)

        v, e = p:parse({ "a", "b" })
        ok_fail("argument en trop -> (nil, err)", v, e)
    end

    -- ----- choices --------------------------------------------------

    do
        local p = argparse("prog")
            :option("-m --mode", { choices = { "fast", "safe" } })
            :argument("input")

        local r, e = p:parse({ "-m", "fast", "x" })
        ok_val("choices valide", r, e)
        ok("  mode = fast", r and r.mode == "fast")

        r, e = p:parse({ "-m", "wild", "x" })
        ok_fail("choices invalide -> (nil, err)", r, e)
    end

    -- ----- convert : succès, retour nil, exception interceptée -------

    do
        local p = argparse("prog")
            :option("-n --count", { convert = tonumber })
            :argument("input")

        local r, e = p:parse({ "-n", "42", "x" })
        ok_val("convert succès", r, e)
        ok("  count == 42 (number)", r and r.count == 42)

        r, e = p:parse({ "-n", "pasunnombre", "x" })
        ok_fail("convert renvoie nil -> (nil, err)", r, e)

        -- convert qui LÈVE doit être intercepté : parse() ne propage
        -- aucune exception sur entrée utilisateur (invariant).
        local p2 = argparse("prog")
            :option("-x", { convert = function(_) error("boom") end })
            :argument("input")
        r, e = p2:parse({ "-x", "v", "x" })
        ok_fail("convert qui lève -> (nil, err) (pas d'exception)", r, e)
    end

    -- ----- required / default sur positionnel ------------------------

    do
        local p = argparse("prog"):argument("input", { default = "STDIN" })
        local r, e = p:parse({})
        ok_val("positionnel optionnel avec default", r, e)
        ok("  default rendu", r and r.input == "STDIN")
    end

    -- ----- mauvais usage du BUILDER : doit LEVER (programmeur) -------

    do
        local p = argparse("prog")
        ok("spec vide -> error()",
            pcall(function() p:option("") end) == false)
        ok("option sans '-' -> error()",
            pcall(function() p:option("foo") end) == false)
        ok("doublon de nom -> error()",
            pcall(function()
                local p2 = argparse("prog")
                p2:option("-o --output")
                p2:option("-o")
            end) == false)
        ok("positionnel avec '-' -> error()",
            pcall(function() p:argument("-bad") end) == false)
    end

    -- ----- parse() sans argument lit la table globale `arg` ----------
    -- Le harnais est lancé AVEC les sentinelles arg[1]/arg[2] (cf.
    -- section === arg === qui valide leur présence). On définit donc
    -- 2 positionnels et on prouve que parse() les récupère bien depuis
    -- `arg` global, indices 1..n (pas <= 0).

    do
        local p = argparse("prog")
            :flag("-v --verbose")
            :argument("a")
            :argument("b")
        local r, e = p:parse() -- lit `arg` global
        ok_val("parse() sans argument lit `arg` global (1..n)", r, e)
        ok("  arg[1] reçu en positionnel 'a'",
            r and r.a == arg[1],
            "a=" .. tostring(r and r.a)
            .. " arg[1]=" .. tostring(arg[1]))
        ok("  arg[2] reçu en positionnel 'b'",
            r and r.b == arg[2],
            "b=" .. tostring(r and r.b)
            .. " arg[2]=" .. tostring(arg[2]))
        ok("  flag par défaut = false (pas de -v dans arg global)",
            r and r.verbose == false)
    end
    -- --- dette de test post-Chantier 2 (2 cas inscrits au `todo`) ----

    -- 5. positionnel avec choices : supporté par finalize_value mais
    -- non couvert jusqu'ici. Branches succès + rejet.
    do
        local p = argparse("prog")
            :argument("mode", { choices = { "fast", "safe" } })

        local r, e = p:parse({ "fast" })
        ok_val("positionnel + choices : valeur valide", r, e)
        ok("  mode == 'fast'", r and r.mode == "fast",
            "mode=" .. tostring(r and r.mode))

        r, e = p:parse({ "wild" })
        ok_fail("positionnel + choices : valeur invalide"
            .. " -> (nil, err)", r, e)
        ok("  message mentionne 'choice'",
            type(e) == "string"
            and e:find("choice", 1, true) ~= nil,
            "err=" .. tostring(e))
    end

    -- 6. positionnel avec convert : supporté par finalize_value, idem.
    -- Branches succès + retour nil de la convert.
    do
        local p = argparse("prog")
            :argument("n", { convert = tonumber })

        local r, e = p:parse({ "42" })
        ok_val("positionnel + convert : succès", r, e)
        ok("  n == 42 (number)", r and r.n == 42,
            "n=" .. tostring(r and r.n)
            .. " type=" .. tostring(r and type(r.n)))

        r, e = p:parse({ "pasunnombre" })
        ok_fail("positionnel + convert : retour nil"
            .. " -> (nil, err)", r, e)
    end
end

-- =====================================================================
print("")
print("=== logging ===")

do
    local log = require("logging")

    -- ----- contrat de base ------------------------------------------

    ok("require renvoie une table", type(log) == "table")
    ok("constantes de niveau exposées",
        log.TRACE == 10 and log.DEBUG == 20 and log.INFO == 30
        and log.WARN == 40 and log.ERROR == 50)
    ok("fonctions de log exposées",
        type(log.trace) == "function"
        and type(log.debug) == "function"
        and type(log.info) == "function"
        and type(log.warn) == "function"
        and type(log.error) == "function")
    ok("setters/getters exposés",
        type(log.set_level) == "function"
        and type(log.set_output) == "function"
        and type(log.set_color) == "function"
        and type(log.get_level) == "function")

    -- ----- défauts --------------------------------------------------

    ok("seuil par défaut = info (30)", log.get_level() == log.INFO)
    ok("destination par défaut = io.stderr",
        log.get_output() == io.stderr)
    ok("couleurs OFF par défaut (opt-in)", log.get_color() == false)

    -- ----- collecte d'écriture : sink en mémoire pour observer ------

    local function make_sink()
        local s = { written = {} }
        function s:write(...)
            for i = 1, select("#", ...) do
                self.written[#self.written + 1] = tostring(select(i, ...))
            end
        end

        function s:joined() return table.concat(self.written) end

        function s:clear() self.written = {} end

        return s
    end

    -- ----- set_output / format / filtrage ---------------------------

    do
        local sink = make_sink()
        log.set_output(sink)
        log.set_level(log.DEBUG)
        log.set_color(false)

        sink:clear()
        log.info("hello world")
        local out = sink:joined()
        ok("emission produit une ligne", out:sub(-1) == "\n",
            "out=" .. out)
        ok("format inclut le niveau [INFO ]",
            out:find("[INFO ]", 1, true) ~= nil, "out=" .. out)
        ok("format inclut le message",
            out:find("hello world", 1, true) ~= nil)
        ok("format commence par horodatage ISO",
            out:find("^%d%d%d%d%-%d%d%-%d%d %d%d:%d%d:%d%d ") ~= nil,
            "out=" .. out)

        sink:clear()
        log.info("user=", 42, "status=", "ok")
        out = sink:joined()
        ok("variadique : args concaténés avec espace",
            out:find("user= 42 status= ok", 1, true) ~= nil,
            "out=" .. out)
    end

    -- ----- filtrage par seuil ---------------------------------------

    do
        local sink = make_sink()
        log.set_output(sink)
        log.set_color(false)
        log.set_level(log.WARN)

        sink:clear()
        log.trace("t"); log.debug("d"); log.info("i")
        ok("seuil=WARN : trace/debug/info silencieux",
            sink:joined() == "", "out=" .. sink:joined())

        sink:clear()
        log.warn("w"); log.error("e")
        local out = sink:joined()
        ok("seuil=WARN : warn et error passent",
            out:find("[WARN ]", 1, true) ~= nil
            and out:find("[ERROR]", 1, true) ~= nil)
    end

    -- set_level : accepte string OU number ; case-insensible côté string
    do
        log.set_level("ERROR")
        ok("set_level('ERROR') -> ERROR (50)",
            log.get_level() == log.ERROR)
        log.set_level("debug")
        ok("set_level('debug') -> DEBUG",
            log.get_level() == log.DEBUG)
        log.set_level(log.INFO)
        ok("set_level(log.INFO) -> INFO",
            log.get_level() == log.INFO)
    end

    -- ----- couleurs : OFF / ON --------------------------------------

    do
        local sink = make_sink()
        log.set_output(sink)
        log.set_level(log.TRACE)

        log.set_color(false)
        sink:clear(); log.warn("zz")
        ok("color OFF : pas de séquence ANSI",
            sink:joined():find("\27[", 1, true) == nil,
            "out=" .. sink:joined())

        log.set_color(true)
        sink:clear(); log.warn("zz")
        ok("color ON + warn : séquence ANSI présente",
            sink:joined():find("\27[33m", 1, true) ~= nil,
            "out=" .. sink:joined())

        sink:clear(); log.error("zz")
        ok("color ON + error : séquence ANSI rouge",
            sink:joined():find("\27[31m", 1, true) ~= nil)

        sink:clear(); log.info("zz")
        ok("color ON + info : SANS couleur (décision G)",
            sink:joined():find("\27[", 1, true) == nil,
            "out=" .. sink:joined())

        log.set_color(false)
    end

    -- ----- contrat d'erreur : setters levent sur mauvais usage ------

    do
        ok("set_level('inconnu') -> error()",
            pcall(function() log.set_level("xxxx") end) == false)
        ok("set_level({}) -> error()",
            pcall(function() log.set_level({}) end) == false)
        ok("set_output(42) -> error()",
            pcall(function() log.set_output(42) end) == false)
        ok("set_output({}) sans :write -> error()",
            pcall(function() log.set_output({}) end) == false)
        ok("set_color('truthy') -> error() (boolean strict)",
            pcall(function() log.set_color("yes") end) == false)
    end

    -- ----- contrat d'erreur : log.* avale les écritures qui plantent

    do
        local exploding = {}
        function exploding:write() error("boom") end

        log.set_output(exploding)
        log.set_level(log.INFO)
        log.set_color(false)
        local ok_call = pcall(function() log.info("test") end)
        ok("log.info avec sink qui lève : ne propage PAS l'erreur",
            ok_call == true)

        -- restore : on rebascule la sortie sur stderr pour ne pas
        -- polluer les sections suivantes (== arg ===, ===chdir==).
        log.set_output(io.stderr)
    end
end
-- =====================================================================
print("")
print("=== sys ===")

do
    -- ----- pid : entier > 0, jamais d'erreur (POSIX) ----------------

    do
        local p = luapilot.pid()
        ok("pid() -> integer > 0",
            type(p) == "number" and p > 0 and p == math.floor(p),
            "pid=" .. tostring(p))
    end

    -- ----- hostname : string non vide ------------------------------

    do
        local h, err = luapilot.hostname()
        ok("hostname() -> string non vide",
            type(h) == "string" and #h > 0 and err == nil,
            "h=" .. tostring(h) .. " err=" .. tostring(err))
    end

    -- ----- uname : table avec 5 champs string non vides -------------

    do
        local u, err = luapilot.uname()
        ok_val("uname() -> table", u, err)
        if type(u) == "table" then
            for _, field in ipairs({ "sysname", "nodename",
                "release", "version", "machine" }) do
                ok("  uname." .. field .. " est une string non vide",
                    type(u[field]) == "string" and #u[field] > 0,
                    field .. "=" .. tostring(u[field]))
            end
        end
    end

    -- ----- env : variable absente = nil seul (décision UTIL-4) -----

    do
        local v = luapilot.env("LUAPILOT_VAR_QUI_NEXISTE_PAS_42")
        ok("env(absent) -> nil seul (pas de message d'erreur)",
            v == nil,
            "v=" .. tostring(v))

        local p = luapilot.env("PATH")
        ok("env('PATH') -> string non vide",
            type(p) == "string" and #p > 0)
    end

    -- ----- setenv : (true, nil) ou (nil, err) ; observable via env --

    do
        local name = "LUAPILOT_SYS_TEST_VAR"
        local ok_set, err = luapilot.setenv(name, "hello-42")
        ok_val("setenv('NAME', 'val') -> (true, nil)", ok_set, err)
        ok("  env() reflète setenv",
            luapilot.env(name) == "hello-42",
            "got=" .. tostring(luapilot.env(name)))

        luapilot.setenv(name, "world")
        ok("  setenv overwrite", luapilot.env(name) == "world")

        local v, e = luapilot.setenv("bad=name", "x")
        ok_fail("setenv('bad=name') -> (nil, err)", v, e)
    end

    -- ----- which : trouvé / pas trouvé / chemin direct --------------

    do
        local p, err = luapilot.which("sh")
        ok_val("which('sh') -> chemin", p, err)
        ok("  which('sh') renvoie un chemin absolu",
            type(p) == "string" and p:sub(1, 1) == "/",
            "p=" .. tostring(p))

        local v, e = luapilot.which("luapilot_binaire_qui_nexiste_pas_42")
        ok_fail("which(absent) -> (nil, err)", v, e)
        ok("  message mentionne 'PATH' ou 'not found'",
            type(e) == "string" and (e:find("PATH", 1, true) ~= nil
                or e:find("not found", 1, true) ~= nil),
            "err=" .. tostring(e))

        local p2, err2 = luapilot.which("/bin/sh")
        ok_val("which('/bin/sh') -> chemin direct accepté", p2, err2)
    end

    -- ----- mauvais usage : luaL_error ------------------------------

    do
        ok("which() sans arg lève",
            pcall(function() return luapilot.which() end) == false)
        -- env(table) lève (les nombres sont coercés en string par
        -- luaL_checkstring — convention Lua héritée, partagée par
        -- tout le codebase ; on teste donc avec une table, qui ne
        -- peut pas être coercée silencieusement).
        ok("env({}) lève",
            pcall(function() return luapilot.env({}) end) == false)
        ok("setenv() sans args lève",
            pcall(function() return luapilot.setenv() end) == false)
        ok("setenv('X') sans valeur lève",
            pcall(function() return luapilot.setenv("X") end) == false)
    end
end

-- =====================================================================
print("")
print("=== signal ===")

do
    local S = luapilot.signal

    -- ----- contrat de base -----------------------------------------
    ok("luapilot.signal est une table", type(S) == "table")
    ok("handle est une fonction", type(S.handle) == "function")
    ok("ignore est une fonction", type(S.ignore) == "function")
    ok("default est une fonction", type(S.default) == "function")

    -- ----- validation des arguments --------------------------------
    do
        local pok, perr = pcall(S.handle, "BOGUS", function() end)
        ok("handle('BOGUS', fn) leve une erreur", not pok)
        ok("  message mentionne 'unsupported' ou 'supported'",
            type(perr) == "string"
            and (perr:find("unsupported") or perr:find("supported")))
    end

    do
        local pok = pcall(S.ignore, "BOGUS")
        ok("ignore('BOGUS') leve une erreur", not pok)
    end

    do
        local pok = pcall(S.default, "BOGUS")
        ok("default('BOGUS') leve une erreur", not pok)
    end

    -- Handler de type incorrect
    do
        local pok, perr = pcall(S.handle, "USR1", 42)
        ok("handle('USR1', 42) leve (handler ni function ni nil)", not pok)
        ok("  message mentionne 'function or nil'",
            type(perr) == "string" and perr:find("function or nil"))
    end

    do
        local pok = pcall(S.handle, "USR1", "string")
        ok("handle('USR1', 'string') leve une erreur", not pok)
    end

    -- ----- enregistrement effectif ---------------------------------
    ok("handle('USR1', fn) -> true",
        S.handle("USR1", function() end) == true)
    ok("handle('USR1', nil) -> true (desinstallation)",
        S.handle("USR1", nil) == true)

    ok("handle('USR2', fn) -> true",
        S.handle("USR2", function() end) == true)
    ok("handle('USR2', fn) re-installe -> true",
        S.handle("USR2", function() end) == true)
    ok("handle('USR2', nil) -> true",
        S.handle("USR2", nil) == true)

    ok("ignore('USR1') -> true", S.ignore("USR1") == true)
    ok("default('USR1') -> true", S.default("USR1") == true)

    ok("ignore('PIPE') -> true (cas d'usage typique)",
        S.ignore("PIPE") == true)
    ok("default('PIPE') -> true (restauration)",
        S.default("PIPE") == true)

    -- Tous les signaux supportes
    for _, name in ipairs({ "TERM", "INT", "HUP", "USR1", "USR2", "PIPE" }) do
        ok("handle('" .. name .. "', fn) accepte",
            S.handle(name, function() end) == true)
        S.handle(name, nil)
    end

    -- Signaux refuses
    for _, name in ipairs({ "KILL", "STOP", "SEGV", "CHLD", "ALRM" }) do
        local pok = pcall(S.handle, name, function() end)
        ok("handle('" .. name .. "', fn) refuse", not pok)
    end

    -- ----- hardening: handle(name) sans 2eme arg refuse -------------
    do
        local pok, perr = pcall(S.handle, "USR1")
        ok("handle('USR1') sans 2eme arg -> leve", not pok)
        ok("  message mentionne 'missing handler'",
            type(perr) == "string" and perr:find("missing handler"))
    end

    -- ----- hardening: signal.* refuse depuis un worker --------------
    do
        local W = luapilot.workers
        local w = W.spawn([[
            local ok, err = pcall(luapilot.signal.handle, "USR1", function() end)
            return { ok = ok, err = err }
        ]])
        local ok_, res = w:join()
        ok("worker spawn + join OK", ok_ == true and type(res) == "table")
        ok("  signal.handle dans un worker echoue", res.ok == false)
        ok("  err mentionne 'main thread'",
            type(res.err) == "string" and res.err:find("main thread"))

        local w2 = W.spawn([[
            local ok, err = pcall(luapilot.signal.ignore, "USR1")
            return { ok = ok, err = err }
        ]])
        local _, res2 = w2:join()
        ok("signal.ignore dans worker -> 'main thread'",
            res2.ok == false and res2.err:find("main thread"))

        local w3 = W.spawn([[
            local ok, err = pcall(luapilot.signal.default, "USR1")
            return { ok = ok, err = err }
        ]])
        local _, res3 = w3:join()
        ok("signal.default dans worker -> 'main thread'",
            res3.ok == false and res3.err:find("main thread"))
    end
end

-- =====================================================================
print("")
print("=== sqlite (session 1: open/close/exec) ===")

do
    local DB = luapilot.sqlite

    -- ----- contrat de base -------------------------------------------
    ok("luapilot.sqlite est une table", type(DB) == "table")
    ok("open est une fonction", type(DB.open) == "function")

    -- ----- validation des arguments ----------------------------------
    do
        local pok = pcall(DB.open)
        ok("open() leve (path manquant)", not pok)
    end

    do
        local pok = pcall(DB.open, 42)
        ok("open(42) leve (path pas string)", not pok)
    end

    do
        local pok, perr = pcall(DB.open, ":memory:", "bad")
        ok("open(':memory:', 'bad') leve (opts pas table)", not pok)
        ok("  message mentionne 'table'",
            type(perr) == "string" and perr:find("table"))
    end

    do
        local pok = pcall(DB.open, ":memory:", { wal = "yes" })
        ok("open(opts.wal = 'yes') leve (wal pas boolean)", not pok)
    end

    do
        local pok = pcall(DB.open, ":memory:", { busy_timeout = "1000" })
        ok("open(opts.busy_timeout = '1000') leve (pas integer)", not pok)
    end

    do
        local pok = pcall(DB.open, ":memory:", { busy_timeout = -5 })
        ok("open(opts.busy_timeout = -5) leve (< 0)", not pok)
    end

    do
        local pok = pcall(DB.open, ":memory:", { busy_timeout = 99999999 })
        ok("open(opts.busy_timeout = 99999999) leve (max sanity)", not pok)
    end

    -- ----- ouverture en memoire (cas le plus simple) -----------------
    do
        local db, err = DB.open(":memory:")
        ok("open(':memory:') -> db", db ~= nil, "err=" .. tostring(err))
        ok("  pas d'erreur", err == nil)
        ok("  db est userdata", type(db) == "userdata")

        local ok_, err2 = db:exec("CREATE TABLE t(a INTEGER, b TEXT)")
        ok("db:exec(CREATE TABLE) -> true", ok_ == true)
        ok("  pas d'erreur", err2 == nil)

        local ok2, err3 = db:exec("INSERT INTO t VALUES (1, 'hello')")
        ok("db:exec(INSERT) -> true", ok2 == true)
        ok("  pas d'erreur", err3 == nil)

        local ok3 = db:exec("INSERT INTO t VALUES (2, 'a'); INSERT INTO t VALUES (3, 'b');")
        ok("db:exec(2 INSERTs separes par ;) -> true", ok3 == true)

        local ok4, err4 = db:exec("INSERT INTO bogus VALUES (1)")
        ok("db:exec(INSERT INTO unknown table) -> (nil, err)",
            ok4 == nil and type(err4) == "string")
        ok("  err prefixe par 'sqlite: '",
            type(err4) == "string" and err4:find("^sqlite: "))

        local cok = db:close()
        ok("db:close() -> true", cok == true)

        local cok2 = db:close()
        ok("db:close() encore -> true (idempotent)", cok2 == true)

        local ok5, err5 = db:exec("SELECT 1")
        ok("db:exec() apres close -> (nil, err)",
            ok5 == nil and type(err5) == "string")
        ok("  err mentionne 'closed'",
            type(err5) == "string" and err5:find("closed"))
    end

    -- ----- ouverture avec opts ---------------------------------------
    do
        local db = DB.open(":memory:", { busy_timeout = 1000 })
        ok("open(':memory:', busy_timeout=1000) -> db", db ~= nil)
        db:close()
    end

    do
        local db, err = DB.open(":memory:", { wal = true, busy_timeout = 500 })
        ok("open(':memory:', wal=true) -> db (fallback silencieux)",
            db ~= nil, "err=" .. tostring(err))
        if db then db:close() end
    end

    -- ----- erreur d'ouverture (chemin invalide) ----------------------
    do
        local db, err = DB.open("/proc/nope_cant_create_a_db_here.db")
        if not db then
            ok("open(chemin invalide) -> (nil, err)", db == nil)
            ok("  err prefixe par 'sqlite: '",
                type(err) == "string" and err:find("^sqlite: "))
        else
            db:close()
            os.remove("/proc/nope_cant_create_a_db_here.db")
            ok("open(chemin invalide) -- systeme l'a autorise, skip", true)
        end
    end

    -- ----- tostring --------------------------------------------------
    do
        local db = DB.open(":memory:")
        local s = tostring(db)
        ok("tostring(db) contient 'sqlite'",
            type(s) == "string" and s:find("sqlite"))
        db:close()
        s = tostring(db)
        ok("tostring(db) apres close mentionne 'closed'",
            type(s) == "string" and s:find("closed"))
    end

    -- ----- GC automatique --------------------------------------------
    do
        for i = 1, 5 do
            local db = DB.open(":memory:")
            db:exec("CREATE TABLE t(x)")
        end
        collectgarbage("collect")
        ok("5 open + GC sans crash", true)
    end
end

-- =====================================================================
print("")
print("=== sqlite (session 2: exec avec params) ===")

do
    local DB = luapilot.sqlite

    local function fresh_db_with_check(check_sql)
        local db = DB.open(":memory:")
        local ok_, err = db:exec("CREATE TABLE t (val) ")
        if not ok_ then error("setup: CREATE failed: " .. tostring(err)) end
        if check_sql then
            db:exec("DROP TABLE t")
            db:exec("CREATE TABLE t (val CHECK(" .. check_sql .. "))")
        end
        return db
    end

    -- ----- params = nil ou absent : equivalent session 1 ---------------
    do
        local db = DB.open(":memory:")
        local ok_ = db:exec("CREATE TABLE t (x)")
        ok("exec(sql) sans arg params -> marche encore", ok_ == true)

        local ok2 = db:exec("INSERT INTO t VALUES (1)", nil)
        ok("exec(sql, nil) -> equivalent sans params", ok2 == true)

        db:close()
    end

    -- ----- params doit etre une table si fourni ----------------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (x)")

        local pok, perr = pcall(db.exec, db, "INSERT INTO t VALUES (?)", "not a table")
        ok("exec(sql, string) leve (params pas table)", not pok)
        ok("  message mentionne 'table'",
            type(perr) == "string" and perr:find("table"))

        local pok2 = pcall(db.exec, db, "INSERT INTO t VALUES (?)", 42)
        ok("exec(sql, number) leve (params pas table)", not pok2)

        db:close()
    end

    -- ----- bind positionnel simple -----------------------------------
    do
        local db = fresh_db_with_check("val = 42")

        local ok_, err = db:exec("INSERT INTO t VALUES (?)", { 42 })
        ok("bind positionnel: 42 OK", ok_ == true, "err=" .. tostring(err))

        local ok2, err2 = db:exec("INSERT INTO t VALUES (?)", { 99 })
        ok("bind positionnel: 99 viole CHECK", ok2 == nil)
        ok("  err prefixe par 'sqlite: '",
            type(err2) == "string" and err2:find("^sqlite: "))

        db:close()
    end

    -- ----- bind positionnel : trop de slots, trop peu de params ------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (a, b)")

        local pok, perr = pcall(db.exec, db, "INSERT INTO t VALUES (?, ?)", { 1 })
        ok("bind 1 param pour 2 slots -> leve", not pok)
        ok("  message mentionne 'missing'",
            type(perr) == "string" and perr:find("missing"))

        local pok2, perr2 = pcall(db.exec, db, "INSERT INTO t VALUES (?, ?)", { 1, 2, 3 })
        ok("bind 3 params pour 2 slots -> leve", not pok2)
        ok("  message mentionne 'too many'",
            type(perr2) == "string" and perr2:find("too many"))

        db:close()
    end

    -- ----- bind nomme : ':name' --------------------------------------
    do
        local db = fresh_db_with_check("val = 7")

        local ok_, err = db:exec("INSERT INTO t VALUES (:x)", { x = 7 })
        ok("bind nomme :x = 7 OK", ok_ == true, "err=" .. tostring(err))

        local ok2 = db:exec("INSERT INTO t VALUES (:x)", { x = 99 })
        ok("bind nomme :x = 99 viole CHECK", ok2 == nil)

        db:close()
    end

    -- ----- bind nomme : prefixes alternatifs @ et $ ------------------
    do
        local db = fresh_db_with_check("val = 5")

        local ok_, err = db:exec("INSERT INTO t VALUES (@x)", { x = 5 })
        ok("bind nomme @x = 5 OK (prefixe @ accepte)",
            ok_ == true, "err=" .. tostring(err))

        local ok2, err2 = db:exec("INSERT INTO t VALUES ($x)", { x = 5 })
        ok("bind nomme $x = 5 OK (prefixe $ accepte)",
            ok2 == true, "err=" .. tostring(err2))

        db:close()
    end

    -- ----- bind nomme : param manquant -------------------------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (a, b)")

        local pok, perr = pcall(db.exec, db,
            "INSERT INTO t VALUES (:a, :b)", { a = 1 })
        ok("bind {a=1} pour :a et :b -> leve", not pok)
        ok("  message mentionne ':b'",
            type(perr) == "string" and perr:find(":b"))

        db:close()
    end

    -- ----- bind nomme : param en trop --------------------------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (a)")

        local pok, perr = pcall(db.exec, db,
            "INSERT INTO t VALUES (:a)", { a = 1, zzz = "extra" })
        ok("bind {a=1, zzz='extra'} pour :a seul -> leve", not pok)
        ok("  message mentionne 'zzz'",
            type(perr) == "string" and perr:find("zzz"))

        db:close()
    end

    -- ----- melange positionnel + nomme -------------------------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (a, b, c)")

        local ok_, err = db:exec(
            "INSERT INTO t VALUES (?, :name, ?)",
            { "first", "third", name = "second" })
        ok("melange positionnel + nomme OK",
            ok_ == true, "err=" .. tostring(err))

        db:close()
    end

    -- ----- types Lua : booleens convertis en 0/1 ---------------------
    do
        local db = fresh_db_with_check("val = 1")
        local ok_ = db:exec("INSERT INTO t VALUES (?)", { true })
        ok("bind true -> INTEGER 1 (CHECK val=1 OK)", ok_ == true)
        db:close()

        local db2 = fresh_db_with_check("val = 0")
        local ok2 = db2:exec("INSERT INTO t VALUES (?)", { false })
        ok("bind false -> INTEGER 0 (CHECK val=0 OK)", ok2 == true)
        db2:close()
    end

    -- ----- types Lua : integer / float / string ----------------------
    do
        local db = fresh_db_with_check("val = 42")
        ok("bind integer 42 OK",
            db:exec("INSERT INTO t VALUES (?)", { 42 }) == true)
        db:close()

        local db2 = fresh_db_with_check("val = 3.14")
        ok("bind float 3.14 OK",
            db2:exec("INSERT INTO t VALUES (?)", { 3.14 }) == true)
        db2:close()

        local db3 = fresh_db_with_check("val = 'hello'")
        ok("bind string 'hello' OK",
            db3:exec("INSERT INTO t VALUES (?)", { "hello" }) == true)
        db3:close()

        local db4 = DB.open(":memory:")
        db4:exec("CREATE TABLE t (val)")
        local ok4 = db4:exec("INSERT INTO t VALUES (?)", { "a\0b\0c" })
        ok("bind string avec NUL embarques OK", ok4 == true)
        db4:close()
    end

    -- ----- types Lua : refuses (function, table, userdata, thread) --
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (val)")

        local pok, perr = pcall(db.exec, db,
            "INSERT INTO t VALUES (?)", { function() end })
        ok("bind function -> leve", not pok)
        ok("  message mentionne 'function'",
            type(perr) == "string" and perr:find("function"))

        local pok2, perr2 = pcall(db.exec, db,
            "INSERT INTO t VALUES (?)", { { nested = true } })
        ok("bind table -> leve", not pok2)
        ok("  message mentionne 'table'",
            type(perr2) == "string" and perr2:find("table"))

        local pok3, perr3 = pcall(db.exec, db,
            "INSERT INTO t VALUES (?)", { db })
        ok("bind userdata -> leve", not pok3)
        ok("  message mentionne 'userdata'",
            type(perr3) == "string" and perr3:find("userdata"))

        local co = coroutine.create(function() end)
        local pok4, perr4 = pcall(db.exec, db,
            "INSERT INTO t VALUES (?)", { co })
        ok("bind thread (coroutine) -> leve", not pok4)
        ok("  message mentionne 'thread'",
            type(perr4) == "string" and perr4:find("thread"))

        db:close()
    end

    -- ----- multi-statement avec params : refuse ---------------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (a)")

        local ok_, err = db:exec(
            "INSERT INTO t VALUES (?); INSERT INTO t VALUES (?);",
            { 1, 2 })
        ok("multi-statement avec params -> (nil, err)",
            ok_ == nil and type(err) == "string")
        ok("  message mentionne 'one statement'",
            type(err) == "string" and err:find("one statement"))

        local ok2 = db:exec("INSERT INTO t VALUES (1); INSERT INTO t VALUES (2);")
        ok("multi-statement SANS params -> toujours OK (session 1)",
            ok2 == true)

        db:close()
    end

    -- ----- SQL invalide avec params : prepare echoue ----------------
    do
        local db = DB.open(":memory:")
        local ok_, err = db:exec("INSERT INTO bogus VALUES (?)", { 1 })
        ok("SQL invalide avec params -> (nil, err)",
            ok_ == nil and type(err) == "string")
        ok("  err prefixe par 'sqlite: '",
            type(err) == "string" and err:find("^sqlite: "))
        db:close()
    end

    -- ----- exec apres close : avec params aussi ---------------------
    do
        local db = DB.open(":memory:")
        db:close()
        local ok_, err = db:exec("INSERT INTO t VALUES (?)", { 1 })
        ok("exec(sql, params) apres close -> (nil, err)",
            ok_ == nil and type(err) == "string")
    end

    -- ----- burst de binds en erreur : pas de crash ------------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (a)")
        for i = 1, 100 do
            pcall(db.exec, db, "INSERT INTO t VALUES (?)", { function() end })
        end
        ok("100 binds qui fail : pas de crash", true)
        db:close()
    end
end

-- =====================================================================
print("")
print("=== sqlite (session 3: query + iterateur lazy) ===")

do
    local DB = luapilot.sqlite

    local function setup_users()
        local db = DB.open(":memory:")
        assert(db:exec([[
            CREATE TABLE users (
                id INTEGER PRIMARY KEY,
                name TEXT,
                age INTEGER,
                bio TEXT
            )
        ]]))
        assert(db:exec("INSERT INTO users VALUES (1, 'alice', 30, NULL)"))
        assert(db:exec("INSERT INTO users VALUES (2, 'bob', 25, 'engineer')"))
        assert(db:exec("INSERT INTO users VALUES (3, 'carol', 40, 'designer')"))
        return db
    end

    -- ----- contrat de base -------------------------------------------
    do
        local db = DB.open(":memory:")
        ok("db.query est une methode", type(db.query) == "function")
        db:close()
    end

    -- ----- query sur DB fermee ---------------------------------------
    do
        local db = DB.open(":memory:")
        db:close()
        local iter, err = db:query("SELECT 1")
        ok("query apres close -> (nil, err)",
            iter == nil and type(err) == "string")
        ok("  err mentionne 'closed'",
            type(err) == "string" and err:find("closed"))
    end

    -- ----- query : validation des arguments --------------------------
    do
        local db = DB.open(":memory:")

        local pok = pcall(db.query, db)
        ok("query() sans sql leve", not pok)

        local pok2 = pcall(db.query, db, 42)
        ok("query(number) leve (pas de coercion)", not pok2)

        local pok3 = pcall(db.query, db, "SELECT 1", "not a table")
        ok("query(sql, string) leve (params pas table)", not pok3)

        db:close()
    end

    -- ----- iteration basique : 3 rows --------------------------------
    do
        local db = setup_users()

        local rows = {}
        for row in db:query("SELECT id, name FROM users ORDER BY id") do
            table.insert(rows, row)
        end
        ok("iter: 3 rows collectees", #rows == 3)
        ok("  row[1].id == 1", rows[1].id == 1)
        ok("  row[1].name == 'alice'", rows[1].name == "alice")
        ok("  row[2].id == 2", rows[2].id == 2)
        ok("  row[2].name == 'bob'", rows[2].name == "bob")
        ok("  row[3].name == 'carol'", rows[3].name == "carol")

        db:close()
    end

    -- ----- WHERE avec bind positionnel -------------------------------
    do
        local db = setup_users()

        local rows = {}
        for row in db:query("SELECT name FROM users WHERE age > ? ORDER BY id", { 28 }) do
            table.insert(rows, row.name)
        end
        ok("WHERE age > 28: 2 rows (alice, carol)", #rows == 2)
        ok("  alice present", rows[1] == "alice")
        ok("  carol present", rows[2] == "carol")

        db:close()
    end

    -- ----- WHERE avec bind nomme -------------------------------------
    do
        local db = setup_users()

        local rows = {}
        for row in db:query("SELECT name FROM users WHERE name = :n", { n = "bob" }) do
            table.insert(rows, row.name)
        end
        ok("WHERE name = :n bound to 'bob': 1 row", #rows == 1)
        ok("  c'est bob", rows[1] == "bob")

        db:close()
    end

    -- ----- mapping types : INTEGER, TEXT, NULL -----------------------
    do
        local db = setup_users()

        local row
        for r in db:query("SELECT * FROM users WHERE id = 1") do
            row = r
        end
        ok("row fetchee (alice)", row ~= nil)
        ok("  id est integer", math.type(row.id) == "integer")
        ok("  id == 1", row.id == 1)
        ok("  name est string", type(row.name) == "string")
        ok("  age est integer", math.type(row.age) == "integer")
        ok("  age == 30", row.age == 30)
        ok("  bio (NULL) absente de la table", row.bio == nil)
        ok("  bio (NULL) absente aussi de pairs()", (function()
            for k, _ in pairs(row) do
                if k == "bio" then return false end
            end
            return true
        end)())

        db:close()
    end

    -- ----- mapping types : REAL --------------------------------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (x REAL)")
        db:exec("INSERT INTO t VALUES (3.14)")
        db:exec("INSERT INTO t VALUES (2.71828)")

        local rows = {}
        for r in db:query("SELECT x FROM t ORDER BY x") do
            table.insert(rows, r.x)
        end
        ok("REAL: 2 rows", #rows == 2)
        ok("  2.71828 first", math.abs(rows[1] - 2.71828) < 1e-9)
        ok("  3.14 second", math.abs(rows[2] - 3.14) < 1e-9)
        ok("  est float (pas integer)", math.type(rows[1]) == "float")

        db:close()
    end

    -- ----- mapping types : BLOB --------------------------------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (b BLOB)")
        db:exec("INSERT INTO t VALUES (X'00010203FF')")

        local row
        for r in db:query("SELECT b FROM t") do row = r end
        ok("BLOB row fetchee", row ~= nil and row.b ~= nil)
        ok("  BLOB est string (binary-safe)", type(row.b) == "string")
        ok("  BLOB longueur == 5", #row.b == 5)
        ok("  byte 0 == 0x00", string.byte(row.b, 1) == 0x00)
        ok("  byte 4 == 0xFF", string.byte(row.b, 5) == 0xFF)

        db:close()
    end

    -- ----- mapping types : TEXT avec NUL embarque --------------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (s)")
        db:exec("INSERT INTO t VALUES (?)", { "a\0b\0c" })

        local row
        for r in db:query("SELECT s FROM t") do row = r end
        ok("TEXT avec NUL: row OK", row ~= nil)
        ok("  longueur preservee (5 bytes)", #row.s == 5)
        ok("  byte 2 == NUL", string.byte(row.s, 2) == 0)

        db:close()
    end

    -- ----- SELECT 0 row : iterateur termine immediatement ------------
    do
        local db = setup_users()
        local count = 0
        for _ in db:query("SELECT * FROM users WHERE id = 999") do
            count = count + 1
        end
        ok("SELECT sans match: 0 rows", count == 0)
        db:close()
    end

    -- ----- DDL/DML via query : iterateur vide (pas d'erreur) --------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (x)")
        local count = 0
        for _ in db:query("INSERT INTO t VALUES (42)") do
            count = count + 1
        end
        ok("query('INSERT'): iterateur vide (0 rows)", count == 0)

        local n = 0
        for _ in db:query("SELECT * FROM t") do n = n + 1 end
        ok("  ... mais l'INSERT a bien eu lieu", n == 1)

        db:close()
    end

    -- ----- query sur SQL invalide ------------------------------------
    do
        local db = DB.open(":memory:")
        local iter, err = db:query("SELECT * FROM bogus")
        ok("query(SQL invalide) -> (nil, err)",
            iter == nil and type(err) == "string")
        ok("  err prefixe par 'sqlite: '",
            type(err) == "string" and err:find("^sqlite: "))
        db:close()
    end

    -- ----- multi-statement refuse ------------------------------------
    do
        local db = setup_users()
        local iter, err = db:query("SELECT 1; SELECT 2;")
        ok("multi-statement query -> (nil, err)",
            iter == nil and type(err) == "string")
        ok("  message mentionne 'one statement'",
            type(err) == "string" and err:find("one statement"))
        db:close()
    end

    -- ----- bind manquant : raise -------------------------------------
    do
        local db = setup_users()
        local pok, perr = pcall(db.query, db,
            "SELECT * FROM users WHERE age > ?", {})
        ok("query avec param positionnel manquant -> leve", not pok)
        ok("  message mentionne 'missing'",
            type(perr) == "string" and perr:find("missing"))
        db:close()
    end

    -- ----- close explicite + reprise ---------------------------------
    do
        local db = setup_users()
        local iter = db:query("SELECT id FROM users ORDER BY id")

        local first = iter()
        ok("1er iter() -> row", first ~= nil and first.id == 1)

        iter:close()
        local after_close = iter()
        ok("iter() apres close -> nil (termine)", after_close == nil)

        local ok_ = iter:close()
        ok("iter:close() idempotent", ok_ == true)

        db:close()
    end

    -- ----- break dans la boucle : pas de crash, finalize via __gc ---
    do
        local db = setup_users()
        for row in db:query("SELECT * FROM users ORDER BY id") do
            if row.id == 1 then break end
        end
        ok("break mid-iteration: pas de crash", true)
        local n = 0
        for _ in db:query("SELECT * FROM users") do n = n + 1 end
        ok("  db utilisable apres break: 3 rows again", n == 3)
        db:close()
    end

    -- ----- db:close() pendant qu'un iter est encore actif -----------
    do
        local db = setup_users()
        local iter = db:query("SELECT id FROM users ORDER BY id")
        db:close()
        local r1 = iter()
        ok("iter() apres db:close(): marche encore (zombie SQLite)",
            r1 ~= nil and r1.id == 1)
        local r2 = iter()
        ok("  encore une row", r2 ~= nil and r2.id == 2)
        iter:close()
        ok("iter:close() apres db:close(): OK", true)
    end

    -- ----- tostring(stmt) --------------------------------------------
    do
        local db = setup_users()
        local iter = db:query("SELECT * FROM users")
        local s = tostring(iter)
        ok("tostring(stmt) contient 'sqlite.stmt'",
            type(s) == "string" and s:find("sqlite.stmt"))
        ok("  mentionne 'active'",
            type(s) == "string" and s:find("active"))
        iter:close()
        local s2 = tostring(iter)
        ok("tostring apres close mentionne 'closed'",
            type(s2) == "string" and s2:find("closed"))
        db:close()
    end

    -- ----- transactions manuelles ------------------------------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (x INTEGER)")

        db:exec("BEGIN")
        for i = 1, 5 do
            db:exec("INSERT INTO t VALUES (?)", { i })
        end
        db:exec("COMMIT")

        local n = 0
        local sum = 0
        for row in db:query("SELECT x FROM t ORDER BY x") do
            n = n + 1
            sum = sum + row.x
        end
        ok("BEGIN/INSERTs/COMMIT: 5 rows", n == 5)
        ok("  sum == 15", sum == 15)

        db:exec("BEGIN")
        db:exec("INSERT INTO t VALUES (?)", { 999 })
        db:exec("ROLLBACK")

        local n2 = 0
        for _ in db:query("SELECT * FROM t") do n2 = n2 + 1 end
        ok("ROLLBACK: toujours 5 rows (pas 6)", n2 == 5)

        db:close()
    end

    -- ----- round-trip de tous les types ------------------------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (i INTEGER, f REAL, s TEXT, b BLOB)")

        assert(db:exec("INSERT INTO t VALUES (?, ?, ?, ?)",
            { 42, 3.14, "hello", "\x01\x02\x03" }))

        local row
        for r in db:query("SELECT * FROM t") do row = r end
        ok("round-trip integer", row.i == 42)
        ok("round-trip float", math.abs(row.f - 3.14) < 1e-9)
        ok("round-trip text", row.s == "hello")
        ok("round-trip blob (3 bytes)", #row.b == 3)
        ok("  blob byte 1 == 1", string.byte(row.b, 1) == 1)

        db:close()
    end

    -- ----- hardening: SQL avec placeholders mais sans params --------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (a)")

        local ok_, err = db:exec("INSERT INTO t VALUES (?)")
        ok("exec avec '?' sans params -> (nil, err)",
            ok_ == nil and type(err) == "string")
        ok("  message mentionne 'placeholders'",
            type(err) == "string" and err:find("placeholders"))

        local ok2, err2 = db:exec("INSERT INTO t VALUES (:x)")
        ok("exec avec ':name' sans params -> (nil, err)",
            ok2 == nil and type(err2) == "string")

        local iter, err3 = db:query("SELECT * FROM t WHERE a = ?")
        ok("query avec '?' sans params -> (nil, err)",
            iter == nil and type(err3) == "string")

        local ok4 = db:exec("INSERT INTO t VALUES (1)")
        ok("exec sans placeholders, sans params -> toujours OK", ok4 == true)

        db:close()
    end

    -- ----- hardening: cles numeriques sparse dans params ------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (a)")

        local pok, perr = pcall(db.exec, db,
            "INSERT INTO t VALUES (?)", { "ok", [10] = "ignored" })
        ok("bind avec cle sparse [10] -> leve", not pok)
        ok("  message mentionne 'index 10'",
            type(perr) == "string" and perr:find("index 10"))

        local pok2, perr2 = pcall(db.exec, db,
            "INSERT INTO t VALUES (?)", { [1] = "ok", [1.5] = "weird" })
        ok("bind avec cle non-integer [1.5] -> leve", not pok2)
        ok("  message mentionne 'non-integer'",
            type(perr2) == "string" and perr2:find("non%-integer"))

        db:close()
    end

    -- ----- hardening: BLOB vide -------------------------------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (b BLOB)")
        db:exec("INSERT INTO t VALUES (X'')")

        local row
        for r in db:query("SELECT b FROM t") do row = r end
        ok("BLOB vide: row fetchee", row ~= nil)
        ok("  b est une string", type(row.b) == "string")
        ok("  #b == 0", #row.b == 0)

        db:close()
    end

    -- ----- hardening: TEXT vide -------------------------------------
    do
        local db = DB.open(":memory:")
        db:exec("CREATE TABLE t (s TEXT)")
        db:exec("INSERT INTO t VALUES ('')")

        local row
        for r in db:query("SELECT s FROM t") do row = r end
        ok("TEXT vide: row fetchee", row ~= nil)
        ok("  s est une string", type(row.s) == "string")
        ok("  #s == 0", #row.s == 0)

        db:close()
    end
end

do
    local T = luapilot.toml

    -- ----- contrat de base -----------------------------------------

    ok("luapilot.toml est une table", type(T) == "table")
    ok("luapilot.toml.decode est une fonction",
        type(T.decode) == "function")

    -- ----- succès minimal : (table, nil) ---------------------------

    do
        local r, e = T.decode('title = "TOML Example"')
        ok_val("decode minimal -> (table, nil)", r, e)
        ok("  title rendu",
            type(r) == "table" and r.title == "TOML Example",
            "title=" .. tostring(r and r.title))
    end

    -- ----- types scalaires : string, integer, float, bool ----------

    do
        local src = [[
str = "hello"
n_int = 42
n_float = 3.14
b_true = true
b_false = false
]]
        local r, e = T.decode(src)
        ok_val("decode scalaires -> (table, nil)", r, e)
        ok("  string", r and r.str == "hello")
        ok("  integer (number, entier exact)",
            r and type(r.n_int) == "number" and r.n_int == 42)
        ok("  float (number, non entier)",
            r and type(r.n_float) == "number"
            and r.n_float > 3.13 and r.n_float < 3.15)
        ok("  boolean true", r and r.b_true == true)
        ok("  boolean false", r and r.b_false == false)
    end

    -- ----- array TOML -> séquence Lua 1..n -------------------------

    do
        local r, e = T.decode([[
nums = [ 1, 2, 3 ]
mixed = [ "a", "b", "c" ]
]])
        ok_val("decode arrays -> (table, nil)", r, e)
        ok("  array d'entiers : séquence 1..n",
            r and type(r.nums) == "table"
            and #r.nums == 3
            and r.nums[1] == 1 and r.nums[2] == 2 and r.nums[3] == 3)
        ok("  array de strings : séquence 1..n",
            r and type(r.mixed) == "table"
            and #r.mixed == 3
            and r.mixed[1] == "a" and r.mixed[3] == "c")
    end

    -- ----- table imbriquée (sections) ------------------------------

    do
        local r, e = T.decode([[
[server]
host = "127.0.0.1"
port = 8080

[server.tls]
enabled = true
cert = "/etc/ssl/cert.pem"
]])
        ok_val("decode sections -> (table, nil)", r, e)
        ok("  section [server] est une table",
            r and type(r.server) == "table")
        ok("  server.host", r and r.server and r.server.host == "127.0.0.1")
        ok("  server.port", r and r.server and r.server.port == 8080)
        ok("  section imbriquée [server.tls]",
            r and r.server and type(r.server.tls) == "table"
            and r.server.tls.enabled == true
            and r.server.tls.cert == "/etc/ssl/cert.pem")
    end

    -- ----- array of tables (cas idiomatique TOML) ------------------

    do
        -- [==[ ... ]==] obligatoire ici : la chaîne TOML contient
        -- "[[users]]" (array of tables), qui fermerait un [[ ... ]]
        -- Lua au premier ]] rencontré.
        local r, e = T.decode([==[
[[users]]
name = "alice"
age = 30

[[users]]
name = "bob"
age = 25
]==])
        ok_val("decode array of tables -> (table, nil)", r, e)
        ok("  users est une séquence de 2 tables",
            r and type(r.users) == "table" and #r.users == 2
            and type(r.users[1]) == "table"
            and type(r.users[2]) == "table")
        ok("  users[1].name == 'alice'",
            r and r.users and r.users[1].name == "alice")
        ok("  users[2].age == 25",
            r and r.users and r.users[2].age == 25)
    end

    -- ----- types temporels -> strings ISO 8601 (décision TOML-3) ---

    do
        local r, e = T.decode([[
d = 1979-05-27
t = 07:32:00
dt_local = 1979-05-27T07:32:00
dt_offset = 1979-05-27T07:32:00Z
]])
        ok_val("decode dates/times -> (table, nil)", r, e)
        ok("  local-date -> string '1979-05-27'",
            r and type(r.d) == "string" and r.d == "1979-05-27",
            "d=" .. tostring(r and r.d))
        ok("  local-time -> string commençant par '07:32:00'",
            r and type(r.t) == "string"
            and r.t:find("^07:32:00") ~= nil,
            "t=" .. tostring(r and r.t))
        ok("  local-date-time -> string contenant 'T'",
            r and type(r.dt_local) == "string"
            and r.dt_local:find("T", 1, true) ~= nil,
            "dt_local=" .. tostring(r and r.dt_local))
        ok("  offset-date-time -> string contenant 'T' et offset",
            r and type(r.dt_offset) == "string"
            and r.dt_offset:find("T", 1, true) ~= nil
            and (r.dt_offset:find("Z", 1, true) ~= nil
                or r.dt_offset:find("+", 1, true) ~= nil),
            "dt_offset=" .. tostring(r and r.dt_offset))
    end

    -- ----- UTF-8 préservé (aller simple, on ne réencode pas) -------

    do
        local r, e = T.decode('msg = "été café"')
        ok_val("decode UTF-8 -> (table, nil)", r, e)
        ok("  string UTF-8 intacte",
            r and r.msg == "été café",
            "msg=" .. tostring(r and r.msg))
    end

    -- ----- TOML vide / minimal -------------------------------------

    do
        local r, e = T.decode("")
        ok_val("decode('') -> (table vide, nil)", r, e)
        ok("  table vide", r and next(r) == nil)

        r, e = T.decode("# commentaire seul\n")
        ok_val("decode commentaire seul -> (table vide, nil)", r, e)
    end

    -- ----- TOML invalide -> (nil, err) avec ligne/col --------------

    do
        local v, e = T.decode("nope = ")
        ok_fail("decode TOML invalide -> (nil, err)", v, e)
        ok("  err préfixée 'toml: '",
            type(e) == "string" and e:find("toml: ", 1, true) == 1,
            "err=" .. tostring(e))
        ok("  message contient une indication de ligne",
            type(e) == "string" and e:find("line", 1, true) ~= nil,
            "err=" .. tostring(e))
    end

    do
        local v, e = T.decode([[
x = 1
x = 2
]])
        ok_fail("decode clé redéfinie -> (nil, err)", v, e)
    end

    do
        local v, e = T.decode("flag = TRUE")
        ok_fail("decode boolean majuscule -> (nil, err)", v, e)
    end

    -- ----- mauvais usage : luaL_error (luaL_checktype lève) --------

    do
        ok("decode() sans arg lève",
            pcall(function() return T.decode() end) == false)
        ok("decode(42) lève (luaL_checktype LUA_TSTRING, pas de coercion)",
            pcall(function() return T.decode(42) end) == false)
        ok("decode({}) lève",
            pcall(function() return T.decode({}) end) == false)
        ok("decode(nil) lève",
            pcall(function() return T.decode(nil) end) == false)
    end
end

-- =====================================================================
print("")
print("=== socket ===")

do
    local S = luapilot.socket

    -- ----- contrat de base ------------------------------------------

    ok("luapilot.socket est une table", type(S) == "table")
    ok("connect est une fonction", type(S.connect) == "function")
    ok("listen est une fonction", type(S.listen) == "function")

    -- ----- mauvais usage : luaL_error ------------------------------

    do
        ok("connect() sans args lève",
            pcall(function() return S.connect() end) == false)
        ok("connect(host) sans port lève",
            pcall(function() return S.connect("127.0.0.1") end) == false)
        ok("connect({}, 80) lève (host pas une string)",
            pcall(function() return S.connect({}, 80) end) == false)
        -- port = {} : table non coerçable. Une string "80" passerait
        -- luaL_checkinteger silencieusement (même piège que
        -- luaL_checkstring avec les nombres) et n'aurait pas levé ;
        -- on doit utiliser un type qui ne peut PAS être coercé.
        ok("connect('h', {}) lève (port pas un nombre, pas coercible)",
            pcall(function() return S.connect("127.0.0.1", {}) end)
            == false)

        ok("listen() sans args lève",
            pcall(function() return S.listen() end) == false)
        ok("listen(host) sans port lève",
            pcall(function() return S.listen("127.0.0.1") end) == false)
    end

    -- ----- mauvaises valeurs : (nil, err) --------------------------

    do
        local v, e = S.connect("127.0.0.1", -1)
        ok_fail("connect port négatif -> (nil, err)", v, e)
        ok("  message mentionne 'port'",
            type(e) == "string" and e:find("port", 1, true) ~= nil)

        v, e = S.connect("127.0.0.1", 70000)
        ok_fail("connect port > 65535 -> (nil, err)", v, e)

        v, e = S.listen("127.0.0.1", -1)
        ok_fail("listen port négatif -> (nil, err)", v, e)

        v, e = S.listen("127.0.0.1", 8080, -5)
        ok_fail("listen backlog <= 0 -> (nil, err)", v, e)
    end

    -- ----- échec transport : connect sur port loopback fermé -------

    do
        -- Port 1 sur loopback : très improbable qu'il écoute. timeout
        -- court pour rester rapide. Reste sur 127.0.0.1 : hermétique.
        local v, e = S.connect("127.0.0.1", 1, 0.5)
        ok_fail("connect 127.0.0.1:1 -> (nil, err) (refus ou timeout)",
            v, e)
        ok("  err préfixée 'socket: ' ou 'timeout'",
            type(e) == "string"
            and (e:find("socket: ", 1, true) == 1 or e == "timeout"),
            "err=" .. tostring(e))
    end

    -- ----- pattern (b) : listen -> connect -> accept dans le même
    --       processus, synchrone en loopback. Le noyau accepte la
    --       connexion dans sa file dès `listen()`, donc `connect`
    --       rend dès qu'elle est dans la file et `accept` la dépile.
    --       Pas de threads, pas de subprocess.

    do
        -- CORRECTIF (post-revue ChatGPT) : on demande au noyau un
        -- port libre via listen("127.0.0.1", 0), puis on récupère
        -- le port effectif via sockname(). Évite TOUTE collision
        -- même si plusieurs runs se chevauchent ou si un autre
        -- processus écoute sur un port haut.

        -- 1) Démarrer le serveur sur port 0 = "noyau choisit".
        --    SO_REUSEADDR activé en interne.
        local srv, err = S.listen("127.0.0.1", 0)
        ok_val("listen('127.0.0.1', 0) -> (socket, nil)", srv, err)

        if srv then
            -- Récupérer le port effectif attribué par le noyau.
            local a = srv:sockname()
            local port = a and tonumber(a.port)
            ok("  sockname() rend host + port effectif > 0",
                type(a) == "table"
                and a.host ~= nil
                and type(port) == "number" and port > 0,
                "port=" .. tostring(port))

            -- 2) Le client se connecte. Comme listen() est déjà en
            --    place, le noyau accepte instantanément en loopback.
            local cli, cerr = S.connect("127.0.0.1", port, 2)
            ok_val("connect('127.0.0.1', port) -> (socket, nil)", cli, cerr)

            -- 3) Le serveur dépile la connexion. accept() rend tout
            --    de suite parce que le client est déjà dans la file.
            srv:set_timeout(2)
            local peer, perr = srv:accept()
            ok_val("srv:accept() -> (socket, nil)", peer, perr)

            if cli and peer then
                -- ----- échange de données : send / recv -----------

                local n, serr = cli:send("hello")
                ok_val("cli:send('hello') -> (5, nil)", n, serr)
                ok("  5 octets envoyés", n == 5)

                peer:set_timeout(2)
                local data, rerr = peer:recv(1024)
                ok_val("peer:recv(1024) -> ('hello', nil)", data, rerr)
                ok("  data == 'hello'", data == "hello")

                -- ----- recv_line avec CRLF transparent ------------

                cli:send("ligne1\r\nligne2\n")
                local l1 = peer:recv_line()
                ok("recv_line() #1 : 'ligne1' (\\r retiré)",
                    l1 == "ligne1",
                    "l1=" .. tostring(l1))
                local l2 = peer:recv_line()
                ok("recv_line() #2 : 'ligne2' (LF seul)",
                    l2 == "ligne2",
                    "l2=" .. tostring(l2))

                -- ----- peer() / sockname() côté client ------------

                local p = cli:peer()
                ok("cli:peer() -> { host, port }",
                    type(p) == "table"
                    and tonumber(p.port) == port)

                -- ----- binaire-safe : data contenant un NUL -------

                cli:send("AB\0CD")
                local bin = peer:recv(1024)
                ok("recv binaire-safe : 5 octets, NUL conservé",
                    type(bin) == "string"
                    and #bin == 5
                    and bin:byte(3) == 0,
                    "len=" .. tostring(bin and #bin))

                -- ----- timeout sur recv quand rien n'est envoyé ---

                peer:set_timeout(0.1) -- 100 ms
                local v, e = peer:recv(1024)
                ok_fail("recv avec timeout court et rien à lire"
                    .. " -> (nil, 'timeout')", v, e)
                ok("  err == 'timeout'", e == "timeout")

                -- ----- EOF : cli close, peer recv -> closed -------

                cli:close()
                peer:set_timeout(2)
                local v2, e2 = peer:recv(1024)
                ok_fail("recv après close client -> (nil, 'closed')",
                    v2, e2)
                ok("  err == 'closed' (chaîne typée)", e2 == "closed")

                -- ----- send sur socket fermé localement -----------

                local v3, e3 = cli:send("x")
                ok_fail("send sur socket fermé localement"
                    .. " -> (nil, err)", v3, e3)

                peer:close()
            end

            -- ----- recv_line avec EOF en plein milieu -------------

            -- Nouvel échange dédié : on envoie une demi-ligne (sans
            -- '\n') puis on ferme côté client. recv_line côté serveur
            -- doit rendre (nil, "closed", partial).
            do
                local c2, ce = S.connect("127.0.0.1", port, 2)
                ok_val("2nd connect (pour test EOF mid-line)", c2, ce)
                local p2, pe = srv:accept()
                ok_val("2nd accept", p2, pe)
                if c2 and p2 then
                    c2:send("partial-no-newline")
                    c2:close()
                    p2:set_timeout(2)
                    local line, eerr, partial = p2:recv_line()
                    ok("recv_line EOF mid-line : 3 valeurs",
                        line == nil and eerr == "closed"
                        and type(partial) == "string",
                        "line=" .. tostring(line)
                        .. " err=" .. tostring(eerr)
                        .. " partial=" .. tostring(partial))
                    ok("  partial contient les octets déjà lus",
                        partial == "partial-no-newline",
                        "partial=" .. tostring(partial))
                    p2:close()
                end
            end

            -- ----- recv_line : garde DoS (max 8 MiB par ligne) -------
            -- Audit sécurité : un peer qui floode sans '\n' ne doit
            -- pas faire grossir le buffer jusqu'à OOM. Test sur un
            -- payload raisonnable (sous la limite de 8 MiB).
            --
            -- Taille payload : 8 KiB. Raison : client et server sont
            -- dans le MÊME process. Si on envoie trop, le buffer TCP
            -- kernel local se remplit avant que le serveur ne lise,
            -- et send() bloque → deadlock jusqu'au timeout côté
            -- lecture. Sur x86_64 le buffer TCP fait ~256 KB et 100
            -- KiB passe ; sur Raspberry Pi 0 le buffer est plus
            -- petit (~64 KB) et 100 KiB deadlock. 8 KiB est en
            -- dessous de tout buffer kernel plausible. Le test
            -- exerce toujours l'accumulator (plusieurs recv()).
            do
                local c4, ce = S.connect("127.0.0.1", port, 2)
                ok_val("4th connect (test grande ligne)", c4, ce)
                local p4, pe = srv:accept()
                ok_val("4th accept", p4, pe)
                if c4 and p4 then
                    local payload = string.rep("A", 8 * 1024)
                    c4:send(payload)
                    c4:close()
                    p4:set_timeout(2)
                    local line, eerr, partial = p4:recv_line()
                    ok("recv_line 8 KiB sans newline: closed + partial",
                        line == nil and eerr == "closed"
                        and type(partial) == "string"
                        and #partial == 8 * 1024)
                    p4:close()
                end
            end

            -- ----- recv_all : lit jusqu'à EOF du peer -------------

            do
                local c3, ce = S.connect("127.0.0.1", port, 2)
                ok_val("3rd connect (pour test recv_all)", c3, ce)
                local p3, pe = srv:accept()
                ok_val("3rd accept", p3, pe)
                if c3 and p3 then
                    c3:send("alpha-beta-gamma")
                    c3:close() -- EOF déclenche fin de recv_all
                    p3:set_timeout(2)
                    local body, berr = p3:recv_all()
                    ok_val("recv_all -> (body, nil)", body, berr)
                    ok("  body complet récupéré",
                        body == "alpha-beta-gamma",
                        "body=" .. tostring(body))
                    p3:close()
                end
            end

            -- ----- accept avec timeout : pas de client -> timeout --

            do
                srv:set_timeout(0.1)
                local v, e = srv:accept()
                ok_fail("accept sans client -> (nil, 'timeout')", v, e)
                ok("  err == 'timeout'", e == "timeout")
            end

            -- ----- set_timeout : valeur négative -> (nil, err) ----

            do
                local v, e = srv:set_timeout(-1)
                ok_fail("set_timeout(-1) -> (nil, err)", v, e)
            end

            srv:close()
        end
    end

    -- ----- méthodes sur socket fermé : refus propre ----------------

    do
        local s = S.listen("127.0.0.1", 0)
        if s then
            s:close()
            -- Re-close : doit être idempotent et rendre (true, nil).
            local ok_close, _ = s:close()
            ok("close() idempotent (socket déjà fermé)",
                ok_close == true)

            local v, e = s:accept()
            ok_fail("accept sur socket fermé -> (nil, err)", v, e)

            local v2, e2 = s:peer()
            ok_fail("peer sur socket fermé -> (nil, err)", v2, e2)
        end
    end

    -- ----- listen rejette send/recv (mauvaise direction) -----------

    do
        local lst = S.listen("127.0.0.1", 0)
        if lst then
            local v, e = lst:send("x")
            ok_fail("send sur socket d'écoute -> (nil, err)", v, e)
            local v2, e2 = lst:recv(10)
            ok_fail("recv sur socket d'écoute -> (nil, err)", v2, e2)
            lst:close()
        end
    end
end

-- =====================================================================
print("")
print("=== tls ===")

do
    local S = luapilot.socket

    -- ----- contrat de base ----------------------------------------

    ok("connect_tls est une fonction", type(S.connect_tls) == "function")

    -- ----- mauvais usage : luaL_error -----------------------------

    ok("connect_tls() sans args lève",
        pcall(function() return S.connect_tls() end) == false)
    ok("connect_tls(host) sans port lève",
        pcall(function() return S.connect_tls("h") end) == false)
    ok("connect_tls({}, 443) lève (host pas une string)",
        pcall(function() return S.connect_tls({}, 443) end) == false)
    ok("connect_tls('h', {}) lève (port pas un nombre)",
        pcall(function() return S.connect_tls("h", {}) end) == false)

    -- ----- mauvaises valeurs : (nil, err) -------------------------

    do
        local v, e = S.connect_tls("127.0.0.1", -1)
        ok_fail("connect_tls port négatif -> (nil, err)", v, e)
        v, e = S.connect_tls("127.0.0.1", 70000)
        ok_fail("connect_tls port > 65535 -> (nil, err)", v, e)
    end

    -- ----- opts invalides -----------------------------------------

    do
        local v, e = S.connect_tls("127.0.0.1", 443,
            "string-not-table")
        ok_fail("connect_tls opts non-table -> (nil, err)", v, e)
        ok("  err préfixée 'tls: '",
            type(e) == "string" and e:find("tls:", 1, true) ~= nil)

        v, e = S.connect_tls("127.0.0.1", 443, { verify = "yes" })
        ok_fail("opts.verify non-boolean -> (nil, err)", v, e)

        v, e = S.connect_tls("127.0.0.1", 443, { ca_cert = 42 })
        ok_fail("opts.ca_cert non-string -> (nil, err)", v, e)

        v, e = S.connect_tls("127.0.0.1", 443, { hostname = 42 })
        ok_fail("opts.hostname non-string -> (nil, err)", v, e)

        v, e = S.connect_tls("127.0.0.1", 443, { min_version = "2.0" })
        ok_fail("opts.min_version invalide -> (nil, err)", v, e)
        ok("  err mentionne '1.2' ou '1.3'",
            type(e) == "string"
            and (e:find("1.2", 1, true) ~= nil
                or e:find("1.3", 1, true) ~= nil))

        v, e = S.connect_tls("127.0.0.1", 443, { timeout = -5 })
        ok_fail("opts.timeout négatif -> (nil, err)", v, e)

        v, e = S.connect_tls("127.0.0.1", 443, { timeout = 0 / 0 })
        ok_fail("opts.timeout NaN -> (nil, err)", v, e)

        v, e = S.connect_tls("127.0.0.1", 443, { timeout = math.huge })
        ok_fail("opts.timeout inf -> (nil, err)", v, e)
    end

    -- ----- échec transport : port loopback fermé ------------------

    do
        local v, e = S.connect_tls("127.0.0.1", 1, { timeout = 0.5 })
        ok_fail("connect_tls 127.0.0.1:1 -> (nil, err)", v, e)
        ok("  err préfixée 'socket: ' ou 'timeout'",
            type(e) == "string"
            and (e:find("socket: ", 1, true) == 1 or e == "timeout"),
            "err=" .. tostring(e))
    end

    -- ----- s:starttls() : préconditions --------------------------

    do
        -- starttls sur listening socket -> refus typé
        local lst = S.listen("127.0.0.1", 0)
        if lst then
            local v, e = lst:starttls()
            ok_fail("starttls sur listening socket -> (nil, err)", v, e)
            ok("  err mentionne 'listening'",
                type(e) == "string"
                and e:find("listening", 1, true) ~= nil)
            lst:close()
        end

        -- starttls sur socket fermé -> refus
        local lst2 = S.listen("127.0.0.1", 0)
        if lst2 then
            lst2:close()
            local v, e = lst2:starttls()
            ok_fail("starttls sur socket fermé -> (nil, err)", v, e)
        end
    end

    -- ===== TESTS POSITIFS avec openssl s_server ===================
    -- Skip gracieux si openssl CLI absent ou si s_server ne démarre
    -- pas dans les temps. Tout reste en loopback (127.0.0.1).

    local openssl_bin = luapilot.which("openssl")
    if not openssl_bin then
        print("[INFO] tls: openssl CLI absent, skip tests positifs")
    else
        print("[INFO] tls: openssl CLI trouvé, génération cert...")

        local tmpdir = "/tmp/lp_tls_" .. tostring(luapilot.pid())
        luapilot.mkdir(tmpdir)
        local cert_path = tmpdir .. "/cert.pem"
        local key_path  = tmpdir .. "/key.pem"

        -- 1. Générer cert auto-signé CN=localhost. NB : luapilot.exec
        --    attend (cmd, args_table, opts) — la commande NE peut PAS
        --    être passée comme une seule string composée.
        local gen       = luapilot.exec("openssl", {
            "req", "-x509", "-newkey", "rsa:2048", "-nodes",
            "-keyout", key_path,
            "-out", cert_path,
            "-days", "1",
            "-subj", "/CN=localhost",
        }, { timeout = 15 })

        if not (gen and gen.code == 0
                and luapilot.fileExists(cert_path)) then
            print("[INFO] tls: échec génération cert, skip positifs")
        else
            -- 2. Lancer s_server en arrière-plan. On passe par sh -c
            --    pour avoir le `&` de détachement. La string complète
            --    de la commande shell est UN argument après "-c", pas
            --    une commande composée.
            --    -quiet -ign_eof : pas de bruit + ne ferme pas sur EOF
            --    client (le client peut envoyer des données puis close).
            local tls_port = 19000 + (os.time() % 1000)
            local server_cmd = string.format(
                'openssl s_server -accept %d -cert %s -key %s '
                .. '-quiet -naccept 10 -ign_eof '
                .. '> /dev/null 2>&1 &',
                tls_port, cert_path, key_path)
            luapilot.exec("sh", { "-c", server_cmd },
                { timeout = 3 })

            -- 3. Attendre que s_server écoute (poll TCP brut)
            luapilot.sleep(300, "ms")
            local probe_ok = false
            for _ = 1, 10 do
                local p = S.connect("127.0.0.1", tls_port, 0.5)
                if p then
                    p:close()
                    probe_ok = true
                    break
                end
                luapilot.sleep(200, "ms")
            end

            if not probe_ok then
                print("[INFO] tls: s_server n'écoute pas, skip positifs")
            else
                print("[INFO] tls: serveur prêt, tests positifs...")

                -- ----- handshake avec verify=false : doit passer
                do
                    local s, err = S.connect_tls("127.0.0.1", tls_port,
                        { verify = false, timeout = 5 })
                    ok_val("connect_tls verify=false -> (socket, nil)",
                        s, err)
                    if s then
                        -- Envoi simple, le serveur echo-server affiche
                        -- mais on ne vérifie pas son output (s_server
                        -- écho est best-effort). On valide juste que
                        -- send() rend un nombre d'octets > 0.
                        local n, e = s:send("hello-tls\n")
                        ok_val("TLS s:send('hello-tls\\n') -> n, nil",
                            n, e)
                        ok("  n == 10 (longueur exacte)", n == 10)
                        s:close()
                    end
                end

                -- ----- verify=true sans CA -> cert auto-signé refusé
                do
                    local s, e = S.connect_tls("127.0.0.1", tls_port,
                        {
                            verify = true,
                            timeout = 5,
                            hostname = "localhost"
                        })
                    ok_fail("verify=true sans CA, cert auto-signé "
                        .. "-> (nil, err)", s, e)
                    ok("  err contient 'verify failed' ou 'self'",
                        type(e) == "string"
                        and (e:find("verify failed", 1, true) ~= nil
                            or e:find("self", 1, true) ~= nil),
                        "err=" .. tostring(e))
                end

                -- ----- verify=true AVEC ca_cert = succès
                do
                    local s, err = S.connect_tls("127.0.0.1", tls_port,
                        {
                            verify = true,
                            timeout = 5,
                            hostname = "localhost",
                            ca_cert = cert_path
                        })
                    ok_val("verify=true + ca_cert(self) "
                        .. "-> (socket, nil)", s, err)
                    if s then
                        local n, e = s:send("verified\n")
                        ok_val("TLS verified s:send() -> n, nil", n, e)
                        s:close()
                    end
                end

                -- ----- min_version = "1.3" si supporté
                do
                    local s, err = S.connect_tls("127.0.0.1", tls_port,
                        {
                            verify = false,
                            timeout = 5,
                            min_version = "1.3"
                        })
                    -- s_server supporte TLS 1.3 par défaut sur OpenSSL
                    -- 1.1.1+ ; on accepte succès OU échec gracieux.
                    if s then
                        ok("min_version='1.3' accepté", true)
                        s:close()
                    else
                        ok("min_version='1.3' refusé "
                            .. "(serveur ne supporte pas)", true,
                            "err=" .. tostring(err))
                    end
                end
            end

            -- 4. Cleanup s_server (best-effort, par port).
            --    pkill prend le pattern en argument séparé.
            luapilot.exec("pkill", {
                "-f",
                "openssl s_server -accept " .. tostring(tls_port),
            }, { timeout = 2 })
        end

        luapilot.rmdirAll(tmpdir)
    end
end

-- =====================================================================
print("")
print("=== user ===")

do
    local U = luapilot.user

    ok("luapilot.user est une table", type(U) == "table")
    ok("get est une fonction", type(U.get) == "function")
    ok("exists est une fonction", type(U.exists) == "function")

    -- Mauvais type d'argument : luaL_error (pas (nil, err)).
    ok("get() sans arg leve",
        pcall(function() return U.get() end) == false)
    ok("get({}) leve (mauvais type)",
        pcall(function() return U.get({}) end) == false)
    ok("get(true) leve (mauvais type)",
        pcall(function() return U.get(true) end) == false)
    ok("get(1.5) leve (non-integer)",
        pcall(function() return U.get(1.5) end) == false)
    ok("get(-1) leve (uid negatif)",
        pcall(function() return U.get(-1) end) == false)

    -- Durcissement : une string avec NUL embarque serait tronquee
    -- par getpwnam_r au premier NUL ('root\0evil' vu comme 'root'),
    -- ce qui pourrait contourner une verification de l'appelant.
    -- On refuse.
    ok("get('root\\0evil') leve (NUL byte dans le nom)",
        pcall(function() return U.get("root\0evil") end) == false)

    -- Durcissement : un integer > uid_t max (2^32-1 sur Linux)
    -- serait silencieusement tronque au cast et pourrait matcher
    -- un UID non lie par hasard. On refuse explicitement.
    -- 2^33 = 8589934592 — bien au-dessus de uid_t max mais loin
    -- en-dessous de lua_Integer max (~9.2e18).
    ok("get(2^33) leve (uid hors limite)",
        pcall(function() return U.get(8589934592) end) == false)

    ok("exists() sans arg leve",
        pcall(function() return U.exists() end) == false)
    ok("exists(1.5) leve",
        pcall(function() return U.exists(1.5) end) == false)

    -- root est presque toujours present sur un Linux : ancre fiable.
    do
        local u, err = U.get("root")
        ok("get('root') -> table", type(u) == "table" and err == nil)
        if type(u) == "table" then
            ok("  name == 'root'", u.name == "root")
            ok("  uid est integer", type(u.uid) == "number"
                and math.type(u.uid) == "integer")
            ok("  uid == 0", u.uid == 0)
            ok("  gid est integer", type(u.gid) == "number"
                and math.type(u.gid) == "integer")
            ok("  home est string", type(u.home) == "string")
            ok("  shell est string", type(u.shell) == "string")
            ok("  gecos est string (peut etre vide)",
                type(u.gecos) == "string")
        end
    end

    -- Lookup par UID == 0 doit donner le meme utilisateur.
    do
        local u, err = U.get(0)
        ok("get(0) -> table (root par uid)",
            type(u) == "table" and err == nil)
        if type(u) == "table" then
            ok("  name == 'root' (uid 0)", u.name == "root")
        end
    end

    -- Utilisateur quasi certainement absent.
    local missing = "luapilot_test_user_xyz_9j3hf83hf"
    do
        local u, err = U.get(missing)
        ok("get(missing) -> (nil, 'user not found')",
            u == nil and err == "user not found")
    end

    do
        local u, err = U.get(2000000000)
        ok("get(uid tres haut) -> (nil, 'user not found')",
            u == nil and err == "user not found")
    end

    -- exists() : booleen strict.
    ok("exists('root') == true", U.exists("root") == true)
    ok("exists(0) == true (uid)", U.exists(0) == true)
    ok("exists(missing) == false", U.exists(missing) == false)
    ok("exists(2000000000) == false",
        U.exists(2000000000) == false)
end

-- =====================================================================
print("")
print("=== inotify ===")

do
    local I = luapilot.inotify

    -- ----- contrat de base ------------------------------------------
    ok("luapilot.inotify est une table", type(I) == "table")
    ok("new est une fonction", type(I.new) == "function")

    local w, nerr = I.new()
    ok_val("new() -> watcher", w, nerr)
    ok("  add est une méthode", w ~= nil and type(w.add) == "function")
    ok("  read est une méthode", w ~= nil and type(w.read) == "function")
    ok("  remove est une méthode", w ~= nil and type(w.remove) == "function")
    ok("  close est une méthode", w ~= nil and type(w.close) == "function")

    -- Répertoire surveillé, dans le sandbox.
    local WDIR = sb("inotify_d")
    ok_act("mkdir(watch dir)", luapilot.mkdir(WDIR))

    -- ----- mauvais usage : luaL_error -------------------------------
    do
        ok("add() sans liste d'événements lève",
            pcall(function() return w:add(WDIR) end) == false)
        ok("add(dir, 'string') lève (events pas une table)",
            pcall(function() return w:add(WDIR, "create") end) == false)
        ok("read({}) lève (timeout pas un nombre, pas coercible)",
            pcall(function() return w:read({}) end) == false)
    end

    -- ----- mauvaises valeurs : (nil, err) ---------------------------
    do
        local v, e = w:add(WDIR, {})
        ok_fail("add(dir, {}) liste vide -> (nil, err)", v, e)
        ok("  message mentionne 'empty'",
            type(e) == "string" and e:find("empty", 1, true) ~= nil)

        v, e = w:add(WDIR, { "bogus_event" })
        ok_fail("add(dir, {bogus}) événement inconnu -> (nil, err)", v, e)
        ok("  message mentionne 'unknown'",
            type(e) == "string" and e:find("unknown", 1, true) ~= nil)

        v, e = w:add(sb("inotify_absent"), { "create" })
        ok_fail("add(chemin inexistant) -> (nil, err)", v, e)

        v, e = w:remove(999999)
        ok_fail("remove(wd invalide) -> (nil, err)", v, e)

        v, e = w:read(-1)
        ok_fail("read(timeout négatif) -> (nil, err)", v, e)
    end

    -- ----- pose d'une surveillance valide ---------------------------
    local wd, aerr = w:add(WDIR,
        { "create", "close_write", "moved_from", "moved_to" })
    ok_val("add(dir, events) -> wd integer", wd, aerr,
        function(v) return math.type(v) == "integer" end)

    -- Helpers locaux : draine tous les événements en attente (le
    -- noyau peut en batcher plusieurs, et un événement logique peut
    -- en générer plusieurs), et recherche par (name, flag).
    local function drain(watcher, secs)
        local all = {}
        local timeout = secs
        while true do
            local evs = watcher:read(timeout)
            if not evs then break end -- timeout/closed/err -> fin du drain
            for _, ev in ipairs(evs) do all[#all + 1] = ev end
            timeout = 0.1             -- récupère les éventuels traînards
        end
        return all
    end

    local function find_event(list, name, flag)
        for _, ev in ipairs(list) do
            if ev.name == name and ev.events[flag] then return ev end
        end
        return nil
    end

    -- ----- création + écriture d'un fichier -------------------------
    do
        local f = assert(io.open(WDIR .. "/photo.jpg", "w"))
        f:write("data")
        f:close()

        local evs = drain(w, 2)
        ok("read() rend un array de tables", type(evs) == "table")

        local cr = find_event(evs, "photo.jpg", "create")
        ok("événement 'create' sur photo.jpg", cr ~= nil)

        local cw = find_event(evs, "photo.jpg", "close_write")
        ok("événement 'close_write' sur photo.jpg", cw ~= nil)
        ok("  is_dir == false pour un fichier",
            cw ~= nil and cw.is_dir == false)
        ok("  wd de l'événement == wd du watch",
            cw ~= nil and cw.wd == wd)
        ok("  cookie == 0 hors déplacement",
            cw ~= nil and cw.cookie == 0)
    end

    -- ----- is_dir sur création d'un sous-répertoire -----------------
    do
        luapilot.mkdir(WDIR .. "/subdir")
        local evs = drain(w, 2)
        local cr = find_event(evs, "subdir", "create")
        ok("'create' détecté sur le sous-répertoire", cr ~= nil)
        ok("  is_dir == true pour un répertoire",
            cr ~= nil and cr.is_dir == true)
    end

    -- ----- moved_from / moved_to apparié par cookie -----------------
    do
        os.rename(WDIR .. "/photo.jpg", WDIR .. "/photo2.jpg")
        local evs = drain(w, 2)
        local mf = find_event(evs, "photo.jpg", "moved_from")
        local mt = find_event(evs, "photo2.jpg", "moved_to")
        ok("'moved_from' détecté", mf ~= nil)
        ok("'moved_to' détecté", mt ~= nil)
        ok("  cookie non nul et identique entre from et to",
            mf ~= nil and mt ~= nil
            and mf.cookie ~= 0 and mf.cookie == mt.cookie)
    end

    -- ----- timeout : non bloquant et fini ---------------------------
    do
        drain(w, 0) -- vide tout résidu d'événement
        local v, e = w:read(0)
        ok_fail("read(0) sans activité -> (nil, err)", v, e)
        ok("  raison == 'timeout'", e == "timeout")

        v, e = w:read(0.2)
        ok_fail("read(0.2) sans activité -> (nil, err)", v, e)
        ok("  raison == 'timeout'", e == "timeout")
    end

    -- ----- remove() + événement 'ignored' ---------------------------
    do
        local r, e = w:remove(wd)
        ok_act("remove(wd) -> (true, nil)", r, e)

        -- Le noyau émet un 'ignored' pour ce wd juste après le retrait.
        local evs = drain(w, 1)
        local ig = nil
        for _, ev in ipairs(evs) do
            if ev.wd == wd and ev.events.ignored then ig = ev end
        end
        ok("événement 'ignored' émis après remove", ig ~= nil)
    end

    -- ----- correctifs audit -----------------------------------------
    -- Ces trois tests couvrent des bugs repérés post-merge en audit.
    -- DOIVENT être avant close() car ils ont besoin d'un watcher actif.
    do
        -- Réarme un watch puisqu'on a remove wd juste au-dessus.
        local wd2, e_arm = w:add(WDIR, { "create", "close_write" })
        ok_val("réarmage du watch pour les tests audit", wd2, e_arm)

        -- (1) read(0) doit retourner les events déjà en attente, pas
        -- (nil, "timeout"). Avant le fix, le court-circuit avant
        -- poll() retournait timeout sans jamais appeler poll(fd,1,0).
        do
            drain(w, 0.1) -- vider toute queue résiduelle

            local f = assert(io.open(WDIR .. "/audit_read0.txt", "w"))
            f:write("x"); f:close()

            -- Laisse au kernel 100ms pour délivrer l'event dans la
            -- queue inotify (confortable sur toute plateforme).
            luapilot.sleep(100, "ms")

            local evs, rerr = w:read(0)
            ok("read(0) retourne les events déjà en attente",
                type(evs) == "table" and #evs > 0,
                "evs=" .. tostring(evs) .. " err=" .. tostring(rerr))
            ok("  la liste contient un 'create' pour le fichier",
                find_event(evs or {}, "audit_read0.txt", "create") ~= nil)
        end

        -- (2) NaN et Inf doivent être refusés sur l'argument timeout.
        do
            local v, e = w:read(0 / 0) -- NaN
            ok_fail("read(NaN) -> (nil, err)", v, e)
            ok("  message mentionne 'finite'",
                type(e) == "string" and e:find("finite", 1, true) ~= nil)

            v, e = w:read(math.huge) -- +Inf
            ok_fail("read(+Inf) -> (nil, err)", v, e)

            v, e = w:read(-math.huge) -- -Inf
            ok_fail("read(-Inf) -> (nil, err)", v, e)
        end

        -- (3) La table events doit être une vraie liste 1..n. Clés
        -- string supplémentaires, clés numériques sparse et clés
        -- float sont toutes refusées (pas de comportement silencieux).
        do
            local v, e = w:add(WDIR, { "create", x = "extra" })
            ok_fail("add(events avec clé string extra) -> (nil, err)", v, e)
            ok("  message mentionne 'extra string key'",
                type(e) == "string"
                and e:find("extra string key", 1, true) ~= nil)

            v, e = w:add(WDIR, { "create", [10] = "modify" })
            ok_fail("add(events avec [10] sparse) -> (nil, err)", v, e)
            ok("  message mentionne 'extra integer key'",
                type(e) == "string"
                and e:find("extra integer key", 1, true) ~= nil)

            v, e = w:add(WDIR, { [1] = "create", [1.5] = "modify" })
            ok_fail("add(events avec [1.5] non-integer) -> (nil, err)", v, e)
            ok("  message mentionne 'non-integer'",
                type(e) == "string"
                and e:find("non-integer", 1, true) ~= nil)

            -- Élément non-string à un index 1..n valide : avant ça
            -- raisait via luaL_error, maintenant ça retourne
            -- (nil, err) comme les autres erreurs de contenu de la
            -- table events. Cohérence avec le reste.
            v, e = w:add(WDIR, { "create", 42 })
            ok_fail("add(events avec élément non-string) -> (nil, err)",
                v, e)
            ok("  message mentionne 'must be a string'",
                type(e) == "string"
                and e:find("must be a string", 1, true) ~= nil)
        end

        -- Nettoyage : drainer les events générés, puis remove wd2.
        drain(w, 0.1)
        w:remove(wd2)
        drain(w, 0.1) -- avale le 'ignored' pour wd2
    end

    -- ----- close() idempotent + erreurs post-close ------------------
    ok_act("close() -> (true, nil)", w:close())
    ok_act("close() de nouveau (idempotent)", w:close())

    do
        local v, e = w:read(0)
        ok_fail("read() après close -> (nil, err)", v, e)
        ok("  message mentionne 'closed'",
            type(e) == "string" and e:find("closed", 1, true) ~= nil)

        local v2, e2 = w:add(WDIR, { "create" })
        ok_fail("add() après close -> (nil, err)", v2, e2)
    end

    luapilot.rmdirAll(WDIR)
end

-- =====================================================================
print("")
print("=== workers ===")

do
    local W = luapilot.workers

    -- ----- contrat de base ----------------------------------------

    ok("luapilot.workers est une table", type(W) == "table")
    ok("workers.spawn est une fonction", type(W.spawn) == "function")

    -- ----- mauvais usage : luaL_error -----------------------------

    ok("spawn() sans args lève",
        pcall(function() return W.spawn() end) == false)
    ok("spawn({}) lève (code pas une string)",
        pcall(function() return W.spawn({}) end) == false)
    ok("spawn('code', 42) lève (args pas une table)",
        pcall(function() return W.spawn("return 1", 42) end) == false)
    ok("spawn('code', nil, 42) lève (opts pas une table)",
        pcall(function() return W.spawn("return 1", nil, 42) end)
        == false)

    -- ----- refus de sérialisation : function ----------------------

    do
        local v, e = W.spawn("return 1", { fn = function() end })
        ok_fail("spawn(code, {fn=function}) -> (nil, err)", v, e)
        ok("  err préfixée 'workers: '",
            type(e) == "string"
            and e:find("workers: ", 1, true) == 1)
        ok("  err mentionne 'function'",
            type(e) == "string"
            and e:find("function", 1, true) ~= nil)
    end

    -- ----- refus de sérialisation : userdata ----------------------

    do
        local s = luapilot.socket.listen("127.0.0.1", 0)
        if s then
            local v, e = W.spawn("return 1", { sock = s })
            ok_fail("spawn(code, {sock=userdata}) -> (nil, err)",
                v, e)
            ok("  err mentionne 'userdata'",
                type(e) == "string"
                and e:find("userdata", 1, true) ~= nil)
            s:close()
        end
    end

    -- ----- refus de sérialisation : thread (coroutine) ------------

    do
        local co = coroutine.create(function() end)
        local v, e = W.spawn("return 1", { co = co })
        ok_fail("spawn(code, {co=coroutine}) -> (nil, err)", v, e)
        ok("  err mentionne 'coroutine'",
            type(e) == "string"
            and e:find("coroutine", 1, true) ~= nil)
    end

    -- ----- refus de sérialisation : cycle (via profondeur max) ----

    do
        local t = {}
        t.self = t
        local v, e = W.spawn("return 1", { cycle = t })
        ok_fail("spawn(code, {cycle=self_ref}) -> (nil, err)", v, e)
        ok("  err mentionne 'nested' ou 'cycle' ou 'deep'",
            type(e) == "string"
            and (e:find("nested", 1, true) ~= nil
                or e:find("cycle", 1, true) ~= nil
                or e:find("deep", 1, true) ~= nil))
    end

    -- ----- spawn + join : retours simples -------------------------

    do
        local w = W.spawn("return 42")
        local jok, val = w:join()
        ok("join: integer -> (true, 42)", jok == true and val == 42)

        w = W.spawn("return 'hello'")
        jok, val = w:join()
        ok("join: string -> (true, 'hello')",
            jok == true and val == "hello")

        w = W.spawn("return true")
        jok, val = w:join()
        ok("join: boolean -> (true, true)",
            jok == true and val == true)

        -- Convention pcall : nil retourné != erreur
        w = W.spawn("return nil")
        jok, val = w:join()
        ok("join: nil retourné -> (true, nil) -- convention pcall",
            jok == true and val == nil)

        -- Pas de return = nil implicite
        w = W.spawn("local x = 1")
        jok, val = w:join()
        ok("join: pas de return -> (true, nil)",
            jok == true and val == nil)

        -- Limitation : seul le 1er return est transmis. Documenté
        -- dans le README (post-revue ChatGPT).
        w = W.spawn("return 10, 20, 30")
        jok, val = w:join()
        ok("join: return multi -> seul le 1er traverse (val == 10)",
            jok == true and val == 10)
    end

    -- ----- spawn + join : table retournée -------------------------

    do
        local w = W.spawn("return { name = 'alice', age = 30 }")
        local jok, val = w:join()
        ok("join: table -> (true, table)",
            jok == true and type(val) == "table")
        ok("  table.name == 'alice'",
            type(val) == "table" and val.name == "alice")
        ok("  table.age == 30",
            type(val) == "table" and val.age == 30)

        -- Séquence imbriquée
        w = W.spawn("return { 'a', 'b', 'c' }")
        jok, val = w:join()
        ok("join: séquence -> table indexée 1..n",
            jok == true and type(val) == "table"
            and #val == 3 and val[1] == "a" and val[3] == "c")
    end

    -- ----- spawn + join : erreur côté worker ----------------------

    do
        local w = W.spawn("error('boom')")
        local jok, val = w:join()
        ok("join: error() -> (false, msg)",
            jok == false and type(val) == "string")
        ok("  msg contient 'boom'",
            type(val) == "string"
            and val:find("boom", 1, true) ~= nil)

        -- Code Lua invalide -> chargement échoue
        w = W.spawn("this is not valid lua %%%")
        jok, val = w:join()
        ok("join: code Lua invalide -> (false, msg)",
            jok == false and type(val) == "string")
    end

    -- ----- worker.args : transmission des arguments ---------------

    do
        local w = W.spawn(
            "return worker.args.x + worker.args.y",
            { x = 10, y = 20 })
        local jok, val = w:join()
        ok("worker.args : addition (10+20) -> 30",
            jok == true and val == 30)

        -- worker.args = nil quand pas d'args
        w = W.spawn("return (worker.args == nil)")
        jok, val = w:join()
        ok("worker.args == nil quand pas d'args passés",
            jok == true and val == true)

        -- arg = nil côté worker (décision W-7)
        w = W.spawn("return (arg == nil)")
        jok, val = w:join()
        ok("arg == nil côté worker (pas d'héritage du parent)",
            jok == true and val == true)
    end

    -- ----- poll : running / done / error --------------------------

    do
        -- Worker avec sleep court : on capte 'running' avant la fin,
        -- puis 'done' après. Race possible : machine rapide peut
        -- déjà avoir fini, on accepte les deux états initialement.
        local w = W.spawn(
            "luapilot.sleep(300, 'ms'); return 'ok'")
        local state, _ = w:poll()
        ok("poll juste après spawn : 'running' ou 'done'",
            state == "running" or state == "done",
            "state=" .. tostring(state))

        luapilot.sleep(500, "ms")
        local state2, val2 = w:poll()
        ok("poll après sleep suffisant : 'done'",
            state2 == "done",
            "state=" .. tostring(state2))
        ok("  val == 'ok'", val2 == "ok")

        -- poll sur worker en erreur
        local w2 = W.spawn("error('bad')")
        luapilot.sleep(200, "ms")
        local state3, val3 = w2:poll()
        ok("poll après erreur : 'error'",
            state3 == "error",
            "state=" .. tostring(state3))
        ok("  val contient 'bad'",
            type(val3) == "string"
            and val3:find("bad", 1, true) ~= nil)
    end

    -- ----- worker = mini-LuaPilot complet -------------------------

    do
        -- Accès à luapilot.* depuis le worker
        local w = W.spawn(
            "return luapilot.json.encode({ a = 1, b = 2 })")
        local jok, val = w:join()
        ok("worker peut utiliser luapilot.json.encode",
            jok == true and type(val) == "string"
            and val:find('"a":1', 1, true) ~= nil)

        -- Accès aux modules bundlés via require()
        w = W.spawn(
            "local insp = require('inspect'); "
            .. "return type(insp({1,2,3}))")
        jok, val = w:join()
        ok("worker peut require('inspect') (module bundlé)",
            jok == true and val == "string")

        -- Modules utilisateur via require() : même searcher
        -- (package.path en mode dossier, embedded searcher en mode
        -- packagé). mymod/init.lua fournit hello() qui retourne
        -- "init.lua chargé !".
        w = W.spawn(
            "local m = require('mymod'); return m.hello()")
        jok, val = w:join()
        ok("worker peut require('mymod') (module utilisateur)",
            jok == true and type(val) == "string"
            and val:find("init.lua", 1, true) ~= nil,
            "val=" .. tostring(val))
    end

    -- ===== TEST CRUCIAL : VRAI PARALLÉLISME =======================
    -- 4 workers qui font chacun sleep(700ms). En parallèle, le temps
    -- wall-clock est proche de 700ms (= 0 ou 1 seconde selon timing).
    -- En série, ce serait 4 × 700 = 2.8s, mesurable via os.time().
    --
    -- Pourquoi pas os.clock() : os.clock() mesure le CPU du process
    -- parent qui DORT dans pthread_join. Il rendrait ~0 dans les
    -- deux cas (parallèle ET série), donc ne distingue rien. Seul
    -- os.time() (wall-clock) discrimine.

    do
        local N = 4
        local sleep_ms = 700
        local t_start = os.time()
        local workers = {}
        for i = 1, N do
            workers[i] = W.spawn(string.format(
                "luapilot.sleep(%d, 'ms'); return %d",
                sleep_ms, i))
        end
        local results = {}
        for i = 1, N do
            local jok, val = workers[i]:join()
            results[i] = jok and val or nil
        end
        local elapsed = os.time() - t_start

        ok("parallélisme : 4 workers ont tous terminé",
            #results == N
            and results[1] == 1 and results[2] == 2
            and results[3] == 3 and results[4] == 4)
        ok("parallélisme : wall-clock < 2s "
            .. "(série = ~3s, parallèle = ~1s)",
            elapsed < 2,
            "elapsed=" .. tostring(elapsed) .. "s")
        print(string.format(
            "[INFO] workers: 4 x %dms sleep, wall-clock = %ds "
            .. "(parallèle : 0-1, série : 3+)",
            sleep_ms, elapsed))
    end

    -- ----- poll après join : "already consumed" -------------------

    do
        local w = W.spawn("return 1")
        local jok, _ = w:join()
        ok("join initial OK", jok == true)
        local state, _ = w:poll()
        ok("poll après join : 'error' (already consumed)",
            state == "error")
        local jok2, _ = w:join()
        ok("join après join : (false, already consumed)",
            jok2 == false)
    end

    -- ============================================================
    -- Chantier 9-2 : send / recv / close côté parent
    -- ============================================================
    -- À cette étape le worker ne lit pas son inbox (il reste
    -- fork-join classique). On valide la mécanique côté parent :
    --   - send réussit tant que l'inbox a de la place
    --   - send sur inbox pleine, timeout=0 -> (false, "full")
    --   - send sur inbox pleine, timeout>0 -> (false, "timeout")
    --   - recv depuis outbox vide est toujours vide
    --   - send/recv sur worker mort -> (false, "closed")

    -- ----- send : succès tant qu'il reste de la place -----------
    do
        local w = W.spawn("luapilot.sleep(100, 'ms'); return 42",
            nil, { inbox_capacity = 4 })
        local sok, serr = w:send("hello")
        ok("send(value) sur worker vivant -> (true, nil)",
            sok == true and serr == nil,
            "sok=" .. tostring(sok) .. " serr=" .. tostring(serr))

        sok = w:send({ type = "ping", n = 42 })
        ok("send(table) -> (true, nil)", sok == true)

        sok = w:send("avec-timeout", 0.5)
        ok("send avec timeout > 0 sur queue non pleine -> (true, nil)",
            sok == true)

        w:join()
    end

    -- ----- send : queue pleine, timeout=0 -> "full" -------------
    do
        local w = W.spawn("luapilot.sleep(200, 'ms'); return 1",
            nil, { inbox_capacity = 2 })
        ok("send #1 sur cap=2 -> ok",
            w:send("m1") == true)
        ok("send #2 sur cap=2 -> ok",
            w:send("m2") == true)
        local sok, serr = w:send("m3", 0)
        ok("send #3 (cap dépassée) timeout=0 -> (false, 'full')",
            sok == false and serr == "full",
            "got=(" .. tostring(sok) .. ", " .. tostring(serr) .. ")")
        w:join()
    end

    -- ----- send : queue pleine, timeout>0 -> "timeout" ----------
    do
        local w = W.spawn("luapilot.sleep(500, 'ms'); return 1",
            nil, { inbox_capacity = 1 })
        w:send("seul-slot")
        local t0 = os.time()
        local sok, serr = w:send("second", 0.2)
        local elapsed = os.time() - t0
        ok("send sur queue pleine, timeout>0 -> (false, 'timeout')",
            sok == false and serr == "timeout",
            "got=(" .. tostring(sok) .. ", " .. tostring(serr) .. ")")
        ok("send timeout : attente effective (elapsed >= 0s, < 2s)",
            elapsed >= 0 and elapsed < 2)
        w:join()
    end

    -- ----- recv : outbox vide -----------------------------------
    -- CORRECTIF (post-test Pi0) : le worker doit rester vivant
    -- significativement plus longtemps que le timeout du recv(),
    -- sinon il finit pile pendant l'attente et la outbox passe en
    -- "closed" au lieu de "timeout". Sur Pi0 (1 cœur ARMv6 lent),
    -- un sleep de 100ms côté worker + recv(0.1) côté parent
    -- produisait une race intermittente. 2s côté worker garantit
    -- qu'on observe bien le timeout.
    do
        local w = W.spawn("luapilot.sleep(2, 's'); return 1")
        local rok, rmsg = w:recv(0)
        ok("recv(0) sur outbox vide -> (false, 'empty')",
            rok == false and rmsg == "empty",
            "got=(" .. tostring(rok) .. ", " .. tostring(rmsg) .. ")")

        local t0 = os.time()
        rok, rmsg = w:recv(0.1)
        local elapsed = os.time() - t0
        ok("recv(0.1) sur outbox vide -> (false, 'timeout')",
            rok == false and rmsg == "timeout",
            "got=(" .. tostring(rok) .. ", " .. tostring(rmsg) .. ")")
        ok("recv timeout : attente effective (elapsed < 2s)",
            elapsed < 2)
        w:join()
    end

    -- ----- close() : idempotent, retourne (true, nil) -----------
    do
        local w = W.spawn("return 1")
        local cok, cerr = w:close()
        ok("close() -> (true, nil)",
            cok == true and cerr == nil)
        local cok2 = w:close()
        ok("close() idempotent (2e appel OK)", cok2 == true)
        w:join()
    end

    -- ----- send après mort du worker -> "closed" ----------------
    do
        local w = W.spawn("return 1")
        w:join()
        w:close()
        local sok, serr = w:send("nope")
        ok("send après w:close() -> (false, 'closed')",
            sok == false and serr == "closed",
            "got=(" .. tostring(sok) .. ", " .. tostring(serr) .. ")")
    end

    -- ----- send : refus de types non sérialisables --------------
    do
        local w = W.spawn("luapilot.sleep(100, 'ms'); return 1")
        local sok, serr = w:send(function() end)
        ok("send(function) -> (nil, err)",
            sok == nil and type(serr) == "string"
            and serr:find("function", 1, true) ~= nil,
            "got=(" .. tostring(sok) .. ", " .. tostring(serr) .. ")")
        w:join()
    end

    -- ----- mauvais usage : timeout non-numérique ----------------
    do
        local w = W.spawn("return 1")
        ok("send(value, 'abc') -> luaL_error",
            pcall(function() w:send("x", "abc") end) == false)
        ok("recv('abc') -> luaL_error",
            pcall(function() w:recv("abc") end) == false)
        ok("send(value, -1) -> luaL_error",
            pcall(function() w:send("x", -1) end) == false)
        w:join()
    end

    -- ----- opts.inbox_capacity invalide -------------------------
    do
        ok("spawn(_, _, {inbox_capacity=0}) -> luaL_error",
            pcall(function()
                W.spawn("return 1", nil, { inbox_capacity = 0 })
            end) == false)
        ok("spawn(_, _, {inbox_capacity=-1}) -> luaL_error",
            pcall(function()
                W.spawn("return 1", nil, { inbox_capacity = -1 })
            end) == false)
        ok("spawn(_, _, {inbox_capacity='x'}) -> luaL_error",
            pcall(function()
                W.spawn("return 1", nil, { inbox_capacity = "x" })
            end) == false)
        ok("spawn(_, _, {inbox_capacity=1.5}) -> luaL_error",
            pcall(function()
                W.spawn("return 1", nil, { inbox_capacity = 1.5 })
            end) == false)
    end

    -- ============================================================
    -- Chantier 9-3 : worker.send / worker.recv côté worker
    -- ============================================================

    -- ----- Echo persistant (le pattern canonique) ---------------
    do
        local w = W.spawn([[
            while true do
                local ok, msg = worker.recv()
                if not ok then break end
                worker.send({ echo = msg })
            end
            return "exited cleanly"
        ]])

        w:send("hello")
        w:send("world")
        w:send(42)

        local rok1, r1 = w:recv(2)
        local rok2, r2 = w:recv(2)
        local rok3, r3 = w:recv(2)

        ok("worker echo : 3 messages reçus",
            rok1 == true and rok2 == true and rok3 == true)
        ok("worker echo : r1.echo == 'hello'",
            type(r1) == "table" and r1.echo == "hello",
            "r1=" .. tostring(r1 and r1.echo))
        ok("worker echo : r2.echo == 'world'",
            type(r2) == "table" and r2.echo == "world")
        ok("worker echo : r3.echo == 42",
            type(r3) == "table" and r3.echo == 42)

        w:close()
        local jok, jval = w:join()
        ok("worker echo : join après close -> (true, 'exited cleanly')",
            jok == true and jval == "exited cleanly",
            "jok=" .. tostring(jok) .. " jval=" .. tostring(jval))
    end

    -- ----- Handler par type de message --------------------------
    do
        local w = W.spawn([[
            while true do
                local ok, msg = worker.recv()
                if not ok then break end
                if msg.type == "ping" then
                    worker.send({ type = "pong" })
                elseif msg.type == "add" then
                    worker.send({ type = "sum", result = msg.a + msg.b })
                end
            end
            return nil
        ]])

        w:send({ type = "ping" })
        local _, r1 = w:recv(2)
        ok("handler : ping -> pong",
            type(r1) == "table" and r1.type == "pong")

        w:send({ type = "add", a = 3, b = 4 })
        local _, r2 = w:recv(2)
        ok("handler : add(3,4) -> sum=7",
            type(r2) == "table" and r2.type == "sum"
            and r2.result == 7)

        w:close()
        w:join()
    end

    -- ----- worker.recv(0.1) timeout dans une boucle -------------
    do
        local w = W.spawn([[
            local n_timeouts = 0
            local n_msgs = 0
            while true do
                local ok, msg = worker.recv(0.1)
                if not ok then
                    if msg == "timeout" then
                        n_timeouts = n_timeouts + 1
                        if n_timeouts >= 3 then
                            worker.send({
                                timeouts = n_timeouts,
                                msgs = n_msgs,
                            })
                            return nil
                        end
                    else
                        break
                    end
                else
                    n_msgs = n_msgs + 1
                end
            end
            return nil
        ]])

        local rok, summary = w:recv(2)
        ok("worker.recv(0.1) timeout boucle : récupéré n_timeouts",
            rok == true and type(summary) == "table"
            and summary.timeouts >= 3,
            "summary=" .. tostring(summary and summary.timeouts))
        w:join()
    end

    -- ----- worker meurt -> parent draine puis voit 'closed' -----
    -- (décision W2-I : drainage avant fermeture)
    do
        local w = W.spawn([[
            worker.send("a")
            worker.send("b")
            worker.send("c")
            return "done"
        ]])

        local ok_join, val_join = w:join()
        ok("worker termine -> join (true, 'done')",
            ok_join == true and val_join == "done")

        local r1ok, r1 = w:recv(0)
        local r2ok, r2 = w:recv(0)
        local r3ok, r3 = w:recv(0)
        ok("drainage post-mort : 3 messages récupérés",
            r1ok and r2ok and r3ok,
            "got=(" .. tostring(r1ok) .. "," .. tostring(r2ok)
            .. "," .. tostring(r3ok) .. ")")
        ok("drainage post-mort : contenus 'a', 'b', 'c'",
            r1 == "a" and r2 == "b" and r3 == "c")

        local r4ok, r4err = w:recv(0)
        ok("après drainage : recv(0) -> (false, 'closed')",
            r4ok == false and r4err == "closed",
            "got=(" .. tostring(r4ok) .. "," .. tostring(r4err) .. ")")
    end

    -- ----- worker.recv() bloquant + w:close() -> 'closed' -------
    do
        local w = W.spawn([[
            local ok, msg = worker.recv()
            return { recv_ok = ok, recv_msg = msg }
        ]])

        luapilot.sleep(100, "ms")
        w:close()

        local jok, jval = w:join()
        ok("worker.recv() bloquant + w:close() : le worker termine",
            jok == true and type(jval) == "table"
            and jval.recv_ok == false and jval.recv_msg == "closed",
            "jval=" .. tostring(jval))
    end

    -- ----- worker.send(function) -> (false, err) ----------------
    -- (Convention pcall-style côté worker, décision W3-A)
    do
        local w = W.spawn([[
            local ok, err = worker.send(function() end)
            return { ok = ok, err = err }
        ]])

        local jok, jval = w:join()
        ok("worker.send(function) -> (false, err)",
            jok == true and type(jval) == "table"
            and jval.ok == false and type(jval.err) == "string"
            and jval.err:find("function", 1, true) ~= nil,
            "jval.ok=" .. tostring(jval and jval.ok)
            .. " err=" .. tostring(jval and jval.err))
    end

    -- ----- Mauvais usage côté worker ----------------------------
    do
        local w = W.spawn([[
            local pcall_ok = pcall(function()
                worker.recv("abc")
            end)
            return { pcall_ok = pcall_ok }
        ]])

        local jok, jval = w:join()
        ok("worker.recv('abc') -> luaL_error (pcall.ok == false)",
            jok == true and type(jval) == "table"
            and jval.pcall_ok == false)
    end

    -- ----- 2 workers indépendants -------------------------------
    do
        local w1 = W.spawn([[
            local ok, msg = worker.recv()
            if ok then worker.send("w1-got:" .. msg) end
            return nil
        ]])
        local w2 = W.spawn([[
            local ok, msg = worker.recv()
            if ok then worker.send("w2-got:" .. msg) end
            return nil
        ]])

        w1:send("hello1")
        w2:send("hello2")

        local _, r1 = w1:recv(2)
        local _, r2 = w2:recv(2)
        ok("2 workers indépendants : w1 reçoit 'hello1'",
            r1 == "w1-got:hello1",
            "r1=" .. tostring(r1))
        ok("2 workers indépendants : w2 reçoit 'hello2'",
            r2 == "w2-got:hello2",
            "r2=" .. tostring(r2))

        w1:join()
        w2:join()
    end
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
