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
## Tests en local

#### Installer les dépendances 

```bash
sudo apt-get update
sudo apt-get install libgtest-dev libgmock-dev
```
#### Configurer le projet avec CMake

```bash
cd backend
rm -rf build
mkdir build
cd build
cmake ..
make
```
#### Lancer tous les tests :

```bash
ctest
# Avec des détails
ctest --verbose
```
#### Sortie en cas d'échec

```bash
ctest --output-on-failure
```

#### Lister les tests :
```bash
ctest -N
```

#### Lancer un test en particulier :

```bash
./tests/nom_test
# Exemple :
./tests/unit_tests
```

## Installation

### 1. Cloner le projet
```bash
git clone https://github.com/ENSG-TSI25/Projet-SIGm4.git
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
```


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
