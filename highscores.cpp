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
QString ExtHighscores::gameTypeLabel(uint level, LabelType type) const
{
    const Level::Data &data = Level::data((Level::Type)level);
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
    switch ((Level::Type)level) {
    case Level::Easy:     group = "Easy level"; break;
    case Level::Normal:   group = "Normal level"; break;
    case Level::Expert:   group = "Expert level"; break;
    case Level::NbLevels: Q_ASSERT(false);
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

ItemBase *ExtHighscores::scoreItem() const
{
    ItemBase *item = Highscores::scoreItem();
    item->setPrettyFormat(ItemBase::Time);
    return item;
}

ItemBase *ExtHighscores::bestScoreItem() const
{
    ItemBase *item = Highscores::bestScoreItem();
    item->setPrettyFormat(ItemBase::Time);
    return item;
}

ItemBase *ExtHighscores::meanScoreItem() const
{
    ItemBase *item = Highscores::meanScoreItem();
    item->setPrettyFormat(ItemBase::Time);
    return item;
}
