#ifndef HIGHSCORES_H
#define HIGHSCORES_H

#include "ghighscores.h"
#include "defines.h"

//-----------------------------------------------------------------------------
class ExtScore : public Score
{
 public:
    ExtScore(Level, uint score = 0);

    static QString formatScore(uint);
};

//-----------------------------------------------------------------------------
class ExtPlayerInfos : public PlayerInfos
{
 public:
    ExtPlayerInfos(Level);

 private:
    void convertLegacy(Level, const QString &group) const;
};

#endif
