> [English](../../en/modules/toml.md) | **Français**

# `luapilot.toml` — parseur TOML

Basé sur [toml++](https://marzer.github.io/tomlplusplus/), un
parser TOML 1.0 strict. Décodage seulement — encoder du TOML
fidèlement depuis une table Lua est ambigu (Lua ne distingue pas
les inline tables et les dotted tables, par exemple).

## Pourquoi

TOML est le format de config naturel des outils modernes (Cargo,
pyproject, systemd-credentials, etc.). Un script LuaPilot lisant
sa propre config devrait pouvoir le faire directement, pas via
une conversion TOML → JSON via `tomlq`.

## API

| Fonction | Renvoie |
| --- | --- |
| `luapilot.toml.decode(text)` | `table` (TOML parsé) \| `(nil, err)` |

La table renvoyée reflète la structure TOML :

- Strings → strings Lua (UTF-8)
- Integers → integers Lua
- Floats → numbers Lua
- Booleans → booleans Lua
- Datetimes → strings Lua au format ISO 8601 (TOML n'a pas
  d'équivalent natif Lua ; utilise `os.date` ou une bibliothèque
  date si tu as besoin des composants parsés)
- Arrays → tables Lua (1-indexées)
- Tables (`[section]` ou `{ inline }`) → tables Lua (avec clés
  strings)

## Exemple rapide

```lua
local body = [[
title = "My App"
[server]
host = "localhost"
port = 8080
features = ["auth", "metrics"]

[database]
url = "sqlite:///var/lib/app.db"
]]

local cfg, err = luapilot.toml.decode(body)
if not cfg then error("parsing config échoué : " .. err) end

print(cfg.title)                  -- "My App"
print(cfg.server.host)            -- "localhost"
print(cfg.server.features[1])     -- "auth"
print(cfg.database.url)           -- "sqlite:///var/lib/app.db"
```

## Contrat d'erreur

- **`decode`** : `(nil, err)` en cas d'erreur de parse. Le message
  d'erreur inclut la ligne/colonne où le parse a échoué, et une
  courte description de ce qui n'allait pas (par toml++).
- **Argument non-string** → lève via `luaL_error`.

## Décisions de design

- **Décode seulement, pas d'encode**. Round-tripper un fichier
  TOML via une table Lua perd la mise en forme (commentaires,
  dotted vs inline tables, newlines entre sections) que les vrais
  utilisateurs apprécient. Si tu dois écrire du TOML, construis la
  string toi-même — TOML est conçu pour être écrit à la main. Un
  encodeur fidèle serait un gros chantier pour une valeur limitée.
- **Datetimes en strings**. Le type datetime TOML est riche
  (offset, local, date-only, time-only) mais Lua n'a pas
  d'équivalent natif. Renvoyer une string ISO 8601 préserve la
  donnée sans perte et laisse le script la parser avec les outils
  de son choix.

## Hors v1

- Encodeur. Pourrait être ajouté si un vrai cas d'usage apparaît,
  mais peu probable.
- Un validateur de schéma. Construis-en un en Lua si nécessaire ;
  la structure décodée n'est qu'une table.
