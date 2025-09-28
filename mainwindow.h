//
// Created by Marco on 5/7/2025.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <iostream>

#include "Tokenizer.h"
#include "Grammaryzer.h"
#include "Highlighter.h"

namespace ui
{
    QT_BEGIN_NAMESPACE

    namespace Ui
    {
        class MainWindow;
    }

    QT_END_NAMESPACE

    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() override;

    protected:
        void closeEvent(QCloseEvent* event);

    private:
        Ui::MainWindow* ui;
        QFile file;
        QString filename;
        bool isFileSaved = false;

        QFileDialog* fileDialog;

        Grammaryzer* grammaryzer;
        Highlighter* highlighter;

        void updateCounters() const;

        std::vector<Token> tokens;
        std::string title = "Talos - ";

    signals:
        void openedFile();
        void closedFile();
        void savedFile();
        void editedFile();

    public slots:
        void openFileDialog();
        void openFile();
        void closeFile();
        void saveFile();
        void updateTitle();
        void updateTables(const Token&);
        void beginAnalysis() const;
        void clear() const;
        void exit();
    };
} // ui

#endif //MAINWINDOW_H
