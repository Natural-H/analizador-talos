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

    void addOperation(const Asserter::OperationQuadruple*, int index) const;
    void addAssign(const Asserter::AssignQuadruple*, int index) const;
    void addSI(const Asserter::SIQuadruple*, int index) const;
    void addSF(const Asserter::SFQuadruple*, int index) const;
    void addSV(const Asserter::SVQuadruple*, int index) const;
    void addWrite(const Asserter::WriteQuadruple*, int index) const;
    void addRead(const Asserter::ReadQuadruple*, int index) const;

public slots:
    void drawQuadruples(const std::vector<Asserter::Quadruple*>&) const;
private:
    Ui::QuadsWindow *ui;
};


#endif //ANALIZADORTALOS_QUADSWINDOW_H