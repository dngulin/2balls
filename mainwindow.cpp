#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QtMath>
#include <QTime>
#include <QKeyEvent>
#include <QMessageBox>
#include <QFontDatabase>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(size());
    setFocusPolicy(Qt::ClickFocus);
    grabKeyboard();

    // Init controls
    leftPressed = false;
    rightPressed = false;

    // Setup brushes
    bgBrush.setColor(Qt::black);
    bgBrush.setStyle(Qt::SolidPattern);

    ballBrush.setColor(Qt::cyan);
    ballBrush.setStyle(Qt::SolidPattern);

    platformBrush.setColor(Qt::gray);
    platformBrush.setStyle(Qt::SolidPattern);

    brickBrush.setColor(Qt::blue);
    brickBrush.setStyle(Qt::SolidPattern);

    outlinePen.setColor(Qt::white);
    outlinePen.setWidth(2);

    keepoutPen.setColor(Qt::red);
    keepoutPen.setWidth(2);

    // Stup scene
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setBackgroundBrush(bgBrush);

    // Create border
    scene->addLine(0, 0, 200, 0, outlinePen);
    scene->addLine(200, 0, 200, 250, outlinePen);
    scene->addLine(0, 250, 0, 0, outlinePen);
    scene->addLine(200, 250, 0, 250, keepoutPen);
    scene->addRect(0, -15, 200, 10, outlinePen);

    // Create platform
    platform = new QGraphicsRectItem(0, 0, 30, 5);
    platform->setBrush(platformBrush);
    scene->addItem(platform);
    platform->moveBy(85, 230);

    // Create Ball
    ball = new QGraphicsEllipseItem(0, 0, 6, 6);
    ball->setBrush(ballBrush);
    scene->addItem(ball);

    resetBall();

    this->show();
    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);

    this->startTimer(dtimer);

    // Read the level
    QFile level("LEVEL");
    if (level.exists()) {
        if (level.open(QIODevice::ReadOnly)) {

            QByteArray levelData = level.readAll();
            QList<QByteArray> lines = levelData.split('\n');

            for (int y = 0; y < lines.count(); y++) {

                QByteArray line = lines[y];

                for (int x = 0; x < line.length(); x++) {
                    if (line[x] == '1') {

                        QGraphicsRectItem* brick = new QGraphicsRectItem(0, 0, 20, 10);
                        brick->setBrush(brickBrush);
                        brick->setPen(outlinePen);
                        brick->setData(0, 1);
                        scene->addItem(brick);
                        brick->moveBy(x * 20, y * 10);

                    }
                }

            }
        } else {
            QMessageBox::critical(this, "Ошибка!",  "Не удалось открыть файл-уровень.\n");
        }
    } else {
        QMessageBox::critical(this, "Ошибка!",  "Файл-уровень не существует.\n");
    }

    // Create message
    message = new QGraphicsTextItem();
    QFont messageFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    messageFont.setPointSizeF(10);
    message->setFont(messageFont);
    message->setDefaultTextColor(Qt::yellow);
    message->setTextWidth(180);
    message->setPlainText("Береги свои шары, их всего два!\nРазбей ими все блоки и ощути вкус победы!\n\nПРОБЕЛ = НАЧАТЬ ИГРУ");
    scene->addItem(message);
    message->setPos(20, 30);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent* e)
{
    // Move ball
    if (ballRunning) {

        ball->moveBy(dx, 0);
        if (ball->collidingItems().count() == 0) {

            ball->moveBy(0, dy);
            if (ball->collidingItems().count() != 0) {

                removeItemIfBrick();

                // Invert direction
                dy = dy * -1;
                ball->moveBy(0, dy);
                shiftVector(collisionDelta);

                if (ball->y() > platform->y()) resetBall();
            }

        } else {

            removeItemIfBrick();

            // Invert direction
            dx = dx * -1;
            ball->moveBy(dx, 0);
            shiftVector(collisionDelta);
        }

        shiftVector(flyDelta);
    }

    // Move platform
    if (rightPressed) {

        platform->moveBy(dplatf, 0);
        if (!ballRunning) ball->moveBy(dplatf, 0);

        // Pull back
        if (platform->collidingItems().count() != 0) {
            platform->moveBy(-dplatf, 0);
            if (!ballRunning) ball->moveBy(-dplatf, 0);
        }


    } else if (leftPressed) {

        if (!ballRunning) ball->moveBy(-dplatf, 0);
        platform->moveBy(-dplatf, 0);

        // Pull back
        if (platform->collidingItems().count() != 0) {
            platform->moveBy(dplatf, 0);
            if (!ballRunning) ball->moveBy(dplatf, 0);
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Right)
        rightPressed = true;
    else if (e->key() == Qt::Key_Left)
        leftPressed = true;
    else if (e->key() == Qt::Key_Space) {
        if (!ballRunning) ballRunning = true;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Right)
        rightPressed = false;
    else if (e->key() == Qt::Key_Left)
        leftPressed = false;
}

void MainWindow::shiftVector(qreal delta)
{
    // Go to polar coordiante system
    qreal r = qSqrt((dx*dx) + (dy*dy));
    qreal phi = qAtan2(dy,dx);

    // Calculate real delta
    qsrand(QTime::currentTime().msec());
    qreal mult = (qrand() % 101) - 100;
    delta = delta * mult / 100;
    delta = (delta / 180) * M_PI; // Go to radians

    // Rotate vector
    phi += delta;

    // Return to decart coordinates
    dx = r * qCos(phi);
    dy = r * qSin(phi);

}

void MainWindow::resetBall()
{
    ballRunning = false;

    ball->setX(platform->x()
               + (platform->boundingRect().width() / 2)
               - (ball->boundingRect().width() / 2));

    ball->setY(platform->y() - ball->boundingRect().height());

    // Initial direction
    dx = 0;
    dy = -dball;
    shiftVector(launchDelta);
}

void MainWindow::removeItemIfBrick()
{
    QGraphicsItem* item = ball->collidingItems().first();
    if (item->data(0) == 1) {
        scene->removeItem(item);
        delete item;
    }
}
