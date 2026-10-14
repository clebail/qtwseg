//-----------------------------------------------------------------------------------------------
#ifndef C3DVIEW_H
#define C3DVIEW_H
//-----------------------------------------------------------------------------------------------
#include <QOpenGLWidget>
#include <QOpenGLFunctions_2_1>
#include "CVase.h"
//-----------------------------------------------------------------------------------------------
// Aperçu 3D des anneaux segmentés, montés en quinconce (une couche sur deux décalée d'un demi-segment).
class C3dView : public QOpenGLWidget, protected QOpenGLFunctions_2_1 {
    Q_OBJECT
public:
    explicit C3dView(CVase *vase, QWidget *parent = nullptr);
    ~C3dView(void);
protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;
    void wheelEvent(QWheelEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
private:
    CVase *vase;
    float scale;
    float roty, rotx, rotz;
    QPoint lastPos;
    GLuint texture;

    void draw(void);
    void drawRing(const CCouche &couche);
    void drawDisc(const CCouche &couche);
    void quad(const double n[3], const double v[4][3]);
    void loadTexture( QString textureName, GLuint *texture);
};
//-----------------------------------------------------------------------------------------------
#endif // C3DVIEW_H
//-----------------------------------------------------------------------------------------------
