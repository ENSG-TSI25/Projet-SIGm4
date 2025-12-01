# Projet SIGM4

## Architecture
```
Projet-SIGm4/
├── backend/          
│   ├── include/      # Headers (.hpp)
│   ├── src/          # Sources (.cpp)
│   ├── CMakeLists.txt
│   └── Dockerfile
├── frontend/         
├── infrastructure/   # Configuration Docker
│   ├── docker-compose.yml
│   └── .env
└── README.md
```

## Installation

### 1. Cloner le projet
```bash
git clone <repo-url>
cd Projet-SIGm4
```

### 2. Configuration
```bash
cd infrastructure
```

Créer `.env` avec :
```env
DB_USER=postgres
DB_PASS=postgres
DB_NAME=sigm4db
DB_PORT=5432
```

### 3. Lancer les services
```bash
docker-compose up --build
```

Les services démarrent :
- PostGIS : `localhost:5432`
- Backend : `localhost:3000`


### 4. Arrêter les services
```bash
docker-compose down
```

Pour supprimer aussi les volumes :
```bash
docker-compose down -v


## Dépannage

### Erreur "port already in use"
Arrêter PostgreSQL système :
```bash
sudo systemctl stop postgresql
```

### Erreur "ContainerConfig"
```bash
docker-compose down
docker rm backend_app postgis_db
docker-compose up --build
```
