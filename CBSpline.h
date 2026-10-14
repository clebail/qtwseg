//-----------------------------------------------------------------------------------------------
#ifndef CBSPLINE_H
#define CBSPLINE_H
//-----------------------------------------------------------------------------------------------
#include <QPointF>
#include <QVector>
//-----------------------------------------------------------------------------------------------
// B-spline uniforme "clampée" (passe par le premier et le dernier point de contrôle).
// Degré 3, réduit automatiquement s'il y a trop peu de points.
class CBSpline
{
public:
    static QVector<QPointF> sample(const QVector<QPointF> &ctrl, int nbSamples);
private:
    static QPointF deBoor(const QVector<QPointF> &ctrl, const QVector<double> &knots, int degree, double t);
};
//-----------------------------------------------------------------------------------------------
#endif // CBSPLINE_H
//-----------------------------------------------------------------------------------------------
