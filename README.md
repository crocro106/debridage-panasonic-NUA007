# debridage-panasonic-NUA007
Licence GPL 3.0

module à base de PIC12HV615 permettant - à la demande - de diviser par deux la vitesse mesurée sur NUA007 - Testé sur VAE Peugeot 

ce module permet - à la demande - de diviser par deux la fréquence des impulsions du capteur de roues pour permettre de maintenir une assistance au delà de 24 km/h. 
De manière à rester réglementaire lorsque l'on est sur route, la fonction est désactivée par défaut et s'active lors d'un appui long (2s) sur le bouton "phares" - elle se désactive de la même manière (bistable)

Un appui court sur la fonction "phare" permet de mettre en route ou éteindre les phares sans impacter la fonction débridage

Le principe est le suivant : 
  Alimentation par "vol" de courant sur le fil rouge du connecteur du display (voir fichier panasonic NUA007.pdf pour les pin utilisées)
  (0V si display éteint, Vbatt si display allumé, soit environ 42V chargé mini 32 V environ si batterie déchargée). 
  Le pic est alimenté via une résistance de 22K. Ce modèle possède un shunt interne qui permet de ramener la tension à 4,75V.
  pour ceux qui voudraient le remplace par un attiny il faudra réfléchir à l'alimentation.... 
  
  Récupération de l'info "appui sur bouton phare" sur le fil blanc du connecteur display. il sort aussi un "pic" à 42 V puis environ 2,5s à 28V...
  
  Le connecteur de l'ILS est débranché, les deux connecteurs sont rebranchés sur la carte d'interface (j'ai du remplacer les connecteurs - trop petits)
  
  l'entrée GPIO2 est connectée à l'une des broches de l'ILS et à un "pull up" 
  (le pull up interne n'a pas fonctionné chez moi, vous pouvez tenter de l'activer et supprimer la résistance de pull up associée, ça devrait marcher)
  la seconde broche de l'ILS est à la masse
  
  L'entrée GPIO5 est connecté via un pont diviseur de 68K - 18K à la commande de phares 
  (une zener de 4,7V est montée en parallèle de la 18K pour abaisser la tension lorsque la batterie est pleine)- 
  un pont 100K-18K serait préférable en fait
  
  La sortie GPIO4 est reliée à la porte (gate) d'un NMOS via une résistance d'1K, 
  une résistance de 100K entre G et S permet d'assurer le retour à l'état bloqué
  le drain du NMOS est relié au connecteur du capteur coté moteur 
  Bien repérer le sens du brochage. moteur sous tension, mesurer la tension entre les deux fils (normalement 5V pull up) et se connecter au coté +
  (le + est sur le fil gris chez moi) connecter l'autre (fil noir chez moi) à la masse
  
  Prévoir sur la carte (voir schéma kicad ou image associée) le connecteur nécessaire à la programmation du PIC. (J'utilise un pickit3)
  
Le code
  Normalement il est bien commenté. le principe est de ne diviser par deux le nombre d'impulsion que si celles-ci sont de période inférieure à 320 ms
  cette valeur est à adapter en fonction de votre taille de roue. Cela permet de conserver un affichage cohérent de la vitesse à basse vitesse. 
  
  la fonction n'est activée que si on a appuyé plus de 2000 ms sur le bouton phare (valeur modifiable aussi dans le code) 
   
  
  
  
