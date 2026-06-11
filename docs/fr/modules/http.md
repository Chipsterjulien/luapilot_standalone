> [English](../../en/modules/http.md) | **Français**

# `luapilot.http` — client HTTP

Un client HTTP/HTTPS simple basé sur
[cpp-httplib](https://github.com/yhirose/cpp-httplib), réutilisant
l'OpenSSL embarqué. Synchrone, bloquant, avec timeouts.

## Pourquoi

Presque chaque script non trivial doit parler à une API HTTP. Sans
client intégré, on en vient à utiliser `curl` via `exec`, avec
tout l'overhead de quoting et de spawn de process. `luapilot.http`
rend une requête accessible en une ligne, avec la vérification TLS
par défaut.

## API

| Fonction | Renvoie |
| --- | --- |
| `luapilot.http.request(opts)` | `response` (table) \| `(nil, err)` |
| `luapilot.http.get(url, opts?)` | raccourci pour `request{ method="GET", url=url, ... }` |
| `luapilot.http.post(url, opts?)` | raccourci pour `request{ method="POST", url=url, ... }` |
| `luapilot.http.put(url, opts?)` | raccourci pour `request{ method="PUT", url=url, ... }` |
| `luapilot.http.delete(url, opts?)` | raccourci pour `request{ method="DELETE", url=url, ... }` |

### Table `opts`

| Champ | Type | Défaut |
| --- | --- | --- |
| `url` | string (requis pour `request`) | — |
| `method` | string | `"GET"` |
| `headers` | table de `name = value` | `{}` |
| `body` | string | `""` |
| `timeout` | number (secondes) | 30 |
| `verify` | boolean (vérification cert TLS) | `true` |
| `ca_cert` | string (chemin du fichier CA bundle) | défaut système |
| `ca_path` | string (chemin du dossier CA) | défaut système |
| `follow_redirects` | boolean | `true` |
| `max_redirects` | integer | 5 |

### Table `response`

```lua
{
    status = 200,
    body = "...",
    headers = {                 -- clés normalisées en minuscules
        ["content-type"] = "application/json",
        ["content-length"] = "42",
    },
    -- URL finale après redirects :
    url = "https://api.example.com/v1/data",
}
```

## Exemples rapides

```lua
-- GET simple
local r, err = luapilot.http.get("https://api.example.com/health")
if r then
    print(r.status, r.body)
end

-- POST JSON avec header d'auth
local r, err = luapilot.http.post("https://api.example.com/v1/things", {
    headers = {
        ["Content-Type"] = "application/json",
        ["Authorization"] = "Bearer " .. token,
    },
    body = luapilot.json.encode({ name = "widget", count = 7 }),
    timeout = 10,
})

-- Cert auto-signé (dev / CA privée)
local r = luapilot.http.get("https://internal.svc/", {
    ca_cert = "/etc/myapp/internal-ca.crt",
})

-- Désactiver la vérification (TESTS UNIQUEMENT)
local r = luapilot.http.get("https://expired.badssl.com/",
                            { verify = false })
```

## Contrat d'erreur

- **Mauvais types d'argument** → lève via `luaL_error`.
- **Erreurs réseau** (DNS, connect, timeout, TLS) →
  `(nil, "http: <description>")`.
- **Les erreurs de statut HTTP ne sont pas des erreurs** — un
  `404` renvoie un `response` normal avec `status = 404`. La
  sémantique du statut est la responsabilité de l'appelant.

## TLS / trust store

LuaPilot embarque sa propre OpenSSL statique, donc il n'hérite pas
automatiquement de la config `ca-certificates` de la distro. Deux
mécanismes garantissent que `verify=true` fonctionne d'emblée :

1. L'OpenSSL embarquée est compilée avec `--openssldir=/etc/ssl`,
   couvrant Arch, Debian, Ubuntu, Alpine, Gentoo.
2. À l'init TLS, LuaPilot sonde les chemins de CA bundles connus
   pour Fedora/RHEL, OpenSUSE, FreeBSD, NetBSD.

Tu peux override par appel avec `ca_cert` / `ca_path`, ou
globalement via les variables d'environnement `SSL_CERT_FILE` /
`SSL_CERT_DIR`. Voir [`security`](../security.md) et
[`tls`](tls.md) pour les détails.

## Décisions de design

- **Synchrone, bloquant**. Les scripts font généralement du
  request-response one-shot ; le sync est plus simple et
  suffisant. Pour beaucoup d'appels parallèles, utilise
  [`workers`](workers.md).
- **`verify=true` par défaut**. Désactiver la vérification TLS
  doit être explicite (`verify=false`). Pas de downgrade
  silencieux.
- **Le statut HTTP n'est pas une erreur**. L'appelant décide si
  `404` ou `500` est un échec ; l'appel réseau lui-même a réussi.
- **Headers normalisés en clés minuscules** dans la réponse, pour
  que les lookups case-insensitive marchent directement
  (`r.headers["content-type"]`).
- **Suivi des redirects par défaut**. Limité à 5 hops, à override
  avec `max_redirects` ou à désactiver avec
  `follow_redirects=false`.

## Hors v1

- Streaming du body de réponse (ex : pour télécharger de gros
  fichiers). Actuellement `body` est lu entièrement en mémoire.
- HTTP/2 / HTTP/3.
- WebSockets (utilise [`socket`](socket.md) + TLS + une
  bibliothèque de framing Lua si nécessaire).
- Côté serveur. cpp-httplib le supporte, mais exposer un serveur
  HTTP robuste à Lua est un design à part entière.
