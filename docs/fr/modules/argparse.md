> [English](../../en/modules/argparse.md) | **Français**

# `argparse` — parseur d'arguments CLI (module Lua)

Un parseur d'arguments déclaratif pour les scripts en ligne de
commande. Embarqué en module Lua pur, chargé via
`require("argparse")`. Inspiré de l'`argparse` de Python, adapté
aux idiomes Lua.

## Pourquoi

Un script LuaPilot qui prend des arguments ne devrait pas avoir à
réinventer le parsing (positionnel vs flag, défauts, texte d'aide,
coercion de type, flags répétés) à chaque fois. `argparse` rend
les patterns CLI courants accessibles en une ligne.

## API

```lua
local argparse = require("argparse")
local p = argparse("nom-script", "Description en une ligne")
```

| Méthode | Effet |
| --- | --- |
| `p:argument(name)` | déclare un argument positionnel |
| `p:option(name)` | déclare un flag (`-x` / `--xxx`) qui prend une valeur |
| `p:flag(name)` | déclare un flag booléen (sans valeur) |
| `p:parse()` | parse la table `arg`, renvoie la table parsée ou appelle `os.exit(1)` en erreur |

Chaque déclaration renvoie un builder chaînable :

- `:description("texte")` — texte affiché dans `--help`.
- `:default(value)` — défaut si non fourni.
- `:convert(fn)` — fonction pour coercer la string brute (`tonumber`, etc.).
- `:count("*"|"+"|"?")` — flags répétés ; défaut une fois.
- `:choices({...})` — restreint à un ensemble fini.

## Exemple rapide

```lua
local argparse = require("argparse")

local p = argparse("myapp", "Traite des fichiers.")
p:argument("input"):description("Fichier d'entrée")
p:option("-o --output"):description("Fichier de sortie"):default("out.txt")
p:option("-n --count"):description("Combien"):convert(tonumber):default(1)
p:flag("-v --verbose"):description("Sortie verbeuse")

local args = p:parse()
print(args.input, args.output, args.count, args.verbose)
```

À l'exécution :

```
$ ./myapp.lua data.csv --count 5 -v
data.csv    out.txt    5    true

$ ./myapp.lua --help
Usage: myapp [-o <output>] [-n <count>] [-v] <input>

Traite des fichiers.

Arguments:
   input              Fichier d'entrée

Options:
   -o, --output       Fichier de sortie (default: out.txt)
   -n, --count        Combien (default: 1)
   -v, --verbose      Sortie verbeuse
   -h, --help         Show this help message and exit
```

## Contrat d'erreur

- **Construction du parser** (`argparse(...)`) et déclarations
  (`p:argument(...)` etc.) : lèvent via `error()` en cas de
  mauvais usage.
- **`p:parse()`** : sur une ligne de commande malformée, affiche
  l'erreur + l'usage sur stderr et appelle `os.exit(1)`. Cohérent
  avec les attentes utilisateurs pour les outils CLI (pas besoin
  de gérer les erreurs dans le script).

## Décisions de design

- **Embarqué en Lua, pas en C++**. Le parsing d'arguments est de
  la logique string pure, pas de syscalls. Le Lua pur le garde
  monkey-patchable pour des besoins inhabituels (format d'aide
  personnalisé, etc.).
- **Exit dur en erreur de parse**. Les scripts CLI veulent
  typiquement mourir avec un message d'usage, pas attraper les
  erreurs. Si tu dois les attraper, wrappe l'appel dans `pcall`.
- **`-h` / `--help` est automatique**. Toujours présent, affiche
  l'usage et exit 0.

## Hors v1

- Sous-commandes (style `git commit` / `git push`). Peut être
  ajouté si un cas d'usage réel apparaît.
- Groupes d'arguments (regroupement visuel dans l'aide).
  Cosmétique seulement.

L'implémentation de référence est embarquée depuis le projet
upstream [argparse.lua](https://github.com/mpeterv/argparse). Voir
sa doc pour les patterns avancés (groupes mutuellement exclusifs,
actions personnalisées, etc.).
