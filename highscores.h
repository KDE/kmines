#ifndef HIGHSCORES_H
#define HIGHSCORES_H

#include "ghighscores.h"


namespace KExtHighscores
{

class ExtHighscores : public Highscores
{
 public:
    ExtHighscores();

 private:
    QString gameTypeLabel(uint gameTye, LabelType) const;
    void convertLegacy(uint gameType);
    bool isStrictlyLess(const Score &s1, const Score &s2) const;
};

};

#endif
