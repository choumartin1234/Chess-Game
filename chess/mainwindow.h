#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "piece.h"
#include <QMainWindow>
#include <QtNetwork>
#include <QLineEdit>
#include <QDialog>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStringBuilder>
#include <vector>
#include "dialog.h"
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);

private slots:
    void on_bonus_clicked();
    void on_bonus2_clicked();
    void on_surrender_clicked();
    void on_actionSave_triggered();
    void on_actionLoad_triggered();
    void on_actionConnect_triggered();
    void on_actionClient_triggered();
    void startConnection();
    void waitConnection();
    void cancelConnection();
    void keyPressed(int);
    void pointPressed();
    void checkIP();
    void connectSuccess();
    void getMessage();
    void countdown();
    void upgrade(int type);

private:
    bool endgame = false;
    bool connected = false;
    Dialog* dlg;
    bool mycolor = 0; //当前使用者棋局颜色
    bool turn = 0; //当前轮到谁
    int time = 20; int his_time = 20;
    QByteArray *msg;
    QByteArray *movemsg;
    QByteArray *loadmsg;
    QTimer* timer;
    QDialog* clientDlg;
    QDialog* serverDlg;
    QDialog* waitDlg;
    QTcpServer* Server;
    QTcpSocket* Socket;
    QLineEdit* ClientIP;
    QLineEdit* ServerIP;
    QString address;
    void draw_clicked();
    void draw_can_move();
    void clicked(QPoint p);
    void initialize_board();
    void judge_line(int i, int j);
    void judge_cross(int i, int j);
    bool judge_attack_line(int x, int y, int i, int j);
    bool judge_attack_cross(int x, int y, int i, int j);
    bool able_move(int i, int j);
    bool judge_save(int i, int j, int x, int y);  //判断i,j的棋子移动到了x,y, 王是否还安全
    bool judge_can_move_line(int i, int j);
    bool judge_can_move_cross(int i, int j);
    Ui::MainWindow *ui;
    QPixmap background = QPixmap(":/pic/background.png");
    void Info(QString title, QString sentence);
    Piece list[8][8];
    bool can_move[8][8];
    void reset();
    void move(int x,int y,int i,int j);
    int x2pix(int x);
    int y2pix(int y);
    int upgrade_x = -1;
    int upgrade_y = -1;
    void parser(QString& a, int col);
    void judge_even(); // 逼和
    int x_king;
    int y_king;
    bool judge_short_bonus();
    bool judge_long_bonus();
    bool judge_attack(int x,int y, bool col);
    void judge_bonus();

};

#endif // MAINWINDOW_H
