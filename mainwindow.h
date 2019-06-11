#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QVector>

namespace Ui {
class MainWindow;
}

struct Overlay {
    QString file_path;
    QString name;
    QGraphicsItem* graphics;
};

class Map {
public:
    Map() = default;
    Map(QString path);

    QString path;
    QString name;
    QVector<Overlay> overlays;
};

class MapScene : public QGraphicsScene
{
    Q_OBJECT

public:
    MapScene(QObject * parent = 0);
    void setMap(Map *m);
    Map *getMap() const;
    void highligt(QPoint pos);
    QGraphicsItem *addOverlay(QString path);
    void deleteOverlay(QGraphicsItem *item);

signals:
     void mousepressed(QMouseEvent*);
     void mousemoved(QMouseEvent*);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    QGraphicsRectItem *selectedSquare;
    QGraphicsRectItem *highlightedSquare;
    Map *currentMap;
    QGraphicsPixmapItem *map;

    void addSelectors();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void pressed(QMouseEvent * e);
    void moved(QMouseEvent * e);
    void calibrated(qreal mx, qreal my, qreal cx, qreal cy);
    void map_selected(QAction *a);
    void overlay_selected(QAction *a);
    void recalculate_manual();
    void recalculate_selected();
    void update_params();
    
private slots:
    void on_actionCalibrate_triggered();
    void on_calibrateButton_clicked();
    void on_bookmarkAdd_clicked();
    void on_bookmarkDelete_clicked();
    void on_bookmarkSave_clicked();
    void on_bookmarkTable_itemSelectionChanged();
    void on_actionAbout_triggered();
    void on_actionUsage_triggered();
    void on_calibrationMX_currentIndexChanged();
    void on_calibrationMY_currentIndexChanged();
    void on_calibrationCX_valueChanged();
    void on_calibrationCY_valueChanged();

private:
    Ui::MainWindow *ui;
    MapScene *scene;
    Map *currentMap;
    QButtonGroup* bg;
    qreal mx;
    qreal my;
    qreal cx;
    qreal cy;
    bool selected;
    int sx;
    int sy;

    QVector<Map> maps;
};
Q_DECLARE_METATYPE(Overlay*)

#endif // MAINWINDOW_H
