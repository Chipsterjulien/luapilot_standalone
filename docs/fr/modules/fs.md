> [English](../../en/modules/fs.md) | **Français**

# `luapilot` fs — système de fichiers

Un ensemble plat d'opérations système de fichiers exposées
directement sur `luapilot` (pas de sous-namespace, précède la
convention). Couvre les besoins quotidiens que `os.*` et `io.*` de
Lua ne gèrent pas : listing récursif, copie avec attributs,
hashing, manipulation de chemins, attributs, liens.

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
| `luapilot.getAttributes(path)` | `table` avec `mtime`, `mode`, `size`, `uid`, `gid`, `inode`, etc. \| `(nil, err)` |
| `luapilot.setAttributes(path, uid, gid, mode?)` | `(true, nil)` \| `(nil, err)` — set owner/group, optionnellement le mode |
| `luapilot.symlinkattr(path)` | même shape que `getAttributes`, mais `lstat` (ne suit pas les symlinks) \| `(nil, err)` |
| `luapilot.getMode(path)` | `integer` octal \| `(nil, err)` |
| `luapilot.setMode(path, mode)` | `(true, nil)` \| `(nil, err)` — `mode` est un integer (utilise `0x1ed` pour `0755`, ou construis-le avec `tonumber("755", 8)`) |

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
| `luapilot.joinPath(a, b, ...)` | `string` — comme `path/a/b/c` ; accepte aussi une seule table de segments |
| `luapilot.link(target, link, opts?)` | `(true, nil)` \| `(nil, err)` — `opts.symbolic = true` pour symlink |

## API — Manipulation de chemins

Ces fonctions opèrent sur des *strings* de chemin — elles ne
touchent pas le système de fichiers.

| Fonction | Renvoie | Exemple |
| --- | --- | --- |
| `luapilot.getBasename(path)` | `string` \| `(nil, err)` — dernière composante | `"/etc/hostname"` → `"hostname"` |
| `luapilot.getPath(path)` | `string` \| `(nil, err)` — dossier parent (dirname) | `"/etc/hostname"` → `"/etc"` |
| `luapilot.getFilename(path)` | `string` \| `(nil, err)` — basename sans extension | `"report.tar.gz"` → `"report.tar"` |
| `luapilot.getExtension(path)` | `string` \| `(nil, err)` — extension finale | `"report.tar.gz"` → `".gz"` |

## API — Listing et recherche

| Fonction | Renvoie |
| --- | --- |
| `luapilot.listFiles(path)` | `table` (fichiers réguliers dans `path`) \| `(nil, err)` |
| `luapilot.find(path, opts)` | fonction iterator, voir ci-dessous |
| `luapilot.createFileIterator(path)` | fonction iterator sur les entrées du dossier |

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
minuscule** sauf mention contraire. Note le suffixe `sum` — ce
sont les noms enregistrés dans `luapilot.*`.

| Fonction | Algorithme |
| --- | --- |
| `luapilot.md5sum(path)` | MD5 |
| `luapilot.sha1sum(path)` | SHA-1 |
| `luapilot.sha256sum(path)` | SHA-256 |
| `luapilot.sha384sum(path)` | SHA-384 |
| `luapilot.sha512sum(path)` | SHA-512 |
| `luapilot.sha3_256sum(path)` | SHA3-256 |
| `luapilot.sha3_384sum(path)` | SHA3-384 |
| `luapilot.sha3_512sum(path)` | SHA3-512 |
| `luapilot.blake2b512sum(path)` | BLAKE2b-512 |

Chaque renvoie `string` (hex) ou `(nil, err)`. MD5 et SHA-1 sont
exposés pour la compatibilité (Git, systèmes legacy) — ne pas les
utiliser pour de nouveaux usages cryptographiques.

## Exemples rapides

```lua
-- Prédicats
if luapilot.isdir("/etc") and luapilot.fileExists("/etc/hostname") then
    print("taille :", luapilot.fileSize("/etc/hostname"))
end

-- Manipulation de chemins (string pur)
local base = luapilot.getBasename("/var/log/syslog.1")  -- "syslog.1"
local dir  = luapilot.getPath("/var/log/syslog.1")      -- "/var/log"
local ext  = luapilot.getExtension("/var/log/syslog.1") -- ".1"

-- Listing récursif
for entry in luapilot.find("/var/log",
        { pattern = "*.log", type = "f", recursive = true }) do
    print(entry)
end

-- Copie avec attributs
luapilot.copyTree("./src", "/tmp/backup",
                  { preserve = true, overwrite = true })

-- Hash d'une tarball de release
local sha, err = luapilot.sha256sum("/tmp/luapilot-1.7.0.tar.gz")
if sha then print("SHA256 :", sha) end

-- Owner et permissions
local mode_755 = tonumber("755", 8)  -- octal -> integer
assert(luapilot.setMode("/srv/myapp/run.sh", mode_755))
-- en root seulement :
-- assert(luapilot.setAttributes("/srv/myapp", 1000, 1000))
```

## Contrat d'erreur

- Toutes les fonctions suivent la convention `(nil, err)` en cas
  d'échec runtime (fichier inexistant, permission refusée, etc.).
- Les mauvais **types** d'argument lèvent via `luaL_error`.
- **`setAttributes(path, uid, gid)`** rejette les UIDs/GIDs négatifs
  et les valeurs au-dessus du max `uid_t`/`gid_t` (typiquement
  `2^32 - 1`). Sans ce check de borne haute, une valeur au-dessus
  de la limite serait silencieusement tronquée — pire cas vers
  UID 0 (root). Même logique que le module [`user`](user.md).
- Sécurité des chemins : `copy`, `copyTree`, `moveTree` utilisent
  `lexically_relative()` et des guards `is_within()` composant par
  composant pour empêcher des évasions par symlink hors de
  l'arborescence de destination. Voir [`security`](../security.md).

## Décisions de design

- **Namespace plat** : `fileExists` plutôt que `fs.exists`,
  `getAttributes` plutôt que `fs.attr.get`. Raison purement
  historique — ces fonctions précèdent la convention par module.
  Stable maintenant ; renommer casserait tous les scripts.
- **`get*` / `set*` pour les attributs de chemin**
  (`getMode`/`setMode`, `getAttributes`/`setAttributes`) mais des
  verbes nus pour les actions FS (`remove`, `touch`, `mkdir`). Les
  paires get/set existent parce que les deux directions sont
  nécessaires ; les autres sont des opérations unidirectionnelles.
- **La manipulation de chemins est séparée de l'accès FS**.
  `getBasename`, `getPath`, etc. n'opèrent que sur des strings —
  elles ne stat pas le chemin. Utilise `fileExists` si tu veux
  savoir si le chemin existe réellement.
- **Les hashes ont un suffixe `sum`** pour matcher les outils Unix
  standard (`md5sum`, `sha256sum`, etc.). Découvrable pour qui
  est familier de la ligne de commande.
- **`copy` est mono-fichier, `copyTree` est récursif**. Lua n'a
  pas la surcharge de fonctions par type, donc les séparer évite
  l'ambiguïté "est-ce que `copy('dir', 'dir')` voulait dire
  récursif ?".

## Hors v1

- Glob matching en fonction standalone (`luapilot.glob`). Utilise
  `find` avec `pattern = "*.lua"` pour le cas courant.
- I/O asynchrone. Utilise plutôt [`workers`](workers.md) pour le
  traitement parallèle de fichiers.
- BLAKE2s (seul BLAKE2b-512 est exposé via OpenSSL EVP).
