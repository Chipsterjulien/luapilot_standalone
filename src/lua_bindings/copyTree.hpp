#ifndef COPYTREE_HPP
#define COPYTREE_HPP

#include <lua.hpp>
#include <filesystem>
#include <string>
#include <vector>

/**
 * @brief Copie un répertoire source vers une destination
 *
 * Cette fonction copie récursivement tous les fichiers et sous-répertoires
 * du répertoire source vers le répertoire de destination.
 *
 * @param source Le chemin du répertoire source
 * @param destination Le chemin du répertoire de destination
 * @return True si l'opération de copie a réussi, false sinon
 */
bool copy_directory(const std::filesystem::path& source, const std::filesystem::path& destination, std::string& error_message);

/**
 * @brief Fonction accessible depuis Lua pour copier un répertoire
 *
 * Cette fonction est appelée depuis Lua et utilise la fonction copy_directory
 * pour copier un répertoire. Elle s'attend à recevoir deux chaînes de caractères
 * en tant qu'arguments : le chemin source et le chemin de destination.
 *
 * @param L Pointeur vers l'état Lua
 * @return Nombre de valeurs de retour sur la pile Lua (1 en cas de succès, le résultat booléen)
 */
int lua_copyTree(lua_State* L);

#endif // COPYTREE_HPP
