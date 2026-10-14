//-----------------------------------------------------------------------------------------------
#include "CBSpline.h"
//-----------------------------------------------------------------------------------------------
QVector<QPointF> CBSpline::sample(const QVector<QPointF> &ctrl, int nbSamples) {
    QVector<QPointF> result;

    if(ctrl.size() < 2 || nbSamples < 2) {
        return ctrl;
    }

    int n = ctrl.size() - 1;
    int degree = qMin(3, n);

    // Vecteur nodal : degree+1 zéros, noeuds intérieurs uniformes, degree+1 uns
    QVector<double> knots(n + degree + 2);
    for(int i=0;i<=degree;i++) {
        knots[i] = 0.0;
        knots[n + 1 + i] = 1.0;
    }
    for(int j=1;j<=n-degree;j++) {
        knots[degree + j] = static_cast<double>(j) / (n - degree + 1);
    }

    result.reserve(nbSamples);
    for(int i=0;i<nbSamples;i++) {
        double t = static_cast<double>(i) / (nbSamples - 1);
        result.append(deBoor(ctrl, knots, degree, t));
    }

    return result;
}
//-----------------------------------------------------------------------------------------------
QPointF CBSpline::deBoor(const QVector<QPointF> &ctrl, const QVector<double> &knots, int degree, double t) {
    int n = ctrl.size() - 1;

    // Intervalle k tel que knots[k] <= t < knots[k+1]
    int k = degree;
    while(k < n && t >= knots[k + 1]) {
        k++;
    }

    QVector<QPointF> d(degree + 1);
    for(int j=0;j<=degree;j++) {
        d[j] = ctrl[j + k - degree];
    }

    for(int r=1;r<=degree;r++) {
        for(int j=degree;j>=r;j--) {
            double denom = knots[j + 1 + k - r] - knots[j + k - degree];
            double alpha = denom > 0.0 ? (t - knots[j + k - degree]) / denom : 0.0;
            d[j] = (1.0 - alpha) * d[j - 1] + alpha * d[j];
        }
    }

    return d[degree];
}
//-----------------------------------------------------------------------------------------------
