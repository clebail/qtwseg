//-----------------------------------------------------------------------------------------------
#include <cmath>
#include <QWheelEvent>
#include <QMouseEvent>
#include "C3dView.h"
#include "common.h"
//-----------------------------------------------------------------------------------------------
#define VIEW_SIZE               4.0         // taille de la plus grande dimension du vase à l'écran (unités GL)
#define DISC_SLICES             64
//-----------------------------------------------------------------------------------------------
C3dView::C3dView(CVase *vase, QWidget *parent) : QOpenGLWidget(parent) {
    this->vase = vase;
    texture = 0;
    scale = 1.0;
    rotz = 0.0;
    roty = 0.0;
    rotx = 20.0;

    setMinimumSize(300, 300);
    connect(vase, &CVase::changed, this, qOverload<>(&QWidget::update));
}
//-----------------------------------------------------------------------------------------------
C3dView::~C3dView(void) {
    if(texture != 0) {
        makeCurrent();
        glDeleteTextures(1, &texture);
        doneCurrent();
    }
}
//-----------------------------------------------------------------------------------------------
void C3dView::initializeGL() {
    GLfloat mat_specular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat mat_shininess[] = { 20.0f };
    GLfloat ambient[] = { 0.35f, 0.35f, 0.35f, 1.0f };

    initializeOpenGLFunctions();

    glClearColor(0.15f, 0.15f, 0.17f, 1.0f);
    glShadeModel (GL_SMOOTH);

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    loadTexture(":/textures/boisClair.jpg", &texture);
}
//-----------------------------------------------------------------------------------------------
void C3dView::resizeGL(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLdouble x = static_cast<GLdouble>(width) / (height > 0 ? height : 1);
    glFrustum(-x, x, -1.0, 1.0, 4.0, 15.0);
    glMatrixMode(GL_MODELVIEW);
}
//-----------------------------------------------------------------------------------------------
void C3dView::paintGL() {
    GLfloat light_position[] = { 5.0f, 5.0f, 5.0f, 0.0f };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glTranslatef(0, 0, -11.0);

    glRotatef(rotx, 1.0, 0.0, 0.0);
    glRotatef(roty, 0.0, 1.0, 0.0);
    glRotatef(rotz, 0.0, 0.0, 1.0);

    glScalef(scale, scale, scale);

    draw();
}
//-----------------------------------------------------------------------------------------------
void C3dView::wheelEvent(QWheelEvent * event) {
    event->angleDelta().y() > 0 ? scale += scale*0.1f : scale -= scale*0.1f;
    update();
}
//-----------------------------------------------------------------------------------------------
void C3dView::mousePressEvent(QMouseEvent * event) {
    lastPos = event->position().toPoint();
}
//-----------------------------------------------------------------------------------------------
void C3dView::mouseMoveEvent(QMouseEvent * event) {
    QPoint pos = event->position().toPoint();
    int dx = pos.x() - lastPos.x();
    int dy = pos.y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        rotx += dy;
        roty += dx;
    } else if (event->buttons() & Qt::RightButton) {
        rotx += dy;
        rotz += dx;
    }

    update();

    lastPos = pos;
}
//-----------------------------------------------------------------------------------------------
void C3dView::draw(void) {
    const QVector<CCouche> &couches = vase->couches();
    if(couches.isEmpty()) {
        return;
    }

    // Mise à l'échelle mm -> unités GL, vase centré verticalement
    double rMax = 0.0;
    for(const CCouche &c : couches) {
        rMax = qMax(rMax, c.rOut);
    }
    double yLo = couches.first().y0;
    double yHi = couches.last().y1;
    double s = VIEW_SIZE / qMax(yHi - yLo, 2.0 * rMax);

    glScaled(s, s, s);
    glTranslated(0.0, -(yLo + yHi) / 2.0, 0.0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    for(const CCouche &c : couches) {
        c.isDisc() ? drawDisc(c) : drawRing(c);
    }

    glDisable(GL_TEXTURE_2D);
}
//-----------------------------------------------------------------------------------------------
void C3dView::quad(const double n[3], const double v[4][3]) {
    static const double tex[4][2] = { {0, 1}, {0, 0}, {1, 0}, {1, 1} };

    glNormal3dv(n);
    for(int i=0;i<4;i++) {
        glTexCoord2dv(tex[i]);
        glVertex3dv(v[i]);
    }
}
//-----------------------------------------------------------------------------------------------
void C3dView::drawRing(const CCouche &couche) {
    int n = couche.nbSegments;
    double a = couche.halfAngle();
    double rOutCorner = couche.rOut / std::cos(a);
    double rIn = couche.rIn;
    double y0 = couche.y0;
    double y1 = couche.y1;
    double offset = (couche.index % 2) ? a : 0.0;

    glBegin(GL_QUADS);
    for(int k=0;k<n;k++) {
        double tc = 2.0 * PI * k / n + offset;
        double t0 = tc - a;
        double t1 = tc + a;
        double c0 = std::cos(t0), s0 = std::sin(t0);
        double c1 = std::cos(t1), s1 = std::sin(t1);

        // Segments alternativement clairs et foncés pour voir les joints
        k % 2 ? glColor3d(0.75, 0.6, 0.45) : glColor3d(1.0, 1.0, 1.0);

        double oo[2][2] = { {c0 * rOutCorner, s0 * rOutCorner}, {c1 * rOutCorner, s1 * rOutCorner} };
        double ii[2][2] = { {c0 * rIn, s0 * rIn}, {c1 * rIn, s1 * rIn} };

        double nOut[3] = { std::cos(tc), 0.0, std::sin(tc) };
        double vOut[4][3] = { {oo[0][0], y1, oo[0][1]}, {oo[0][0], y0, oo[0][1]}, {oo[1][0], y0, oo[1][1]}, {oo[1][0], y1, oo[1][1]} };
        quad(nOut, vOut);

        double nIn[3] = { -std::cos(tc), 0.0, -std::sin(tc) };
        double vIn[4][3] = { {ii[1][0], y1, ii[1][1]}, {ii[1][0], y0, ii[1][1]}, {ii[0][0], y0, ii[0][1]}, {ii[0][0], y1, ii[0][1]} };
        quad(nIn, vIn);

        double nTop[3] = { 0.0, 1.0, 0.0 };
        double vTop[4][3] = { {ii[0][0], y1, ii[0][1]}, {oo[0][0], y1, oo[0][1]}, {oo[1][0], y1, oo[1][1]}, {ii[1][0], y1, ii[1][1]} };
        quad(nTop, vTop);

        double nBot[3] = { 0.0, -1.0, 0.0 };
        double vBot[4][3] = { {ii[1][0], y0, ii[1][1]}, {oo[1][0], y0, oo[1][1]}, {oo[0][0], y0, oo[0][1]}, {ii[0][0], y0, ii[0][1]} };
        quad(nBot, vBot);
    }
    glEnd();
}
//-----------------------------------------------------------------------------------------------
void C3dView::drawDisc(const CCouche &couche) {
    double r = couche.rOut;
    double y0 = couche.y0;
    double y1 = couche.y1;

    glColor3d(0.85, 0.75, 0.6);

    glBegin(GL_QUADS);
    for(int k=0;k<DISC_SLICES;k++) {
        double t0 = 2.0 * PI * k / DISC_SLICES;
        double t1 = 2.0 * PI * (k + 1) / DISC_SLICES;
        double n[3] = { std::cos((t0 + t1) / 2.0), 0.0, std::sin((t0 + t1) / 2.0) };
        double v[4][3] = { {std::cos(t0) * r, y1, std::sin(t0) * r}, {std::cos(t0) * r, y0, std::sin(t0) * r},
                           {std::cos(t1) * r, y0, std::sin(t1) * r}, {std::cos(t1) * r, y1, std::sin(t1) * r} };
        quad(n, v);
    }
    glEnd();

    for(int side=0;side<2;side++) {
        double y = side ? y1 : y0;
        glNormal3d(0.0, side ? 1.0 : -1.0, 0.0);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2d(0.5, 0.5);
        glVertex3d(0.0, y, 0.0);
        for(int k=0;k<=DISC_SLICES;k++) {
            double t = 2.0 * PI * k / DISC_SLICES;
            glTexCoord2d(0.5 + 0.5 * std::cos(t), 0.5 + 0.5 * std::sin(t));
            glVertex3d(std::cos(t) * r, y, std::sin(t) * r);
        }
        glEnd();
    }
}
//-----------------------------------------------------------------------------------------------
void C3dView::loadTexture(QString textureName, GLuint *texture) {
    QImage tex = QImage(textureName).convertToFormat(QImage::Format_RGBA8888).flipped(Qt::Vertical);

    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // MODULATE : la texture est combinée à l'éclairage et à la teinte des segments
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width(), tex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.constBits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glDisable(GL_TEXTURE_2D);
}
//-----------------------------------------------------------------------------------------------
