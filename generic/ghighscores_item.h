/*
    This file is part of the KDE games library
    Copyright (C) 2001 Nicolas Hadacek (hadacek@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef G_HIGHSCORES_ITEM_H
#define G_HIGHSCORES_ITEM_H

#include <qvariant.h>
#include <qdatastream.h>
#include <qnamespace.h>
#include <qdatetime.h>
#include <qvaluevector.h>

#include <klocale.h>


namespace KExtHighscores
{

class ItemArray;
class ScoreInfos;

//-----------------------------------------------------------------------------
/**
 * This class defines how to convert and how to display
 * a highscore element (such as the score, the date, ...) or a player
 * info (such as the player name, the best score, ...).
 */
class Item
{
 public:
    /**
     * Possible display format.
     * <ul>
     * <li> @p NoFormat : no formatting (default) </li>
     * <li> @p OneDecimal : with one decimal (only for Double) </li>
     * <li> @p Percentage : with one decimal + % (only for Double) </li>
     * <li> @p MinuteTime : MM:SS ie 3600 is 00:00, 1 is 59:59 and 0 is
     *      undefined ; this format is used by KMines (only for UInt, Int and
     *      Double) </li>
     * <li> @p DateTime : date and time according to locale (only for
     *      DateTime) </li>
     * </ul>
     */
    enum Format { NoFormat, OneDecimal, Percentage, MinuteTime,
		          DateTime };

    /**
     * Possible special value for display format.
     * <ul>
     * <li> @p NoSpecial : no special value ; a null DateTime is replaced by
     *      "--" (default) </li>
     * <li> ZeroNotDefine : 0 is replaced by "--" (only for UInt, Int and
     *      Double) </li>
     * <li> @p NegativeNotDefined : negative values are replaced by "--" (only
     *      for Int and Double) </li>
     * <li> @p Anonymous : replace the special value @ref ItemBase::ANONYMOUS
     *      by i18n("anonymous") (only for String) </li>
     * </ul>
     */
    enum Special { NoSpecial, ZeroNotDefined, NegativeNotDefined,
                   Anonymous };

    /**
     * Constructor.
     *
     * @param def default value ; the QVariant also gives the type of data.
     * @param label the label corresponding to the item. If empty, the item
     *              is not shown.
     * @param alignment the alignment of the item.
     */
    Item(const QVariant &def = QVariant::Invalid,
             const QString &label = QString::null,
             int alignment = Qt::AlignRight);
    virtual ~Item() {}

    /**
     * Set the display format.
     * @see Format
     */
    void setPrettyFormat(Format);

    /**
     * Set the special value for display.
     * @see Special
     */
    void setPrettySpecial(Special);

    /**
     * @return if the item is shown.
     */
    bool isVisible() const    { return !_label.isEmpty(); }

    /**
     * @return the label.
     */
    QString label() const { return _label; }

    /**
     * @return the alignment.
     */
    int alignment() const { return _alignment; }

    /**
     * @return the default value.
     */
    const QVariant &defaultValue() const { return _default; }

    /**
     * @return the converted value (by default the value is left
     * unchanged) Most of the time you don't need to reimplement this method.
     *
     * @param i the element index ("rank" for score / "id" for player)
     */
	virtual QVariant read(uint i, const QVariant &value) const;

    /**
     * @return the string to be displayed. You may need to reimplement this
     * method for special formatting (different from the standard ones).
     *
     * @param i the element index ("rank" for score / "id" for player)
     */
	virtual QString pretty(uint i, const QVariant &value) const;

 private:
	QVariant _default;
    QString  _label;
    int      _alignment;
    Format   _format;
    Special  _special;

    class ItemPrivate;
    ItemPrivate *d;

    static QString timeFormat(uint);
};

//-----------------------------------------------------------------------------
/**
 * @ref Item for the score. By default no special formating.
 */
class ScoreItem : public Item
{
 public:
    ScoreItem()
        : Item((uint)0, i18n("Score"), Qt::AlignRight) {}
};

/**
 * @ref Item for mean score. By default, only show one decimal and
 * 0 is shown "--"
 */
class MeanScoreItem : public Item
{
 public:
    MeanScoreItem()
        : Item((double)0, i18n("Mean score"), Qt::AlignRight) {
            setPrettyFormat(OneDecimal);
            setPrettySpecial(ZeroNotDefined);
    }
};

/**
 * @ref Item for the best highscore. 0 is shown "--".
 */
class BestScoreItem : public Item
{
 public:
    BestScoreItem()
        : Item((uint)0, i18n("Best score"), Qt::AlignRight) {
            setPrettySpecial(ZeroNotDefined);
    }
};

//-----------------------------------------------------------------------------
/**
 * Manage an array of datas each associated with @ref Item.
 */
class DataArray : private QValueVector<QVariant>
{
 public:
    /**
     * @internal
     * This constuctor is internal. You should never need to construct
     * this class by yourself.
     */
    DataArray(const ItemArray &items);

    /**
     * @internal
     */
    void read(uint i);

    /**
     * @internal
     */
    void write(uint i, uint maxNbLines) const;

    /**
     * @internal
     */
    const ItemArray &items() const { return _items; }

    /**
     * @return the data associated with the named @ref Item.
     */
    const QVariant &data(const QString &name) const;

    /**
     * Set the data associated with the named @ref Item. Note that the
     * value should have the same type than the default value of the @ref
     * Item.
     */
    void setData(const QString &name, const QVariant &value);

 private:
    const ItemArray &_items;

    class DataArrayPrivate;
    DataArrayPrivate *d;
};

//-----------------------------------------------------------------------------
/**
 * Possible score type.
 * @p Won the game has been won.
 * @p Lost the game has been lost.
 * @p BlackMark the game has been aborted.
 */
enum ScoreType { Won = 0, Lost = -1, BlackMark = -2 };


/**
 * This class contains data for a score.
 *
 * @see Highscores
 */
class Score : public DataArray
{
 public:
    /**
     * @internal
     * You should not have to construct this class.
     */
    Score(const ScoreInfos &items, ScoreType type);

    /**
     * @return the game type.
     */
    ScoreType type() const { return _type; }

    /**
     * @return the score value.
     */
    uint score() const { return data("score").toUInt(); }

 private:
    ScoreType _type;

    class ScorePrivate;
    ScorePrivate *d;
};

}; // namespace

#endif
