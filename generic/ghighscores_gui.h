#ifndef G_HIGHSCORES_GUI_H
#define G_HIGHSCORES_GUI_H

#include <qptrvector.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtabwidget.h>

#include <klistview.h>
#include <kdialogbase.h>
#include <klocale.h>

#include "gsettings.h"
#include "ghighscores_item.h"


class Score;
class PlayerInfos;

//-----------------------------------------------------------------------------
class ShowHighscoresItem : public KListViewItem
{
 public:
    ShowHighscoresItem(QListView *, bool highlight);

 protected:
    virtual void paintCell(QPainter *, const QColorGroup &, int column,
						   int width, int align);

 private:
    bool _highlight;
};

class ShowScoresList : public KListView
{
 Q_OBJECT
 public:
    ShowScoresList(QWidget *parent);

 protected:
    // index==-1 : header
    void addLine(const ItemContainer &, int index, bool highlight);
    virtual bool showColumn(const ItemBase *) const { return true; }
    virtual QString itemText(const ItemBase *, uint row) const = 0;
};

//-----------------------------------------------------------------------------
class ShowHighscoresList : public ShowScoresList
{
 Q_OBJECT
 public:
    ShowHighscoresList(const ItemContainer &, int highlight, QWidget *parent);

 protected:
    QString itemText(const ItemBase *, uint row) const;
};

class ShowHighscoresWidget : public QWidget
{
 Q_OBJECT
 public:
    ShowHighscoresWidget(int localRank, QWidget *parent, const Score &,
                         const PlayerInfos &, int spacingHint);

 private slots:
    void showURL(const QString &) const;
};

//-----------------------------------------------------------------------------
class ShowMultiScoresList : public ShowScoresList
{
 Q_OBJECT
 public:
    ShowMultiScoresList(const QPtrVector<Score> &, QWidget *parent);

 private:
    const QPtrVector<Score> _scores;

    bool showColumn(const ItemBase *) const;
    QString itemText(const ItemBase *, uint row) const;
};

class ShowMultiScoresDialog : public KDialogBase
{
 Q_OBJECT
 public:
    ShowMultiScoresDialog(const QPtrVector<Score> &, QWidget *parent);
};

//-----------------------------------------------------------------------------
class HighscoresSettingsWidget : public BaseSettingsWidget
{
 Q_OBJECT
 public:
    HighscoresSettingsWidget(BaseSettingsDialog *parent, PlayerInfos *);

    bool writeConfig();

 private:
    QCheckBox *_WWHEnabled;
    QLineEdit *_nickname, *_comment;
};

#endif
