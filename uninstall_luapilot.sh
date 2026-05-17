#!/bin/bash

# Chemin du binaire à supprimer
binary_path="/usr/bin/luapilot"

# Vérifier si le binaire existe et le supprimer
if [ -e "$binary_path" ]; then
    echo "Suppression de $binary_path..."
    sudo rm -f "$binary_path"
    echo "$binary_path a été supprimé."
else
    echo "$binary_path n'existe pas."
fi
