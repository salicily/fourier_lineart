Programme jouet de décomposition d’images "bitmap lineart" en séries de Fourier
===============================================================================

Programmes requis :
- GNU make
- GCC

Compiler le programme :
- make

Emplacement du programme compilé :
- bin/mini_fourier

Options du programme compilé :
- --source "nom_de_fichier" (obligatoire) défini le fichier à décomposer
- --destination_prefix "sortie"           tous les fichiers images commenceront par ce nom (défaut : la source privée du ".bmp")
- --starting_mode N                       toutes les images contiendront les N premières harmoniques (défaut : 0)
- --pictures P                            calculera P images (défaut : 1)
- --mode_increment K                      K harmoniques seront ajoutées à chaque nouvelle image (défaut : 1)
- --mode_quad Q                           l’image i contiendra Q×i harmoniques de plus que la précédente (défaut : 0)
- --xscale Kx                             zoom l’image d’un facteur Kx (abscisses) (défaut : 1.0)
- --xshift Px                             décale l’image de Px (abscisses) (défaut : 0.0)
- --yscale Ky                             zoom l’image d’un facteur Ky (ordonnées) (défaut : 1.0)
- --yshift Py                             décale l’image de Py (ordonnées) (défaut : 0.0)

L’image source doit être une image monochrome, avec un trait si possible d’une épaisseur de 1 pixel (augmenter l’épaisseur va demander de calculer un cycle avec plus de points, et possiblement exploser en mémoire, la contrainte de 1 pixel est là pour minimiser le nombre de points à traiter).
