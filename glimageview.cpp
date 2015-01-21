#include "glimageview.h"
#include <QMouseEvent>
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>
#include <QMutex>
#include "geometryengine.h"


GLImageView::GLImageView(QWidget *parent) :
    QOpenGLWidget(parent),
    geometries(nullptr),
    texture(nullptr),
    frameBuffer(nullptr),
    angularSpeed(0.),
    zTranslate(-5.0)
{

}

GLImageView::~GLImageView()
{
    makeCurrent();
    delete texture;
    delete geometries;
    doneCurrent();
}

void GLImageView::updateTexture(const QImage &img)
{
    localBuffer = img;

    if (!frameBuffer) {
        frameBuffer = new QOpenGLFramebufferObject(localBuffer.size());
    }

    //Start drawing on framebuffer.
    frameBuffer->bind();
    //Save current viewport setting.
    glPushAttrib(GL_VIEWPORT_BIT);
    //Set viewport
    glViewport(0, 0, localBuffer.width(), localBuffer.height());
    //Use texure of bound framebuffer.
    glBindTexture(GL_TEXTURE_2D, frameBuffer->texture());
    //Upload image to framebuffer(its texture buffer).
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 localBuffer.width(), localBuffer.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, localBuffer.bits()
                 );
    //Unbind texture.
    glBindTexture(GL_TEXTURE_2D, 0);
    //Unbind framebuffer.
    frameBuffer->release();
    //Load saved viewport setting.
    glPopAttrib();
}

void GLImageView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        mousePressed = true;
        mousePressPosition = QVector2D(e->localPos());
    } else if (e->button() == Qt::RightButton) {
        angularSpeed = 0;
        rotation = QQuaternion();
        zTranslate = -5.0;
    }
}

void GLImageView::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        // Mouse release position - mouse press position
        QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;

        // Rotation axis is perpendicular to the mouse position difference
        // vector
        QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();

        // Accelerate angular speed relative to the length of the mouse sweep
        qreal acc = diff.length() / static_cast<qreal>(width() + height());
        // Calculate new rotation axis as weighted sum
        rotationAxis = (rotationAxis * angularSpeed + n * acc).normalized();

        // Increase angular speed
        angularSpeed += acc;
        mousePressed = false;
    }
}

void GLImageView::timerEvent(QTimerEvent * /*e*/)
{
    if (mousePressed) {
        update();
        return;
    }
    // Decrease angular speed (friction)
    angularSpeed *= 0.99;

    // Stop rotation when speed goes below threshold
    if (angularSpeed < 0.01) {
        angularSpeed = 0.0;
    } else {
        // Update rotation
        rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;
    }

    update();
}

void GLImageView::wheelEvent(QWheelEvent *event)
{
    if (zTranslate >= -3.0) {
        return event->ignore();
    }
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    if (!numPixels.isNull()) {
        zTranslate += numPixels.y();
    } else {
        zTranslate += numDegrees.y() / 150.;
    }
    event->accept();
}

void GLImageView::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    initShaders();
    initTextures();

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    geometries = new GeometryEngine;

    // Use QBasicTimer because its faster than QTimer
    timer.start(12, this);

}

void GLImageView::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 1.0, zFar = 7.0, fov = 45.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);

}

void GLImageView::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (frameBuffer) {
        glBindTexture(GL_TEXTURE_2D, frameBuffer->texture());
    }
    // Calculate model view transformation
    QMatrix4x4 matrix;
    matrix.translate(0.0, 0.0, zTranslate);
    matrix.rotate(rotation);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix);

    // Use texture unit 0 which contains cube.png
    //program.setUniformValue("texture", 0);

    // Draw cube geometry
    geometries->drawCubeGeometry(&program);

}

void GLImageView::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Resources/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Resources/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();

}

void GLImageView::initTextures()
{
    /*
    // Load cube.png image
    texture = new QOpenGLTexture(QImage(":/Resources/cube.png").mirrored());

    // Set nearest filtering mode for texture minification
    texture->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    texture->setWrapMode(QOpenGLTexture::Repeat);
    */
}
