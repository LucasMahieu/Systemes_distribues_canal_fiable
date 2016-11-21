# Canal Fiable entre 2 processus

## Compilation
La compilation peut se faire à 2 niveaux

- Compilation du canal et du processus de test utilisant le canal
- Compilation du canal seulement et/ou du processus de test utilisant le canal

Pour compiler l'ensemble:  
`make`

Pour executer un proc et son canal:  
`make exeA` ou `make exeB`

Pour compiler myCanal:
`cd canal` puis `make myCanal` ou `make` 

Pour executer myCanal seulement:  
`make exeMyCanal ARG="0/1"`

Pour compiler myProcTestX:   
`cd myProcTestX` puis `make myProcTestX` ou `make`

