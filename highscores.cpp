#include "highscores.h"

#include <klocale.h>
#include <kapplication.h>


//-----------------------------------------------------------------------------
ExtScore::ExtScore(uint score, uint clicks)
    : Score(score)
{
    addData("nb_actions", new ItemBase((uint)0, i18n("Clicks"),
                                       Qt::AlignRight), true, clicks);
}

//-----------------------------------------------------------------------------
class ExtScoreItemScore : public ScoreItemScore
{
 public:
    ExtScoreItemScore() {}

    QString pretty(uint i) const {
        return ExtHighscores::formatScore( read(i).toUInt() );
    }
};

//-----------------------------------------------------------------------------
class ExtPlayerItemMeanScore : public PlayerItemMeanScore
{
 public:
    ExtPlayerItemMeanScore() {}

    QString pretty(uint i) const {
        double n = read(i).toDouble();
        if ( n==0 ) return "--";
        return ExtHighscores::formatScore( (uint)n );
    }
};

class ExtPlayerItemBestScore : public PlayerItemBestScore
{
 public:
    ExtPlayerItemBestScore() {}

    QString pretty(uint i) const {
        uint n = read(i).toUInt();
        if ( n==0 ) return "--";
        return ExtHighscores::formatScore(n);
    }
};

//-----------------------------------------------------------------------------
QString ExtHighscores::gameTypeLabel(uint level, LabelType type) const
{
    const LevelData &data = LEVELS[level];
    switch (type) {
    case Icon:
    case Standard: return data.label;
    case I18N:     return i18n(data.i18nLabel);
    case WW:       return data.wwLabel;
    }
    Q_ASSERT(false);
    return QString::null;
};

void ExtHighscores::convertLegacy(uint level) const
{
    QString group;
    switch ((Level)level) {
    case Easy: group = "Easy level"; break;
    case Normal: group = "Normal level"; break;
    case Expert: group = "Expert level"; break;
    case NbLevels: Q_ASSERT(false);
    }

    KConfig *config = kapp->config();
    config->setGroup(group);
    QString name = config->readEntry("Name", QString::null);
    if ( name.isNull() ) return;
    if ( name.isEmpty() ) name = i18n("anonymous");
    uint minutes = config->readUnsignedNumEntry("Min", 0);
    uint seconds = config->readUnsignedNumEntry("Sec", 0);
    int score = 3600 - (minutes*60 + seconds);
    if ( score<=0 ) return;
    Score s(score);
    submitLocal(s, name);
}

QString ExtHighscores::formatScore(uint n)
{
    n = 3600 - n;
    return QString::number(n / 60).rightJustify(2, '0') + ':'
        + QString::number(n % 60).rightJustify(2, '0');
}

ItemBase *ExtHighscores::scoreItemScore() const
{
    return new ExtScoreItemScore;
}

ItemBase *ExtHighscores::playerItemBestScore() const
{
    return new ExtPlayerItemBestScore;
}

ItemBase *ExtHighscores::playerItemMeanScore() const
{
    return new ExtPlayerItemMeanScore;
}
