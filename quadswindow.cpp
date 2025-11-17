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

void QuadsWindow::drawQuadruples(const std::vector<Asserter::Quadruple *> &quadruples) {
    ui->quadruplesTable->setRowCount(0);

    for (int i = 0; i < quadruples.size(); i++) {
        const auto quad = quadruples[i];

        if (const auto assign = dynamic_cast<Asserter::AssignQuadruple *>(quad)) {
            addAssign(assign, i);
        } else if (const auto op = dynamic_cast<Asserter::OperationQuadruple *>(quad)) {
            addOperation(op, i);
        }
    }
}

void QuadsWindow::addOperation(const Asserter::OperationQuadruple *operation, const int index) {
    const auto number = new QTableWidgetItem(QString::number(index));
    const auto operator_ = new QTableWidgetItem(Asserter::operatorToString[operation->op].data());
    const auto operand1 = new QTableWidgetItem(operation->operand1.name.data());
    const auto operand2 = new QTableWidgetItem(operation->operand2.name.data());
    const auto result = new QTableWidgetItem(operation->result.data());

    ui->quadruplesTable->insertRow(ui->quadruplesTable->rowCount());
    ui->quadruplesTable->setItem(ui->quadruplesTable->rowCount() - 1, 0, number);
    ui->quadruplesTable->setItem(ui->quadruplesTable->rowCount() - 1, 1, operator_);
    ui->quadruplesTable->setItem(ui->quadruplesTable->rowCount() - 1, 2, operand1);
    ui->quadruplesTable->setItem(ui->quadruplesTable->rowCount() - 1, 3, operand2);
    ui->quadruplesTable->setItem(ui->quadruplesTable->rowCount() - 1, 4, result);
}

void QuadsWindow::addAssign(const Asserter::AssignQuadruple *assign, const int index) {
    const auto number = new QTableWidgetItem(QString::number(index));
    const auto operator_ = new QTableWidgetItem(Asserter::operatorToString[assign->op].data());
    const auto operand1 = new QTableWidgetItem(assign->toAssign.name.data());
    const auto operand2 = new QTableWidgetItem(QString(""));
    const auto result = new QTableWidgetItem(assign->result.data());

    ui->quadruplesTable->insertRow(ui->quadruplesTable->rowCount());
    ui->quadruplesTable->setItem(ui->quadruplesTable->rowCount() - 1, 0, number);
    ui->quadruplesTable->setItem(ui->quadruplesTable->rowCount() - 1, 1, operator_);
    ui->quadruplesTable->setItem(ui->quadruplesTable->rowCount() - 1, 2, operand1);
    ui->quadruplesTable->setItem(ui->quadruplesTable->rowCount() - 1, 3, operand2);
    ui->quadruplesTable->setItem(ui->quadruplesTable->rowCount() - 1, 4, result);
}
