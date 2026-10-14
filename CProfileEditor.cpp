//-----------------------------------------------------------------------------------------------
#include <cmath>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include "CProfileEditor.h"
//-----------------------------------------------------------------------------------------------
#define HIT_RADIUS              8.0
//-----------------------------------------------------------------------------------------------
CProfileEditor::CProfileEditor(CVase *vase, QWidget *parent) : QWidget(parent) {
    this->vase = vase;
    dragIndex = -1;
    hoverIndex = -1;

    setMouseTracking(true);
    setMinimumSize(300, 300);
    setToolTip(tr("Glisser : déplacer un point\nDouble-clic : ajouter un point\nClic droit : supprimer un point"));

    connect(vase, &CVase::changed, this, qOverload<>(&QWidget::update));
}
//-----------------------------------------------------------------------------------------------
QTransform CProfileEditor::fitTransform(const CVase *vase, const QRectF &target) {
    double rMax = 10.0;
    double yLo = 0.0;
    double yHi = 10.0;

    for(const QPointF &p : vase->controlPoints()) {
        rMax = qMax(rMax, p.x());
        yHi = qMax(yHi, p.y());
    }
    for(const QPointF &p : vase->outerProfile()) {
        rMax = qMax(rMax, p.x());
    }
    for(const CCouche &c : vase->couches()) {
        rMax = qMax(rMax, c.rOut);
        yHi = qMax(yHi, c.y1);
    }

    QRectF world(-rMax * 1.15, yLo, 2.0 * rMax * 1.15, yHi - yLo);
    world.adjust(0.0, -0.05 * world.height(), 0.0, 0.05 * world.height());

    double s = qMin(target.width() / world.width(), target.height() / world.height());

    QTransform t;
    t.translate(target.center().x(), target.center().y());
    t.scale(s, -s);
    t.translate(-world.center().x(), -world.center().y());
    return t;
}
//-----------------------------------------------------------------------------------------------
void CProfileEditor::render(QPainter &painter, const QRectF &target, const QTransform &t, const CVase *vase, double unit, int hoverIndex, bool showControls) {
    painter.save();
    painter.setClipRect(target);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF world = t.inverted().mapRect(target);

    // Grille : 10 mm, et 50 mm plus marquée
    for(int step : {10, 50}) {
        painter.setPen(QPen(step == 10 ? QColor(235, 235, 235) : QColor(205, 205, 205), unit));
        for(double x = std::floor(world.left() / step) * step; x <= world.right(); x += step) {
            painter.drawLine(t.map(QPointF(x, world.top())), t.map(QPointF(x, world.bottom())));
        }
        for(double y = std::floor(world.top() / step) * step; y <= world.bottom(); y += step) {
            painter.drawLine(t.map(QPointF(world.left(), y)), t.map(QPointF(world.right(), y)));
        }
    }

    // Axe de révolution
    painter.setPen(QPen(QColor(120, 120, 120), unit, Qt::DashDotLine));
    painter.drawLine(t.map(QPointF(0.0, world.top())), t.map(QPointF(0.0, world.bottom())));

    // Couches : section de la matière au milieu d'un segment
    QFont font = painter.font();
    font.setPixelSize(qRound(10 * unit));
    painter.setFont(font);
    for(const CCouche &c : vase->couches()) {
        QColor fill = c.index % 2 ? QColor(222, 184, 135, 150) : QColor(205, 160, 110, 150);
        painter.setPen(QPen(QColor(139, 90, 43), unit));
        painter.setBrush(fill);
        if(c.isDisc()) {
            painter.drawPolygon(t.map(QPolygonF(QRectF(-c.rOut, c.y0, 2.0 * c.rOut, c.y1 - c.y0))));
        } else {
            double rIn = c.rIn * std::cos(c.halfAngle());
            painter.drawPolygon(t.map(QPolygonF(QRectF(rIn, c.y0, c.rOut - rIn, c.y1 - c.y0))));
            painter.drawPolygon(t.map(QPolygonF(QRectF(-c.rOut, c.y0, c.rOut - rIn, c.y1 - c.y0))));
        }

        QPointF label = t.map(QPointF(-c.rOut, (c.y0 + c.y1) / 2.0));
        QRectF labelRect(label.x() - 40 * unit, label.y() - 8 * unit, 34 * unit, 16 * unit);
        painter.setPen(QColor(90, 60, 30));
        painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, QString::number(c.index + 1));
    }
    painter.setBrush(Qt::NoBrush);

    // Profils extérieur et intérieur, des deux côtés de l'axe
    QPolygonF outer(vase->outerProfile());
    QPolygonF inner(vase->innerProfile());
    QTransform mirror = QTransform::fromScale(-1.0, 1.0) * t;

    painter.setPen(QPen(QColor(30, 30, 30), 2.0 * unit));
    painter.drawPolyline(t.map(outer));
    painter.drawPolyline(mirror.map(outer));

    painter.setPen(QPen(QColor(30, 30, 30), unit, Qt::DashLine));
    painter.drawPolyline(t.map(inner));
    painter.drawPolyline(mirror.map(inner));

    if(!showControls) {
        painter.restore();
        return;
    }

    // Polygone et points de contrôle
    QPolygonF ctrl(vase->controlPoints());
    painter.setPen(QPen(QColor(0, 120, 215, 140), unit, Qt::DotLine));
    painter.drawPolyline(t.map(ctrl));

    for(int i=0;i<ctrl.size();i++) {
        bool hover = i == hoverIndex;
        painter.setPen(QPen(QColor(0, 90, 180), unit));
        painter.setBrush(hover ? QColor(255, 140, 0) : QColor(0, 120, 215));
        double radius = (hover ? 6.0 : 4.5) * unit;
        painter.drawEllipse(t.map(ctrl[i]), radius, radius);
    }

    painter.restore();
}
//-----------------------------------------------------------------------------------------------
void CProfileEditor::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    // Pendant un déplacement, le cadrage est figé pour que le point reste sous la souris
    if(dragIndex < 0) {
        transform = fitTransform(vase, QRectF(rect()).adjusted(10, 10, -10, -10));
    }

    render(painter, rect(), transform, vase, 1.0, dragIndex >= 0 ? dragIndex : hoverIndex);
}
//-----------------------------------------------------------------------------------------------
int CProfileEditor::hitTest(const QPointF &pos) const {
    const QVector<QPointF> &ctrl = vase->controlPoints();
    int best = -1;
    double bestDist = HIT_RADIUS;

    for(int i=0;i<ctrl.size();i++) {
        QPointF d = transform.map(ctrl[i]) - pos;
        double dist = std::hypot(d.x(), d.y());
        if(dist <= bestDist) {
            bestDist = dist;
            best = i;
        }
    }
    return best;
}
//-----------------------------------------------------------------------------------------------
QPointF CProfileEditor::toWorld(const QPointF &pos) const {
    QPointF w = transform.inverted().map(pos);
    // Un clic sur la moitié gauche (miroir) agit sur le profil de droite
    return QPointF(std::abs(w.x()), w.y());
}
//-----------------------------------------------------------------------------------------------
void CProfileEditor::mousePressEvent(QMouseEvent *event) {
    int index = hitTest(event->position());

    if(event->button() == Qt::LeftButton) {
        dragIndex = index;
    } else if(event->button() == Qt::RightButton && index >= 0) {
        hoverIndex = -1;
        vase->removeControlPoint(index);
    }
}
//-----------------------------------------------------------------------------------------------
void CProfileEditor::mouseMoveEvent(QMouseEvent *event) {
    QPointF world = toWorld(event->position());
    emit cursorMoved(world);

    if(dragIndex >= 0) {
        vase->moveControlPoint(dragIndex, world);
    } else {
        int index = hitTest(event->position());
        if(index != hoverIndex) {
            hoverIndex = index;
            update();
        }
    }
}
//-----------------------------------------------------------------------------------------------
void CProfileEditor::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton && dragIndex >= 0) {
        dragIndex = -1;
        update();
    }
}
//-----------------------------------------------------------------------------------------------
void CProfileEditor::mouseDoubleClickEvent(QMouseEvent *event) {
    if(event->button() != Qt::LeftButton || hitTest(event->position()) >= 0) {
        return;
    }

    // Insertion sur l'arête du polygone de contrôle la plus proche
    QPointF p = toWorld(event->position());
    const QVector<QPointF> &ctrl = vase->controlPoints();
    int bestEdge = 0;
    double bestDist = -1.0;

    for(int i=0;i+1<ctrl.size();i++) {
        QPointF a = ctrl[i];
        QPointF ab = ctrl[i + 1] - a;
        double len2 = QPointF::dotProduct(ab, ab);
        double u = len2 > 0.0 ? qBound(0.0, QPointF::dotProduct(p - a, ab) / len2, 1.0) : 0.0;
        QPointF d = p - (a + u * ab);
        double dist = std::hypot(d.x(), d.y());
        if(bestDist < 0.0 || dist < bestDist) {
            bestDist = dist;
            bestEdge = i;
        }
    }

    vase->insertControlPoint(bestEdge + 1, p);
}
//-----------------------------------------------------------------------------------------------
void CProfileEditor::leaveEvent(QEvent *) {
    hoverIndex = -1;
    update();
}
//-----------------------------------------------------------------------------------------------
