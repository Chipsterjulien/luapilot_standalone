> [English](../en/getting-started.md) | **Français**

# Pour démarrer

## Téléchargement

Des binaires précompilés pour x86_64, aarch64 (RPi4) et armv6l
(RPi0) sont disponibles sur la
[page des releases](https://github.com/Chipsterjulien/luapilot_standalone/releases).

## Compilation depuis les sources

LuaPilot embarque toutes ses dépendances (Lua, OpenSSL, SQLite,
miniz, nlohmann/json, cpp-httplib, tomlplusplus), donc les seuls
prérequis sur ton système sont :

- un compilateur C++23 (GCC ou Clang récent)
- CMake (≥ 3.20)
- `wget` et `unzip`

```sh
git clone https://github.com/Chipsterjulien/luapilot_standalone.git
cd luapilot_standalone
./build_local.sh        # télécharge les deps et compile
                        # (~5 min au premier lancement, plus vite après)
./run_tests.sh          # harness offline — doit afficher 827 PASS / 0 FAIL
```

Le script de build télécharge chaque dépendance depuis sa source
upstream, vérifie son SHA256, puis compile statiquement. Si la
source upstream est temporairement indisponible (`lua.org` a
notamment connu des pannes), le script bascule sur la Wayback
Machine d'Internet Archive — le check SHA256 reste appliqué.

Le binaire produit est dans `test/luapilot`.

## Trois manières de lancer un script Lua

LuaPilot supporte trois modes de lancement :

### 1. Mode dossier

Pointe le binaire vers un dossier contenant `main.lua` :

```sh
echo 'print("hello from LuaPilot")' > main.lua
./test/luapilot .
```

`require("X")` dans `main.lua` cherche `X.lua` dans le même dossier.

### 2. Mode embarqué (binaire autonome)

Empaquète un script et ses modules dans un binaire autonome :

```sh
./test/luapilot --create-exe . myapp
./myapp        # lance main.lua depuis le ZIP embarqué
```

En interne, LuaPilot append un ZIP à son propre binaire et lit
`main.lua` (plus tous les modules `require`) depuis ce ZIP au
runtime. Le `myapp` résultant est totalement autonome — pas
besoin d'avoir LuaPilot installé sur la machine cible, juste
glibc.

### 3. Embarqué via PATH (dossier + auto-détection)

Si LuaPilot est sur `$PATH` et qu'on fait `chmod +x main.lua`
après avoir ajouté une ligne shebang :

```lua
#!/usr/bin/env luapilot
print("hello")
```

```sh
chmod +x main.lua
./main.lua
```

LuaPilot utilise le dossier du script comme dossier de travail,
donc `require("helpers")` trouvera `helpers.lua` à côté de
`main.lua`.

## Premier script

Une fois le binaire compilé, essaie ceci :

```lua
-- main.lua
print("LuaPilot dit bonjour")
print("PID :", luapilot.pid())
print("Hôte :", luapilot.hostname())

local r, err = luapilot.http.get("https://example.com/")
if r then
    print("Status :", r.status)
else
    print("Échec HTTP :", err)
end
```

Lance-le avec `./test/luapilot .` (ou un autre mode au choix).

## Pour aller plus loin

- Chaque module a sa propre page sous [`modules/`](modules/).
- Le module [`user`](modules/user.md) est un petit exemple complet
  du pattern de documentation utilisé partout — lis-le comme la
  référence canonique.
- Vois [`security.md`](security.md) avant d'exposer des scripts
  LuaPilot à quoi que ce soit qui pourrait recevoir de l'input non
  fiable.
