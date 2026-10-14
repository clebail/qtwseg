//-----------------------------------------------------------------------------------------------
#include <algorithm>
#include <cmath>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "CVase.h"
#include "CBSpline.h"
#include "common.h"
//-----------------------------------------------------------------------------------------------
CVase::CVase(QObject *parent) : QObject(parent) {
    reset();
}
//-----------------------------------------------------------------------------------------------
void CVase::reset(void) {
    ctrl = {
        QPointF(40.0, 0.0),
        QPointF(70.0, 25.0),
        QPointF(85.0, 80.0),
        QPointF(40.0, 140.0),
        QPointF(32.0, 185.0),
        QPointF(48.0, 210.0)
    };
    prm = SParams();
    segmentsOverride.clear();
    recompute();
}
//-----------------------------------------------------------------------------------------------
const QVector<QPointF> & CVase::controlPoints(void) const {
    return ctrl;
}
//-----------------------------------------------------------------------------------------------
void CVase::setControlPoints(const QVector<QPointF> &points) {
    ctrl = points;
    recompute();
}
//-----------------------------------------------------------------------------------------------
void CVase::moveControlPoint(int index, const QPointF &pos) {
    if(index < 0 || index >= ctrl.size()) {
        return;
    }
    ctrl[index] = QPointF(qMax(0.0, pos.x()), qMax(0.0, pos.y()));
    recompute();
}
//-----------------------------------------------------------------------------------------------
void CVase::insertControlPoint(int index, const QPointF &pos) {
    index = qBound(0, index, static_cast<int>(ctrl.size()));
    ctrl.insert(index, QPointF(qMax(0.0, pos.x()), qMax(0.0, pos.y())));
    recompute();
}
//-----------------------------------------------------------------------------------------------
bool CVase::removeControlPoint(int index) {
    if(ctrl.size() <= 3 || index < 0 || index >= ctrl.size()) {
        return false;
    }
    ctrl.remove(index);
    recompute();
    return true;
}
//-----------------------------------------------------------------------------------------------
const SParams & CVase::params(void) const {
    return prm;
}
//-----------------------------------------------------------------------------------------------
void CVase::setParams(const SParams &params) {
    prm = params;
    recompute();
}
//-----------------------------------------------------------------------------------------------
int CVase::layerSegments(int layer) const {
    return segmentsOverride.value(layer, prm.defaultSegments);
}
//-----------------------------------------------------------------------------------------------
void CVase::setLayerSegments(int layer, int nbSegments) {
    if(nbSegments == prm.defaultSegments) {
        segmentsOverride.remove(layer);
    } else {
        segmentsOverride[layer] = qBound(MIN_SEGMENTS, nbSegments, MAX_SEGMENTS);
    }
    recompute();
}
//-----------------------------------------------------------------------------------------------
const QVector<QPointF> & CVase::outerProfile(void) const {
    return outer;
}
//-----------------------------------------------------------------------------------------------
const QVector<QPointF> & CVase::innerProfile(void) const {
    return inner;
}
//-----------------------------------------------------------------------------------------------
const QVector<CCouche> & CVase::couches(void) const {
    return layers;
}
//-----------------------------------------------------------------------------------------------
void CVase::recompute(void) {
    outer = CBSpline::sample(ctrl, NB_SAMPLES_PROFIL);
    inner.clear();
    layers.clear();

    if(outer.size() < 2 || prm.layerHeight <= 0.0) {
        emit changed();
        return;
    }

    // Profil intérieur : décalage de l'épaisseur de paroi selon la normale, vers l'axe
    inner.reserve(outer.size());
    for(int i=0;i<outer.size();i++) {
        QPointF a = outer[qMax(0, i - 1)];
        QPointF b = outer[qMin(static_cast<int>(outer.size()) - 1, i + 1)];
        double dr = b.x() - a.x();
        double dy = b.y() - a.y();
        double len = std::hypot(dr, dy);
        if(len <= 0.0) {
            continue;
        }
        // Pour une paroi montante (dy > 0), la normale (-dy, dr) pointe vers l'axe
        inner.append(QPointF(outer[i].x() - prm.wallThickness * dy / len,
                             outer[i].y() + prm.wallThickness * dr / len));
    }

    double yMin = outer.first().y();
    double yMax = outer.first().y();
    for(const QPointF &p : outer) {
        yMin = qMin(yMin, p.y());
        yMax = qMax(yMax, p.y());
    }
    double floorTop = yMin + prm.bottomThickness;

    // L'intérieur s'arrête sur le dessus du fond
    inner.erase(std::remove_if(inner.begin(), inner.end(), [floorTop](const QPointF &p) {
        return p.y() < floorTop;
    }), inner.end());

    int nbLayers = static_cast<int>(std::ceil((yMax - yMin) / prm.layerHeight - 1e-9));

    for(int i=0;i<nbLayers;i++) {
        CCouche couche;
        couche.index = i;
        couche.y0 = yMin + i * prm.layerHeight;
        couche.y1 = couche.y0 + prm.layerHeight;

        double yTop = qMin(couche.y1, yMax);
        double rOut = 0.0;
        if(!rangeExtreme(outer, couche.y0, yTop, true, &rOut)) {
            continue;
        }
        couche.rOut = rOut + prm.turningMargin;

        double rIn = 0.0;
        bool hasInner = rangeExtreme(inner, qMax(couche.y0, floorTop), yTop, false, &rIn);
        if(!hasInner) {
            // Pas de profil intérieur sur la couche (bord supérieur) : paroi droite
            rIn = rOut - prm.wallThickness;
        }
        couche.rIn = rIn - prm.turningMargin;

        if(couche.y0 < floorTop - 1e-9 || couche.rIn <= 0.0) {
            couche.nbSegments = 0;
            couche.rIn = 0.0;
        } else {
            couche.nbSegments = layerSegments(i);
        }

        layers.append(couche);
    }

    emit changed();
}
//-----------------------------------------------------------------------------------------------
// Rayon extrême (max ou min) atteint par la courbe sur la tranche [y0, y1].
// La courbe est traitée comme une polyligne, la tranche coupe les segments par interpolation.
bool CVase::rangeExtreme(const QVector<QPointF> &curve, double y0, double y1, bool wantMax, double *r) {
    bool found = false;

    if(y1 < y0) {
        return false;
    }

    for(int i=0;i+1<curve.size();i++) {
        QPointF a = curve[i];
        QPointF b = curve[i + 1];
        double lo = qMax(qMin(a.y(), b.y()), y0);
        double hi = qMin(qMax(a.y(), b.y()), y1);
        if(lo > hi) {
            continue;
        }

        double dy = b.y() - a.y();
        double ys[] = {lo, hi};
        for(double y : ys) {
            double t = std::abs(dy) > 1e-12 ? (y - a.y()) / dy : 0.0;
            double x = a.x() + t * (b.x() - a.x());
            if(!found || (wantMax ? x > *r : x < *r)) {
                *r = x;
                found = true;
            }
        }
    }

    return found;
}
//-----------------------------------------------------------------------------------------------
bool CVase::save(const QString &fileName, QString *error) const {
    QJsonObject params;
    params["layerHeight"] = prm.layerHeight;
    params["wallThickness"] = prm.wallThickness;
    params["turningMargin"] = prm.turningMargin;
    params["bottomThickness"] = prm.bottomThickness;
    params["defaultSegments"] = prm.defaultSegments;

    QJsonArray points;
    for(const QPointF &p : ctrl) {
        points.append(QJsonArray({p.x(), p.y()}));
    }

    QJsonObject segments;
    for(auto it = segmentsOverride.cbegin(); it != segmentsOverride.cend(); ++it) {
        segments[QString::number(it.key())] = it.value();
    }

    QJsonObject root;
    root["version"] = 1;
    root["params"] = params;
    root["points"] = points;
    root["segments"] = segments;

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if(error) {
            *error = file.errorString();
        }
        return false;
    }
    file.write(QJsonDocument(root).toJson());
    return true;
}
//-----------------------------------------------------------------------------------------------
bool CVase::load(const QString &fileName, QString *error) {
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
        if(error) {
            *error = file.errorString();
        }
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if(!doc.isObject()) {
        if(error) {
            *error = parseError.errorString();
        }
        return false;
    }

    QJsonObject root = doc.object();
    QVector<QPointF> points;
    for(const QJsonValue &v : root["points"].toArray()) {
        QJsonArray a = v.toArray();
        points.append(QPointF(a.at(0).toDouble(), a.at(1).toDouble()));
    }
    if(points.size() < 3) {
        if(error) {
            *error = tr("Le fichier ne contient pas de profil valide.");
        }
        return false;
    }

    SParams p;
    QJsonObject params = root["params"].toObject();
    p.layerHeight = params["layerHeight"].toDouble(p.layerHeight);
    p.wallThickness = params["wallThickness"].toDouble(p.wallThickness);
    p.turningMargin = params["turningMargin"].toDouble(p.turningMargin);
    p.bottomThickness = params["bottomThickness"].toDouble(p.bottomThickness);
    p.defaultSegments = params["defaultSegments"].toInt(p.defaultSegments);

    QMap<int, int> segments;
    QJsonObject seg = root["segments"].toObject();
    for(auto it = seg.constBegin(); it != seg.constEnd(); ++it) {
        segments[it.key().toInt()] = qBound(MIN_SEGMENTS, it.value().toInt(), MAX_SEGMENTS);
    }

    ctrl = points;
    prm = p;
    segmentsOverride = segments;
    recompute();
    return true;
}
//-----------------------------------------------------------------------------------------------
