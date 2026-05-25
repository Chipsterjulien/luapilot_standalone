-- argparse.lua — analyseur d'arguments en ligne de commande, Lua pur,
-- bundlé dans LuaPilot via tools/embed_lua_module.sh (require("argparse")).
--
-- Contrat observable (décisions actées avec le mainteneur) :
--
--   local argparse = require("argparse")
--   local p = argparse("prog", "description")        -- builder
--   p:flag("-v --verbose", { help = "..." })          -- chaînable
--   p:option("-o --output", { default = "a.out" })    -- chaînable
--   p:argument("input", { help = "..." })             -- chaînable
--   local res, err = p:parse()                        -- (res, err)
--
--   * parse() lit la table globale `arg` aux indices 1..n par défaut
--     (les vrais arguments utilisateur, identiques dans les 3 modes
--     d'exécution de LuaPilot). Les indices <= 0 (binaire / dossier)
--     sont IGNORÉS. p:parse(t) parse une table {1..n} explicite.
--
--   * Succès      : res = { dest = valeur, ... }, err = nil
--   * -h/--help   : res = { help = true, usage = "<texte>" }, err = nil
--                   (le help est un SUCCÈS, jamais une erreur ; aucun
--                    print, aucun os.exit — le script décide)
--   * Échec       : res = nil, err = "message lisible"
--     (option inconnue, valeur manquante, choix invalide, conversion
--      échouée, argument requis manquant, argument en trop)
--
--   parse() ne lève JAMAIS d'erreur Lua sur une entrée utilisateur
--   (invariant de correction) : tout est rapporté via (nil, msg).
--   Seul un mauvais usage du BUILDER (spec invalide passée par le
--   programmeur) lève via error() — c'est une faute de programmation,
--   analogue à luaL_error côté C.
--
-- Périmètre v1 (volontairement limité ; extensible plus tard sans
-- casse, Lua pur additif) : flags booléens, options à valeur,
-- arguments positionnels, required, default, choices, convert.
-- Ordre d'application observable : `choices` est testé sur la valeur
-- BRUTE (string telle que reçue depuis arg / parse(t)), PUIS
-- `convert` est appliqué sur la valeur retenue. Si tu combines les
-- deux, exprime donc `choices` en strings ({"1","2","3"} avec
-- convert = tonumber), pas en valeurs converties.
-- HORS v1 : sous-commandes, nargs variadiques, court collé (-abc,
-- -fvalue). Les formes "--long value", "--long=value", "-s value"
-- et "-s=value" sont supportées ; "--" termine les options.

local argparse = {
  _VERSION = "luapilot argparse 1.0.0",
  _DESCRIPTION = "command-line argument parser (pure Lua, bundled)",
}

local Parser = {}
Parser.__index = Parser

-- --- helpers internes ------------------------------------------------

-- Découpe une spec "-o --output" en { "-o", "--output" }.
local function split_spec(spec)
  local names = {}
  for tok in tostring(spec):gmatch("%S+") do
    names[#names + 1] = tok
  end
  return names
end

-- Déduit le nom de destination : 1er nom long sans tirets, sinon 1er
-- nom court sans tiret. opts.dest force explicitement.
local function derive_dest(names, opts)
  if opts and opts.dest then
    return opts.dest
  end
  for _, n in ipairs(names) do
    if n:sub(1, 2) == "--" then
      return (n:gsub("^%-%-", ""))
    end
  end
  local first = names[1] or ""
  return (first:gsub("^%-+", ""))
end

local function is_optional(name)
  return name:sub(1, 1) == "-"
end

-- --- construction ----------------------------------------------------

-- argparse("prog", "description") -> objet Parser.
local function new_parser(prog, description)
  local self = setmetatable({}, Parser)
  self._prog = prog or "prog"
  self._description = description
  self._options = {}       -- liste ordonnée { names, dest, takes_value, ... }
  self._by_name = {}       -- "-o" / "--output" -> entrée option
  self._positionals = {}   -- liste ordonnée des arguments positionnels
  return self
end

-- Enregistre une option/flag. takes_value distingue option vs flag.
function Parser:_add_optional(spec, opts, takes_value)
  opts = opts or {}
  local names = split_spec(spec)
  if #names == 0 then
    error("argparse: empty option spec", 3)
  end
  for _, n in ipairs(names) do
    if not is_optional(n) then
      error("argparse: option name must start with '-': " .. n, 3)
    end
  end
  local entry = {
    names = names,
    dest = derive_dest(names, opts),
    takes_value = takes_value,
    help = opts.help,
    default = opts.default,
    required = opts.required and true or false,
    choices = opts.choices,
    convert = opts.convert,
    seen = false,
  }
  self._options[#self._options + 1] = entry
  for _, n in ipairs(names) do
    if self._by_name[n] then
      error("argparse: duplicate option name: " .. n, 3)
    end
    self._by_name[n] = entry
  end
  return self
end

function Parser:option(spec, opts)
  return self:_add_optional(spec, opts, true)
end

function Parser:flag(spec, opts)
  return self:_add_optional(spec, opts, false)
end

function Parser:argument(name, opts)
  opts = opts or {}
  if type(name) ~= "string" or name == "" or is_optional(name) then
    error("argparse: positional name must be a non-empty string "
      .. "not starting with '-'", 3)
  end
  local entry = {
    name = name,
    dest = opts.dest or name,
    help = opts.help,
    default = opts.default,
    -- requis par défaut, sauf si une valeur par défaut est fournie
    required = (opts.required ~= nil) and (opts.required and true or false)
        or (opts.default == nil),
    choices = opts.choices,
    convert = opts.convert,
  }
  self._positionals[#self._positionals + 1] = entry
  return self
end

-- --- usage -----------------------------------------------------------

function Parser:get_usage()
  local parts = { "Usage: " .. self._prog .. " [options]" }
  for _, a in ipairs(self._positionals) do
    if a.required then
      parts[#parts + 1] = "<" .. a.name .. ">"
    else
      parts[#parts + 1] = "[" .. a.name .. "]"
    end
  end
  local lines = { table.concat(parts, " ") }
  if self._description then
    lines[#lines + 1] = ""
    lines[#lines + 1] = self._description
  end
  if #self._positionals > 0 then
    lines[#lines + 1] = ""
    lines[#lines + 1] = "Arguments:"
    for _, a in ipairs(self._positionals) do
      lines[#lines + 1] = "  " .. a.name
          .. (a.help and ("  " .. a.help) or "")
    end
  end
  lines[#lines + 1] = ""
  lines[#lines + 1] = "Options:"
  lines[#lines + 1] = "  -h, --help  show this help"
  for _, o in ipairs(self._options) do
    lines[#lines + 1] = "  " .. table.concat(o.names, ", ")
        .. (o.takes_value and " <value>" or "")
        .. (o.help and ("  " .. o.help) or "")
  end
  return table.concat(lines, "\n")
end

-- --- parsing ---------------------------------------------------------

-- Récupère les arguments à analyser. Par défaut : table globale `arg`,
-- indices 1..n uniquement (on s'arrête au premier trou). Les indices
-- <= 0 (binaire / dossier LuaPilot) sont volontairement ignorés.
local function collect_args(src)
  local out = {}
  if src ~= nil then
    for i = 1, #src do
      out[i] = src[i]
    end
    return out
  end
  local g = arg
  if type(g) ~= "table" then
    return out
  end
  local i = 1
  while g[i] ~= nil do
    out[i] = g[i]
    i = i + 1
  end
  return out
end

-- Applique choices (sur la valeur brute) puis convert. Renvoie
-- (valeur, nil) ou (nil, message).
local function finalize_value(entry, raw, label)
  if entry.choices then
    local okc = false
    for _, c in ipairs(entry.choices) do
      if raw == c then
        okc = true
        break
      end
    end
    if not okc then
      return nil, label .. ": invalid choice '" .. tostring(raw)
          .. "'"
    end
  end
  if entry.convert then
    -- pcall : une convert fournie par le script ne doit pas faire
    -- remonter une exception à travers parse() (invariant).
    local ok2, cv, cverr = pcall(entry.convert, raw)
    if not ok2 then
      return nil, label .. ": conversion error"
    end
    if cv == nil then
      return nil, label .. ": "
          .. (cverr and tostring(cverr) or "invalid value")
    end
    return cv, nil
  end
  return raw, nil
end

function Parser:parse(src)
  local args = collect_args(src)
  local res = {}

  -- défauts des options + flags à false
  for _, o in ipairs(self._options) do
    o.seen = false
    if o.takes_value then
      res[o.dest] = o.default
    else
      res[o.dest] = o.default ~= nil and o.default or false
    end
  end

  local positional_vals = {}
  local i = 1
  local no_more_opts = false
  local n = #args

  while i <= n do
    local a = args[i]

    if not no_more_opts and a == "--" then
      no_more_opts = true
      i = i + 1
    elseif not no_more_opts and (a == "-h" or a == "--help") then
      -- help = succès, signalé dans la donnée. Aucun effet de
      -- bord. Après un "--", -h/--help n'est plus traité ici :
      -- il tombe en positionnel littéral (comportement voulu).
      return { help = true, usage = self:get_usage() }, nil
    elseif not no_more_opts and is_optional(a) and a ~= "-" then
      -- forme --long=val ou -s=val : on découpe sur le 1er '='
      local key, inline = a:match("^([^=]+)=(.*)$")
      if not key then
        key = a
      end
      local entry = self._by_name[key]
      if not entry then
        return nil, "unknown option '" .. key .. "'"
      end
      if entry.takes_value then
        local raw
        if inline ~= nil then
          raw = inline
        else
          if i + 1 > n then
            return nil, "option '" .. key
                .. "' requires a value"
          end
          raw = args[i + 1]
          i = i + 1
        end
        local val, verr = finalize_value(entry, raw,
          "option '" .. key .. "'")
        if val == nil and verr then
          return nil, verr
        end
        res[entry.dest] = val
      else
        if inline ~= nil then
          return nil, "flag '" .. key
              .. "' does not take a value"
        end
        res[entry.dest] = true
      end
      entry.seen = true
      i = i + 1
    else
      positional_vals[#positional_vals + 1] = a
      i = i + 1
    end
  end

  -- options requises manquantes
  for _, o in ipairs(self._options) do
    if o.required and not o.seen then
      return nil, "missing required option '"
          .. (o.names[#o.names] or o.dest) .. "'"
    end
  end

  -- positionnels : affectation positionnelle stricte
  for idx, entry in ipairs(self._positionals) do
    local raw = positional_vals[idx]
    if raw == nil then
      if entry.required then
        return nil, "missing required argument '"
            .. entry.name .. "'"
      end
      res[entry.dest] = entry.default
    else
      local val, verr = finalize_value(entry, raw,
        "argument '" .. entry.name .. "'")
      if val == nil and verr then
        return nil, verr
      end
      res[entry.dest] = val
    end
  end

  -- arguments positionnels en trop
  if #positional_vals > #self._positionals then
    local extra = positional_vals[#self._positionals + 1]
    return nil, "unexpected argument '" .. tostring(extra) .. "'"
  end

  return res, nil
end

-- require("argparse") renvoie le constructeur ; argparse.* expose
-- aussi les métadonnées (_VERSION...).
return setmetatable(argparse, {
  __call = function(_, prog, description)
    return new_parser(prog, description)
  end,
})
