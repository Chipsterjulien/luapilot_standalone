#ifndef DEEPCOPYTABLE_HPP
#define DEEPCOPYTABLE_HPP

#include <lua.hpp>
#include <unordered_map>

/**
 * @file deepCopyTable.hpp
 * @brief Déclarations pour la copie profonde des tables Lua.
 *
 * Ce fichier contient les déclarations des fonctions nécessaires pour effectuer
 * une copie profonde des tables Lua. La copie profonde gère les clés numériques et non numériques,
 * les métatables et évite les cycles pour ne pas provoquer de boucles infinies.
 */

constexpr int MAX_DEPTH = 75; // Profondeur maximale pour la copie de table

using VisitedMap = std::unordered_map<const void*, bool>;

/**
 * @brief Copie profondément une table Lua de manière récursive.
 *
 * Cette fonction copie les clés numériques et non numériques, gère les métatables et évite les cycles
 * en suivant les tables déjà visitées.
 *
 * @param L L'état Lua.
 * @param srcIndex Index de la table source.
 * @param destIndex Index de la table de destination.
 * @param nextNumericKey La prochaine clé numérique à utiliser dans la table de destination.
 * @param depth Profondeur actuelle de la récursion.
 * @param maxDepth Profondeur maximale autorisée pour la récursion.
 * @param visited Tables déjà visitées pour détecter les cycles.
 * @return True si la copie est réussie, false sinon.
 */
bool deepCopyTable(lua_State *L, int srcIndex, int destIndex, int &nextNumericKey, int depth, int maxDepth, VisitedMap& visited);

/**
 * @brief Liaison Lua pour effectuer une copie profonde d'une table Lua.
 *
 * Cette fonction peut être appelée depuis Lua avec une table comme seul argument.
 * Elle retourne une copie profonde de la table d'entrée.
 *
 * Usage Lua:
 *     copiedTable = deepCopyTable(originalTable)
 *
 * @param L L'état Lua.
 * @return int Le nombre de valeurs retournées à Lua (1 dans ce cas: la table copiée).
 */
int lua_deepCopyTable(lua_State *L);

#endif // DEEPCOPYTABLE_HPP
