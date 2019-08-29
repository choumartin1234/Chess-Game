#ifndef PIECE_H
#define PIECE_H
#include <QString>
#include <QPixmap>


//棋子类
class Piece
{
public:
    Piece(int _color = -1, int _type = 0, QString path = "", bool _clicked = 0, bool _has_moved = 0);
    int color; //0->white, 1 ->black, -1 ->blank
    int type; //1->兵, 2->城堡 , 3->马 4->主教 5->后 6->王 , 0 -> 空白
    bool clicked;
    bool has_moved;
    QPixmap image;
    bool is_black();
    bool is_white();
    bool is_blank();
};

#endif // PIECE_H
