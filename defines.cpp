#include "defines.h"

#include <klocale.h>


const Level::Data Level::DATA[Level::NbLevels+1] = {
	{ 8,  8, 10, "easy",   "8x8x10",   I18N_NOOP("Easy")   },
	{16, 16, 40, "normal", "16x16x40", I18N_NOOP("Normal") },
	{30, 16, 99, "expert", "30x16x99", I18N_NOOP("Expert") },
    {10, 10, 20, "custom", "",         I18N_NOOP("Custom") }
};

Level::Level(Type type)
{
    Q_ASSERT( type<=NbLevels );
    _width   = DATA[type].width;
    _height  = DATA[type].height;
    _nbMines = DATA[type].nbMines;
}

Level::Level(uint width, uint height, uint nbMines)
    : _width(width), _height(height), _nbMines(nbMines)
{
    if ( type()==Custom ) {
        _width   = QMIN(QMAX(_width, MIN_CUSTOM_SIZE), MAX_CUSTOM_SIZE);
        _height  = QMIN(QMAX(_height, MIN_CUSTOM_SIZE), MAX_CUSTOM_SIZE);
        _nbMines = QMIN(QMAX(_nbMines, 1), maxNbMines());
    } else {
        Q_ASSERT( width>=2 && height>=2 );
        Q_ASSERT( nbMines>0 && nbMines<=maxNbMines() );
    }
}

Level::Type Level::type() const
{
    for (uint i=0; i<NbLevels; i++)
        if ( _width==DATA[i].width && _height==DATA[i].height
             && _nbMines==DATA[i].nbMines ) return (Type)i;
    return Custom;
}

const char *KMines::OP_GROUP = "Options";
