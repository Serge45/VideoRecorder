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
    zTranslate(-5.0),
    textureOffsetX(0),
    textureOffsetY(0)
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
        auto imgSize = localBuffer.size();
        imgWHRatio = static_cast<qreal>(imgSize.width()) / imgSize.height();
        int longSide = std::max<int>(imgSize.width(), imgSize.height());
        frameBuffer = new QOpenGLFramebufferObject(QSize(longSide, longSide));
        textureOffsetX = (longSide - localBuffer.width()) / 2;
        textureOffsetY = (longSide - localBuffer.height()) / 2;
    }

    //Start drawing on framebuffer.
    frameBuffer->bind();
    //Save current viewport setting.
    glPushAttrib(GL_VIEWPORT_BIT);
    //Set viewport
    glViewport(0, 0, frameBuffer->width(), frameBuffer->height());
    //Use texure of bound framebuffer.
    glBindTexture(GL_TEXTURE_2D, frameBuffer->texture());
    //Upload image to framebuffer(its texture buffer).
    glTexSubImage2D(GL_TEXTURE_2D, 0, textureOffsetX, textureOffsetY,
                    localBuffer.width(), localBuffer.height(),
                    GL_RGB, GL_UNSIGNED_BYTE, localBuffer.bits()
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
        qreal acc = diff.length() /100.;
        // Calculate new rotation axis as weighted sum
        rotationAxis = (rotationAxis * angularSpeed + n * acc).normalized();

        // Increase angular speed
        angularSpeed += acc;
        mousePressed = false;
    }
}

void GLImageView::timerEvent(QTimerEvent * /*e*/)
{
    // Decrease angular speed (friction)
    if (!mousePressed) {
        angularSpeed *= 0.99;
    }

    // Stop rotation when speed goes below threshold
    if (angularSpeed < 0.01) {
        angularSpeed = 0.0;
    } else {
        // Update rotation

        if (!mousePressed) {
            rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;
        } else {
            rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed);
        }
    }

    update();
}

void GLImageView::wheelEvent(QWheelEvent *event)
{
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;
    qreal shift = 0;

    if (!numPixels.isNull()) {
        shift = numPixels.y();
    } else {
        shift = numDegrees.y() / 150.;
    }

    zTranslate += shift;

    if (zTranslate < -5) {
        zTranslate = -5;
    }

    if (zTranslate > -2) {
        zTranslate = -2;
    }
    event->accept();
}

void GLImageView::mouseMoveEvent(QMouseEvent *e)
{
    if (e->button() | Qt::LeftButton) {
        // Mouse release position - mouse press position
        QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;

        if (diff.length() < 2.f) {
            return;
        }

        // Rotation axis is perpendicular to the mouse position difference
        // vector
        QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();

        // Accelerate angular speed relative to the length of the mouse sweep
        qreal acc = diff.length() /100.;
        // Calculate new rotation axis as weighted sum
        rotationAxis = (rotationAxis + n * acc).normalized();

        // Increase angular speed
        angularSpeed = (diff.length());
    }
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
}
