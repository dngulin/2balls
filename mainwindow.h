#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsItem>
#include <QGraphicsScene>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    // Game modes
    bool messageDisplayed;
    bool ballRunning;

    // Controls
    bool rightPressed;
    bool leftPressed;

    // Graphics
    QGraphicsScene *scene;
    QGraphicsRectItem *platform;
    QGraphicsEllipseItem *ball;

    // Speed constants
    qreal dx, dy;
    static const qreal dball =  1.5;
    static const qreal dplatf =  2.0;

    static const qreal launchDelta = 30.0;
    static const qreal collisionDelta = 15.0;
    static const qreal flyDelta = 0.15;

    static const int dtimer = 10;


    // Drawing pens && brushes
    QBrush bgBrush;
    QBrush ballBrush;
    QBrush platformBrush;
    QBrush brickBrush;

    QPen outlinePen;
    QPen keepoutPen;

    // small vector rotation, after collision
    void shiftVector(qreal delta);

    void resetBall();
    void removeItemIfBrick();

protected:
  void timerEvent(QTimerEvent* e);
  void keyPressEvent(QKeyEvent* e);
  void keyReleaseEvent(QKeyEvent* e);

};

#endif // MAINWINDOW_H
