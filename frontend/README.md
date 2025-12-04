# Projet-SIGm4
Voici le git pour le développement de l'application SIGm4



### CHACUN CODE SUR LA BRANCHE DE SON GROUPE DE DEV (FRONT et BACK)
## créer une branche (de front ou de back) dès que vous ouvrez un nouveau ticket
## pensez à vérifier que vous ne poussez pas les fichiers build sur le git (à mettre dans le .gitignore)
## pour compiler, il faut suivre la procédure habituelle en créant un dossier build...

### NE PAS TOUCHER A LA BRANCHE RELEASE SAUF A LA FIN DU PROJET

### AVANT DE MERGE SUR LA BRANCHE DEV, DEMANDER UNE REVIEW À D'AUTRES GENS

```
cd frontend

docker build -t qgis_cpp_app .

xhost +local:root

docker run -it --rm \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qgis_cpp_app

```
