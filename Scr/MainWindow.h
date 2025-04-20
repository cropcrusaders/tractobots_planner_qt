#pragma once
#include <QMainWindow>
#include <QGraphicsScene>
#include "PathPlanner.h"

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent=nullptr);
    ~MainWindow();

private slots:
    void on_view_clicked(const QPointF &scenePos);
    void on_generateButton_clicked();
    void on_exportButton_clicked();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene_;
    std::vector<BoostPt> boundaryPts_;
    PathPlanner planner_;
};
