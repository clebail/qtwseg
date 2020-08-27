//-----------------------------------------------------------------------------------------------
#ifndef CCOUCHE_H
#define CCOUCHE_H
//-----------------------------------------------------------------------------------------------
#include <QString>
#include "common.h"
//-----------------------------------------------------------------------------------------------
class CCouche
{
private:
    int nbSegment;
    int nbFace;
    float height;
    float width;
    float diameter;
public:
    typedef struct _SFace {
        float coords[NBSOMMET][DIMENSION];
        QString name;
    }SFace;

    CCouche(int nbSegment, float height, float width, float diameter);
    ~CCouche(void);
    float getHeight(void);
    SFace * getFaces(int numSegment);
private:
    SFace *faces;
};
//-----------------------------------------------------------------------------------------------
#endif // CCOUCHE_H
//-----------------------------------------------------------------------------------------------
