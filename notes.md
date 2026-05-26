# Notes — points reportés / dette technique connue

Liste des sujets identifiés mais volontairement non traités dans la
version courante. Chacun est documenté pour qu'un futur chantier
puisse repartir avec le contexte complet, ou pour qu'un contributeur
externe sache que c'est connu.

## 1. `execve` pur dans `exec.cpp` (hardening++)

**Statut actuel** : `exec.cpp` utilise `execvpe()` (extension glibc)
après `fork()` dans l'enfant, avec un envp préparé côté parent
(chantier 10-B post-revue Gemini).

**Risque résiduel théorique** : `execvpe()` parcourt `$PATH` pour
résoudre le binaire. Cela implique des appels à `strchr`, `strncmp`,
`access()`, `stat()` — tous async-signal-safe — mais aussi une
lecture de `getenv("PATH")` qui n'est pas formellement listée comme
async-signal-safe par POSIX.

En pratique sur glibc, `getenv` est une simple lecture du tableau
`environ` sans allocation. Le risque concret est très faible.

**Fix propre** : résoudre le chemin du binaire côté parent
(reimplementer la recherche `$PATH`) puis appeler `execve(path,
argv, envp)` dans l'enfant. Estimation : ~30-50 lignes.

**Pourquoi reporté** : on est passés de « `setenv()` qui peut
deadlocker à coup sûr en multi-thread » (vrai risque démontrable) à
« `execvpe` qui appelle `getenv` » (risque théorique non observable).
Le delta restant ne justifie pas la complexité supplémentaire pour
l'instant. À reconsidérer si LuaPilot vise jamais des systèmes
non-glibc ou si un cas d'usage révèle un problème concret.

**Référence revue** : ChatGPT, post-chantier 10-A.

## 2. `SSL_CTX` par connexion quand `ca_cert`/`ca_path` est fourni

**Statut actuel** : `socket.cpp` utilise un `SSL_CTX` global unique
(initialisé via `std::call_once` depuis le chantier 10-A). Quand
l'utilisateur passe `opts.ca_cert` ou `opts.ca_path` à
`connect_tls`, on appelle `SSL_CTX_load_verify_locations(g_tls_ctx,
...)`, ce qui **modifie** ce contexte global.

**Risque résiduel** : si deux workers font du TLS avec des CA
*différents* exactement en même temps, la modification du `SSL_CTX`
global est une vraie race. Le résultat est imprévisible : un worker
pourrait voir le CA de l'autre, deux modifications concurrentes
pourraient produire un état corrompu.

**Cas d'usage non bloquant** : l'usage standard `connect_tls(host,
port)` sans `ca_cert`/`ca_path` n'est pas concerné — le verify
utilise le store système posé une seule fois à l'init.

**Fix propre** : créer un `SSL_CTX` dédié par connexion quand
`ca_cert` ou `ca_path` est fourni, partager le global sinon.
Estimation : ~40-60 lignes + revue de la libération propre du CTX
local au close.

**Pourquoi reporté** : sans cas d'usage concret avec CA custom dans
les workers, c'est un fix spéculatif. À traiter dès qu'un cas réel
émerge (par exemple, un bot qui se connecte à plusieurs services
internes avec leur propre PKI).

**Référence revue** : ChatGPT, post-chantier 10-A.

## 3. `workers.channel()` — channels indépendants pour
   communication worker-à-worker

**Statut actuel** : la v1.2.0 des workers offre `w:send` / `w:recv`
parent↔worker uniquement. Pour faire communiquer deux workers, il
faut router via le parent : `worker A → parent → worker B`.

**Limitation** : crée un goulet d'étranglement au niveau du parent
pour des architectures pub/sub ou pipeline. Acceptable pour 95% des
cas (bot IRC, scripts d'admin, jobs parallèles), mais limitant pour
des flux complexes.

**Fix propre** : ajouter `workers.channel()` qui crée un handle
indépendant passable au `spawn` d'un worker, partageable entre
plusieurs workers.

**Pourquoi reporté** : décision prise au cadrage du chantier 9 (cf.
échanges ChatGPT + Claude). Risque de sur-design avant d'avoir un
vrai cas d'usage. À reconsidérer si l'écriture du bot IRC ou d'un
autre projet révèle un manque concret.

## 4. `:kill()` pour workers — force-terminer une thread

**Statut actuel** : pas de `:kill()`. Le seul moyen d'arrêter un
worker est `w:close()` + collaboration du worker (qui doit poll
régulièrement sa inbox).

**Limitation documentée** : un worker bloqué dans une opération
système longue (un `recv()` socket sans timeout, par exemple) ne
peut pas être interrompu de l'extérieur. Le `__gc` du handle parent
attend la fin de la thread, ce qui peut bloquer tout le processus.

**Pourquoi pas en v1** : `pthread_cancel()` est dangereux —
verrouille les mutex, laisse les fichiers/sockets dans des états
indéterminés, peut corrompre les contextes OpenSSL. La seule
implémentation safe demanderait un système de points d'annulation
explicites, ce qui est l'opposé de la simplicité visée.

**Mitigation actuelle** : documentation explicite dans le README
(section « `__gc` will block if the worker is busy »).