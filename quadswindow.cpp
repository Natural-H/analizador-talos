//
// Created by Marco on 11/17/2025.
//

// You may need to build the project (run Qt uic code generator) to get "ui_QuadsWindow.h" resolved

#include "quadswindow.h"

#include <iostream>
#include <ostream>

#include "ui_QuadsWindow.h"

QuadsWindow::QuadsWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::QuadsWindow) {
    ui->setupUi(this);

    connect(ui->closeButton, &QPushButton::clicked, this, &QWidget::close);
}

QuadsWindow::~QuadsWindow() {
    delete ui;
}

void QuadsWindow::drawQuadruples(const std::vector<Asserter::Quadruple *> &quadruples) const {
    ui->quadruplesTable->setRowCount(0);

    for (int i = 0; i < quadruples.size(); i++) {
        const auto quad = quadruples[i];

        if (const auto assign = dynamic_cast<Asserter::AssignQuadruple *>(quad))
            addAssign(assign, i);
        else if (const auto operation = dynamic_cast<Asserter::OperationQuadruple *>(quad))
            addOperation(operation, i);
        else if (const auto si = dynamic_cast<Asserter::SIQuadruple *>(quad))
            addSI(si, i);
        else if (const auto sf = dynamic_cast<Asserter::SFQuadruple *>(quad))
            addSF(sf, i);
        else if (const auto sv = dynamic_cast<Asserter::SVQuadruple *>(quad))
            addSV(sv, i);
    }
}

void QuadsWindow::addOperation(const Asserter::OperationQuadruple *operation, const int index) const {
    const auto number = new QTableWidgetItem(QString::number(index));
    const auto operator_ = new QTableWidgetItem(Asserter::operatorToString[operation->op].data());
    const auto operand1 = new QTableWidgetItem(operation->operand1.name.data());
    const auto operand2 = new QTableWidgetItem(operation->operand2.name.data());
    const auto result = new QTableWidgetItem(operation->result.data());

    const auto rowCount = ui->quadruplesTable->rowCount();

    ui->quadruplesTable->insertRow(rowCount);
    ui->quadruplesTable->setItem(rowCount, 0, number);
    ui->quadruplesTable->setItem(rowCount, 1, operator_);
    ui->quadruplesTable->setItem(rowCount, 2, operand1);
    ui->quadruplesTable->setItem(rowCount, 3, operand2);
    ui->quadruplesTable->setItem(rowCount, 4, result);
}

void QuadsWindow::addAssign(const Asserter::AssignQuadruple *assign, const int index) const {
    const auto number = new QTableWidgetItem(QString::number(index));
    const auto operator_ = new QTableWidgetItem(Asserter::operatorToString[assign->op].data());
    const auto operand1 = new QTableWidgetItem(assign->toAssign.name.data());
    const auto operand2 = new QTableWidgetItem(QString(""));
    const auto result = new QTableWidgetItem(assign->result.data());

    const auto rowCount = ui->quadruplesTable->rowCount();

    ui->quadruplesTable->insertRow(rowCount);
    ui->quadruplesTable->setItem(rowCount, 0, number);
    ui->quadruplesTable->setItem(rowCount, 1, operator_);
    ui->quadruplesTable->setItem(rowCount, 2, operand1);
    ui->quadruplesTable->setItem(rowCount, 3, operand2);
    ui->quadruplesTable->setItem(rowCount, 4, result);
}

void QuadsWindow::addSI(const Asserter::SIQuadruple *siQuadruple, const int index) const {
    const auto number = new QTableWidgetItem(QString::number(index));
    const auto operator_ = new QTableWidgetItem(Asserter::operatorToString[siQuadruple->op].data());
    const auto operand1 = new QTableWidgetItem(QString(""));
    const auto operand2 = new QTableWidgetItem(QString(""));
    const auto result = new QTableWidgetItem(QString::number(siQuadruple->destiny));

    const auto rowCount = ui->quadruplesTable->rowCount();

    ui->quadruplesTable->insertRow(rowCount);
    ui->quadruplesTable->setItem(rowCount, 0, number);
    ui->quadruplesTable->setItem(rowCount, 1, operator_);
    ui->quadruplesTable->setItem(rowCount, 2, operand1);
    ui->quadruplesTable->setItem(rowCount, 3, operand2);
    ui->quadruplesTable->setItem(rowCount, 4, result);
}

void QuadsWindow::addSF(const Asserter::SFQuadruple *sfQuadruple, const int index) const {
    const auto number = new QTableWidgetItem(QString::number(index));
    const auto operator_ = new QTableWidgetItem(Asserter::operatorToString[sfQuadruple->op].data());
    const auto operand1 = new QTableWidgetItem(sfQuadruple->condition.name.data());
    const auto operand2 = new QTableWidgetItem(QString(""));
    const auto result = new QTableWidgetItem(QString::number(sfQuadruple->destiny));

    const auto rowCount = ui->quadruplesTable->rowCount();

    ui->quadruplesTable->insertRow(rowCount);
    ui->quadruplesTable->setItem(rowCount, 0, number);
    ui->quadruplesTable->setItem(rowCount, 1, operator_);
    ui->quadruplesTable->setItem(rowCount, 2, operand1);
    ui->quadruplesTable->setItem(rowCount, 3, operand2);
    ui->quadruplesTable->setItem(rowCount, 4, result);
}

void QuadsWindow::addSV(const Asserter::SVQuadruple *svQuadruple, const int index) const {
    const auto number = new QTableWidgetItem(QString::number(index));
    const auto operator_ = new QTableWidgetItem(Asserter::operatorToString[svQuadruple->op].data());
    const auto operand1 = new QTableWidgetItem(svQuadruple->condition.name.data());
    const auto operand2 = new QTableWidgetItem(QString(""));
    const auto result = new QTableWidgetItem(QString::number(svQuadruple->destiny));

    const auto rowCount = ui->quadruplesTable->rowCount();

    ui->quadruplesTable->insertRow(rowCount);
    ui->quadruplesTable->setItem(rowCount, 0, number);
    ui->quadruplesTable->setItem(rowCount, 1, operator_);
    ui->quadruplesTable->setItem(rowCount, 2, operand1);
    ui->quadruplesTable->setItem(rowCount, 3, operand2);
    ui->quadruplesTable->setItem(rowCount, 4, result);
}
