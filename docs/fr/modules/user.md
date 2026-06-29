> [English](../../en/modules/user.md) | **Français**

# `babet.user`

Lookups d'utilisateurs système via NSS, sans parser `/etc/passwd`
directement.

## Pourquoi

Sur un système Linux moderne, les utilisateurs peuvent venir de
plusieurs sources : `/etc/passwd`, LDAP, SSSD, NIS+, FreeIPA,
systemd-userdb, ou d'autres plugins NSS. Lire `/etc/passwd`
directement rate tout sauf la première source. `babet.user`
s'appuie sur `getpwnam_r(3)` et `getpwuid_r(3)`, qui traversent la
chaîne de résolveurs configurée dans `/etc/nsswitch.conf` —
exactement comme `id` ou `getent passwd` le font.

## API

| Fonction | Renvoie |
| --- | --- |
| `user.get(name_or_uid)` | `table` \| `(nil, "user not found")` \| `(nil, "user: <err>")` |
| `user.exists(name_or_uid)` | `boolean` |

`name_or_uid` accepte :

- une **string** → lookup par nom via `getpwnam_r`
- un **integer non-négatif** → lookup par UID via `getpwuid_r`

Tout autre type, un float, un integer négatif, une string contenant
un NUL embarqué, ou un integer supérieur à `uid_t` max lèvent via
`luaL_error` — ce sont des bugs côté appelant, pas des conditions
runtime à gérer proprement.

### Table renvoyée en cas de succès

```lua
{
    name  = "yaourt",
    uid   = 968,
    gid   = 968,
    gecos = "yaourt AUR build user",
    home  = "/var/cache/yaourt",
    shell = "/usr/sbin/nologin",
}
```

Tous les champs string sont garantis non-nil. Ils peuvent être des
strings vides quand la source NSS sous-jacente n'a pas de valeur
(c'est rare mais possible — typiquement un `gecos` vide).

## Exemple rapide

```lua
-- S'assurer qu'un user système est provisionné avant de lancer
-- le daemon.
if not babet.user.exists("yaourt") then
    error("user yaourt absent — fais d'abord useradd")
end

local u = assert(babet.user.get("yaourt"))
babet.chdir(u.home)
```

## Contrat d'erreur

- **Mauvais type d'argument** (`table`, `boolean`, `nil`, float,
  integer négatif, UID au-dessus de `uid_t` max, string avec NUL
  embarqué) → lève via `luaL_error`. Utilise `pcall` si tu dois
  récupérer.
- **Utilisateur absent** (le résolveur ne renvoie aucun match) →
  `get()` renvoie `(nil, "user not found")` ; `exists()` renvoie
  `false`.
- **Erreur NSS** (le résolveur lui-même est cassé : LDAP injoignable,
  mémoire saturée, etc.) → `get()` renvoie `(nil, "user: <description>")` ;
  `exists()` renvoie `false` (les erreurs sont silencieusement
  assimilées à "absent" pour les prédicats ; utilise `get()` si tu
  dois diagnostiquer).

## Décisions de design

- **NSS uniquement, jamais `/etc/passwd`** : lire le fichier
  directement raterait silencieusement tous les comptes qui ne sont
  pas dans le fichier local. Tout l'intérêt de `babet.user` est
  que le script obtienne la même réponse que `id` ou `getent passwd`.
- **Variantes `_r` thread-safe** : Babet expose `babet.workers`,
  donc on utilise `getpwnam_r`/`getpwuid_r` partout.
- **`gecos` exposé brut** : le format historique est
  `Full Name,Office,WorkPhone,HomePhone[,Other]`, mais en pratique
  la plupart des entrées contiennent juste un nom libre. Parser ça
  en Lua est trivial si nécessaire ; baker un parser en C++ serait
  prématuré.
- **Rejet du NUL embarqué** : `"root\0evil"` serait vu comme `"root"`
  par `getpwnam_r` (qui s'arrête au premier NUL). Sur de l'input
  dérivé d'un utilisateur, ça pourrait contourner une vérification
  d'identité en amont. Le rejet explicite est plus sûr que la
  troncature implicite.
- **Check du range `uid_t`** : sur Linux, `uid_t` est `uint32_t`,
  alors que `lua_Integer` est `int64_t`. Sans check, `5_000_000_000`
  serait silencieusement tronqué à 32 bits et pourrait matcher un
  compte non lié.

## Hors v1

Additif — ces choses pourraient être ajoutées plus tard sans casser
le SemVer :

- Lookups de groupes (`getgrnam` / `getgrgid`) — sera ajouté quand
  un cas concret le demandera.
- Accès à `/etc/shadow` (mots de passe, expiration) — hors scope.
  Exige root et est rarement utile hors scripts admin de niche.
  L'authentification mot de passe devrait passer par PAM, pas par
  Babet.
- Création d'utilisateurs/groupes — déjà faisable via
  `babet.exec("useradd …")` et `groupadd`. Pas besoin d'un
  binding dédié.
