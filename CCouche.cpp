//-----------------------------------------------------------------------------------------------
#include <QtDebug>
#include <math.h>
#include "CCouche.h"
//-----------------------------------------------------------------------------------------------
#define PI          3.14159f
//-----------------------------------------------------------------------------------------------
CCouche::CCouche(int nbSegment, float height, float width, float diameter) {
    int stepAngle = 360 / nbSegment;
    float stepY = height / 2;
    float r = diameter / 2;
    float rW = r - width;

    this->nbSegment = nbSegment;
    this->height = height;
    this->width = width;
    this->diameter = diameter;

    this->nbFace = nbSegment * NB_FACE_SEGMENT;

    faces = new SFace[this->nbFace];

    for(int idFace = 0, angle=0;angle<360;angle+=stepAngle, idFace++) {
        float angleR = static_cast<float>(angle)*PI/180.0f;
        float angleR2 = static_cast<float>(angle + stepAngle)*PI/180.0f;
        float c = cos(angleR);
        float s = sin(angleR);
        float c2 = cos(angleR2);
        float s2 = sin(angleR2);
        float x[] = {c * r, c2 * r, c * rW, c2 * rW};
        float z[] = {s * r, s2 * r, s * rW, s2 * rW};

        //devant
        faces[idFace].name = "devant";
        faces[idFace].coords[0][0] = x[0];
        faces[idFace].coords[0][1] = stepY;
        faces[idFace].coords[0][2] = z[0];

        faces[idFace].coords[1][0] = x[0];
        faces[idFace].coords[1][1] = -stepY;
        faces[idFace].coords[1][2] = z[0];

        faces[idFace].coords[2][0] = x[1];
        faces[idFace].coords[2][1] = -stepY;
        faces[idFace].coords[2][2] = z[1];

        faces[idFace].coords[3][0] = x[1];
        faces[idFace].coords[3][1] = stepY;
        faces[idFace].coords[3][2] = z[1];

        //derrière
        faces[++idFace].name = "derrière";
        faces[idFace].coords[0][0] = x[2];
        faces[idFace].coords[0][1] = stepY;
        faces[idFace].coords[0][2] = z[2];

        faces[idFace].coords[1][0] = x[2];
        faces[idFace].coords[1][1] = -stepY;
        faces[idFace].coords[1][2] = z[2];

        faces[idFace].coords[2][0] = x[3];
        faces[idFace].coords[2][1] = -stepY;
        faces[idFace].coords[2][2] = z[3];

        faces[idFace].coords[3][0] = x[3];
        faces[idFace].coords[3][1] = stepY;
        faces[idFace].coords[3][2] = z[3];

        //haut
        faces[++idFace].name = "haut";
        faces[idFace].coords[0][0] = x[2];
        faces[idFace].coords[0][1] = stepY;
        faces[idFace].coords[0][2] = z[2];

        faces[idFace].coords[1][0] = x[0];
        faces[idFace].coords[1][1] = stepY;
        faces[idFace].coords[1][2] = z[0];

        faces[idFace].coords[2][0] = x[1];
        faces[idFace].coords[2][1] = stepY;
        faces[idFace].coords[2][2] = z[1];

        faces[idFace].coords[3][0] = x[3];
        faces[idFace].coords[3][1] = stepY;
        faces[idFace].coords[3][2] = z[3];

        //bas
        faces[++idFace].name = "bas";
        faces[idFace].coords[0][0] = x[2];
        faces[idFace].coords[0][1] = -stepY;
        faces[idFace].coords[0][2] = z[2];

        faces[idFace].coords[1][0] = x[0];
        faces[idFace].coords[1][1] = -stepY;
        faces[idFace].coords[1][2] = z[0];

        faces[idFace].coords[2][0] = x[1];
        faces[idFace].coords[2][1] = -stepY;
        faces[idFace].coords[2][2] = z[1];

        faces[idFace].coords[3][0] = x[3];
        faces[idFace].coords[3][1] = -stepY;
        faces[idFace].coords[3][2] = z[3];
    }
}
//-----------------------------------------------------------------------------------------------
CCouche::~CCouche(void) {
    delete[] faces;
}
//-----------------------------------------------------------------------------------------------
float CCouche::getHeight(void) {
    return height;
}
//-----------------------------------------------------------------------------------------------
CCouche::SFace * CCouche::getFaces(int numSegment) {
    return &faces[numSegment * NB_FACE_SEGMENT];
}
//-----------------------------------------------------------------------------------------------
