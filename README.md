# debridage-panasonic-NUA007
module à base de PIC12HV615 permettant - à la demande - de diviser par deux la vitesse mesurée sur NUA007 - Testé sur VAE Peugeot 

ce module permet - à la demande - de diviser par deux la fréquence des impulsions du capteur de roues pour permettre de maintenir une assistance au delà de 24 km/h. 
De manière à rester réglementaire lorsque l'on est sur route, la fonction est désactivée par défaut et s'active lors d'un appui long (2s) sur le bouton "phares" - elle se désactive de la même manière (bistable)

Un appui court sur la fonction "phare" permet de mettre en route ou éteindre les phares sans impacter la fonction débridage

Le principe est le suivant : 
  Alimentation par "vol" de courant sur le fil rouge du connecteur du display (voir fichier panasonic NUA007.pdf pour les pin utilisées)
  (0V si display éteint, Vbatt si display allumé, soit environ 42V chargé mini 32 V environ si batterie déchargée)
  
  Récupération de l'info "appui sur bouton phare" sur le fil blanc du connecteur display. il sort aussi un "pic" à 42 V puis environ 2,5s à 28V...
  
  Le connecteur de l'ILS est débranché, les deux connecteurs sont rebranchés sur la carte d'interface (j'ai du remplacer les connecteurs - trop petits)
  
  l
