#include "highscores.h"

#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <kapp.h>


//-----------------------------------------------------------------------------
class ExtScoreItemScore : public ScoreItemScore
{
 public:
    ExtScoreItemScore() {}

    QString pretty(uint i) const {
        return ExtScore::formatScore( readUInt(i) );
    }
};

ExtScore::ExtScore(Level level, uint score)
    : Score(score, QString("scores_") + LEVELS[level].label,
            new ExtScoreItemScore)
{
    Q_ASSERT( level!=Custom );
}

QString ExtScore::formatScore(uint n)
{
    n = 3600 - n;
    return QString::number(n / 60).rightJustify(2, '0') + ':'
        + QString::number(n % 60).rightJustify(2, '0');
}

//-----------------------------------------------------------------------------
class ExtPlayerItemMeanScore : public PlayerItemMeanScore
{
 public:
    ExtPlayerItemMeanScore() {}

    QString pretty(uint i) const {
        double n = readDouble(i);
        if ( n==0 ) return "--";
        return ExtScore::formatScore( (uint)n );
    }
};

class ExtPlayerItemBestScore : public PlayerItemBestScore
{
 public:
    ExtPlayerItemBestScore() {}

    QString pretty(uint i) const {
        uint n = readUInt(i);
        if ( n==0 ) return "--";
        return ExtScore::formatScore(n);
    }
};

ExtPlayerInfos::ExtPlayerInfos(Level level)
    : PlayerInfos(LEVELS[level].label, new ExtPlayerItemBestScore,
                  new ExtPlayerItemMeanScore), _level(level)
{
    if ( !_newPlayer ) return;

    // convert legacy highscores ...
    convertLegacy(Easy, "Easy level");
    convertLegacy(Normal, "Normal level");
    convertLegacy(Expert, "Expert level");
}

QString ExtPlayerInfos::highscoresURL() const
{
    KURL url = URL(Highscores, registeredName());
    addToURL(url, "level", LEVELS[_level].wwLabel);
    return url.url();
}

QString ExtPlayerInfos::showHighscoresCaption() const
{
    return i18n("Highscores : %1").arg(i18n(LEVELS[_level].i18nLabel));
}

void ExtPlayerInfos::additionnalQueries(KURL &url, QueryType type) const
{
    switch (type) {
        case Submit:
            addToURL(url, "level", LEVELS[_level].wwLabel);
            break;
        default: break;
    }
}

void ExtPlayerInfos::convertLegacy(Level level, const QString &group) const
{
    KConfig *config = kapp->config();
    config->setGroup(group);
    QString name = config->readEntry("Name", QString::null);
    if ( !name.isNull() ) {
        if ( name.isEmpty() ) name = i18n("anonymous");
        uint minutes = config->readUnsignedNumEntry("Min", 0);
        uint seconds = config->readUnsignedNumEntry("Sec", 0);
        ExtScore es(level, 3600 - (minutes*60 + seconds));
        es.setName(name);
        es.submit(0, false);
    }
}

