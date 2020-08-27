//-----------------------------------------------------------------------------------------------
#include <QtDebug>
#include <QWheelEvent>
#include <QMouseEvent>
#include "C3dView.h"
#include "common.h"
//-----------------------------------------------------------------------------------------------
C3dView::C3dView(QWidget *parent) : QGLWidget(parent) {
    scale = 1.0;
    rotx = roty = rotz = 0.0;
    roty = 45.0;
    rotx = 30.0;
    timer = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
    timer->setInterval(400);
    //timer->start();
}
//-----------------------------------------------------------------------------------------------
C3dView::~C3dView(void) {
}
//-----------------------------------------------------------------------------------------------
void C3dView::initializeGL() {
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    GLfloat light_position[] = { 5.0, 5.0, 5.0, 0.0 };

    qglClearColor(Qt::black);
    glShadeModel (GL_SMOOTH);

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    loadTexture(":/textures/boisClair.jpg", &texture);
}
//-----------------------------------------------------------------------------------------------
void C3dView::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLdouble x = static_cast<GLdouble>(width / height);
    glFrustum(-x, x, -1.0, 1.0, 4.0, 15.0);
    glMatrixMode(GL_MODELVIEW);
}
//-----------------------------------------------------------------------------------------------
void C3dView::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -11.0);

    glRotatef(rotx, 1.0, 0.0, 0.0);
    glRotatef(roty, 0.0, 1.0, 0.0);
    glRotatef(rotz, 0.0, 0.0, 1.0);

    glScalef(scale, scale, scale);

    draw();
}
//-----------------------------------------------------------------------------------------------
void C3dView::wheelEvent(QWheelEvent * event) {
    event->delta() > 0 ? scale += scale*0.1f : scale -= scale*0.1f;
    updateGL();
}
//-----------------------------------------------------------------------------------------------
void C3dView::mousePressEvent(QMouseEvent * event) {
    lastPos = event->pos();
}
//-----------------------------------------------------------------------------------------------
void C3dView::mouseMoveEvent(QMouseEvent * event) {
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        rotx += 4 * dy;
        roty += 4 * dx;
    } else if (event->buttons() & Qt::RightButton) {
        rotx += 4 * dy;
        rotz += 4 * dx;
    }

    updateGL();

    lastPos = event->pos();
}
//-----------------------------------------------------------------------------------------------
void C3dView::draw(void) {
    float dx = 0.0f;
    float dy = 0.0f;
    float dz = 0.0f;
    float d=3.0f;

    for(dy=-0.8f;dy<0.8f;dy+=0.2f,d-=0.1f)
    {
        CCouche couche(20, 0.2f, 0.2f, d);

        for(int i=0;i<20;i++) {
            CCouche::SFace *faces = couche.getFaces(i);

            for(int f=0;f<NB_FACE_SEGMENT;f++) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, texture);

                glBegin(GL_QUADS);

                glTexCoord2d(0, 1);
                glVertex3f(faces[f].coords[0][0] + dx, faces[f].coords[0][1] + dy, faces[f].coords[0][2] + dz);
                glTexCoord2d(0, 0);
                glVertex3f(faces[f].coords[1][0] + dx, faces[f].coords[1][1] + dy, faces[f].coords[1][2] + dz);
                glTexCoord2d(1, 0);
                glVertex3f(faces[f].coords[2][0] + dx, faces[f].coords[2][1] + dy, faces[f].coords[2][2] + dz);
                glTexCoord2d(1, 1);
                glVertex3f(faces[f].coords[3][0] + dx, faces[f].coords[3][1] + dy, faces[f].coords[3][2] + dz);

                glEnd();
                glDisable(GL_TEXTURE_2D);
            }
        }
    }
}
//-----------------------------------------------------------------------------------------------
void C3dView::loadTexture(QString textureName, GLuint *texture) {
    QImage im(textureName);
    QImage tex = QGLWidget::convertToGLFormat(im);

    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width(), tex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glDisable(GL_TEXTURE_2D);
}
//-----------------------------------------------------------------------------------------------
void C3dView::onTimerTimeout(void) {
    rotx=static_cast<float>((static_cast<int>(rotx) + 5) % 360);
    //roty=static_cast<float>((static_cast<int>(roty) + 5) % 360);
    //rotz=static_cast<float>((static_cast<int>(rotz) + 5) % 360);

    updateGL();
}
//-----------------------------------------------------------------------------------------------

