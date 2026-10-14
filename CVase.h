//-----------------------------------------------------------------------------------------------
#ifndef CVASE_H
#define CVASE_H
//-----------------------------------------------------------------------------------------------
#include <QObject>
#include <QPointF>
#include <QVector>
#include <QMap>
#include "CCouche.h"
//-----------------------------------------------------------------------------------------------
// Paramètres de fabrication. Unités : mm.
struct SParams {
    double layerHeight = 19.0;          // épaisseur des planches = hauteur d'une couche
    double wallThickness = 6.0;         // épaisseur de paroi finale après tournage
    double turningMargin = 3.0;         // surépaisseur de tournage, de chaque côté de la paroi
    double bottomThickness = 10.0;      // épaisseur du fond (couches en disque plein)
    int defaultSegments = 12;
};
//-----------------------------------------------------------------------------------------------
// Modèle du vase : profil extérieur (B-spline dans le plan r/y), profil intérieur décalé
// de l'épaisseur de paroi, et découpage en couches.
class CVase : public QObject
{
    Q_OBJECT
public:
    explicit CVase(QObject *parent = nullptr);

    const QVector<QPointF> & controlPoints(void) const;
    void setControlPoints(const QVector<QPointF> &points);
    void moveControlPoint(int index, const QPointF &pos);
    void insertControlPoint(int index, const QPointF &pos);
    bool removeControlPoint(int index);

    const SParams & params(void) const;
    void setParams(const SParams &params);

    int layerSegments(int layer) const;
    void setLayerSegments(int layer, int nbSegments);

    const QVector<QPointF> & outerProfile(void) const;
    const QVector<QPointF> & innerProfile(void) const;
    const QVector<CCouche> & couches(void) const;

    bool save(const QString &fileName, QString *error = nullptr) const;
    bool load(const QString &fileName, QString *error = nullptr);
    void reset(void);

signals:
    void changed(void);

private:
    QVector<QPointF> ctrl;
    SParams prm;
    QMap<int, int> segmentsOverride;

    QVector<QPointF> outer;
    QVector<QPointF> inner;
    QVector<CCouche> layers;

    void recompute(void);
    static bool rangeExtreme(const QVector<QPointF> &curve, double y0, double y1, bool wantMax, double *r);
};
//-----------------------------------------------------------------------------------------------
#endif // CVASE_H
//-----------------------------------------------------------------------------------------------
