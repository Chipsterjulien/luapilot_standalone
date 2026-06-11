> [English](../en/security.md) | **Français**

# Modèle de sécurité

**LuaPilot exécute des scripts Lua de confiance. Ce n'est pas une sandbox.**

Les scripts s'exécutent avec la bibliothèque standard Lua complète
(`luaL_openlibs`), et LuaPilot expose en plus des primitives système
comme `exec`, `remove`, `rmdirAll` et `chdir`. Un script a donc les
mêmes privilèges que le processus qui le lance : il peut lire,
modifier ou supprimer des fichiers, et lancer des commandes
arbitraires. C'est volontaire — comme `make`, un script shell, ou
n'importe quel outil de build, LuaPilot est conçu pour exécuter du
code que tu contrôles.

Ne lance que des `main.lua` et des modules `require` auxquels tu
fais confiance. **N'utilise pas** LuaPilot pour exécuter du Lua
venant de sources non fiables ; il n'offre aucune isolation contre
du code hostile, et n'est pas pensé pour. Si tu dois exécuter des
scripts non fiables, utilise plutôt une sandbox Lua dédiée à cet
usage.

## Ce contre quoi le durcissement *protège*

Le travail de durcissement dans LuaPilot protège les usages
*légitimes* contre les accidents et les attaques de supply chain :

- **Gestion des chemins / symlinks** utilise `lexically_relative()`
  et des guards `is_within()` composant par composant (jamais des
  checks de préfixe de string), donc un `cp src dst/` ne peut pas
  s'évader dans un arbre voisin via un composant symlinké.
- **Cleanup des groupes de processus** dans `luapilot.exec`
  garantit que les enfants forkés ne survivent pas au parent et
  ne laissent pas de processus zombies quand le script est
  interrompu.
- **Limites de sortie** sur `luapilot.exec` bornent la quantité
  de stdout et stderr capturés en mémoire (par défaut 16 MiB
  chacun, configurable), pour qu'un sous-processus emballé ne
  puisse pas OOM le processus LuaPilot.
- **Checksums des dépendances** : chaque dépendance embarquée est
  SHA256-pinnée dans `build_local.sh`. Un hash vide refuse la
  build. Un mismatch supprime le fichier téléchargé et sort en
  erreur. Ça protège contre un upstream compromis ou une archive
  Wayback Machine altérée (source de fallback).
- **Vérification TLS** est activée par défaut (`verify=true`). La
  désactiver demande un `verify=false` explicite par appel — pas
  de fallback silencieux. Des CA stores personnalisés peuvent
  être passés via `ca_cert` ou `ca_path`, ou via les variables
  d'environnement `SSL_CERT_FILE` / `SSL_CERT_DIR`.

## Ce contre quoi il *ne protège pas*

- Un script malveillant. LuaPilot n'a pas de sandbox. Si tu fais
  `exec` sur de l'input utilisateur, tu as une injection shell.
  Si tu fais `loadstring` sur de l'input utilisateur, tu as une
  exécution de code arbitraire.
- Un attaquant déterminé qui a déjà obtenu une exécution de code
  sur la machine qui fait tourner LuaPilot. Le durcissement rend
  les erreurs accidentelles visibles, pas les attaques
  adversariales impossibles.
- Les problèmes OS-level (exploits kernel, évasions de
  conteneur, escalade de privilèges). LuaPilot est un binaire
  userland ordinaire.

## Pattern recommandé : moindre privilège

Quand tu fais tourner LuaPilot en service, traite-le comme
n'importe quel processus non privilégié :

```sh
# En root, crée un user système dédié, sans shell.
useradd --system \
        --home-dir /var/lib/myapp \
        --create-home \
        --shell /usr/sbin/nologin \
        --comment "myapp LuaPilot service" \
        myapp

# Utilise luapilot.user.exists("myapp") dans ton script d'install
# pour vérifier que le user est provisionné avant de lancer le
# daemon.
```

Combine ça avec des options de unit `systemd` comme `User=myapp`,
`PrivateTmp=yes`, `ProtectSystem=strict`, `NoNewPrivileges=yes`,
et `CapabilityBoundingSet=` (vide sauf si tu as vraiment besoin
d'une capability). Le kernel fait bien plus que LuaPilot ne peut
le faire pour contenir un script qui se comporte mal ; laisse-le
faire.
