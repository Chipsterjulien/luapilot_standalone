> [English](../../en/modules/fs.md) | **Français**

# `luapilot` fs — système de fichiers

Un ensemble plat d'opérations système de fichiers exposées
directement sur `luapilot` (pas de sous-namespace, précède la
convention). Couvre les besoins quotidiens que `os.*` et `io.*` de
Lua ne gèrent pas : listing récursif, copie avec attributs,
hashing, attributs, liens.

## Pourquoi

`io` et `os` de Lua te donnent `open`, `rename`, `remove`, et c'est
à peu près tout. Tout ce qui va au-delà — lister un dossier, copie
récursive, calculer un hash, interroger mtime — demande des
programmes externes ou du FFI bas niveau. `luapilot` rassemble ça
dans un set unique de fonctions qui se comportent de manière
cohérente (chemins en string, erreurs en `(nil, "...")`).

Tous les chemins sont acceptés en string. L'expansion du tilde
(`~/x`) n'est **pas** automatique — l'appelant doit résoudre
`$HOME` lui-même si besoin. Les fonctions n'expansent jamais les
globs ; pour le support glob, utilise `luapilot.find` avec un
pattern explicite.

## API — Existence, type, attributs

| Fonction | Renvoie |
| --- | --- |
| `luapilot.fileExists(path)` | `boolean` |
| `luapilot.isfile(path)` | `boolean` (true seulement pour fichiers réguliers) |
| `luapilot.isdir(path)` | `boolean` |
| `luapilot.fileSize(path)` | `integer` octets \| `(nil, err)` |
| `luapilot.attributes(path)` | `table` avec `mtime`, `mode`, `size`, `uid`, `gid`, `inode`, etc. \| `(nil, err)` |
| `luapilot.symlinkattr(path)` | même shape que `attributes`, mais `lstat` (ne suit pas les symlinks) \| `(nil, err)` |
| `luapilot.mode(path)` | `integer` octal \| `(nil, err)` — raccourci pour `attributes(path).mode` |

## API — Création, suppression, déplacement

| Fonction | Renvoie |
| --- | --- |
| `luapilot.touch(path)` | `(true, nil)` \| `(nil, err)` — crée le fichier ou met à jour mtime |
| `luapilot.mkdir(path, opts?)` | `(true, nil)` \| `(nil, err)` — `opts.parents = true` pour `mkdir -p` |
| `luapilot.rmdir(path)` | `(true, nil)` \| `(nil, err)` — seulement dossiers vides |
| `luapilot.rmdirAll(path)` | `(true, nil)` \| `(nil, err)` — récursif |
| `luapilot.remove(path)` | `(true, nil)` \| `(nil, err)` — fichier ou symlink |
| `luapilot.rename(old, new)` | `(true, nil)` \| `(nil, err)` |
| `luapilot.chdir(path)` | `(true, nil)` \| `(nil, err)` |
| `luapilot.currentDir()` | `string` (absolu) |
| `luapilot.joinPath(a, b, ...)` | `string` — comme `path/a/b/c` |
| `luapilot.link(target, link, opts?)` | `(true, nil)` \| `(nil, err)` — `opts.symbolic = true` pour symlink |

## API — Listing et recherche

| Fonction | Renvoie |
| --- | --- |
| `luapilot.listDir(path)` | `table` (array des noms d'entrées, sans `.` / `..`) \| `(nil, err)` |
| `luapilot.listFiles(path)` | `table` (seulement fichiers réguliers) \| `(nil, err)` |
| `luapilot.find(path, opts)` | fonction iterator, voir ci-dessous |
| `luapilot.fileIterator(path)` | fonction iterator sur les entrées du dossier |

`find` accepte `opts` :

- `pattern = "*.lua"` — glob shell appliqué au nom de fichier seul.
- `recursive = true` — descend dans les sous-dossiers.
- `type = "f"` / `"d"` — restreint aux fichiers ou dossiers.
- `max_depth = N` — limite la profondeur de récursion.

## API — Copie

| Fonction | Renvoie |
| --- | --- |
| `luapilot.copy(src, dst, opts?)` | `(true, nil)` \| `(nil, err)` |
| `luapilot.copyTree(src, dst, opts?)` | `(true, nil)` \| `(nil, err)` — récursif |
| `luapilot.moveTree(src, dst)` | `(true, nil)` \| `(nil, err)` |

Options de `copy` :

- `preserve = true` — garde mtime, mode, owner si possible.
- `overwrite = true` — remplace la destination si elle existe.

## API — Hashes et checksums

Empreintes du contenu des fichiers. Résultat en **string hex
minuscule** sauf mention contraire.

| Fonction | Algorithme |
| --- | --- |
| `luapilot.md5(path)` | MD5 |
| `luapilot.sha1(path)` | SHA-1 |
| `luapilot.sha256(path)` | SHA-256 |
| `luapilot.sha384(path)` | SHA-384 |
| `luapilot.sha512(path)` | SHA-512 |
| `luapilot.sha3_256(path)` | SHA3-256 |
| `luapilot.sha3_384(path)` | SHA3-384 |
| `luapilot.sha3_512(path)` | SHA3-512 |
| `luapilot.blake2s(path)` | BLAKE2s-256 |
| `luapilot.blake2b(path)` | BLAKE2b-512 |

Chaque renvoie `string` (hex) ou `(nil, err)`. MD5 et SHA-1 sont
exposés pour la compatibilité (Git, systèmes legacy) — ne pas les
utiliser pour de nouveaux usages cryptographiques.

## Exemples rapides

```lua
-- Prédicats
if luapilot.isdir("/etc") and luapilot.fileExists("/etc/hostname") then
    print("taille :", luapilot.fileSize("/etc/hostname"))
end

-- Listing récursif
for entry in luapilot.find("/var/log", { pattern = "*.log", type = "f", recursive = true }) do
    print(entry)
end

-- Copie avec attributs
luapilot.copyTree("./src", "/tmp/backup", { preserve = true, overwrite = true })

-- Hash d'une tarball de release
local sha, err = luapilot.sha256("/tmp/luapilot-1.7.0.tar.gz")
if sha then print("SHA256 :", sha) end
```

## Contrat d'erreur

- Toutes les fonctions suivent la convention `(nil, err)` en cas
  d'échec runtime (fichier inexistant, permission refusée, etc.).
- Les mauvais **types** d'argument lèvent via `luaL_error` (passer
  un nombre où un chemin est attendu après coercion a une
  sémantique bizarre).
- Sécurité des chemins : `copy`, `copyTree`, `moveTree` utilisent
  `lexically_relative()` et des guards `is_within()` composant par
  composant pour empêcher des évasions par symlink hors de
  l'arborescence de destination. Voir [`security`](../security.md).

## Décisions de design

- **Namespace plat** : `fileExists` plutôt que `fs.exists`.
  Raison purement historique — ces fonctions précèdent la
  convention par module. Stable maintenant ; renommer casserait
  tous les scripts.
- **Les hashes opèrent sur des chemins de fichiers**, pas des
  octets bruts. Le cas courant est "hash ce fichier", donc l'API
  prend un chemin. Pour les octets bruts, l'implémentation basée
  openssl est interne — pull request bienvenue si un cas d'usage
  émerge.
- **`copy` est mono-fichier, `copyTree` est récursif**. Lua n'a
  pas la surcharge de fonctions par type, donc les séparer évite
  l'ambiguïté "est-ce que `copy('dir', 'dir')` voulait dire
  récursif ?".

## Hors v1

- Glob matching en fonction standalone (`luapilot.glob`). Utilise
  `find` avec `pattern = "*.lua"` pour le cas courant.
- I/O asynchrone. Utilise plutôt [`workers`](workers.md) pour le
  traitement parallèle de fichiers.
