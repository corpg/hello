@echo off
cd ./launch

rem   hello.exe [client] [IP du serveur (par defaut: 127.0.0 )] [port sur lequel on envoie les donnee (par défaut: 3851)]
rem Exemple:
rem		hello.exe client X.X.X.X 80 (pensez a configurer le serveur pour qu'il ecoute sur le port 80)
rem		hello.exe
hello.exe client
