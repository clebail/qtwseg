//-----------------------------------------------------------------------------------------------
#ifndef C3DVIEW_H
#define C3DVIEW_H
//-----------------------------------------------------------------------------------------------
#include <QGLWidget>
#include <QOpenGLTexture>
#include <QTimer>
#include "CCouche.h"
//-----------------------------------------------------------------------------------------------
class C3dView : public QGLWidget {
    Q_OBJECT
public:
    explicit C3dView(QWidget *parent = nullptr);
    ~C3dView(void);
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
protected:
    virtual void wheelEvent(QWheelEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);
private:
    float scale;
    QTimer *timer;
    float roty, rotx, rotz;
    QPoint lastPos;
    GLuint texture;

    void draw(void);
    void loadTexture( QString textureName, GLuint *texture);
    void renderRotate(void);
private slots:
    void onTimerTimeout(void);
};
//-----------------------------------------------------------------------------------------------
#endif // C3DVIEW_H
//-----------------------------------------------------------------------------------------------
