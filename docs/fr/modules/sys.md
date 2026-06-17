> [English](../../en/modules/sys.md) | **Français**

# `luapilot` sys — utilitaires système

Helpers d'introspection processus et hôte : PID, hostname, infos
kernel, lookup d'exécutable, variables d'environnement.

## Pourquoi

La stdlib Lua a `os.getenv` et `os.execute`, mais rien pour le PID,
le hostname, les infos kernel, ou un `which` portable. Ce sont des
besoins de scripting universels qui ne méritent pas un détour par
`popen`.

Ces fonctions vivent directement sur `luapilot` (plat, pas de
sous-table) parce qu'elles précèdent la convention par module
adoptée pour les ajouts récents.

## API

| Fonction | Renvoie |
| --- | --- |
| `luapilot.pid()` | `integer` — PID du processus courant |
| `luapilot.hostname()` | `string` \| `(nil, err)` — hostname système |
| `luapilot.uname()` | `table` avec `sysname`, `nodename`, `release`, `version`, `machine` |
| `luapilot.which(cmd)` | `string` (chemin absolu) \| `(nil, "not found")` |
| `luapilot.env(name)` | `string` \| `nil` — comme `os.getenv`, en cohérent |
| `luapilot.setenv(name, value)` | `(true, nil)` \| `(nil, err)` |
| `luapilot.getMemoryUsage()` | `integer` — mémoire de la VM Lua en octets (après un GC complet) |
| `luapilot.getDetailedMemoryUsage()` | `(integer, integer)` — actuellement les deux valeurs sont égales au compteur GC en octets ; gardé en deux retours pour la stabilité d'API |

## Constantes runtime

La même table `luapilot` expose aussi des constantes qui décrivent
le binaire LuaPilot qui exécute le script. Utile pour le logging,
pour conditionner une feature à une version minimum, ou pour
sanity-checker le runtime.

| Constante | Type | Exemple |
| --- | --- | --- |
| `luapilot.VERSION` | `string` | `"1.7.1"` |
| `luapilot.VERSION_MAJOR` | `integer` | `1` |
| `luapilot.VERSION_MINOR` | `integer` | `7` |
| `luapilot.VERSION_PATCH` | `integer` | `1` |

La version string est ce que `luapilot --version` affiche. Les
composantes integer sont là pour faire des comparaisons
programmatiques sans parser la string.

```lua
-- Logger le runtime
print("tourne sous LuaPilot " .. luapilot.VERSION)

-- Conditionner une feature à une version minimum
local need_minor = 7
if luapilot.VERSION_MAJOR < 1
   or (luapilot.VERSION_MAJOR == 1 and luapilot.VERSION_MINOR < need_minor) then
    error("ce script nécessite LuaPilot >= 1." .. need_minor)
end
```

## Exemple rapide

```lua
print("Sur :", luapilot.hostname(), "PID :", luapilot.pid())

local u = luapilot.uname()
print("Kernel :", u.sysname, u.release, u.machine)

-- Trouver un binaire
local sh, err = luapilot.which("bash")
if sh then
    luapilot.exec({ sh, "-c", "echo hello" })
end

-- Lire/écrire env
print("HOME :", luapilot.env("HOME"))
luapilot.setenv("MY_VAR", "value")

-- Introspection mémoire (force un GC complet d'abord)
print("Mémoire VM Lua :", luapilot.getMemoryUsage(), "octets")
local a, b = luapilot.getDetailedMemoryUsage()
print("Détaillé :", a, b)
```

## Contrat d'erreur

- **`pid()`** : ne peut pas échouer, retourne toujours un integer.
- **`hostname()`** / **`which()`** : `(value, nil)` en succès,
  `(nil, err_string)` en échec.
- **`env(name)`** : `value` si défini, `nil` sinon. Ne lève jamais.
- **`setenv(name, value)`** : `(true, nil)` en succès, `(nil, err)`
  en échec (rare — généralement OOM ou nom invalide).
- **`uname()`** : ne peut pas échouer sur un système supporté,
  renvoie toujours la table complète.
- **Mauvais type d'argument** (ex : `which(42)`) → lève via
  `luaL_error` après coercion string par convention Lua. Passe une
  table ou booléen pour forcer une vraie erreur.

## Décisions de design

- **`env(name)` renvoie `nil`, pas `""`, pour les variables non
  définies**. Cohérent avec `os.getenv`. Les tests doivent utiliser
  `env("X") == nil`, pas `env("X") == ""`.
- **`which` renvoie le chemin absolu**, pas juste "oui/non". C'est
  plus utile : on peut `exec` le chemin renvoyé directement. Pour
  un check booléen, utilise `which(cmd) ~= nil`.
- **`uname()` renvoie une table, pas plusieurs valeurs**. Reste
  forward-compatible si un champ est ajouté un jour (ex :
  `domainname`).

## Hors v1

Additif — pourrait être ajouté plus tard :

- `unsetenv` (juste `setenv(name, nil)` pourrait marcher aussi).
- `getuid` / `getgid` / `getppid` / `getlogin`.
- Dump complet de l'environnement en table.

Pour les lookups d'utilisateurs (UID → nom, etc.) voir le module
dédié [`user`](user.md) — il utilise NSS, qui est le bon backend
pour cette question.
