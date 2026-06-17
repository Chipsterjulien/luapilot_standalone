> [English](../../en/modules/argparse.md) | **Français**

# `argparse` — parseur d'arguments CLI (module Lua)

Un parseur d'arguments déclaratif pour les scripts en ligne de
commande. Embarqué en module Lua pur, chargé via
`require("argparse")`. Renvoie ses résultats sous forme de
données (pas via des `print` ou `os.exit`) — le script décide
quoi en faire.

## Pourquoi

Un script LuaPilot qui prend des arguments ne devrait pas avoir à
réinventer le parsing (positionnel vs flag, défauts, texte d'aide,
coercion de type, choix) à chaque fois. `argparse` rend les
patterns CLI courants accessibles en une ligne — et reste hors du
chemin du script en renvoyant tout sous forme de données : l'aide
est un résultat, pas un effet de bord ; les erreurs sont une
valeur de retour, pas une exception.

## API

```lua
local argparse = require("argparse")
local p = argparse(prog, description?)
```

| Méthode du builder | Renvoie | Usage |
| --- | --- | --- |
| `p:flag(spec, opts?)` | `p` (chaînable) | Flag booléen (sans valeur). |
| `p:option(spec, opts?)` | `p` (chaînable) | Flag qui prend une valeur. |
| `p:argument(name, opts?)` | `p` (chaînable) | Argument positionnel. |
| `p:parse(src?)` | `(res, err)` | Parse les arguments (voir ci-dessous). |
| `p:get_usage()` | `string` | Génère le texte d'aide manuellement. |

`spec` pour `flag` et `option` est **une seule string** contenant
un ou plusieurs noms séparés par des espaces : `"-v"`,
`"--verbose"`, ou `"-v --verbose"`. Les noms doivent commencer par
`-`.

`name` pour `argument` est un identifiant simple (sans tiret
initial).

### Champs de la table `opts`

| Champ | Type | Effet |
| --- | --- | --- |
| `help` | string | Texte d'aide affiché par `get_usage()`. |
| `default` | any | Valeur renvoyée quand le flag/option/argument est absent. |
| `dest` | string | Force la clé de destination dans `res`. Défaut : 1er nom long sans tirets, sinon 1er nom court sans tiret. |
| `required` | boolean | Marque comme requis. Pour les positionnels, défaut à `true` sauf si `default` est défini. |
| `choices` | array de strings | Restreint la valeur brute à un ensemble fini. |
| `convert` | fonction `string -> any` | Coerce la string brute. Renvoyer `nil` est traité comme un échec (la fonction convert peut renvoyer `(nil, "raison")`). |

### Contrat de retour de `parse(src?)`

- **Succès** : `(res, nil)` — `res` est une table indexée par
  `dest`, contenant la valeur parsée (ou défaut) pour chaque
  flag / option / argument déclaré.
- **Aide demandée** : `({ help = true, usage = "<texte>" }, nil)`
  quand l'utilisateur a passé `-h` ou `--help`. L'aide est un
  **succès**, pas une erreur. Le script décide quoi faire
  (typiquement afficher `res.usage` et sortir).
- **Échec** : `(nil, err)` — string lisible pour tout problème
  d'entrée utilisateur (option inconnue, valeur manquante, choix
  invalide, erreur de conversion, argument requis manquant,
  positionnel en trop).

`parse()` **ne lève jamais** sur entrée utilisateur — toute
erreur utilisateur passe par `(nil, err)`. Seul un **mauvais
usage du builder** (spec vide, nom sans `-`, nom dupliqué, etc.)
lève via `error()`, parce que c'est un bug du programmeur.

### Source des arguments

- `p:parse()` (sans argument) lit la table globale `arg`, indices
  `1..n` (on s'arrête au premier trou). Les indices `<= 0`
  (chemin du binaire, dossier) sont ignorés — ce sont les slots
  internes de LuaPilot, pas des arguments utilisateur.
- `p:parse({ "a", "b" })` parse un array explicite. Utile pour
  les tests ou pour parser des arguments qui ne viennent pas du
  shell.

### Formes d'arguments acceptées

- `--long val`
- `--long=val`
- `-s val`
- `-s=val`
- `--` termine le parsing des options — tout token suivant est
  traité comme un positionnel, même s'il commence par `-`.

Les options courtes agglomérées (`-abc`) et la forme `-fvalue`
(option courte immédiatement suivie de sa valeur, sans
séparateur) **ne sont pas supportées** en v1.

## Exemple rapide

```lua
local argparse = require("argparse")

local p = argparse("myapp", "Traite des fichiers.")
    :flag("-v --verbose", { help = "Sortie verbeuse" })
    :option("-o --output",
            { help = "Fichier de sortie", default = "out.txt" })
    :option("-n --count",
            { help = "Combien", convert = tonumber, default = 1 })
    :argument("input", { help = "Fichier d'entrée" })

local args, err = p:parse()

-- L'aide est un succès, pas une erreur.
if args and args.help then
    print(args.usage)
    return
end

if err then
    io.stderr:write("erreur : ", err, "\n")
    io.stderr:write(p:get_usage(), "\n")
    os.exit(2)
end

print(args.input, args.output, args.count, args.verbose)
```

À l'exécution :

```
$ ./myapp.lua data.csv --count 5 -v
data.csv    out.txt    5    true

$ ./myapp.lua --help
Usage: myapp [options] <input>

Traite des fichiers.

Arguments:
  input  Fichier d'entrée

Options:
  -h, --help        show this help
  -v, --verbose     Sortie verbeuse
  -o, --output <value>  Fichier de sortie
  -n, --count <value>   Combien
```

## Contrat d'erreur

- **Mauvais usage du builder** (`p:flag("")`, `p:option("foo")`
  sans `-`, nom dupliqué, `p:argument("-bad")`) → lève via
  `error()`. Ce sont des bugs du programmeur.
- **`p:parse(...)`** ne lève jamais sur entrée utilisateur. Tous
  les problèmes d'entrée utilisateur sont rapportés en
  `(nil, "message")` :
  - `"unknown option '--xxx'"`
  - `"option '--xxx' requires a value"`
  - `"flag '-v' does not take a value"`
  - `"missing required option '--xxx'"`
  - `"missing required argument 'name'"`
  - `"argument 'name': invalid choice 'xxx'"`
  - `"option '--xxx': <message du convert>"`
  - `"unexpected argument 'xxx'"`
- Une fonction `convert` qui lève elle-même est **interceptée**
  par `parse()` et transformée en `(nil, "<...>: conversion
  error")`. Le script ne voit jamais d'exception sortir de
  `parse()`.

## Décisions de design

- **L'aide est un succès, pas un effet de bord**. Renvoyer
  `{ help = true, usage = "..." }` plutôt que d'afficher et de
  sortir laisse le script décider : afficher sur stdout, logger
  dans un fichier, envoyer à une GUI, préfixer une bannière, peu
  importe. La bibliothèque ne décide pas à ta place, cohérent
  avec le reste de LuaPilot où aucun module ne sort
  unilatéralement du process.
- **`(res, err)` partout sur entrée utilisateur, `error()` seulement
  sur bug du builder**. Même convention que le reste de LuaPilot
  (`user`, `json`, `sqlite`, etc.).
- **Spec en une seule string** (`"-v --verbose"`), pas une
  chaîne d'appels. Découvrable, concis, et évite l'ambiguïté
  quand plusieurs noms sont déclarés ensemble.
- **`choices` est testé sur la string brute, puis `convert`
  appliqué**. Si tu combines les deux, exprime `choices` en
  strings — ex. `{ choices = {"1","2","3"}, convert = tonumber }`.
  Piège documenté, mais conscient : faire l'inverse forcerait les
  valeurs de `choices` à être du type converti, ce qui couplerait
  deux features indépendantes.
- **Un positionnel avec `default` est implicitement optionnel**.
  Évite le boilerplate `required = false` dans le cas courant.
- **`parse()` lit `arg[1..n]`** — jamais `arg[0]` ni les indices
  négatifs, où LuaPilot place ses propres slots. Juste ce que
  l'utilisateur a réellement tapé.

## Hors v1

Additif — pourrait être ajouté plus tard sans casser le SemVer :

- Sous-commandes (style `git commit` / `git push`).
- Options courtes agglomérées (`-abc` pour `-a -b -c`).
- Options courtes avec valeur attachée (`-fvalue`).
- Positionnels variadiques (`nargs = "+"` ou `"*"`).
- Groupes d'arguments (regroupement visuel dans l'aide).
