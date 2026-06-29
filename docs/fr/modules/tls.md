> [English](../../en/modules/tls.md) | **Français**

# `babet.socket` TLS — `connect_tls` et `starttls`

La moitié TLS de [`socket`](socket.md) : TCP chiffré pour IRC
over TLS, IMAPS, protocoles sécurisés personnalisés. Deux points
d'entrée : `connect_tls` pour les protocoles qui font TLS dès le
départ (port 6697 IRC, 993 IMAPS, etc.) et `starttls` pour les
protocoles qui upgradent une connexion en clair existante (SMTP,
IMAP, IRC `STARTTLS`).

## Pourquoi

Parler du TCP chiffré sans [`http`](http.md) est un vrai besoin :
bots IRC sur `+6697`, soumission SMTP, services internes custom
derrière TLS. Le faire manuellement avec `openssl s_client` via
`exec` n'est pas fiable ; la bibliothèque C++ `openssl` + les
abstractions choisies de `cpp-httplib` donnent une API Lua propre
qui gère la vérification de certs correctement par défaut.

## API

| Fonction | Renvoie |
| --- | --- |
| `babet.socket.connect_tls(host, port, opts?)` | `tls_socket` \| `(nil, err)` |
| `s:starttls(opts?)` | `(true, nil)` \| `(nil, err)` — upgrade un socket en clair existant |

`opts` (fusionnés avec les opts socket habituels) :

| Champ | Type | Défaut |
| --- | --- | --- |
| `verify` | boolean | `true` |
| `ca_cert` | string (chemin) | bundle système |
| `ca_path` | string (chemin) | bundle système |
| `server_name` | string | argument host |
| `alpn` | array de strings | aucun |

Un `tls_socket` a les mêmes méthodes qu'un socket en clair
(`send`, `recv`, `recv_line`, `recv_all`, `set_timeout`,
`close`, `peer`). Le chiffrement est transparent.

## Exemples rapides

### Connexion à IRC over TLS

```lua
local s = assert(babet.socket.connect_tls("irc.libera.chat", 6697, {
    timeout = 30,
}))

s:send("NICK babet-bot\r\n")
s:send("USER babet 0 * :babet bot\r\n")

while true do
    local line, err = s:recv_line()
    if not line then break end
    print(line)
    if line:match("^PING (.+)$") then
        s:send("PONG " .. line:match("^PING (.+)$") .. "\r\n")
    end
end
```

### Upgrade STARTTLS (soumission SMTP)

```lua
local s = assert(babet.socket.connect("mail.example.com", 587))
print(s:recv_line())                              -- bannière 220
s:send("EHLO myhost\r\n")
repeat
    line = s:recv_line()
until not line:match("^250%-")                    -- dernière ligne 250

s:send("STARTTLS\r\n")
print(s:recv_line())                              -- 220 ready to start TLS

assert(s:starttls({ server_name = "mail.example.com" }))
-- s est maintenant chiffré ; continue avec EHLO + AUTH + …
```

### Serveur auto-signé (dev / CA privée)

```lua
local s = assert(babet.socket.connect_tls("internal.svc", 5555, {
    ca_cert = "/etc/myapp/internal-ca.crt",
}))
```

### Skip de la vérification (TESTS UNIQUEMENT)

```lua
local s = assert(babet.socket.connect_tls("self-signed.local", 8443, {
    verify = false,
}))
```

## Contrat d'erreur

- Toutes les erreurs préfixées `"tls: ..."` (handshake, cert
  verify, etc.) ou `"socket: ..."` (DNS, connect, timeout).
- `(nil, err)` toujours — pas d'exceptions pour les échecs
  "attendus" comme un cert mismatch.
- **`verify=true` + cert invalide** → `(nil, "tls: certificate
  verify failed: <reason>")`. Raisons courantes : expiré,
  auto-signé, hostname mismatch, CA inconnue.
- **Timeouts NaN/Inf**, mauvais types → lève via `luaL_error`.

## Trust store

Babet embarque sa propre OpenSSL statiquement liée. D'emblée,
`verify=true` fonctionne sur :

- **Arch, Debian, Ubuntu, Alpine, Gentoo** : via `--openssldir=/etc/ssl`
  baké dans l'OpenSSL embarquée.
- **Fedora, RHEL, CentOS, Rocky, Alma, OpenSUSE, FreeBSD, NetBSD**
  : via sondage runtime des chemins de CA bundles connus.

Overrides, par ordre de priorité :

1. `ca_cert` / `ca_path` par appel.
2. Variables d'env `SSL_CERT_FILE` / `SSL_CERT_DIR`.
3. Les deux mécanismes ci-dessus.

Si aucun ne trouve de CA store et que tu utilises `verify=true`,
le handshake échoue avec une erreur claire. Voir
[`security`](../security.md) pour le tableau complet.

## Décisions de design

- **TLS 1.2 minimum**. SSL 3, TLS 1.0, TLS 1.1 sont
  inconditionnellement refusés — ils sont cassés, aucun serveur
  moderne ne devrait s'y reposer. TLS 1.3 est négocié
  automatiquement quand disponible.
- **`verify=true` par défaut, `verify=false` exige un opt-in
  explicite**. Aucun moyen de désactiver accidentellement la
  vérification de cert.
- **`server_name` défaut sur l'argument host** pour que SNI
  marche tout seul. Passe-le explicitement si tu te connectes par
  IP à un serveur TLS qui utilise SNI.
- **Mêmes méthodes que socket en clair**, donc une fonction qui
  prend un `socket` fonctionne avec les deux de manière
  transparente.

## Hors v1

- Authentification client par certificat. Possible à ajouter
  plus tard en option `client_cert` / `client_key`.
- TLS session resumption / session tickets. Pas courant au niveau
  script.
- Vérification OCSP stapling. Reposition sur les défauts d'OpenSSL.
