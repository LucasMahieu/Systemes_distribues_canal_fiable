# Canal Fiable entre 2 processus
ATTENTION : SEULE LA BRANCHE 'backup' EST A JOUR ET FONCTIONNELLE

## Compilation
La compilation peu se faire à 2 niveaux

- Compilation du canal et de processus de test utilisant le canal
- Compilation de chaque entité séparément

### Pour compiler l'ensemble:  
`make`

### Pour compiler une entité:
`cd nom_entite`
`make`

### Pour l'exécution 
Nous vous conseillons d'ouvrir 2 terminaux et de lancer les deux processus de
tests dans l'ordre d'ouverture des terminal proposé ci-dessous:

NB: Pour terminer les processus qui ne se terminent pas seul, enfoncer
simplement Ctrl+C.

#### Pour lancer le test de base qui consiste à avoir deux procs qui s'envoient des messages par le canal fiable 
Dans terminal 1: 
`make B`  
Dans terminal 2: 
`make A`  

#### Pour lancer le test de débit 
Dans terminal 1: 
`make Bd`  
Dans terminal 2: 
`make Ad`  

#### Pour lancer le test de latence
Dans terminal 1: 
`make Bl`  
Dans terminal 2: 
`make Al`  

#### Pour lancer lancer le détecteur de faute 
Dans terminal 1: 
`make Bf`  
Dans terminal 2: 
`make Af`  
