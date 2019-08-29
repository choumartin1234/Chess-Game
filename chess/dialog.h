#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    void reject();
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

signals:
    void choose(int);

protected:
     void closeEvent(QCloseEvent *event);

private slots:
    void on_btn2_clicked();
    void on_btn3_clicked();
    void on_btn4_clicked();
    void on_btn5_clicked();

private:
    Ui::Dialog *ui;
    bool button_clicked = false;
};

#endif // DIALOG_H
