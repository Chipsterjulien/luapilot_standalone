// user.hpp — Module luapilot.user : lookup d'utilisateurs système
//
// API exposée :
//   luapilot.user.get(name_or_uid) -> table | (nil, err)
//   luapilot.user.exists(name_or_uid) -> boolean
//
// Détails de design :
//
// 1. Backend : NSS via getpwnam_r(3) / getpwuid_r(3).
//
//    On ne lit JAMAIS /etc/passwd directement. Les utilisateurs
//    peuvent venir de LDAP, SSSD, NIS+, systemd-userdb, FreeIPA,
//    ou tout autre source configurée dans /etc/nsswitch.conf.
//    getpw*_r traverse cette chaîne, c'est ce que `id` ou `getent`
//    utilisent.
//
// 2. Thread-safety : versions _r utilisées partout (compatible
//    workers).
//
// 3. Argument get/exists :
//      - string  → lookup par nom (getpwnam_r)
//      - integer → lookup par UID (getpwuid_r)
//      - autre type → luaL_error (mauvais type, comme le reste du
//        projet ; pas de coercion silencieuse)
//
// 4. Retour de get() :
//    Trouvé → table :
//      { name = "yaourt",
//        uid = 968,
//        gid = 968,
//        gecos = "yaourt AUR build user",
//        home = "/var/cache/yaourt",
//        shell = "/usr/sbin/nologin" }
//    Absent → (nil, "user not found")
//    Erreur NSS → (nil, "user: <description>")
//
// 5. Retour de exists() : booléen strict.
//    Trouvé → true. Absent OU erreur NSS → false.
//    Pour distinguer absent et erreur, utiliser get().
//
// 6. Hors scope :
//      - /etc/shadow et mots de passe (pas dans le périmètre, et
//        getspnam exige root) ;
//      - groupes (getgrnam) — sera ajouté si un cas concret le
//        demande ;
//      - création/modification d'utilisateurs (déjà faisable via
//        luapilot.exec("useradd ...")).

#pragma once

extern "C"
{
#include <lua.h>
}

// Crée la sous-table luapilot.user avec les fonctions get et exists.
// Précondition de pile : la table luapilot est au sommet.
// Postcondition : la pile est inchangée (la sous-table est posée
// comme champ "user" de luapilot).
void register_user(lua_State *L);
