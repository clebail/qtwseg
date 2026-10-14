//-----------------------------------------------------------------------------------------------
#include <cmath>
#include "CCouche.h"
#include "common.h"
//-----------------------------------------------------------------------------------------------
bool CCouche::isDisc(void) const {
    return nbSegments == 0;
}
//-----------------------------------------------------------------------------------------------
double CCouche::halfAngle(void) const {
    return isDisc() ? 0.0 : PI / nbSegments;
}
//-----------------------------------------------------------------------------------------------
double CCouche::miterAngle(void) const {
    return isDisc() ? 0.0 : 180.0 / nbSegments;
}
//-----------------------------------------------------------------------------------------------
double CCouche::outerLength(void) const {
    return isDisc() ? 2.0 * rOut : 2.0 * rOut * std::tan(halfAngle());
}
//-----------------------------------------------------------------------------------------------
double CCouche::innerLength(void) const {
    return isDisc() ? 0.0 : 2.0 * rIn * std::sin(halfAngle());
}
//-----------------------------------------------------------------------------------------------
double CCouche::boardWidth(void) const {
    return isDisc() ? 2.0 * rOut : rOut - rIn * std::cos(halfAngle());
}
//-----------------------------------------------------------------------------------------------
double CCouche::boardLength(void) const {
    return isDisc() ? 2.0 * rOut : nbSegments * (outerLength() + innerLength()) / 2.0;
}
//-----------------------------------------------------------------------------------------------
