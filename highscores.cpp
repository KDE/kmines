#include "highscores.h"

#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>


namespace KExtHighscores
{

ExtHighscores::ExtHighscores()
    : Highscores(VERSION, HOMEPAGE, Level::NbLevels)
{
    ScoreItem *scoreItem = new ScoreItem;
    scoreItem->setPrettyFormat(Item::MinuteTime);
    setScoreItem(scoreItem);

    MeanScoreItem *meanScoreItem = new MeanScoreItem;
    meanScoreItem->setPrettyFormat(Item::MinuteTime);
    setMeanScoreItem(meanScoreItem);

    BestScoreItem *bestScoreItem = new BestScoreItem;
    bestScoreItem->setPrettyFormat(Item::MinuteTime);
    setBestScoreItem(bestScoreItem);

    addItemToScore("nb_actions",
                   new Item((uint)0, i18n("Clicks"), Qt::AlignRight));
}

QString ExtHighscores::gameTypeLabel(uint gameType, LabelType type) const
{
    const Level::Data &data = Level::data((Level::Type)gameType);
    switch (type) {
    case Icon:
    case Standard: return data.label;
    case I18N:     return i18n(data.i18nLabel);
    case WW:       return data.wwLabel;
    }
    return QString::null;
};

void ExtHighscores::convertLegacy(uint gameType)
{
    QString group;
    switch ((Level::Type)gameType) {
    case Level::Easy:     group = "Easy level"; break;
    case Level::Normal:   group = "Normal level"; break;
    case Level::Expert:   group = "Expert level"; break;
    case Level::NbLevels: Q_ASSERT(false);
    }

    KConfigGroupSaver cg(kapp->config(), group);
    QString name = cg.config()->readEntry("Name", QString::null);
    if ( name.isNull() ) return;
    if ( name.isEmpty() ) name = i18n("anonymous");
    uint minutes = cg.config()->readUnsignedNumEntry("Min", 0);
    uint seconds = cg.config()->readUnsignedNumEntry("Sec", 0);
    int score = 3600 - (minutes*60 + seconds);
    if ( score<=0 ) return;
    Score *s = newScore(Won);
    s->setData("score", score);
    s->setData("name", name);
    submitLegacyScore();
}

};
