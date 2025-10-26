#ifndef ANALIZADORTALOS_LOGSWINDOW_H
#define ANALIZADORTALOS_LOGSWINDOW_H

#include <QMainWindow>
#include <sstream>

QT_BEGIN_NAMESPACE

namespace Ui {
    class logswindow;
}

QT_END_NAMESPACE

class logswindow : public QMainWindow {
    Q_OBJECT

public:
    explicit logswindow(QWidget *parent = nullptr);

    ~logswindow() override;

private:
    Ui::logswindow *ui;
public slots:
    void setLogs(const std::ostringstream &) const;
signals:

};


#endif //ANALIZADORTALOS_LOGSWINDOW_H