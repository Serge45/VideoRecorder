#ifndef GLIMAGEVIEW_H
#define GLIMAGEVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QBasicTimer>
#include <QOpenGLShaderProgram>

class GeometryEngine;
class QOpenGLTexture;
class QOpenGLFramebufferObject;

class GLImageView : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit GLImageView(QWidget *parent = 0);
    ~GLImageView();

public slots:
    void updateTexture(const QImage &img);

protected:
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    void initShaders();
    void initTextures();

private:
    QBasicTimer timer;
    QOpenGLShaderProgram program;
    GeometryEngine *geometries;
    QOpenGLTexture *texture;
    QOpenGLFramebufferObject *frameBuffer;
    QMatrix4x4 projection;
    QVector2D mousePressPosition;
    QVector3D rotationAxis;
    qreal angularSpeed;
    QQuaternion rotation;
    bool mousePressed;
    QImage localBuffer;
    qreal zTranslate;
};

#endif // GLIMAGEVIEW_H
