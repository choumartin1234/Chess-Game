#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QDebug>
#include <QString>
#include <QCharRef>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFileDialog>
#include <math.h>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(560,710);
    this->setWindowTitle("国际象棋游戏");
    initialize_board();
    Server = new QTcpServer(); Socket = new QTcpSocket();
    msg =new QByteArray(); movemsg = new QByteArray(); loadmsg = new QByteArray();
    clientDlg = new QDialog; serverDlg = new QDialog; waitDlg = new QDialog;
    dlg = new Dialog(this);
    timer = new QTimer(this);
    ui->mybox->setVisible(false);
    ui->hisbox->setVisible(false);
    ui->bonus->setVisible(false);
    ui->bonus2->setVisible(false);
    ui->surrender->setVisible(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(countdown()));
    connect(Socket,SIGNAL(connected()),this,SLOT(connectSuccess()));
    reset();
}

void MainWindow::initialize_board()
{   //Piece(int _color = -1, int _type = 0, QString path = "", bool _clicked = 0, bool _has_moved = 0);
    //color:0->white 1 ->black,-1 ->blank
    //type: 1->兵, 2->城堡 , 3->马 4->主教 5->后 6->王 ; 0->blank
    for (int i = 0; i <= 7; i++)
    {
        list[i][1] = Piece(0,1,":/pic/white_pawn.png");
        list[i][6] = Piece(1,1,":/pic/black_pawn.png");
    }
    for (int i = 0; i <2; i ++)
    {
        list[0+7*i][0] = Piece(0,2,":/pic/white_rook.png",0);
        list[0+7*i][7] = Piece(1,2,":/pic/black_rook.png",0);
        list[1+5*i][0] = Piece(0,3,":/pic/white_knight.png",0);
        list[1+5*i][7] = Piece(1,3,":/pic/black_knight.png",0);
        list[2+3*i][0] = Piece(0,4,":/pic/white_bishop.png",0);
        list[2+3*i][7] = Piece(1,4,":/pic/black_bishop.png",0);
        list[3][0] = Piece(0,5,":/pic/white_queen.png",0);
        list[3][7] = Piece(1,5,":/pic/black_queen.png",0);
        list[4][0] = Piece(0,6,":/pic/white_king.png",0);
        list[4][7] = Piece(1,6,":/pic/black_king.png",0);
    }
    //list[][1] =   Piece(0,5,":/pic/white_queen.png",0);
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_bonus_clicked() //短易位
{
    list[6][0+mycolor*7] = list[4][0+mycolor*7];
    list[4][0+mycolor*7] = Piece();
    list[5][0+mycolor*7] = list[7][0+mycolor*7];
    list[7][0+mycolor*7] = Piece();
    this->update();
    movemsg->clear();
    movemsg->append("b1");
    this->Socket->write(movemsg->data());
    turn = !turn;
    timer->stop();
    time = his_time = 20;
    ui->mylcd->display(time);
    judge_even();
    judge_bonus();
}
void MainWindow::on_bonus2_clicked() //长易位
{
    movemsg->clear();
    movemsg->append("b2");
    this->Socket->write(movemsg->data());
    list[2][0+mycolor*7] = list[4][0+mycolor*7];
    list[4][0+mycolor*7] = Piece();
    list[3][0+mycolor*7] = list[0][0+mycolor*7];
    list[0][0+mycolor*7] = Piece();
    this->update();
    turn = !turn;
    timer->stop();
    time = his_time = 20;
    ui->mylcd->display(time);
    judge_even();
    judge_bonus();
}
void MainWindow::on_surrender_clicked()
{
    if (!endgame)
    {
        msg->clear();
        msg->append("s");
        this->Socket->write(msg->data());
        QString s = (mycolor? "黑方": "白方");
        Info("比赛结束","您已投降,"% s %"落败");
        endgame = true;
        timer->stop();
    }
}
void MainWindow::countdown()
{
    time--;
    msg->clear();
    msg->append("-");
    this->Socket->write(msg->data());
    ui->mylcd->display(time);
    if (time == 0)
    {
        timer->stop();
        QString s = (mycolor? "黑方": "白方");
        Info("比赛结束","超过时间,"% s %"落败");
        endgame = true;

    }
}
QString pos_to_QString(int i, int j)
{
    char x = i+'a';
    char y = j+'1';
    QString s = "";
    s.append(x); s.append(y);
    return s;
}
void MainWindow::on_actionSave_triggered()
{
    timer->stop();
    msg->clear();
    msg->append('f');
    this->Socket->write(msg->data());
    QString PATH = QFileDialog::getSaveFileName(this,tr("选取储存路径"), "", tr("TXT(*.txt)"));
    QFile file(PATH);
    if (!file.open(QFile::WriteOnly | QFile::Text))     //检测文件是否打开
    {
       QMessageBox::information(this, "存档错误", "未输入txt挡案!");
    }
    else
    {
        QTextStream out(&file); //分行写入文件
        //1->pawn, 2->rook, 3->knight 4->bishop 5->queen 6->king
        vector<QString> b[7],w[7];
        b[1].push_back("pawn");
        b[2].push_back("rook");
        b[3].push_back("knight");
        b[4].push_back("bishop");
        b[5].push_back("queen");
        b[6].push_back("king");
        w[1].push_back("pawn");
        w[2].push_back("rook");
        w[3].push_back("knight");
        w[4].push_back("bishop");
        w[5].push_back("queen");
        w[6].push_back("king");
        for (int i = 0; i <8; i++)
            for (int j = 0;j <8; j++)
            {
                int col = list[i][j].color;
                int type = list[i][j].type;
                if (type)
                {
                    if (col == 1)
                        b[type].push_back(pos_to_QString(i,j));
                    else if (col == 0)
                        w[type].push_back(pos_to_QString(i,j));
                }
            }
        if (turn == 1)
        {
            out << "black" << endl;
            for (int i = 1; i <= 6; i++)
            {
                int n = b[i].size()-1;
                if(n != 0)
                {
                    out << b[i][0] << " " << n << " ";
                    for (int j = 1; j<=n; j++)
                        out << b[i][j] << " ";
                    out << endl;
                }
            }
            out << "white" << endl;
            for (int i = 1; i <= 6; i++)
            {

                int n = b[i].size()-1;
                if(n != 0)
                {
                    out << w[i][0] << " " << n << " ";
                    for (int j = 1; j<=n; j++)
                        out << w[i][j] << " ";
                    out << endl;
                }
            }
        }else if (turn == 0)
        {
            out << "white" << endl;
            for (int i = 1; i <= 6; i++)
            {
                int n = b[i].size()-1;
                if(n != 0)
                {
                    out << w[i][0] << " " << n << " ";
                    for (int j = 1; j<=n; j++)
                        out << w[i][j] << " ";
                    out << endl;
                }
            }
            out << "black" << endl;
            for (int i = 1; i <= 6; i++)
            {
                int n = b[i].size()-1;
                if(n != 0)
                {
                    out << b[i][0] << " " << n << " ";
                    for (int j = 1; j<=n; j++)
                        out << b[i][j] << " ";
                    out << endl;
                }
            }
        }
    }
    msg->clear();
    msg->append('o');
    this->Socket->write(msg->data());
    time = his_time = 20;
    QString player = (turn? "黑方": "白方");
    Info("存档成功", player %"请开始行动");
    if(turn == mycolor)
        timer->start(1000);
}
void string_to_type(QString a, int& type, QString& p, bool col)
{   if (!col)
    {
        if (a == "pawn")
        {
            type = 1;
            p = ":/pic/white_pawn.png";
        }else if (a == "rook")
        {
            type = 2;
            p = ":/pic/white_rook.png";
        }else if (a == "knight")
        {
            type = 3;
            p = ":/pic/white_knight.png";
        }else if (a == "bishop")
        {
            type = 4;
            p = ":/pic/white_bishop.png";
        }else if (a == "queen")
        {
            type = 5;
            p = ":/pic/white_queen.png";
        }else if (a == "king")
        {
            type = 6;
            p = ":/pic/white_king.png" ;
        }
    }else
    {
        if (a == "pawn")
        {
            type = 1;
            p = ":/pic/black_pawn.png";
        }else if (a == "rook")
        {
            type = 2;
            p = ":/pic/black_rook.png";
        }else if (a == "knight")
        {
            type = 3;
            p = ":/pic/black_knight.png";
        }else if (a == "bishop")
        {
            type = 4;
            p = ":/pic/black_bishop.png";
        }else if (a == "queen")
        {
            type = 5;
            p = ":/pic/black_queen.png";
        }else if (a == "king")
        {
            type = 6;
            p = ":/pic/black_king.png" ;
        }
    }

}
void MainWindow::parser(QString& a, int col)
{
    QStringList ls = a.split(' ');
    int type; QString p;
    string_to_type(ls[0],type,p,col);
    int n = ls[1].toInt();
    for (int i = 1; i <= n; i++)
    {
        QString s = ls[1+i];
        int x = s[0].toLatin1() - 'a';
        int y = s[1].digitValue() - 1;
        bool has_moved = 0;
        if(type == 1 && ((col == 1 && y != 6) || (col == 0 && y != 1)))
            has_moved = 1;
        list[x][y] = Piece(col,type,p,0,has_moved);
        QString t = "," %QString::number(x) %"," %QString::number(y) %","%QString::number(col) %","%QString::number(type);
        loadmsg->append(t);
        //this->Socket->write(loadmsg->data());
    }
}
void MainWindow::on_actionLoad_triggered()
{
    timer->stop();
    time = his_time = 20;
    msg->clear();
    msg->append('f');
    this->Socket->write(msg->data());
    QString PATH = QFileDialog::getOpenFileName(this,tr("開啟檔案"),"",tr("TXT(*.txt)"));
    QFile f(PATH);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox* msgBox = new QMessageBox(QMessageBox::Warning,"读档错误", "挡案未打开!", nullptr, this);
        msgBox->show();
        if (turn == mycolor)
            timer->start(1000);
    }
    loadmsg->clear();
    loadmsg->append("l");
    //this->Socket->write(msg->data());
    for (int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++)
            list[i][j] = Piece();
    reset();
    QTextStream in(&f);
    QString a;
    int col = 0;
    //1->兵, 2->城堡 , 3->马 4->主教 5->后 6->王 , 0 -> 空白
    if(in.readLine().left(5) == "white")
    {
        loadmsg->append(",0");
        turn = 0;
        while(in.readLineInto(&a))
        {
            if (a.left(5) == "black")
            {
                col = 1;
                break;
            }
            parser(a,col);
        }
        while(in.readLineInto(&a))
        {
            parser(a,col);

        }
    }else
    {
        col = 1;
        loadmsg->append(",1");
        turn = 1;
        while(in.readLineInto(&a))
        {
            if (a.left(5) == "white")
            {
                col = 0;
                break;
            }
            parser(a,col);
        }
        while(in.readLineInto(&a))
        {
            parser(a,col);
        }
    }
    this->Socket->write(loadmsg->data());
    QString player = (turn? "黑方": "白方");
    Info("读档成功", player %"请开始行动");
    if(turn == mycolor)
        timer->start(1000);
    judge_even();
    judge_bonus();
    endgame = false;
}
void MainWindow::on_actionConnect_triggered()
{
    QLabel* label_1 = new QLabel;
    QPushButton* button = new QPushButton;
    ServerIP = new QLineEdit;
    QVBoxLayout* layout = new QVBoxLayout;
    QHBoxLayout* layout_1 = new QHBoxLayout;
    QString ipAddress;
        QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
        for (int i = 0; i < ipAddressesList.size(); ++i) {
            //qDebug()<<ipAddressesList.at(i).toString();
             if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                 ipAddressesList.at(i).toIPv4Address()) {
                 ipAddress = ipAddressesList.at(i).toString();
                 break;
           }
        }
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    address =  ipAddress;
    ServerIP->setText(address);
    label_1->setText("您的IP:");
    button->setText("Ok");
    layout_1->addWidget(label_1);
    layout_1->addWidget(ServerIP);
    layout->addLayout(layout_1);
    layout->addWidget(button);
    serverDlg->setLayout(layout);
    connect(button,SIGNAL(clicked(bool)),this,SLOT(waitConnection()));
    serverDlg->setGeometry(this->x()+width()/4,this->y()+height()*9/20,this->width()/2,this->height()/10);
    serverDlg->show();

}
void MainWindow::waitConnection()
{
    serverDlg->close();
    QLabel* label_1 = new QLabel;
    QLabel* label_2 = new QLabel;
    QPushButton* button = new QPushButton;
    button->setText("Cancel");
    label_2->setText("已输入IP: " + address);
    label_1->setText("等待连接中...");
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label_2);
    layout->addWidget(label_1);
    layout->addWidget(button);
    waitDlg->setLayout(layout);
    this->Server->listen(QHostAddress::Any,8888); //开始监听
    QObject::connect(this->Server,SIGNAL(newConnection()),this,SLOT(startConnection()));
    QObject::connect(button,SIGNAL(clicked()),this,SLOT(cancelConnection()));
    waitDlg->setGeometry(this->x()+width()/4,this->y()+height()*9/20,this->width()/2,this->height()/10);
    waitDlg->show();
    //qDebug() << address;
}
void MainWindow::cancelConnection()
{
    waitDlg->close();
    Server->close();
}
void MainWindow::on_actionClient_triggered()
{
    QLabel* label = new QLabel;
    QVBoxLayout* Layout = new QVBoxLayout;
    QHBoxLayout* layout[3];
    for (int i = 0; i <3; i++)
        layout[i] = new QHBoxLayout;
    ClientIP = new QLineEdit;
    QPushButton* button[12];
    for (int i = 0 ; i <12; i ++)
        button[i] = new QPushButton;
    QSignalMapper* m = new QSignalMapper(this);
    for (int i = 0; i <= 9 ; i++)
    {
        button[i]->setText(QString::number(i));
        connect(button[i], SIGNAL(clicked()), m, SLOT(map()));
        m->setMapping(button[i], i);
    }
    label->setText("输入主机IP:");
    button[11]->setText("连接");
    button[10]->setText(".");
    layout[0]->addWidget(label); layout[0]->addWidget(ClientIP);
    for (int i = 1; i <= 2; i++ )
        for(int j = 0; j <6; j++)
            layout[i]->addWidget(button[6*(i-1)+j]);
    for (int i = 0; i <3; i++) Layout->addLayout(layout[i]);
    clientDlg->setLayout(Layout);
    connect(button[11],SIGNAL(clicked()),this,SLOT(checkIP()));
    connect(m, SIGNAL(mapped(int)), this, SLOT(keyPressed(int)));
    connect(button[10],SIGNAL(clicked()),this, SLOT(pointPressed()));
    clientDlg->setGeometry(this->x()-width()/20,this->y()+height()*9/20,this->width()/2,this->height()/10);
    clientDlg->show();
}
void MainWindow::getMessage()
{ //s,-,w,m,l,f,o,e,b1/b2
    QString msg;
    msg = this->Socket->readAll();
    //qDebug() << msg;
    if (msg[0] == 's') //surrender
    {
        QString s = (mycolor? "黑方": "白方");
        Info("比赛结束","对手投降，"%s%"获胜");
        timer->stop();
        endgame = true;

    }
    else if (msg[0] == '-') //time --
    {
        his_time--;
        ui->hislcd->display(his_time);
        if (his_time == 0)
        {
            QString s = (mycolor? "黑方": "白方");
            Info("比赛结束","对手超时，"%s%"获胜");
            endgame = true;
        }
    }
    else if (msg[0] == 'w') //win
    {
        QString s = (mycolor? "黑方": "白方");
        Info("比赛结束","国王被吃," %s% "落败");
        endgame = true;

    }
    else if (msg[0] == 'm') //move
    {
        his_time = 20;
        time = 20;
        turn = !turn;
        //Info("收到情报",msg);
        QStringList ls = msg.split(',');
        int x = ls[1].toInt(); int y =ls[2].toInt(); int i = ls[3].toInt(); int j =ls[4].toInt();
        int type = list[i][j].type;
        list[i][j] = list[x][y];
        list[x][y] = Piece();
        list[i][j].has_moved = true;
        if(ls.size() > 5) //兵升变
        {
            int col = list[i][j].color;
            int _type = ls[5].toInt();
            list[i][j].type = ls[5].toInt();
            if (col)
            {
                if (_type == 2)
                    list[i][j].image.load(":/pic/black_rook.png");
                else if (_type == 3)
                    list[i][j].image.load(":/pic/black_knight.png");
                else if (_type == 4)
                    list[i][j].image.load(":/pic/black_bishop.png");
                else if (_type == 5)
                    list[i][j].image.load(":/pic/black_queen.png");
            }else
            {
                if (_type == 2)
                    list[i][j].image.load(":/pic/white_rook.png");
                else if (_type == 3)
                    list[i][j].image.load(":/pic/white_knight.png");
                else if (_type == 4)
                    list[i][j].image.load(":/pic/white_bishop.png");
                else if (_type == 5)
                    list[i][j].image.load(":/pic/white_queen.png");
            }
        }
        if(type == 6)
        {
            QString s = (mycolor? "黑方": "白方");
            Info("比赛结束","国王被吃，"%s%"落败");
            endgame = true;

        }else
        {
            timer->start(1000);
            ui->hislcd->display(his_time);
        }
        judge_even();
        judge_bonus();
    }
    else if (msg[0] == 'l') //load
    {
        for (int i = 0; i < 8; i++)
            for(int j = 0; j < 8; j++)
                list[i][j] = Piece();
        reset();
        qDebug() << msg;
        QStringList ls = msg.split(',');
        turn = ls[1].toInt();
        for (int i = 0; i < ls.size()/4; i++)
        {
            QString p;
            int x = ls[4*i+2].toInt(); int y = ls[4*i+3].toInt(); int color = ls[4*i+4].toInt(); int type = ls[4*i+5].toInt();
            if (color == 0)
            {
                if (type == 1)
                    p = ":/pic/white_pawn.png";
                else if (type == 2)
                    p = ":/pic/white_rook.png";
                else if (type == 3)
                    p = ":/pic/white_knight.png";
                else if (type == 4)
                    p = ":/pic/white_bishop.png";
                else if (type == 5)
                    p = ":/pic/white_queen.png";
                else if (type == 6)
                    p = ":/pic/white_king.png" ;
            }else if (color == 1)
            {
                if (type == 1)
                    p = ":/pic/black_pawn.png";
                else if (type == 2)
                    p = ":/pic/black_rook.png";
                else if (type == 3)
                    p = ":/pic/black_knight.png";
                else if (type == 4)
                    p = ":/pic/black_bishop.png";
                else if (type == 5)
                    p = ":/pic/black_queen.png";
                else if (type == 6)
                    p = ":/pic/black_king.png" ;
            }
            bool has_moved = 0;
            if(type == 1 && ((color == 1 && y != 6) || (color == 0 && y != 1)))
                has_moved = 1;
            list[x][y] = Piece(color, type, p,0,has_moved);
        }
        time = his_time = 20;
        if (turn == mycolor)
            timer->start(1000);
        QString player = (turn? "黑方": "白方");
        Info("对手读档结束", player %"请开始行动");
        endgame = false;
        judge_even();
        judge_bonus();
    }else if (msg[0] == 'f')
    {
        time = his_time = 20;
        Info("请稍候","对手进行文档操作!");
        timer->stop();
    }else if (msg[0] == 'o')
    {
        time = his_time = 20;
        QString player = (turn? "黑方": "白方");
        Info("对手存档结束", player %"请开始行动");
        if(turn == mycolor)
            timer->start(1000);
    }else if (msg[0] == 'e')
    {
        QString player = (turn ? "黑方":"白方");
        Info("比赛结束", player%"无子可动, 此局和棋!");
        endgame = true;
        timer->stop();
    }else if (msg[0] == 'b')
    {
        bool hiscolor = !mycolor;
        if (msg[1] == '1')
        {

            list[6][0+hiscolor*7] = list[4][0+hiscolor*7];
            list[4][0+hiscolor*7] = Piece();
            list[5][0+hiscolor*7] = list[7][0+hiscolor*7];
            list[7][0+hiscolor*7] = Piece();
        }else if (msg[1] == '2')
        {
            list[2][0+hiscolor*7] = list[4][0+hiscolor*7];
            list[4][0+hiscolor*7] = Piece();
            list[3][0+hiscolor*7] = list[0][0+hiscolor*7];
            list[0][0+hiscolor*7] = Piece();
        }
            turn = !turn;
            time = his_time = 20;
            if (turn == mycolor)
                timer->start(1000);
            judge_even();
            judge_bonus();
    }
    this->update();

}
void MainWindow::mousePressEvent(QMouseEvent *ev)
{
    if(connected && mycolor == turn && !endgame)
        clicked(ev->pos());

}
void MainWindow::clicked(QPoint p)
{
    int i = p.x()/70;
    int j = 7-(p.y()-30)/70;
    if (list[i][j].color == mycolor)
    {
        if(list[i][j].clicked)
        {
            list[i][j].clicked = false;
            reset();
        }else
        {
            reset();
            list[i][j].clicked = true;
            switch(list[i][j].type) //1->兵, 2->城堡
            {
            case 1:
            {
                if(mycolor)//黑
                {
                    if(list[i][j-1].is_blank())
                        can_move[i][j-1] = true;
                    if(i>=1 && list[i-1][j-1].is_white())//可吃
                        can_move[i-1][j-1] = true;
                    if(i<=7 &&list[i+1][j-1].is_white())
                        can_move[i+1][j-1] = true;
                    if (!list[i][j].has_moved)
                        if(list[i][j-2].is_blank() && list[i][j-1].is_blank())
                        can_move[i][j-2] = true;
                }else
                {
                    if(list[i][j+1].is_blank())
                        can_move[i][j+1] = true;
                    if(i>=1 && list[i-1][j+1].is_black())
                        can_move[i-1][j+1] = true;
                    if(i<=7 &&list[i+1][j+1].is_black())
                        can_move[i+1][j+1] = true;
                    if (!list[i][j].has_moved)
                        if(list[i][j+2].is_blank() && list[i][j+1].is_blank())
                        can_move[i][j+2] = true;
                }
                    break;
            }
            case 2:
            {
                judge_line(i,j);
                break;
            }
            case 3:
            {
                vector<int> dx = {-2,-1,-2,-1, 2, 1,2,1};
                vector<int> dy = {-1,-2, 1, 2,-1,-2,1,2};
                for (int n = 0; n <8; n++)
                {

                    int x = i+dx[n]; int y = j+dy[n];
                    qDebug() << "x=" << x << "\t y=" << y;
                    if(x>=0 && x<=7 && y>=0 && y<=7 && list[x][y].color != mycolor)
                        can_move[x][y] = true;

                }
                break;
            }
            case 4:
            {
                judge_cross(i,j);
                break;
            }
            case 5:
            {
                judge_line(i,j);
                judge_cross(i,j);
                break;
            }
            case 6: //3->马 4->主教 5->后 6->王 , 0 -> 空白
            {
                for (int dx = -1; dx<=1; dx++)
                    for (int dy = -1; dy <= 1; dy++)
                        if(i+dx >=0 && i+dx <= 7 && j+dy >=0 && j+dy <= 7 && list[i+dx][j+dy].color != mycolor)
                            can_move[i+dx][j+dy] = true;
                break;
            }
        }
        }
    }
    if(can_move[i][j])
    {
        for(int x = 0 ; x <=7; x++)
            for(int y = 0; y <= 7; y++)
                if(list[x][y].clicked)
                {
                    move(x,y,i,j);
                    reset();
                    break;
                }

    }
    this->update();
}
void MainWindow::judge_line(int i, int j)
{
    int left = i-1; int right = i+1; int up = j+1; int down = j-1;
    while(left >= 0 && list[left][j].color != mycolor)
    {
        can_move[left][j] = true;
        if (!list[left][j].is_blank()) //如果是不同色的棋子就会被挡住,退出。
            break;
        left--;
    }
    while(right <= 7 && list[right][j].color != mycolor)
    {
        can_move[right][j] = true;
        if (!list[right][j].is_blank()) //如果是不同色的棋子就会被挡住,退出。
            break;
        right++;
    }
    while(up <= 7 && list[i][up].color != mycolor)
    {
        can_move[i][up] = true;
        if (!list[i][up].is_blank()) //如果是不同色的棋子就会被挡住,退出。
            break;
        up++;
    }
    while(down >= 0  && list[i][down].color != mycolor)
    {
        can_move[i][down] = true;
        if (!list[i][down].is_blank()) //如果是不同色的棋子就会被挡住,退出。
            break;
        down--;
    }
}
void MainWindow::judge_cross(int i, int j)
{
    int leftup[2] = {i-1,j+1};
    int rightup[2] = {i+1,j+1};
    int leftdown[2] = {i-1,j-1};
    int rightdown[2] = {i+1,j-1};
    while(leftup[0] >= 0 && leftup[1]<=7 && list[leftup[0]][leftup[1]].color != mycolor)
    {
        can_move[leftup[0]][leftup[1]] = true;
        if (!list[leftup[0]][leftup[1]].is_blank()) //如果是不同色的棋子就会被挡住,退出。
            break;
        leftup[0]--; leftup[1]++;
    }
    while(rightup[0] <= 7 && rightup[1]<=7 && list[rightup[0]][rightup[1]].color != mycolor)
    {
        can_move[rightup[0]][rightup[1]] = true;
        if (!list[rightup[0]][rightup[1]].is_blank()) //如果是不同色的棋子就会被挡住,退出。
            break;
        rightup[0]++; rightup[1]++;
    }
    while(leftdown[0] >= 0 && leftdown[1] >=0 && list[leftdown[0]][leftdown[1]].color != mycolor)
    {
        can_move[leftdown[0]][leftdown[1]] = true;
        if (!list[leftdown[0]][leftdown[1]].is_blank()) //如果是不同色的棋子就会被挡住,退出。
            break;
        leftdown[0]--; leftdown[1]--;
    }
    while(rightdown[0] <= 7 && rightdown[1] >=0 && list[rightdown[0]][rightdown[1]].color != mycolor)
    {
        can_move[rightdown[0]][rightdown[1]] = true;
        if (!list[rightdown[0]][rightdown[1]].is_blank()) //如果是不同色的棋子就会被挡住,退出。
            break;
        rightdown[0]++; rightdown[1]--;
    }
}
void MainWindow::reset()
{
    for(int i = 0; i <=7; i++)
        for (int j = 0; j <=7; j++)
        {
            can_move[i][j] = 0;
            list[i][j].clicked = 0;
        }
    this->update();
}
void MainWindow::move(int x,int y,int i,int j)
{

    if(list[i][j].type == 6)
    {
        QString s = (mycolor? "黑方": "白方");
        Info("比赛结束","吃掉国王，"%s%"获胜");
        endgame = true;
        timer->stop();
        list[i][j] = list[x][y];
        list[x][y] = Piece();
        list[i][j].has_moved = true;
        movemsg->clear();
        movemsg->append("m," % QString::number(x) % ","  % QString::number(y) % ","  % QString::number(i) % ","  % QString::number(j));
        this->Socket->write(movemsg->data());
        return;
    }
    list[i][j] = list[x][y];
    list[x][y] = Piece();
    list[i][j].has_moved = true;
    movemsg->clear();
    movemsg->append("m," % QString::number(x) % ","  % QString::number(y) % ","  % QString::number(i) % ","  % QString::number(j));
    if(list[i][j].type == 1 && ((mycolor == 0 && j == 7 ) || (mycolor == 1 && j == 0))) //兵升变
    {
        QDialog* a = new Dialog(this);
        upgrade_x = i;
        upgrade_y = j;
        connect(a,SIGNAL(choose(int)),this, SLOT(upgrade(int)));
        a->show();
    }
    else
    {
        this->Socket->write(movemsg->data());
        turn = !turn;
        timer->stop();
        time = his_time = 20;
        ui->mylcd->display(time);
        judge_even();
        judge_bonus();
    }
}
void MainWindow::judge_bonus()
{
    if(judge_short_bonus())
        ui->bonus->setEnabled(true);
    else
        ui->bonus->setEnabled(false);
    if(judge_long_bonus())
        ui->bonus2->setEnabled(true);
    else
        ui->bonus2->setEnabled(false);
}
bool MainWindow::judge_short_bonus()
{
    if(mycolor != turn)
        return false;
    if(list[4][0+7*mycolor].type != 6 || list[4][0+7*mycolor].color != mycolor)
        return false;
    if(list[7][0+7*mycolor].type != 2 || list[4][0+7*mycolor].color != mycolor)
        return false;
    if(list[5][0+7*mycolor].type != 0 ||  list[6][0+7*mycolor].type != 0)
        return false;
    for (int i = 4; i <= 6; i ++)
        if(judge_attack(i,0+7*mycolor, mycolor))
            return false;
    return true;
}
bool MainWindow::judge_long_bonus()
{
    if(mycolor != turn)
        return false;
    if(list[4][0+7*mycolor].type != 6 || list[4][0+7*mycolor].color != mycolor)
        return false;
    if(list[0][0+7*mycolor].type != 2 || list[0][0+7*mycolor].color != mycolor)
        return false;
    if(list[1][0+7*mycolor].type != 0 ||  list[2][0+7*mycolor].type != 0 || list[3][0+7*mycolor].type != 0)
        return false;
    for (int i = 4; i >= 2; i --)
        if(judge_attack(i,0+7*mycolor, mycolor))
            return false;
    return true;
}
void MainWindow::judge_even()
{
    for (int i = 0; i <8; i++)
        for (int j = 0 ; j < 8; j++)
            if(list[i][j].color == turn && list[i][j].type == 6)
            {
                x_king = i;
                y_king = j;
                break;
            }

    //1.王没被将军   //判断该位置是否被攻击;
    if(judge_attack(x_king,y_king,turn))
        return;

    //2.没合法移动
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if(list[i][j].color == turn && able_move(i,j))
                    return;
    timer->stop();
    QString player = (turn ? "黑方":"白方");
    Info("比赛结束", player%"无子可动且未被将军, 此局和棋!");
    endgame = true;

}
bool MainWindow::judge_can_move_line(int i, int j)
{
    int color = list[i][j].color;
    int left = i-1; int right = i+1; int up = j+1; int down = j-1;
    if(left >= 0 && list[left][j].color != color && judge_save(i,j,left,j))
        return true;
    if (right <= 7 && list[right][j].color != color && judge_save(i,j,right,j))
        return true;
    if(up <= 7 && list[i][up].color != color && judge_save(i,j,i,up))
        return true;
    if(down >= 0  && list[i][down].color != color && judge_save(i,j,i,down))
        return true;
    return false;
}
bool MainWindow::judge_can_move_cross(int i, int j)
{
    int color = list[i][j].color;
    if(i-1 >= 0 && j-1 >= 0 && list[i-1][j-1].color != color && judge_save(i,j,i-1,j-1))
        return true;
    if(i-1 >= 0 && j+1 <= 7 && list[i-1][j+1].color != color && judge_save(i,j,i-1,j+1))
        return true;
    if(i+1 <= 7 && j-1 >= 0 && list[i+1][j-1].color != color && judge_save(i,j,i+1,j-1))
        return true;
    if(i+1 <= 7 && j+1 <= 7  && list[i+1][j+1].color != color && judge_save(i,j,i+1,j+1))
        return true;
    return false;
}
bool MainWindow::judge_save(int i, int j, int x, int y)//i,j移动到了x,y后, 王是否还安全?
{
    int col = list[i][j].color;
    auto temp1 = list[i][j];
    auto temp2 = list[x][y];
    bool mark = true;
    list[x][y] = list[i][j];
    list[i][j] = Piece();
    if(judge_attack(x_king,y_king,col))
    {
        mark = false;
        qDebug()<< "(" << i << "," << j << ")移动到" << "(" << x << "," << y << ")不安全!";
    }
    list[i][j] = temp1;
    list[x][y] = temp2;
    return mark;
}
bool MainWindow::able_move(int i, int j)
{
    bool color = list[i][j].color;
    switch(list[i][j].type) //1->兵, 2->城堡
    {
    case 1:
    {
        if(color)//黑
        {
            if(list[i][j-1].is_blank() && judge_save(i,j,i,j-1))
                return true;
            if(i>=1 && list[i-1][j-1].is_white() && judge_save(i,j,i-1,j-1))//可吃
                return true;
            if(i<=7 &&list[i+1][j-1].is_white()&& judge_save(i,j,i+1,j-1))
                return true;
            if (!list[i][j].has_moved)
                if(list[i][j-2].is_blank() && list[i][j-1].is_blank() && judge_save(i,j,i,j-2))
                return true;
        }else
        {
            if(list[i][j+1].is_blank()&& judge_save(i,j,i,j+1))
                return true;
            if(i>=1 && list[i-1][j+1].is_black() && judge_save(i,j,i-1,j+1))
                return true;
            if(i<=7 &&list[i+1][j+1].is_black()&& judge_save(i,j,i+1,j+1))
                return true;
            if (!list[i][j].has_moved)
                if(list[i][j+2].is_blank() && list[i][j+1].is_blank()&& judge_save(i,j,i,j+2))
                return true;
        }
        break;
    }
    case 2:
    {
        if(judge_can_move_line(i,j))
            return true;
        break;
    }
    case 3:
    {
        vector<int> dx = {-2,-1,-2,-1, 2, 1,2,1};
        vector<int> dy = {-1,-2, 1, 2,-1,-2,1,2};
        for (int n = 0; n <8; n++)
        {
            int x = i+dx[n]; int y = j+dy[n];
            if(x>=0 && x<=7 && y>=0 && y<=7 && list[x][y].color != color && judge_save(i,j,x,y))
                return true;
        }
        break;
    }
    case 4:
    {
        if(judge_can_move_cross(i,j))
            return true;
        break;
    }
    case 5:
    {
        if (judge_can_move_line(i,j) || judge_can_move_cross(i,j))
            return true;
        break;
    }
    case 6: //王:要该位置没人且不会被攻击。
    {
        for (int dx = -1; dx<=1; dx++)
            for (int dy = -1; dy <= 1; dy++)
                if (dx != 0 || dy != 0)
                    if(i+dx >=0 && i+dx <= 7 && j+dy >=0 && j+dy <= 7 && list[i+dx][j+dy].color != color && !judge_attack(i+dx,j+dy,color))
                        return true;
        break;
    }
    }
    return false;
}
bool MainWindow::judge_attack_line(int x, int y, int i, int j) //判断x,y这一格有没有被i,j水平垂直攻击到
{
    if (x!=i && y != j)
        return false;
    bool color = list[i][j].color;
    if (y == j)
    {
        int left = i-1; int right = i+1;
        while(left >= 0 && list[left][j].color != color)
        {
            if (left == x )
                return true;
            if (!list[left][j].is_blank()) //如果是不同色的棋子就会被挡住,退出。
                break;
            left--;
        }
        while(right <= 7 && list[right][j].color != color)
        {
            if (right == x)
                return true;
            if (!list[right][j].is_blank()) //如果是不同色的棋子就会被挡住,退出。
                break;
            right++;
        }
    }
    else if ( x == i )
    {
        int up = j+1; int down = j-1;
        while(up <= 7 && list[i][up].color != color)
        {
            if (up == y)
                return true;
            if (!list[i][up].is_blank()) //如果是不同色的棋子就会被挡住,退出。
                break;
            up++;
        }
        while(down >= 0  && list[i][down].color != color)
        {
            if (down == y)
                return true;
            can_move[i][down] = true;
            if (!list[i][down].is_blank()) //如果是不同色的棋子就会被挡住,退出。
                break;
            down--;
        }
    }
    return false;
}
bool MainWindow::judge_attack_cross(int x, int y, int i, int j) //判断x,y这一格有没有被i,j叉叉攻击到
{
    if(abs(x-i)!= abs(y-j))
        return false;
    int color = list[i][j].color;
    int leftup[2] = {i-1,j+1};
    int rightup[2] = {i+1,j+1};
    int leftdown[2] = {i-1,j-1};
    int rightdown[2] = {i+1,j-1};
    if (x < i && y > j)
    {
        while(leftup[0] >= 0 && leftup[1]<=7 && list[leftup[0]][leftup[1]].color != color)
        {
            if (leftup[0] == x && leftup[1] == y)
                return true;
            if (!list[leftup[0]][leftup[1]].is_blank()) //如果是不同色的棋子就会被挡住,退出。
                break;
            leftup[0]--; leftup[1]++;
        }
    }else if (x > i && y > j)
    {
        while(rightup[0] <= 7 && rightup[1]<=7 && list[rightup[0]][rightup[1]].color != color)
        {
            if (rightup[0] == x && rightup[1] == y)
                return true;
            if (!list[rightup[0]][rightup[1]].is_blank()) //如果是不同色的棋子就会被挡住,退出。
                break;
            rightup[0]++; rightup[1]++;
        }
    }
    else if (x < i && y < j)
    {
        while(leftdown[0] >= 0 && leftdown[1] >=0 && list[leftdown[0]][leftdown[1]].color != color)
        {
            if (leftdown[0] == x && leftdown[1] == y)
                return true;
            if (!list[leftdown[0]][leftdown[1]].is_blank()) //如果是不同色的棋子就会被挡住,退出。
                break;
            leftdown[0]--; leftdown[1]--;
        }
    }else if (x > i && y < j)
    {
        while(rightdown[0] <= 7 && rightdown[1] >=0 && list[rightdown[0]][rightdown[1]].color != color)
        {
            if (rightdown[0] == x && rightdown[1] == y)
                return true;
            if (!list[rightdown[0]][rightdown[1]].is_blank()) //如果是不同色的棋子就会被挡住,退出。
                break;
            rightdown[0]++; rightdown[1]--;
        }
    }
    return false;
}
bool MainWindow::judge_attack(int x, int y, bool col) //判断(x,y)这格,颜色为col的棋子有没有被敌方的棋子攻击到
{
    //1->兵, 2->城堡 , 3->马 4->主教 5->后 6->王 , 0 -> 空白
    auto temp = list[x][y].color;
    list[x][y].color = col;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
        {
            int n = list[i][j].type;
            int color = list[i][j].color;
            if(color != col && n != 0) //如果是敌方棋子
                switch (n)
                {
                case 1:
                    if ((x == i-1 || x == i+1) && ((color == 1 && y == j-1) || (color == 0 && y == j+1)))
                    {
                        list[x][y].color = temp;
                        return true;
                    }
                    break;
                case 2:
                    if (judge_attack_line(x,y,i,j))
                    {
                        list[x][y].color = temp;
                        return true;
                    }
                    break;
                case 3:
                    if (x!=i && y!=j && (abs(x-i)+abs(y-j)==3)) //曼哈顿距离为3且不在同一排
                    {
                        list[x][y].color = temp;
                        return true;
                    }
                    break;
                case 4:
                    if (judge_attack_cross(x,y,i,j))
                    {
                        list[x][y].color = temp;
                        return true;
                    }
                    break;
                case 5:
                    if (judge_attack_cross(x,y,i,j) || judge_attack_line(x,y,i,j))
                    {
                        list[x][y].color = temp;
                        return true;
                    }
                    break;
                case 6:
                    if (abs(x-i) <= 1 && abs(y-j) <= 1)
                    {
                        list[x][y].color = temp;
                        return true;
                    }
                    break;
                }
        }
    //qDebug()<< x << "," << y << "这格没被攻击到";
    list[x][y].color = temp;
    return false;
}

void MainWindow::upgrade(int _type)
{
    list[upgrade_x][upgrade_y].type = _type;
    if (list[upgrade_x][upgrade_y].color)
    {
        if (_type == 2)
            list[upgrade_x][upgrade_y].image.load(":/pic/black_rook.png");
        else if (_type == 3)
            list[upgrade_x][upgrade_y].image.load(":/pic/black_knight.png");
        else if (_type == 4)
            list[upgrade_x][upgrade_y].image.load(":/pic/black_bishop.png");
        else if (_type == 5)
            list[upgrade_x][upgrade_y].image.load(":/pic/black_queen.png");
    }else
    {
        if (_type == 2)
            list[upgrade_x][upgrade_y].image.load(":/pic/white_rook.png");
        else if (_type == 3)
            list[upgrade_x][upgrade_y].image.load(":/pic/white_knight.png");
        else if (_type == 4)
            list[upgrade_x][upgrade_y].image.load(":/pic/white_bishop.png");
        else if (_type == 5)
            list[upgrade_x][upgrade_y].image.load(":/pic/white_queen.png");
    }
    movemsg->append("," % QString::number(_type));
    this->Socket->write(movemsg->data());
    turn = !turn;
    timer->stop();
    time = 20;
    ui->mylcd->display(time);
    judge_even();
    judge_bonus();

}
void MainWindow::Info(QString a, QString b)
{
    QMessageBox* msg = new QMessageBox(QMessageBox::Information,a,b);
    msg->setGeometry(this->x()+width()/4,this->y()+height()*9/20,this->width()/2,this->height()/10);
    msg->show();
}
void MainWindow::checkIP()
{
    QString IPaddress = ClientIP->text();
    this->Socket->connectToHost(QHostAddress(IPaddress),8888);
    if(!Socket->waitForConnected(3000))
    {
        Info("输入IP错误","连接失败");
    }
}
void MainWindow::startConnection()
{
    connected = true;
    ui->actionLoad->setEnabled(true);
    ui->actionSave->setEnabled(true);
    ui->mybox->setVisible(true);
    ui->hisbox->setVisible(true);
    ui->bonus->setVisible(true);
    ui->bonus2->setVisible(true);
    ui->surrender->setVisible(true);
    ui->label->setVisible(false);
    ui->label2->setVisible(false);
    waitDlg->close();
    Info("等待连接成功","你是白方，开始游戏吧!");
    this->Socket = this->Server->nextPendingConnection();
    connect(this->Socket,SIGNAL(readyRead()),this,SLOT(getMessage()));
    timer->start(1000);
}

void MainWindow::connectSuccess()
{
    connected = true;
    ui->actionLoad->setEnabled(true);
    ui->actionSave->setEnabled(true);
    ui->mybox->setVisible(true);
    ui->hisbox->setVisible(true);
    ui->bonus->setVisible(true);
    ui->bonus2->setVisible(true);
    ui->surrender->setVisible(true);
    ui->label->setVisible(false);
    ui->label2->setVisible(false);
    mycolor = 1;
    clientDlg->close();
    Info("连接主机成功","你是黑方，开始游戏吧!");
    connect(this->Socket,SIGNAL(readyRead()),this,SLOT(getMessage()));
}

void MainWindow::keyPressed(int a)
{
    if(ClientIP)
        ClientIP->insert(QString::number(a));
}
void MainWindow::pointPressed()
{
    if(ClientIP)
        ClientIP->insert(".");
}
void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0,30,560,560,background);
    for (int i = 0; i <=7 ; i++)
        for (int j = 0; j <=7 ; j++)
            if (list[i][j].type)
                p.drawPixmap(x2pix(i),y2pix(j),70,70,list[i][j].image);
    draw_can_move();
    draw_clicked();
}
void MainWindow::draw_can_move()
{
    QPainter p(this);
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setColor("red");
    pen.setWidth(5);
    p.setPen(pen);
    for (int i = 0; i < 8 ; i++)
        for (int j = 0; j < 8; j++)
            if (can_move[i][j])
                p.drawRect(x2pix(i),y2pix(j),70,70);
}
void MainWindow::draw_clicked()
{
    QPainter p(this);
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setColor("blue");
    pen.setWidth(5);
    p.setPen(pen);
    for (int i = 0; i < 8 ; i++)
        for (int j = 0; j < 8; j++)
            if (list[i][j].clicked)
            {
                p.drawRect(x2pix(i),y2pix(j),70,70);
                return;
            }
}
int MainWindow::x2pix(int x)
{
    return 70*x;
}
int MainWindow::y2pix(int y)
{
    return 30 + 70*(7-y);
}
