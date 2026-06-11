> [English](../en/README.md) | **Français**

# LuaPilot — Manuel utilisateur

LuaPilot est un binaire Lua 5.5 standalone pour le scripting et
l'automatisation sous Linux. Ceci est le manuel de référence,
organisé en un fichier par module pour que chaque section reste
ciblée et facile à maintenir.

## Pour démarrer

- [`getting-started.md`](getting-started.md) — installation, premier
  script, les trois modes de lancement (dossier / embarqué / via
  PATH).
- [`security.md`](security.md) — ce contre quoi le durcissement
  protège et ne protège pas, patterns de moindre privilège.

## Modules

Chaque module vit dans son propre fichier sous [`modules/`](modules/) :

| Module | Rôle |
| --- | --- |
| [`argparse`](modules/argparse.md) | Parseur d'arguments de ligne de commande. |
| [`exec`](modules/exec.md) | Exécuter des programmes externes et capturer la sortie. |
| [`fs`](modules/fs.md) | Système de fichiers : listing, copie, hash, attrs. |
| [`http`](modules/http.md) | Client HTTP (basé sur cpp-httplib). |
| [`inotify`](modules/inotify.md) | Surveillance d'événements fichier (`inotify(7)`). |
| [`json`](modules/json.md) | Encode/décode JSON (nlohmann/json). |
| [`logging`](modules/logging.md) | Logger avec niveaux. |
| [`signal`](modules/signal.md) | Signaux POSIX : SIGTERM, SIGINT, … |
| [`socket`](modules/socket.md) | TCP client/serveur avec timeouts. |
| [`sqlite`](modules/sqlite.md) | Base de données SQL embarquée. |
| [`strings`](modules/strings.md) | Manipulation de chaînes (`split`, etc.). |
| [`sys`](modules/sys.md) | `which`, `hostname`, `env`, `pid`, … |
| [`tables`](modules/tables.md) | Manipulation de tables (`mergeTables`, `deepCopyTable`). |
| [`time`](modules/time.md) | Horloges monotone et temps réel. |
| [`tls`](modules/tls.md) | Sockets TLS : `connect_tls`, `starttls`. |
| [`toml`](modules/toml.md) | Parseur TOML (tomlplusplus). |
| [`user`](modules/user.md) | Lookups d'utilisateurs système via NSS. |
| [`workers`](modules/workers.md) | Jobs parallèles (vrais threads OS). |

Le module [`user`](modules/user.md) est la référence canonique du
pattern de documentation : *Pourquoi*, *API*, *Exemple rapide*,
*Contrat d'erreur*, *Décisions de design*, *Hors v1*. Les autres
modules varient dans le respect strict de ce template ; l'objectif
au fil du temps est de les aligner.

## Cookbook

[`cookbook.md`](cookbook.md) — recettes concrètes : watch de
dossier + email, fetches HTTP parallèles, arrêt propre,
TOML+SQLite, etc.

## Génération du PDF

Le manuel peut être exporté en un seul PDF :

```sh
cd docs
./build_doc.sh         # utilise pandoc + LaTeX
```

Voir [`docs/manual_order_fr.txt`](../manual_order_fr.txt) pour
l'ordre des fichiers ; c'est le seul endroit à éditer quand on
ajoute ou réordonne des chapitres.

## Conventions utilisées dans la doc

- **Tables d'API** : chaque module commence par une petite table
  qui liste les fonctions, leurs signatures, et ce qu'elles
  renvoient.
- **Contrat d'erreur** : une section dédiée explique quelles
  erreurs sont levées (`luaL_error`) et lesquelles sont renvoyées
  en `(nil, err)`. Cohérent entre tous les modules.
- **Décisions de design** : quand un choix n'est pas évident
  ("pourquoi pas récursif ?", "pourquoi obligatoire ?"), il y a un
  bref rationnel.
- **Hors v1** : une courte liste de ce qui pourrait être ajouté de
  manière additive plus tard sans casser le SemVer.
