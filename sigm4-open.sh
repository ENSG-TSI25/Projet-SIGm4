
#!/bin/bash
FILE_PATH="$1"
CONTAINER_PATH=$(echo "$FILE_PATH" | sed "s|^$HOME|/home/user|")

cd ~/SIGM4/Projet-SIGm4/infrastructure

# Vérifier si le conteneur tourne déjà
if ! docker ps --format '{{.Names}}' | grep -q "^qt_frontend$"; then
echo "Démarrage des conteneurs..."
docker-compose up -d
# Attendre seulement au premier démarrage
sleep 3
else
echo "Conteneurs déjà actifs"
fi

# Tuer les anciennes instances frontend seulement si elles existent
docker exec qt_frontend pkill -9 frontend 2>/dev/null || true

# Lancer immédiatement (pas besoin d'attendre)
docker exec -e DISPLAY=$DISPLAY qt_frontend /app/frontend/build/frontend "$CONTAINER_PATH"
