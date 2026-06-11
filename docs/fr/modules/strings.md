> [English](../../en/modules/strings.md) | **Français**

# `luapilot` strings — manipulation de chaînes

Une seule fonction : découper une chaîne par séparateur en table.
Précède la convention par sous-namespace, vit directement sur
`luapilot`.

## Pourquoi

Découper une chaîne par délimiteur est l'une des opérations string
les plus courantes, et la stdlib Lua n'en a pas par défaut
(`string.gmatch` marche mais demande d'échapper les patterns). Une
fonction dédiée est plus découvrable et évite le piège du pattern
escaping.

## API

| Fonction | Renvoie |
| --- | --- |
| `luapilot.split(s, sep)` | `table` (array) des sous-chaînes |

- `s` : la chaîne à découper.
- `sep` : la chaîne séparateur (littéral, pas de pattern).

Si `sep` n'apparaît pas dans `s`, le résultat est une table à un
élément contenant `s` en entier. Si `s` est vide, renvoie une table
vide.

## Exemple rapide

```lua
local parts = luapilot.split("a,b,c,d", ",")
-- parts == { "a", "b", "c", "d" }

local one = luapilot.split("hello", ",")
-- one == { "hello" }
```

## Contrat d'erreur

- **Mauvais type d'argument** → lève via `luaL_error`.
- Sinon, réussit toujours.

## Décisions de design

- **Séparateur littéral, pas un pattern Lua**. Le cas courant est
  le découpage de données CSV-like, où on veut que `.` signifie un
  vrai point, pas "n'importe quel caractère". Si on a besoin de
  pattern matching, on retombe sur `string.gmatch`.
- **Les entrées vides sont préservées**. `"a,,b"` se découpe en
  `{"a", "", "b"}`. Les consommateurs qui veulent filtrer les
  strings vides le font en une ligne de Lua.

## Hors v1

- Découpage basé pattern (avec règles d'échappement). Utilise
  `string.gmatch` si nécessaire.
- Miroir `joinTable(t, sep)` — `table.concat` existe déjà dans la
  stdlib.
