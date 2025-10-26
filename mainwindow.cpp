#include "mainwindow.h"

#include "Grammaryzer.h"
#include "logswindow.h"
#include "ui_MainWindow.h"

namespace ui {
    MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
        ui->setupUi(this);
        setWindowTitle((title + "No File").data());
        fileDialog = new QFileDialog(this);
        fileDialog->setNameFilter("Talos Files (*.tls);;All Files (*)");

        loggerWindow = new logswindow(this);

        grammaryzer = new Grammaryzer();
        highlighter = new Highlighter(ui->sourcePanel->document());

        ui->sourcePanel->setPlainText("class\n\nendclass");

        updateCounters();

        connect(ui->checkUseStyles, &QCheckBox::checkStateChanged, this, [&] {
            highlighter->setEnableStyling(ui->checkUseStyles->isChecked());
        });

        // connect(ui->sourcePanel, &QPlainTextEdit::textChanged, this, &MainWindow::updateTitle);
        connect(ui->sourcePanel, &QPlainTextEdit::textChanged, this, &MainWindow::updateCounters);
        connect(ui->openButton, &QPushButton::clicked, this, &MainWindow::openFileDialog);
        connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::saveFile);
        connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::clear);
        connect(ui->exitButton, &QPushButton::clicked, this, &MainWindow::exit);
        connect(ui->analyzeButton, &QPushButton::clicked, this, &MainWindow::beginAnalysis);

        connect(ui->showLogsButton, &QPushButton::clicked, this, [&] {
            loggerWindow->show();
        });

        connect(grammaryzer->tokenizer, &Tokenizer::tokenFound, this, &MainWindow::updateTables);

        connect(ui->sourcePanel, &QPlainTextEdit::cursorPositionChanged, this, [&] {
            ui->labelCursorPos->setText(QString::number(ui->sourcePanel->textCursor().blockNumber() + 1) + ":" +
                                        QString::number(ui->sourcePanel->textCursor().positionInBlock() + 1));
        });

        connect(ui->sourcePanel, &QPlainTextEdit::textChanged, this, [&] {
            emit editedFile();
            if (ui->checkBoxAuto->isChecked())
                beginAnalysis();
        });

        connect(ui->checkBoxAuto, &QCheckBox::checkStateChanged, this, [&]() {
            ui->analyzeButton->setEnabled(!ui->checkBoxAuto->isChecked());

            if (ui->checkBoxAuto->isChecked())
                beginAnalysis();
        });

        connect(grammaryzer, &Grammaryzer::newLogs, loggerWindow, &logswindow::setLogs);

        ui->checkUseStyles->setCheckState(Qt::Checked);
        ui->checkBoxAuto->setCheckState(Qt::Unchecked);
        ui->checkBoxAuto->setDisabled(true);

        ui->sourcePanel->setTabStopDistance(ui->sourcePanel->fontMetrics().horizontalAdvance(' ') * 4);

        connect(this, &MainWindow::closedFile, this, [&] {
            updateTitle();
        });

        connect(this, &MainWindow::savedFile, this, [&] {
            isFileSaved = true;
            updateTitle();
        });

        connect(this, &MainWindow::editedFile, this, [&] {
            isFileSaved = false;
            updateTitle();
        });

        connect(this, &MainWindow::openedFile, this, [&] {
            isFileSaved = true;
            updateTitle();
        });
    }

    MainWindow::~MainWindow() {
        delete highlighter;
        delete grammaryzer;
        delete fileDialog;

        delete loggerWindow;
        delete ui;
    }

    void MainWindow::closeEvent(QCloseEvent *event) {
        if (isFileSaved || this->ui->sourcePanel->document()->isEmpty())
            return;

        const auto reply = QMessageBox::question(this, "Advertencia",
                                                 "¿Quieres guardar el archivo?",
                                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (reply == QMessageBox::Yes)
            saveFile();
        else if (reply == QMessageBox::Cancel)
            event->ignore();
    }

    void MainWindow::updateCounters() const {
        const auto charCount = QString::number(ui->sourcePanel->document()->characterCount());
        const auto lineCount = QString::number(ui->sourcePanel->blockCount());
        ui->labelCharCount->setText("Carácteres: " + charCount);
        ui->labelLineCount->setText("Líneas: " + lineCount);
    }

    void MainWindow::openFileDialog() {
        fileDialog->setAcceptMode(QFileDialog::AcceptOpen);

        if (!fileDialog->exec())
            return;

        if (file.isOpen())
            file.close();

        openFile();
        ui->sourcePanel->setPlainText(file.readAll());
        emit savedFile();
    }

    void MainWindow::openFile() {
        filename = fileDialog->selectedFiles().first();
        file.setFileName(filename);
        if (const auto couldOpen = file.open(QIODevice::ReadWrite | QIODevice::Text); !couldOpen) {
            QMessageBox::critical(this, "Error", "Error opening file: " + file.errorString());
            return;
        }

        emit openedFile();
    }

    void MainWindow::closeFile() {
        if (file.isOpen()) {
            file.close();
            clear();
            emit closedFile();
        }
    }

    void MainWindow::saveFile() {
        if (!file.isOpen()) {
            fileDialog->setAcceptMode(QFileDialog::AcceptSave);

            if (!fileDialog->exec())
                return;

            openFile();
        }

        setWindowTitle((title + filename.toStdString()).data());
        const auto data = ui->sourcePanel->toPlainText().toUtf8();
        file.resize(0); // Clear the file before writing, truncating after this is often inconsistent
        file.seek(0);
        if (file.write(data) == -1) {
            QMessageBox::critical(this, "Error", "Error writing to file");
            return;
        }

        emit savedFile();
    }

    void MainWindow::updateTitle() {
        if (file.isOpen() && !isFileSaved)
            setWindowTitle((title + "*" + filename.toStdString()).data());
        else if (file.isOpen() && isFileSaved)
            setWindowTitle((title + filename.toStdString()).data());
        else
            setWindowTitle((title + "No File").data());
    }

    void MainWindow::updateTables(const Token &token) {
        const auto state = new QTableWidgetItem(QString::number(token.state));
        const auto lexema = new QTableWidgetItem(token.type.data());
        const auto gramema = new QTableWidgetItem(token.content.data());

        const auto refTable = token.isError() ? ui->tableErrors : ui->tableTokens;

        refTable->insertRow(refTable->rowCount());
        refTable->setItem(refTable->rowCount() - 1, 0, state);
        refTable->setItem(refTable->rowCount() - 1, 1, lexema);
        refTable->setItem(refTable->rowCount() - 1, 2, gramema);
    }

    void MainWindow::beginAnalysis() const {
        ui->tableTokens->setRowCount(0);
        ui->tableErrors->setRowCount(0);
        ui->tableGrammarResults->setRowCount(0);

        grammaryzer->tokenizer->setText(ui->sourcePanel->toPlainText().toStdString());
        ui->tableGrammarResults->insertRow(ui->tableGrammarResults->rowCount());
        const auto results = new QTableWidgetItem(grammaryzer->checkGrammar().data());

        emit grammaryzer->newLogs(grammaryzer->logsStream);

        results->setTextAlignment(Qt::AlignTop | Qt::AlignLeft);

        ui->tableGrammarResults->setItem(ui->tableGrammarResults->rowCount() - 1, 0, results);

        if (!ui->checkBoxAuto->isChecked())
            highlighter->rehighlight();
    }

    void MainWindow::clear() const {
        ui->sourcePanel->clear();
        ui->tableTokens->setRowCount(0);
        ui->tableErrors->setRowCount(0);
    }

    void MainWindow::exit() {
        if (file.isOpen())
            file.close();

        QApplication::quit();
    }
} // ui
