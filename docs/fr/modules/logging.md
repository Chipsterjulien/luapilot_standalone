> [English](../../en/modules/logging.md) | **Français**

# `logging` — logger avec niveaux (module Lua)

Un logger standard avec niveaux : `debug`, `info`, `warn`,
`error`, `fatal`. Embarqué en tant que module Lua pur, chargé via
`require("logging")`. Pas dans le namespace `luapilot` — c'est un
module bibliothèque, comme `inspect`.

## Pourquoi

Chaque script non trivial finit par réinventer une variante de
"niveaux + timestamps + sortie fichier optionnelle". Standardiser
enlève le bikeshed et rend la sortie de log cohérente entre les
scripts LuaPilot.

## API

```lua
local log = require("logging")
```

| Fonction | Renvoie |
| --- | --- |
| `log.set_level(name)` | `(true, nil)` — `name` est l'un de `"debug"`, `"info"`, `"warn"`, `"error"`, `"fatal"` |
| `log.set_output(path)` | `(true, nil)` \| `(nil, err)` — chemin du fichier (défaut stderr) |
| `log.debug(...)`, `log.info(...)`, `log.warn(...)`, `log.error(...)`, `log.fatal(...)` | chacun affiche si son niveau est ≥ seuil courant |

Les messages sont timestampés (`YYYY-MM-DD HH:MM:SS`) et taggés
avec le niveau (`[DEBUG]`, `[INFO]`, etc.). Plusieurs arguments
sont concaténés avec des espaces, comme `print`.

## Exemple rapide

```lua
local log = require("logging")
log.set_level("info")   -- les appels debug en-dessous deviennent no-ops

log.info("démarrage, pid =", luapilot.pid())
log.warn("tentative", 3, "sur", 5)
log.error("connexion échouée :", err)

-- Optionnel : router vers un fichier
local ok, err = log.set_output("/var/log/myapp.log")
if not ok then
    log.fatal("impossible d'ouvrir le log :", err)
    os.exit(1)
end
```

Sortie :

```
2026-06-11 14:32:01 [INFO] démarrage, pid = 12345
2026-06-11 14:32:05 [WARN] tentative 3 sur 5
2026-06-11 14:32:05 [ERROR] connexion échouée : timeout
```

## Contrat d'erreur

- **`set_level(name)`** : nom inconnu lève via `error()`.
- **`set_output(path)`** : `(nil, err)` si le fichier ne peut pas
  être ouvert en append. Le logging continue vers le sink précédent
  (stderr si rien n'était défini).
- Les fonctions de logging elles-mêmes n'échouent jamais — si le
  fichier devient non-écrivable en cours de route, le message est
  silencieusement perdu (l'alternative — crasher le script parce
  que le volume de log est plein — est pire).

## Décisions de design

- **Module Lua pur**. Pas besoin de C++, et être en Lua signifie
  que les scripts peuvent le monkey-patcher (ex : ajouter une
  sortie format JSON) au niveau appelant sans recompiler LuaPilot.
- **Cinq niveaux, pas de `trace`**. Cinq suffisent pour presque
  tout le monde ; en ajouter d'autres invite chacun à inventer
  les siens.
- **Sink global unique**. Plusieurs loggers / loggers
  hiérarchiques / niveaux par module est un piège de feature creep.
  Si tu as besoin de plusieurs destinations, écris un wrapper.

## Hors v1

- Sortie JSON / logging structuré. Facile à bricoler au niveau
  script si nécessaire.
- Rotation des logs. Utilise `logrotate(8)` sur le fichier de
  sortie.
- Sortie asynchrone / buffered. Optimisation prématurée pour
  l'usage typique.
