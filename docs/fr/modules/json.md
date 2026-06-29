> [English](../../en/modules/json.md) | **Français**

# `babet.json` — encode/décode JSON

Basé sur [nlohmann/json](https://github.com/nlohmann/json), la
bibliothèque JSON C++ la plus utilisée. Gère le round-trip
classique plus la sentinelle "tableau vide" explicite que les
tables Lua ne peuvent pas désambiguïser de "objet vide".

## Pourquoi

Le mismatch d'impédance Lua/JSON piège toujours quelqu'un : Lua ne
distingue pas une liste vide d'une table vide. La plupart des
bibliothèques Lua/JSON ad-hoc choisissent une option et se
trompent une fois sur deux. `babet.json` est explicite : `{}`
se décode en `{}` et se ré-encode en `{}` (objet), mais si tu veux
`[]` tu utilises la sentinelle `babet.json.empty_array`.

## API

| Fonction | Renvoie |
| --- | --- |
| `babet.json.encode(value, opts?)` | `string` (texte JSON) \| `(nil, err)` |
| `babet.json.decode(text)` | `value` \| `(nil, err)` |
| `babet.json.empty_array` | sentinelle qui s'encode en `[]` |

Options d'encodage (table `opts`) :

- `pretty = true` — pretty-print avec indentation (défaut `false`).
- `indent = N` — largeur d'indentation quand `pretty=true` (défaut `2`).

## Exemple rapide

```lua
local J = babet.json

-- Décode
local data, err = J.decode([[
{ "name": "alice", "tags": ["admin", "ops"], "active": true }
]])
print(data.name, data.tags[1])

-- Encode (compact)
local out = J.encode({ a = 1, b = { 2, 3 } })
-- out == '{"a":1,"b":[2,3]}'

-- Encode pretty
local pretty = J.encode({ a = 1 }, { pretty = true, indent = 4 })

-- Tableau vide vs objet vide
J.encode({})                    --> "{}"
J.encode(J.empty_array)         --> "[]"
J.encode({ items = J.empty_array })  --> '{"items":[]}'
```

## Contrat d'erreur

- **`encode`** : `(nil, err)` en cas d'erreur d'encodage (cycle
  dans le graphe, valeur qui ne mappe pas en JSON comme une
  fonction ou userdata, nombres NaN/Inf, etc.).
- **`decode`** : `(nil, err)` en cas d'erreur de parsing. Le
  message d'erreur inclut la ligne/colonne où le parse a échoué.
- **Mauvais type d'argument** → lève via `luaL_error`.

## Décisions de design

- **Sentinelle `empty_array`** plutôt que de deviner depuis la
  structure de la table (certaines bibliothèques traitent
  `{1, 2, 3}` comme array et `{a=1}` comme objet, ce qui est
  correct, mais `{}` est ambigu). Une sentinelle explicite enlève
  toute surprise.
- **NaN/Inf refusés**, pas encodés silencieusement en `null`. Les
  consommateurs JSON gèrent `null` différemment pour les champs
  numériques ; mieux vaut échouer bruyamment et laisser l'appelant
  décider.
- **Pas d'API streaming**. Les round-trips sont bornés par la
  mémoire du script de toute façon ; si tu dois traiter du JSON
  plus gros que la RAM, utilise un vrai parser streaming dans un
  process séparé.

## Hors v1

- Encodeur/décodeur streaming.
- Un validateur de schéma (utilise-en un en Lua pur — c'est
  trivial à ajouter au niveau script).
