# Présentation
SimuWorld est un simulateur d'écosystème simple et flexible, permettant de simuler des populations de plusieurs milliers d'animaux en temps réel.
Réalisé dans le cadre de mon projet de première année de prépa MPSI, la problématique posée était la suivante :
> Comment permettre à un gérant de parc naturel de prédire l'évolution de ses populations ?

Son point fort se situe dans sa forte customisation, tout est paramétrable à souhait :
- Attributs des animaux (espérance de vie, vitesse de déplacement, points de vie, régime alimentaire...)
- Attributs des ressources naturelles (valeur nutritionnelle, fréquence de réapparition...)
- Effectifs des populations et des ressources naturelles présentes au départ
- Taille du parc naturel

**Observez vos prédateurs partir à la chasse en meute pour se nourrir :**

<img src="https://user-images.githubusercontent.com/44875665/56867385-d1831700-69e4-11e9-8428-06fd5a51319d.gif" alt="hunting">

**Observez-les chercher le partenaire idéal et se reproduire :**

<img src="https://user-images.githubusercontent.com/44875665/56867496-24a99980-69e6-11e9-84e7-bb435a8bded5.gif" alt="reproduction">

**Prenez un peu de hauteur et monitorez les déplacements de vos populations et l'état des ressources naturelles :**

<img src="https://user-images.githubusercontent.com/44875665/56867518-47d44900-69e6-11e9-9496-1850415106d4.gif" alt="zoom">

**Enfin, accélérez le temps et visualiser les effectifs des populations et des ressources évoluer sous vos yeux :**

<img src="https://user-images.githubusercontent.com/44875665/56868161-f8dee180-69ee-11e9-970c-784e3701af0a.gif" alt="graphs">

# Paramétrage
Les paramétrages passent pas le dossier "settings" qui s'agence de la façon suivante :
- blood : gère l'apparition des tâches de sang lors de combat
- font : gère la taille et l'espacement de la police
- map : gère la taille et la répartition des biomes
- nutritional_value : gère la valeur nutritionnelle de chaque ressource et de chaque espèce animale
- resource_1 ... resource_X : gère les attributs de chaque ressource naturelle
- resources_init : gère le nombre de ressources initialement présente
- resources_name : gère le nom des ressources affichés à l'écran et sauvegardés dans le fichier de résultat
- specie_1 ... specie_X : gère les attributs de chaque espèce animale
- species_init : gère l'effectif des espèces initialement présentes
- species_name : gère le nom des espèces affichés à l'écran et sauvegardés dans le fichier de résultat

Il n'y a pas de limite concernant le nombre de ressource ou d'espèce pris en charge, il suffit de rajouter un fichier .txt et une texture correspondante et le tour est joué !

**Attention :** Pensez à rajouter les lignes nécessaires dans les autres fichiers également (par exemple, l'effectif initial de la nouvelle espèce dans species_init)

# Résultats
Au moment de quitter la simulation, il vous sera demandé si vous voulez sauvegarder les résultats de la simulation.
Si c'est le cas, ils seront conservés dans un fichier .txt dans le dossier "results" avec les paramètres utilisés lors de la simulation. Les résultats peuvent être ensuite importés facilement sous Excel pour tracer les graphiques.
