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

#ifndef G_SETTINGS_H
#define G_SETTINGS_H

#include <qvariant.h>
#include <qguardedptr.h>

#include <kdialogbase.h>

class QButtonGroup;


//-----------------------------------------------------------------------------
/**
 * @internal
 *
 * This class is used as a private member of the @ref KSettingGeneric class
 * so that @ref KSettingGeneric does not need to inherit from @ref QObject.
 */
class KSettingProxy : public QObject
{
 Q_OBJECT
 public:
    KSettingProxy();

    void emitChanged() { emit changed(); }

 signals:
    void changed();
};

/**
 * The abstract class for loading and saving a setting.
 */
class KSettingGeneric
{
 public:
    KSettingGeneric();
    virtual ~KSettingGeneric() {}

    /**
     * Load the setting.
     */
    virtual void load() = 0;

    /**
     * Save the setting.
     */
    virtual void save() = 0;

    /**
     * @return true if the setting has been saved successfully. It is
     * sometimes useful to have the possibility to fail (for e.g.
     * when it implies contacting a remote server).
     */
    virtual bool isSaved() const { return true; }

    /**
     * Set the setting to its default.
     */
    virtual void setDefaults() = 0;

    /**
     * @return true if the setting has it default.
     */
    virtual bool hasDefaults() const = 0;

    /**
     * @internal
     */
    KSettingProxy *proxy() { return &_proxy; }

 private:
    KSettingProxy _proxy;

    class KSettingGenericPrivate;
    KSettingGenericPrivate *d;
};

//-----------------------------------------------------------------------------
/**
 * This class manages a list of @ref KSettingGeneric.
 */
class KSettingList : public KSettingGeneric
{
 public:
    KSettingList();

    /**
     * Plug a @ref KSettingGeneric.
     *
     * Note : the setting will be deleted at destruction time.
     */
    void plug(KSettingGeneric *setting);

    void load();
    void save();
    bool isSaved() const;
    void setDefaults();
    bool hasDefaults() const;

 private:
    QPtrList<KSettingGeneric> _settings;
};

//-----------------------------------------------------------------------------
/**
 * This class manages a list of objects that load/save their states from
 * a config file with @ref KConfig.
 */
class KSettingCollection : public KSettingList
{
 public:
    KSettingCollection();

    /**
     * Plug an object (QWidget or KAction).
     *
     * The object should inherit from (default value type in bracket) :
     * <ul>
     * <li> QCheckBox (bool) </li>
     * <li> QLineEdit/KLineEdit (QString) </li>
     * <li> KColorButton (QColor) </li>
     * <li> read-only QComboBox/KComboBox (QString ; use @ref map) </li>
     * <li>
     * <li> KIntNumInput (int) </li>
     * <li> KDoubleNumInput (double) </li>
     * <li> QSpinBox/KIntSpinBox (int) </li>
     * <li> QSlider (int) </li>
     * <li> QButtonGroup (QString ; use @ref map) : only manages
     *      the QRadioButtons </li>
     * <li> KToggleAction (bool) </li>
     * <li> KSelectAction (QString ; use @ref map) </li>
     * <li> KColorComboBox (QColor) </li>
     * <li> KDatePicker (QDate) </li>
     * <li> KFontAction (QString) </li>
     * <li> KFontSizeAction (int) </li>
     * <li> QDateTimeEdit (QDateTime) </li>
     * <li> QDial (int) </li>
     * <li> QTextEdit (QString) </li>
     * <li> KSelector (int) </li>
     * </ul>
     *
     * @param group the group in the config file.
     * @param key the key in the config file.
     * @param def the default value ; its type should be compatible with the
     *     object (see list above). For object that have multiple choices,
     *     the default value should be one of the entry associated with an
     *     item id with the @ref map method (see @ref Widget example).
     */
    void plug(QObject *object, const QString &group, const QString &key,
              const QVariant &def);

    /**
     * Set the entry string that will be saved in the config file when the
     * item is selected (this is needed only for ComboBox, QButtonGroup and
     * KSelectAction).
     *
     * @param object the container object (for eg QComboBox).
     * @param id the item index (as passed to the container object insert()
     * methods).
     * @param entry the string that will be saved (this string should probably
     * not be translated).
     *
     * Note : if the entry from the
     * config file cannot be mapped to the text of an item, it will try to
     * convert it to an index and set the corresponding item.
     */
    void map(const QObject *object, int id, const QString &entry);

    /**
     * @return the value read from the config file corresponding
     * to the given object. It does not modify its current value.
     *
     * NB: if the object has a range (KIntNumInput, KDoubleNumInput,
     * QSpinBox, QSlider, QDial, KSelector), the value returned is constrained to its range.
     */
    QVariant readValue(const QObject *o) const;

    /**
     * @return the id converted from the value read in the config file
     * corresponding to the given object. It only makes sense for object that
     * have multiple choices (for eg QComboBox).
     */
    int readId(const QObject *o) const;

 private:

    class Item : public KSettingGeneric {
    public:
        Item(QObject *object,
             const QString &group, const QString &key, const QVariant &def);
        bool objectRecognized() const { return _type!=NB_TYPES; }

        void map(int id, const QString &entry);
        QVariant currentValue() const;
        void setCurrentValue(const QVariant &);
        QVariant loadValue() const;
        QVariant read() const;
        int readId() const;

        void load();
        void save();
        void setDefaults();
        bool hasDefaults() const;

        bool contains(const QObject *object) const { return object==_obj; }

    private:
        enum Type { CheckBox = 0, ToggleAction,
                    LineEdit, TextEdit, //URLRequester,
                    DatePicker, DateTimeEdit,
                    ColorButton, ButtonGroup,
                    ColorComboBox, ComboBox,
                    SpinBox, Slider, Dial, Selector,
                    IntInput, DoubleInput,
                    FontAction, FontSizeAction, SelectAction,
                    NB_TYPES };

        struct Data {
            const char     *className, *signal;
            QVariant::Type  type;
            bool            multi;
        };
        static const Data DATA[NB_TYPES];

        const QString         _group, _key;
        QGuardedPtr<QObject>  _gobj;
        QObject              *_obj;
        Type                  _type;
        QVariant              _def;
        QMap<int, QString>    _entries;

        bool isMulti() const;
        int mapToId(const QString &entry) const;
        static int findRadioButtonId(const QButtonGroup *group);
    };
    QPtrList<Item> _items;

    class KSettingCollectionPrivate;
    KSettingCollectionPrivate *d;

    Item *find(const QObject *object) const;
};

//-----------------------------------------------------------------------------
/**
 * This is a helper class to make widget that contains a collection of
 * setting widgets.
 *
 * Example of usage :
 * <pre>
 * MiscSettings::MiscSettings()
 *    : KSettingWidget(i18n("Misc"), "misc")
 * {
 *     QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());
 *
 *     QCheckBox *ws = new QCheckBox(i18n("Boat with sails"), this);
 *     top->addWidget(ws);
 *     plug(ws, "Options", "with sails", true);
 *
 *     KIntNumInput *l = new KIntNumInput(i18n("Length"), this);
 *     top->addWidget(l);
 *     plug(l, "Options", "length", 30);
 *
 *     QHBox *hbox = new QHBox(top);
 *     (void)new QLabel(i18n("Type"), hbox)
 *     QComboBox *t = new QComboBox(hbox);
 *     plug(t, "Options", "type", "catmaran");
 *     t->insertItem(i18n("single hull"), 0);
 *     map(t, 0, "single hull");
 *     t->insertItem(i18n("catamaran"), 1);
 *     map(t, 1, "catamaran");
 *     t->insertItem(i18n("trimaran", 2);
 *     map(t, 2, "trimaran");
 * }
 *
 * ...
 *
 * void configureSettings()
 * {
 *     KSettingDialog dialog(this);
 *     dialog.append(new MiscSettings);
 *     ... // other settings widgets
 *     dialog.exec();
 * }
 * </pre>
 * That's it : all the loading/saving/defaulting is done automagically.
 *
 */
class KSettingWidget : public QWidget, public KSettingCollection
{
 Q_OBJECT
 public:
    /**
     * Constructor.
     *
     * @param title the title used by @ref Dialog.
     * @param icon the icon used by @ref Dialog.
     */
    KSettingWidget(const QString &title = QString::null,
                   const QString &icon = QString::null,
                   QWidget *parent = 0, const char *name = 0);

    QString title() const { return _title; }
    QString icon() const { return _icon; }

 private:
    QString _title, _icon;

    class KSettingWidgetPrivate;
    KSettingWidgetPrivate *d;
};

//-----------------------------------------------------------------------------
/**
 * This class is used to display one or several @ref KSettingWidget.
 *
 * See @ref KSettingWidget for an example of usage.
 */
class KSettingDialog : public KDialogBase
{
 Q_OBJECT
 public:
	KSettingDialog(QWidget *parent, const char *name = 0);

    /**
     * Append a @KSettingWidget to the dialog.
     */
    void append(KSettingWidget *widget);

 signals:
    /**
     * Emitted when some settings have been modified and saved (with
     * Apply button or with Ok button).
     */
    void settingsSaved();

 private slots:
    void accept();
    void slotDefault();
    void slotApply();
    void changed();
    void slotAboutToShowPage(QWidget *page);

 private:
    QPtrList<KSettingWidget> _widgets;
    QValueList<bool>         _changed;

    class KSettingDialogPrivate;
    KSettingDialogPrivate *d;

    bool apply();
};

#endif
