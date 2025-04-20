#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QGraphicsEllipseItem>
#include <QFileDialog>
#include <boost/geometry.hpp>

using BoostPolygon = boost::geometry::model::polygon<BoostPt>;

MainWindow::MainWindow(QWidget *parent)
 : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    scene_ = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene_);

    connect(scene_, &QGraphicsScene::selectionChanged, [=](){});
    // Alternatively, capture mouse clicks:
    ui->graphicsView->viewport()->installEventFilter(this);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_view_clicked(const QPointF &scenePos) {
    // Add a point to boundary
    boundaryPts_.emplace_back(scenePos.x(), scenePos.y());
    scene_->addEllipse(scenePos.x()-3, scenePos.y()-3, 6, 6,
                       QPen(Qt::red), QBrush(Qt::red));
}

void MainWindow::on_generateButton_clicked() {
    double spacing = ui->rowSpacingSpin->value();
    double headland = ui->headlandSpin->value();
    auto rows = planner_.computeRows(boundaryPts_, spacing, headland);
    // Draw them:
    for (auto &line : rows) {
      auto a = line.first, b = line.second;
      scene_->addLine(a.x(), a.y(), b.x(), b.y(), QPen(Qt::blue));
    }
}

void MainWindow::on_exportButton_clicked() {
    QString file = QFileDialog::getSaveFileName(this, "Save G-Code", "", "*.gcode");
    if (file.isEmpty()) return;
    planner_.exportGCode(file.toStdString());
}
