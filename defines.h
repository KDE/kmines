#ifndef DEFINES_H
#define DEFINES_H

#include <qcolor.h>


class Level
{
 public:
    enum Type { Easy = 0, Normal, Expert, NbLevels, Custom = NbLevels };
    struct Data {
        uint        width, height, nbMines;
        const char *label, *wwLabel, *i18nLabel;
    };

    Level(Type);
    Level(uint width, uint height, uint nbMines);

    static const Data &data(Type type) { return DATA[type]; }
    static QCString actionName(Type type)
        { return QCString("level_") + data(type).label; }

    uint width() const   { return _width; }
    uint height() const  { return _height; }
    uint nbMines() const { return _nbMines; }
    Type type() const;
    const Data &data() const { return data(type()); }
    uint maxNbMines() const  { return _width*_height - 2; }

    static const uint MAX_CUSTOM_SIZE = 50;
    static const uint MIN_CUSTOM_SIZE = 5;

 private:
    static const Data DATA[NbLevels+1];
    uint _width, _height, _nbMines;
};

class KMines
{
 public:
    enum CaseState { Covered, Uncovered, Uncertain, Marked, Exploded, Error };
    struct Case {
        bool      mine;
        CaseState state;
    };

    enum GameState   { Stopped, Playing, Paused };
    enum MouseAction { Reveal = 0, AutoReveal, Mark, UMark, None };
    enum MouseButton { Left = 0, Mid, Right };

    enum Color { FlagColor = 0, ExplosionColor, ErrorColor, NB_COLORS };
    enum NumberColor { NB_NUMBER_COLORS = 8 };

    struct CaseProperties {
        uint size;
        QColor numberColors[NB_NUMBER_COLORS];
        QColor colors[NB_COLORS];
    };

    static const char *OP_GROUP;
};

#endif
