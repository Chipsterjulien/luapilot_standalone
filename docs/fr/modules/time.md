> [English](../../en/modules/time.md) | **Français**

# `luapilot` time — horloges monotone et temps réel

Deux horloges avec précision sub-seconde et un sleep qui respecte
les unités `s`/`ms`/`us`/`ns` ainsi que les signaux. Comble les
trous de `os.time` / `os.clock`. Vit directement sur `luapilot`,
pas de sous-namespace.

## Pourquoi

`os.time()` ne donne que des secondes, et `os.clock()` mesure le
temps CPU, pas le temps réel. Aucun des deux n'est approprié pour
mesurer des durées réelles (latence de requête, backoff de retry,
timeouts). Et `os` n'a pas de sleep portable à la nanoseconde.

`luapilot.monotonic()` et `luapilot.now()` exposent
`CLOCK_MONOTONIC` (durées) et `CLOCK_REALTIME` (timestamps), avec
`luapilot.sleep()` qui respecte l'interruption par signal.

## API

| Fonction | Renvoie |
| --- | --- |
| `luapilot.monotonic()` | `number` — secondes depuis une epoch arbitraire (immune aux changements d'horloge) |
| `luapilot.now()` | `number` — temps POSIX (secondes depuis 1970-01-01 UTC) |
| `luapilot.sleep(amount, unit?)` | `(true, nil)` \| `(nil, "interrupted")` |

`unit` pour `sleep` est l'un de `"s"` (défaut), `"ms"`, `"us"`,
`"ns"`. Les floats sont acceptés.

## Exemple rapide

```lua
-- Mesurer une durée en toute sécurité (immune aux sauts NTP / DST)
local t0 = luapilot.monotonic()
do_something()
local elapsed = luapilot.monotonic() - t0
print(string.format("durée %.3f s", elapsed))

-- Timestamper un événement
local now = luapilot.now()
db:exec("INSERT INTO events (ts, kind) VALUES (?, ?)", { now, "ping" })

-- Dormir 100 ms, mais se réveiller sur un signal
local ok, err = luapilot.sleep(100, "ms")
if not ok and err == "interrupted" then
    print("réveillé par un signal")
end
```

## Contrat d'erreur

- **`monotonic()` / `now()`** : ne peuvent pas échouer. Renvoient
  toujours un number.
- **`sleep(amount, unit)`** :
  - `(true, nil)` si le sleep s'est terminé normalement.
  - `(nil, "interrupted")` si un signal géré est arrivé pendant le
    sleep (voir [`signal`](signal.md)).
- **NaN / Inf / amount négatif** → lève via `luaL_error`.
- **String d'unit inconnue** → lève via `luaL_error`.

## Décisions de design

- **Deux horloges, deux fonctions, pas de flag**. `monotonic()`
  pour les durées, `now()` pour les timestamps. Les fondre en une
  fonction avec paramètre inviterait au mauvais choix.
- **`now()` pour le temps réel**, pas `realtime()` — plus court,
  et le sens naturel en anglais matche ce qu'on attend (l'heure
  actuelle).
- **`sleep` conscient des signaux**. Un sleep de 60 secondes qui
  ignore `SIGTERM` est le bug classique de l'arrêt gracieux.
  `sleep()` renvoie `(nil, "interrupted")` pour que l'appelant
  puisse sortir proprement.
- **Pas de helper "format date"**. `os.date(format, luapilot.now())`
  marche très bien, et une fonction dupliquée ne serait qu'un
  wrapper.

## Hors v1

- `luapilot.boottime()` (`CLOCK_BOOTTIME` — inclut le suspend).
  Rarement utile, facile à ajouter plus tard.
- Helpers `luapilot.utcnow()` / `localnow()` qui renvoient des
  strings pré-formatées. Trivial en Lua, sans valeur ajoutée.
