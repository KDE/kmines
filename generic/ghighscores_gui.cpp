#include "ghighscores_gui.h"
#include "ghighscores_gui.moc"

#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qheader.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qgrid.h>

#include <kapplication.h>
#include <kmessagebox.h>
#include <kurllabel.h>
#include <kopenwith.h>
#include <krun.h>

#include "ghighscores.h"


//-----------------------------------------------------------------------------
ShowHighscoresItem::ShowHighscoresItem(QListView *list, bool highlight)
    : KListViewItem(list), _highlight(highlight)
{}

void ShowHighscoresItem::paintCell(QPainter *p, const QColorGroup &cg,
                                   int column, int width, int align)
{
    QColorGroup cgrp(cg);
    if (_highlight) cgrp.setColor(QColorGroup::Text, red);
    KListViewItem::paintCell(p, cgrp, column, width, align);
}

//-----------------------------------------------------------------------------
ShowScoresList::ShowScoresList(QWidget *parent)
    : KListView(parent)
{
    setSelectionMode(QListView::NoSelection);
    setItemMargin(3);
    setAllColumnsShowFocus(true);
    setSorting(-1);
    header()->setClickEnabled(false);
    header()->setMovingEnabled(false);
}

void ShowScoresList::addLine(const ItemContainer &container, int index,
                             bool highlight)
{
    QListViewItem *line
        = (index==-1 ? 0 : new ShowHighscoresItem(this, highlight));
    int i = -1;
    QPtrListIterator<ItemBase> it(container.items());
    while ( it.current() ) {
        const ItemBase *item = it.current();
        ++it;
        if ( !item->shown() || !showColumn(item) ) continue;
        i++;
        if (line) line->setText(i, itemText(item, index));
        else {
            addColumn( item->label() );
            setColumnAlignment(i, item->alignment());
        }
    }
}

//-----------------------------------------------------------------------------
ShowHighscoresList::ShowHighscoresList(const ItemContainer &container,
                                       int highlight, QWidget *parent)
    : ShowScoresList(parent)
{
    uint nb = container.nbEntries();
    for (int j=nb; j>=0; j--)
        addLine(container, (j==(int)nb ? -1 : j), j==highlight);
}

QString ShowHighscoresList::itemText(const ItemBase *item, uint row) const
{
    return item->pretty(row);
}

//-----------------------------------------------------------------------------
ShowHighscoresWidget::ShowHighscoresWidget(int localRank,
         QWidget *parent, const Score &score, const PlayerInfos &player,
         int spacingHint)
    : QWidget(parent, "show_highscores_widget")
{
    QVBoxLayout *vbox = new QVBoxLayout(this, spacingHint);

    QTabWidget *tw = new QTabWidget(this);
    vbox->addWidget(tw);

    QWidget *w;
    if ( score.nbEntries()==0 ) {
        QLabel *lab = new QLabel(i18n("no score entry"), this);
        lab->setAlignment(AlignCenter);
        w = lab;
    } else w = new ShowHighscoresList(score, localRank, this);
    tw->addTab(w, i18n("Best &scores"));

    w = new ShowHighscoresList(player, player.id(), this);
    tw->addTab(w, i18n("&Players"));

    if ( highscores().isWWHSAvailable() ) {
        KURLLabel *urlLabel = new KURLLabel(highscores().highscoresURL(),
                                 i18n("View world-wide highscores"), this);
        connect(urlLabel, SIGNAL(leftClickedURL(const QString &)),
                SLOT(showURL(const QString &)));
        vbox->addWidget(urlLabel);

        urlLabel = new KURLLabel(highscores().playersURL(),
                                 i18n("View world-wide players"), this);
        connect(urlLabel, SIGNAL(leftClickedURL(const QString &)),
                SLOT(showURL(const QString &)));
        vbox->addWidget(urlLabel);
    }
}

void ShowHighscoresWidget::showURL(const QString &url) const
{
    KFileOpenWithHandler foo;
    (void)new KRun(KURL(url));
}

//-----------------------------------------------------------------------------
ShowMultiScoresList::ShowMultiScoresList(const QPtrVector<Score> &scores,
                                         QWidget *parent)
    : ShowScoresList(parent), _scores(scores)
{
    addLine(*scores[0], -1, false);
    for (uint i=0; i<scores.size(); i++)
        addLine(*scores[i], i, false);
}

QString ShowMultiScoresList::itemText(const ItemBase *item, uint row) const
{
    return _scores[row]->prettyData(item->name());
}

bool ShowMultiScoresList::showColumn(const ItemBase *item) const
{
    return item->name()!="rank";
}

//-----------------------------------------------------------------------------
ShowMultiScoresDialog::ShowMultiScoresDialog(const QPtrVector<Score> &scores,
                                             QWidget *parent)
: KDialogBase(Plain, i18n("Multiplayers scores"), Close, Close,
			  parent, "show_multiplayers_score", true, true)
{
    QVBoxLayout *vbox = new QVBoxLayout(plainPage());
    QWidget *list = new ShowMultiScoresList(scores, plainPage());
    vbox->addWidget(list);

    enableButtonSeparator(false);
}

//-----------------------------------------------------------------------------
HighscoresSettingsWidget::HighscoresSettingsWidget(BaseSettingsDialog *parent,
                                                   PlayerInfos *infos)
    : BaseSettingsWidget(new BaseSettings(i18n("Highscores"), "highscores"),
						 parent, "highscores_settings"),
      _WWHEnabled(0)
{
    QVBoxLayout *top = new QVBoxLayout(this, parent->spacingHint());

    QGrid *grid = new QGrid(2, this);
    grid->setSpacing(parent->spacingHint());
    top->addWidget(grid);
    (void)new QLabel(i18n("Nickname"), grid);
    _nickname = new QLineEdit((infos->isAnonymous() ? QString::null
                               : infos->name()), grid);
    _nickname->setMaxLength(16);
    QString name = infos->registeredName();
    if ( !infos->key().isEmpty() && !name.isEmpty() ) {
        (void)new QLabel(i18n("Registered nickname :"), grid);
        (void)new QLabel(name, grid);
    }

    (void)new QLabel(i18n("Comment"), grid);
    _comment = new QLineEdit(infos->comment(), grid);
    _comment->setMaxLength(50);

    if ( highscores().isWWHSAvailable() ) {
        _WWHEnabled
            = new QCheckBox(i18n("world-wide highscores enabled"), this);
        _WWHEnabled->setChecked(infos->WWEnabled());
        top->addWidget(_WWHEnabled);
    }
}

bool HighscoresSettingsWidget::writeConfig()
{
    bool enabled = (_WWHEnabled ? _WWHEnabled->isChecked() : false);
    bool res
        =  highscores().modifySettings(_nickname->text().lower(),
                                       _comment->text(), enabled,
                                       (QWidget *)parent());
    if ( !res ) emit showPage();
    return res;
}
