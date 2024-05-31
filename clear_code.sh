#!/bin/bash

# Répertoires et fichiers à supprimer
items_to_delete=(
    "build"
    "luapilot"
)

# Supprimer chaque élément
for item in "${items_to_delete[@]}"; {
    if [ -e "$item" ]; then
        echo "Deleting $item..."
        rm -rf "$item"
    else
        echo "$item does not exist."
    fi
}

echo "Cleanup complete."
