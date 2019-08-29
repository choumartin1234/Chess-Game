#include "piece.h"

Piece::Piece(int _color, int _type, QString PATH, bool _clicked, bool _has_moved )
    :color(_color),type(_type),clicked(_clicked),has_moved(_has_moved)
{
    image.load(PATH);
}

bool Piece::is_black()
{
    return color == 1;
}
bool Piece::is_white()
{
    return color == 0;
}
bool Piece::is_blank()
{
    return type == 0;
}
