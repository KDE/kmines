#ifndef HIGHSCORES_H
#define HIGHSCORES_H

#include "ghighscores.h"
#include "defines.h"

//-----------------------------------------------------------------------------
class ExtScore : public Score, public KMines
{
 public:
    ExtScore(Level, uint score = 0);

    static QString formatScore(uint);
};

//-----------------------------------------------------------------------------
class ExtPlayerInfos : public PlayerInfos, public KMines
{
 public:
    ExtPlayerInfos(Level);

 private:
    Level _level;

    QString highscoresURL() const;
    QString showHighscoresCaption() const;
    void additionnalQueries(KURL &, QueryType) const;
    void convertLegacy(Level, const QString &group) const;
};

#endif
