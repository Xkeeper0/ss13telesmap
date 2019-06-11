#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "calibrationdialog.h"
#include "usagedialog.h"

#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    scene(new MapScene(this)),
    mx(0),
    my(0),
    cx(0),
    cy(0),
    selected(false),
    sx(0),
    sy(0)
{
    ui->setupUi(this);

    QSettings settings("maps.ini", QSettings::IniFormat);

    QActionGroup *action_group = new QActionGroup(this);

    QStringList map_entries = settings.childGroups();
    for(auto const& map: map_entries) {
        settings.beginGroup(map);
        Map m;
        m.path = settings.value("path").toString();
        m.name = map;

        int map_overlays = settings.beginReadArray("overlays");
        for(int i = 0; i< map_overlays; ++i) {
            Overlay o;
            settings.setArrayIndex(i);
            o.file_path = settings.value("path").toString();
            o.name = settings.value("name").toString();
            o.graphics = nullptr;
            m.overlays.append(o);
        }
        settings.endArray();

        maps.append(m);
        settings.endGroup();

        QAction *action = new QAction(map,ui->menuMaps);
        action->setData(maps.size() - 1); // Index to the current map
        action->setCheckable(true);
        action->setActionGroup(action_group);
        ui->menuMaps->addAction(action);
    }

    if(maps.size() > 0) {
        map_selected(ui->menuMaps->actions()[0]);
    }

    ui->mapArea->setScene(scene);
    ui->mapArea->calculateZoom();
    QApplication::setOverrideCursor( Qt::ArrowCursor );
    ui->mapArea->show();

    //ui->stackedWidget->hide();
    ui->bookmarkTable->setHorizontalHeaderLabels({"Name", "Map", "X", "Y"});
    ui->bookmarkTable->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter);
    ui->bookmarkTable->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    ui->bookmarkTable->horizontalHeader()->setResizeMode(0, QHeaderView::Interactive);
    ui->bookmarkTable->horizontalHeader()->setResizeMode(1, QHeaderView::Interactive);
    ui->bookmarkTable->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);
    ui->bookmarkTable->horizontalHeader()->setResizeMode(3, QHeaderView::ResizeToContents);

    ui->bookmarkTable->horizontalHeader()->resizeSection(2, 30);
    ui->bookmarkTable->horizontalHeader()->resizeSection(3, 30);

    QSettings bookmark_settings("bookmarks.ini", QSettings::IniFormat);

    int bookmark_size = bookmark_settings.beginReadArray("bookmark");
    for(int i=0; i < bookmark_size; i++) {
        bookmark_settings.setArrayIndex(i);
        int row = ui->bookmarkTable->rowCount();
        ui->bookmarkTable->insertRow(row);
        ui->bookmarkTable->setItem(row, 0, new QTableWidgetItem(bookmark_settings.value("name").toString()));
        ui->bookmarkTable->setItem(row, 1, new QTableWidgetItem(bookmark_settings.value("map").toString()));
        ui->bookmarkTable->setItem(row, 2, new QTableWidgetItem(bookmark_settings.value("x").toString()));
        ui->bookmarkTable->setItem(row, 3, new QTableWidgetItem(bookmark_settings.value("y").toString()));
    }
    bookmark_settings.endArray();

    QObject::connect(ui->menuMaps, SIGNAL(triggered(QAction*)), this, SLOT(map_selected(QAction*)));
    QObject::connect(ui->menuOverlays, SIGNAL(triggered(QAction*)), this, SLOT(overlay_selected(QAction*)));
    QObject::connect(scene, SIGNAL(mousepressed(QMouseEvent*)), this, SLOT(pressed(QMouseEvent*)));
    QObject::connect(scene, SIGNAL(mousemoved(QMouseEvent*)), this, SLOT(moved(QMouseEvent*)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::pressed(QMouseEvent *e)
{
    if(e->button() == Qt::MiddleButton) {
        ui->mapArea->resetZoom();
    } else if(e->button() == Qt::RightButton) {


        selected = true;
        sx = 1+ e->x()/32;
        sy = 1+ e->y()/32;

        ui->coordinateOX->setText(QString("%1").arg(sx));
        ui->coordinateOY->setText(QString("%1").arg(sy));


        bool e_mx_ok = false;
        bool e_my_ok = false;
        ui->calibrationMX->currentText().toInt(&e_mx_ok);
        ui->calibrationMY->currentText().toInt(&e_my_ok);

        if(e_mx_ok && e_my_ok) {
            float tx = (static_cast<float>(sx)+cx) / mx;
            float ty = (static_cast<float>(sy)+cy) / my;
            ui->coordinateTX->setText(QString("%1").arg(tx));
            ui->coordinateTY->setText(QString("%1").arg(ty));
        }
        e->accept();
    }
}

void MainWindow::moved(QMouseEvent *e)
{

    int ex = e->x();
    int ey = e->y();
    int x = 1+ex/32;
    int y = 1+ey/32;

    ui->statusBar->showMessage(QString("X: %1 / Y: %2").arg(x).arg(y));

}

MapScene::MapScene(QObject *parent)
    : QGraphicsScene(parent),
      currentMap(nullptr),
      map(nullptr)
{
}

void MapScene::addSelectors() {
    QBrush b(QColor(255,0,0,64));
    QPen p;
    p.setColor(QColor(255,0,0,255));
    p.setWidth(2);
    p.setJoinStyle(Qt::RoundJoin);

    selectedSquare = this->addRect(0,0,32,32,p,b);
    selectedSquare->setZValue(101);

    b.setColor(QColor(0,255,0,64));
    p.setColor(QColor(0,64,0,255));

    highlightedSquare = this->addRect(0,0,32,32,p,b);
    highlightedSquare->setZValue(100);
}

void MapScene::setMap(Map *m)
{
    currentMap = m;
    this->clear();
    // Clear the internal QPixmap cache, so the previous map images are not constantly stored in memory
    QPixmapCache::clear();
    addSelectors();

    QPixmap map_pix(currentMap->path);
    map = this->addPixmap(map_pix);
    map->setZValue(0);
    setSceneRect(0,0,map_pix.width(), map_pix.height());
    selectedSquare->hide();
    highlightedSquare->hide();
}

QGraphicsItem* MapScene::addOverlay(QString path)
{
    QPixmap overlay_pixmap(path);
    QGraphicsItem* overlay = this->addPixmap(overlay_pixmap);
    overlay->setZValue(1);
    return overlay;
}

void MapScene::deleteOverlay(QGraphicsItem* overlay)
{
    this->removeItem(overlay);
}

Map *MapScene::getMap() const
{
    return currentMap;
}

void MapScene::highligt(QPoint pos)
{
    highlightedSquare->setPos(pos);
    highlightedSquare->show();
}

void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pf(event->scenePos().x(), height() - event->scenePos().y());
    QMouseEvent ne(event->type(), pf.toPoint(), event->button(), event->buttons(), event->modifiers());
    emit mousepressed(&ne);

    if(event->button() == Qt::RightButton) {
        int x = event->scenePos().x()/32;
        int y = (event->scenePos().y() + 0)/32; // TODO: ?

        selectedSquare->setX(x*32);
        selectedSquare->setY(y*32 - 1); // TODO: ?
        selectedSquare->show();
    }

    QGraphicsScene::mousePressEvent(event);
}

void MapScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pf(event->scenePos().x(), height() - event->scenePos().y());
    QMouseEvent ne(event->type(), pf.toPoint(), event->button(), event->buttons(), event->modifiers());
    emit mousemoved(&ne);

    int x = event->scenePos().x()/32;
    int y = (event->scenePos().y() + 0)/32; // TODO: ?
    int x_pos = x*32;
    int y_pos = y*32 - 1; // TODO: ?

    highligt(QPoint{x_pos, y_pos});

    QGraphicsScene::mouseMoveEvent(event);
}

void MainWindow::on_actionCalibrate_triggered()
{
    CalibrationDialog d(this);
    QObject::connect(&d, SIGNAL(calibrated(qreal,qreal,qreal,qreal)), this, SLOT(calibrated(qreal,qreal,qreal,qreal)));
    d.exec();
}

void MainWindow::on_calibrateButton_clicked()
{
    on_actionCalibrate_triggered();
}

/*
void MainWindow::toolbutton_pressed(int id)
{

    if(ui->stackedWidget->isVisible() && ui->stackedWidget->currentIndex() == id) {
        ui->stackedWidget->hide();
        bg->setExclusive(false);
        bg->checkedButton()->setChecked(false);
        bg->setExclusive(true);
    } else {
        ui->stackedWidget->setCurrentIndex(id);
        ui->stackedWidget->show();
    }
}
*/

void MainWindow::calibrated(qreal mx, qreal my, qreal cx, qreal cy)
{
    if(mx == 1 || mx == 2) {
        ui->calibrationMX->setCurrentIndex(mx);
    } else if (mx == 4) {
        ui->calibrationMX->setCurrentIndex(3);
    }

    if(my == 1 || my == 2) {
        ui->calibrationMY->setCurrentIndex(my);
    } else if (my == 4) {
        ui->calibrationMY->setCurrentIndex(3);
    }

    ui->calibrationCX->setValue(cx);
    ui->calibrationCY->setValue(cy);
    update_params();
}

void MainWindow::on_calibrationMX_currentIndexChanged()
{
    update_params();
}
void MainWindow::on_calibrationMY_currentIndexChanged()
{
    update_params();
}
void MainWindow::on_calibrationCX_valueChanged()
{
    update_params();
}
void MainWindow::on_calibrationCY_valueChanged()
{
    update_params();
}

void MainWindow::map_selected(QAction *a)
{
    for(QAction *i: ui->menuMaps->actions())
    {
        i->setChecked(false);
    }

    currentMap = &maps[a->data().toInt()];
    scene->setMap(&maps[a->data().toInt()]);

    ui->menuOverlays->clear();
    if(!currentMap->overlays.isEmpty()) {
        ui->menuOverlays->setEnabled(true);
        for(auto& overlay: currentMap->overlays) {
            QAction* overlay_action = ui->menuOverlays->addAction(overlay.name);
            overlay_action->setCheckable(true);
            overlay_action->setData(QVariant::fromValue(&overlay));
        }
    }else{
        ui->menuOverlays->setEnabled(false);
    }

    a->setChecked(true);
    setWindowTitle(QString("%1 - %2").arg(QCoreApplication::applicationName()).arg(maps[a->data().toInt()].name));
}

void MainWindow::overlay_selected(QAction *a)
{
    Overlay *overlay = a->data().value<Overlay*>();
    if(a->isChecked() && !overlay->graphics) {
        overlay->graphics = scene->addOverlay(overlay->file_path);
    } else {
        scene->deleteOverlay(overlay->graphics);
        delete overlay->graphics;
        overlay->graphics = nullptr;
    }
}

void MainWindow::recalculate_manual()
{
    bool e_mx_ok = false;
    bool e_my_ok = false;
    ui->calibrationMX->currentText().toInt(&e_mx_ok);
    ui->calibrationMY->currentText().toInt(&e_my_ok);

    if(e_mx_ok && e_my_ok) {
        qreal mox = ui->manualOX->value();
        qreal moy = ui->manualOY->value();
        float tx = (mox+cx) / mx;
        float ty = (moy+cy) / my;
        ui->manualTX->setText(QString("%1").arg(tx));
        ui->manualTY->setText(QString("%1").arg(ty));
    }
}

void MainWindow::recalculate_selected()
{

    bool e_mx_ok = false;
    bool e_my_ok = false;
    ui->calibrationMX->currentText().toInt(&e_mx_ok);
    ui->calibrationMY->currentText().toInt(&e_my_ok);

    float cox = ui->coordinateOX->text().toFloat();
    float coy = ui->coordinateOY->text().toFloat();

    if(e_mx_ok && e_my_ok) {
        float tx = (cox+cx) / mx;
        float ty = (coy+cy) / my;
        ui->coordinateTX->setText(QString("%1").arg(tx));
        ui->coordinateTY->setText(QString("%1").arg(ty));
    }
}

void MainWindow::update_params()
{
    cx = ui->calibrationCX->value();
    cy = ui->calibrationCY->value();

    bool mx_ok = false;
    bool my_ok = false;
    mx = ui->calibrationMX->currentText().toInt(&mx_ok);
    my = ui->calibrationMY->currentText().toInt(&my_ok);

    // Recalculate only with valid constants
    if(mx_ok && my_ok) {
        recalculate_manual();
        recalculate_selected();
        on_bookmarkTable_itemSelectionChanged();
    } else {
        ui->manualTX->clear();
        ui->manualTY->clear();
        ui->bookmarkTX->clear();
        ui->bookmarkTY->clear();
        ui->coordinateTX->clear();
        ui->coordinateTY->clear();
    }

}


Map::Map(QString path)
    : path(path)
{
}

/*
void MainWindow::on_bookmarkAdd_clicked()
{
    ui->bookmarkTable->insertRow(ui->bookmarkTable->rowCount());
}
*/
void MainWindow::on_bookmarkDelete_clicked()
{
    ui->bookmarkTable->removeRow(ui->bookmarkTable->currentRow());
}

void MainWindow::on_bookmarkSave_clicked()
{
    QSettings bookmark_settings("bookmarks.ini", QSettings::IniFormat);
    bookmark_settings.clear();

    bookmark_settings.beginWriteArray("bookmark");
    for(int i=0; i < ui->bookmarkTable->rowCount(); i++) {
        bookmark_settings.setArrayIndex(i);
        bookmark_settings.setValue("name", ui->bookmarkTable->item(i, 0) ? ui->bookmarkTable->item(i, 0)->text() : "");
        bookmark_settings.setValue("map" , ui->bookmarkTable->item(i, 1) ? ui->bookmarkTable->item(i, 1)->text() : "");
        bookmark_settings.setValue("x"   , ui->bookmarkTable->item(i, 2) ? ui->bookmarkTable->item(i, 2)->text() : "");
        bookmark_settings.setValue("y"   , ui->bookmarkTable->item(i, 3) ? ui->bookmarkTable->item(i, 3)->text() : "");
    }
    bookmark_settings.endArray();

}

void MainWindow::on_bookmarkTable_itemSelectionChanged()
{
    int row = ui->bookmarkTable->currentRow();
    if(row == -1) return;

    if (!ui->bookmarkTable->item(row, 2) ||
        !ui->bookmarkTable->item(row, 3)) return;

    int bx = ui->bookmarkTable->item(row, 2)->text().toInt();
    int by = ui->bookmarkTable->item(row, 3)->text().toInt();

    if(currentMap->name == ui->bookmarkTable->item(row,1)->text()) {
        int x_pos = bx*32;
        int y_pos = scene->height() - by*32;
        ui->mapArea->centerOn(x_pos+16, y_pos+16);

        scene->highligt(QPoint{x_pos -32, y_pos});
    }

    if(mx != 0 &&
       my != 0) {
        float tx = (bx+cx) / mx;
        float ty = (by+cy) / my;

        ui->bookmarkName->setText(ui->bookmarkTable->item(row, 0)->text());
        ui->bookmarkMap->setText(ui->bookmarkTable->item(row, 1)->text());
        ui->bookmarkOX->setText(QString("%1").arg(bx));
        ui->bookmarkOY->setText(QString("%1").arg(by));
        ui->bookmarkTX->setText(QString("%1").arg(tx));
        ui->bookmarkTY->setText(QString("%1").arg(ty));
    }
}

void MainWindow::on_bookmarkAdd_clicked()
{
    if(selected) {
        int row = ui->bookmarkTable->rowCount();
        ui->bookmarkTable->insertRow(row);

        ui->bookmarkTable->setItem(row, 0, new QTableWidgetItem("Bookmark"));
        ui->bookmarkTable->setItem(row, 1, new QTableWidgetItem(currentMap->name));
        ui->bookmarkTable->setItem(row, 2, new QTableWidgetItem(QString("%1").arg(sx)));
        ui->bookmarkTable->setItem(row, 3, new QTableWidgetItem(QString("%1").arg(sy)));
    }
}




void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About Telescience Manager"),
        trUtf8("Telescience Manager\n© 2013-2019 by mysha, ZeWaka, et al.\n"
            "\n"
            "This particular build maintained on GitHub at\n"
            "https://github.com/Xkeeper0/ss13telesmap")
        );
}


void MainWindow::on_actionUsage_triggered()
{
    UsageDialog d(this);
    //QObject::connect(&d, SIGNAL(calibrated(qreal,qreal,qreal,qreal)), this, SLOT(calibrated(qreal,qreal,qreal,qreal)));
    d.exec();
}
