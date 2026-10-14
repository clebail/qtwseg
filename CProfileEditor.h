//-----------------------------------------------------------------------------------------------
#ifndef CPROFILEEDITOR_H
#define CPROFILEEDITOR_H
//-----------------------------------------------------------------------------------------------
#include <QWidget>
#include <QTransform>
#include "CVase.h"
//-----------------------------------------------------------------------------------------------
// Éditeur 2D du profil : demi-coupe du vase (r horizontal, y vertical), symétrisée.
//  - glisser un point de contrôle pour le déplacer ;
//  - double-clic pour insérer un point sur le polygone de contrôle ;
//  - clic droit sur un point pour le supprimer.
class CProfileEditor : public QWidget
{
    Q_OBJECT
public:
    explicit CProfileEditor(CVase *vase, QWidget *parent = nullptr);

    // Rendu partagé avec l'export PDF. unit = taille d'un pixel écran dans le périphérique cible.
    static QTransform fitTransform(const CVase *vase, const QRectF &target);
    static void render(QPainter &painter, const QRectF &target, const QTransform &t, const CVase *vase, double unit, int hoverIndex = -1, bool showControls = true);

signals:
    void cursorMoved(const QPointF &world);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    CVase *vase;
    QTransform transform;
    int dragIndex;
    int hoverIndex;

    int hitTest(const QPointF &pos) const;
    QPointF toWorld(const QPointF &pos) const;
};
//-----------------------------------------------------------------------------------------------
#endif // CPROFILEEDITOR_H
//-----------------------------------------------------------------------------------------------
