#ifndef HIGHSCORES_H
#define HIGHSCORES_H

#include "ghighscores.h"
#include "defines.h"
#include "version.h"


class ExtScore : public Score
{
 public:
    ExtScore(uint score = 0, uint clicks = 0);
};

class ExtHighscores : public Highscores, public KMines
{
 public:
    ExtHighscores()
        : Highscores(VERSION, WORLD_WIDE_HS_URL, NbLevels) {}

    static QString formatScore(uint);

 private:
    QString gameTypeLabel(uint level, LabelType) const;
    void convertLegacy(uint level) const;
    Score *score() const { return new ExtScore; }
    ItemBase *scoreItemScore() const;
    ItemBase *playerItemBestScore() const;
    ItemBase *playerItemMeanScore() const;
};

#endif
