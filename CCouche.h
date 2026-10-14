//-----------------------------------------------------------------------------------------------
#ifndef CCOUCHE_H
#define CCOUCHE_H
//-----------------------------------------------------------------------------------------------
// Une couche (anneau) du vase segmenté. Unités : mm.
//
// L'anneau est un polygone régulier de N segments trapézoïdaux :
//  - rOut est l'apothème extérieur : le polygone contient le cercle de rayon rOut ;
//  - rIn est le rayon des coins intérieurs : le polygone reste dans le cercle de rayon rIn.
// Ainsi le profil (plus la surépaisseur de tournage) est toujours contenu dans la matière.
// nbSegments == 0 : disque plein (fond).
class CCouche
{
public:
    int index = 0;
    double y0 = 0.0;
    double y1 = 0.0;
    double rOut = 0.0;
    double rIn = 0.0;
    int nbSegments = 0;

    bool isDisc(void) const;
    double halfAngle(void) const;       // radians, PI / N
    double miterAngle(void) const;      // degrés, angle de coupe par rapport à la perpendiculaire
    double outerLength(void) const;     // longueur du grand côté du trapèze
    double innerLength(void) const;     // longueur du petit côté du trapèze
    double boardWidth(void) const;      // largeur de planche nécessaire
    double boardLength(void) const;     // longueur de planche pour l'anneau (coupes alternées, sans trait de scie)
};
//-----------------------------------------------------------------------------------------------
#endif // CCOUCHE_H
//-----------------------------------------------------------------------------------------------
