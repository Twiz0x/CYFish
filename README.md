## Utilisation:

1) Ouvrir le terminal linux en plein écran
2) Effectuer la commande **git clone https://github.com/Twiz0x/CYFish.git** (il faut git)
3) Effectuer la commande **cd CYFish**
4) Lancer le Makefile avec la commande **make**
5) Executer le programme avec la commande **./exec**















Consignes CY FISH:
- taille de la map fixée (ligne + colonnes) minimum 9 colonnes et 9 lignes (81 hexagones)
- demander nb de joueurs entre 2 et 6:
2 joueurs => 4 pinguins par joueur
3 joueurs => 3 pinguins par joueur
4 joueurs => 2 pinguins par joueur
5-6 joueurs ?

affichages:
- noms + scores joueurs en HUD

cases:
- distinguer celles suppr de celles encore présentes
- entre 1 et 3 poissons (fixé aléatoirement au début)
- pingouin d'un joueur sur case identifiable (ex: couleur différente pour le joueur)
- les pingouins démarrent tjr sur une case avec 1 poisson => verif qu'il y a assez de case avec 1 seul poisson
- nb de poisson visible sur chaque case

deplacement:
- 1 pingouin par tour, au moins 1 case:
> choix du pingouin à déplacer -> direction de déplacement (sur 6) -> distance de déplacement
> 
> Comment savoir si placement valide ? (1 pingouin max par case + pas en dehors de la map + case non vide + interdit de traverser une case non vide)
> 
> placement non valide => redemander
> 
> aucun déplacement possible => tour passé
> 
> déplacement validé -> tp le pingouin, suppr ancienne case et ajouter poissons ancienne case

fin de jeu -> plus aucun joueur ne peut bouger
afficher gagnant
