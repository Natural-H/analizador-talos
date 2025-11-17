//
// Created by Marco on 11/17/2025.
//

#ifndef ANALIZADORTALOS_QUADSWINDOW_H
#define ANALIZADORTALOS_QUADSWINDOW_H

#include <QMainWindow>

#include "asserter.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class QuadsWindow;
}

QT_END_NAMESPACE

class QuadsWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit QuadsWindow(QWidget *parent = nullptr);

    ~QuadsWindow() override;

    void addOperation(const Asserter::OperationQuadruple*, int index);
    void addAssign(const Asserter::AssignQuadruple*, int index);

public slots:
    void drawQuadruples(const std::vector<Asserter::Quadruple*>&);
private:
    Ui::QuadsWindow *ui;
};


#endif //ANALIZADORTALOS_QUADSWINDOW_H