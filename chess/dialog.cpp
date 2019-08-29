#include "dialog.h"
#include "ui_dialog.h"
#include <QMessageBox>
#include <QCloseEvent>
Dialog::Dialog(QWidget *parent) :
    QDialog(parent,Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

}

Dialog::~Dialog()
{
    delete ui;
}
void Dialog::closeEvent(QCloseEvent *event)
{
    if (!button_clicked) //拦截非由按钮触发的closeEvent
    {
        QMessageBox* msg = new QMessageBox
                   (QMessageBox::Warning,"错误操作","请选择升变目标!");
        msg->show();
        event->ignore();
    }
}
void Dialog::reject() //拦截Esc键
{
}
void Dialog::on_btn2_clicked()
{
    button_clicked = true;
    emit choose(2);
    this->close();
}
void Dialog::on_btn3_clicked()
{
    button_clicked = true;
    emit choose(3);
    this->close();
}
void Dialog::on_btn4_clicked()
{
    button_clicked = true;
    emit choose(4);
    this->close();
}
void Dialog::on_btn5_clicked()
{
    button_clicked = true;
    emit choose(5);
    this->close();
}
