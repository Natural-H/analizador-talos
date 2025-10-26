#include "logswindow.h"
#include "ui_logswindow.h"

logswindow::logswindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::logswindow) {
    ui->setupUi(this);
    setWindowTitle("Logs del análisis semántico");

    ui->plainTextEdit->setReadOnly(true);

    connect(ui->closeButton, &QPushButton::clicked, this, [&] {
        close();
    });
}

void logswindow::setLogs(const std::ostringstream &logs) const {
    ui->plainTextEdit->setPlainText(logs.str().data());
}

logswindow::~logswindow() {
    delete ui;
}