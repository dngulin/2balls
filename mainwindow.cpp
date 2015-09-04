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
    qsrand(QTime::currentTime().msec());

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

    // Setup inidicators
    QFont indFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    indFont.setBold(true);
    indFont.setPointSizeF(5);

    ballsIndicator = new QGraphicsTextItem();
    ballsIndicator->setFont(indFont);
    ballsIndicator->setDefaultTextColor(Qt::white);
    scene->addItem(ballsIndicator);
    ballsIndicator->moveBy(1, -19);

    bricksIndicator = new QGraphicsTextItem();
    bricksIndicator->setFont(indFont);
    bricksIndicator->setDefaultTextColor(Qt::white);
    scene->addItem(bricksIndicator);
    bricksIndicator->moveBy(120, -19);

    // Display window
    this->show();
    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);

    resetLevel();

    // Create message
    message = new MessageItem();
    showWellcomeMessage();

    // Run main loop
    this->startTimer(dtimer);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent* e)
{
    if (!messageDisplayed) {

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

                    if (ball->y() > platform->y()) {

                        resetBall();

                        ballscount--;
                        updateBallsIndicator();
                        if (ballscount < 1) showLoseMessage();
                    }
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

    e->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Right)
        rightPressed = true;
    else if (e->key() == Qt::Key_Left)
        leftPressed = true;
    else if (e->key() == Qt::Key_Space) {
        if (messageDisplayed) {

            if (gameFinished) resetLevel();
            scene->removeItem(message);
            messageDisplayed = false;

        } else {
            if (!ballRunning)
                ballRunning = true;
            else
                showPauseMessage();
        }
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
    qreal phi = qRadiansToDegrees(qAtan2(dy,dx));

    // Calculate delta
    qreal mult = qrand() % 201 - 100;
    delta = delta * mult / 100;

    // Rotate vector and make more vertical
    if      ((phi > 0  ) && (phi <  delta       )) phi += qAbs(delta);
    else if ((phi > 180) && (phi < (180 + delta))) phi += qAbs(delta);
    else if ((phi < 180) && (phi > (180 - delta))) phi -= qAbs(delta);
    else if ((phi < 360) && (phi > (360 - delta))) phi -= qAbs(delta);
    else                                           phi += delta;

    phi = qDegreesToRadians(phi);

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

        brickscount--;
        updateBricksIndicator();
        if (brickscount < 1) showWinMessage();
    }
}

void MainWindow::showMessage(QString text)
{
    message->setPlainText(text);
    scene->addItem(message);
    message->setPos(10, 100 - (message->boundingRect().height() / 2));
    messageDisplayed = true;
}

void MainWindow::showWellcomeMessage()
{
    showMessage("Береги свои шары - их всего два!\n"
                "Разбей ими все блоки и ощути вкус победы!\n\n"
                "[<-] = ДВИГАТЬ ВЛЕВО\n"
                "[->] = ДВИГАТЬ ВПРАВО\n"
                "[ПРОБЕЛ] = НАЧАТЬ ИГРУ, ЗАПУСТИТЬ ШАР, ПАУЗА");
}

void MainWindow::showPauseMessage()
{
    showMessage("Игра на паузе, можно передохнуть...\n\n\n"
                "[ПРОБЕЛ] = ПРОДОЛЖИТЬ");
}

void MainWindow::showWinMessage()
{
    showMessage("ЭТО ПОБЕДА!\n"
                "Ура! Ты разбил все блоки!\n\n\n"
                "[ПРОБЕЛ] = ИГРАТЬ ЕЩЁ");
    gameFinished = true;
}

void MainWindow::showLoseMessage()
{
    showMessage("ЭТО ПРОВАЛ!\n"
                "Ты не уберёг свои шары!\n\n\n"
                "[ПРОБЕЛ] = ИГРАТЬ ЕЩЁ");
    gameFinished = true;
}

void MainWindow::updateBallsIndicator()
{
    ballsIndicator->setPlainText("BALLS: " + QString::number(ballscount));
}

void MainWindow::updateBricksIndicator()
{
    bricksIndicator->setPlainText("BRICKS: " + QString::number(brickscount));
}

void MainWindow::resetLevel()
{
    resetBall();

    // Really drop old bricks
    QList<QGraphicsItem*> items = scene->items();
    foreach (QGraphicsItem* item, items) {
        if (item->data(0) == 1) {
            scene->removeItem(item);
            delete item;
        }
    }

    // Update indicators
    ballscount = 2;
    brickscount = 0;

    updateBallsIndicator();
    updateBricksIndicator();

    // Build level
    QFile level("LEVEL");
    if (level.exists()) {
        if (level.open(QIODevice::ReadOnly)) {

            QByteArray levelData = level.readAll();
            QList<QByteArray> lines = levelData.split('\n');

            for (int y = 0; y < lines.count(); y++) {

                if (y >= 15) continue;
                QByteArray line = lines[y];

                for (int x = 0; x < line.length(); x++) {

                    if (x >= 10) continue;
                    if (line[x] == '1') {

                        QGraphicsRectItem* brick = new QGraphicsRectItem(0, 0, 20, 10);
                        brick->setBrush(brickBrush);
                        brick->setPen(outlinePen);
                        brick->setData(0, 1);
                        scene->addItem(brick);
                        brick->moveBy(x * 20, y * 10);

                        brickscount++;
                        updateBricksIndicator();

                    }
                }

            }

            level.close();
        } else {
            QMessageBox::critical(this, "Ошибка!",  "Не удалось открыть файл-уровень.\n");
        }
    } else {
        QMessageBox::critical(this, "Ошибка!",  "Файл-уровень не существует.\n");
    }

    if (brickscount == 0) {
        brickscount = 666;
        updateBricksIndicator();
    }

    gameFinished = false;
}
