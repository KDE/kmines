#ifndef HIGHSCORES_H
#define HIGHSCORES_H

#include "generic/ghighscores.h"
#include "defines.h"
#include "version.h"


namespace KExtHighscores
{

class ExtHighscores : public Highscores
{
 public:
    ExtHighscores();

 private:
    QString gameTypeLabel(uint gameTye, LabelType) const;
    void convertLegacy(uint gameType);
};

};

#endif
