/*
    This file is part of the KDE games library
    Copyright (C) 2001-02 Nicolas Hadacek (hadacek@kde.org)

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

#include <kdialogbase.h>


//-----------------------------------------------------------------------------
/**
 * The abstract class for loading and saving a setting.
 */
class KSettingGeneric : public QObject
{
 Q_OBJECT
 public:
    KSettingGeneric(QObject *parent = 0);
    virtual ~KSettingGeneric();

    /**
     * Load the setting.
     */
    void load();

    /**
     * Save the setting.
     *
     * @return true if the setting has been saved successfully. It is
     * sometimes useful to have the possibility to fail (for e.g.
     * when it implies contacting a remote server).
     */
    bool save();

    /**
     * Set the setting to its default.
     */
    void setDefaults();

    /**
     * @return true if the setting has its default state.
     */
    virtual bool hasDefaults() const = 0;

 public slots:
    /**
     * Should be called when state has been modified.
     */
    void hasBeenModifiedSlot();

 signals:
    /**
     * Emitted when the setting state has been modified.
     */
    void hasBeenModified();

    /**
     * Emitted when the setting state has been saved.
     */
    void hasBeenSaved();

 protected:
    /**
     * Implement this method to load the setting.
     */
    virtual void loadState() = 0;

    /**
     * Implement this method to save the setting.
     * @return true on success.
     */
    virtual bool saveState() = 0;

    /**
     * Implement this method to set the default state.
     */
    virtual void setDefaultsState() = 0;

 private:
    bool _modified;

    class KSettingGenericPrivate;
    KSettingGenericPrivate *d;
};

//-----------------------------------------------------------------------------
/**
 * This class manages a list of @ref KSettingGeneric.
 */
class KSettingList : public KSettingGeneric
{
 Q_OBJECT
 public:
    KSettingList(QObject *parent = 0);
    ~KSettingList();

    /**
     * Append the given @ref KSettingGeneric.
     *
     * Note : the given object will be deleted at destruction time.
     */
    void insert(KSettingGeneric *setting);

    /**
     * Remove the given @ref KSettingGeneric.
     *
     * Note : the given object will be deleted.
     */
    void remove(KSettingGeneric *setting);

    bool hasDefaults() const;

 protected:
    QPtrList<KSettingGeneric> _settings;

    void loadState();
    bool saveState();
    void setDefaultsState();

 private slots:
    void settingDestroyed(QObject *object);
};

//-----------------------------------------------------------------------------
class KSettingCollection;
class KConfigBase;
class QLabel;
class QButtonGroup;

/**
 * This class mangages an object that load/save its state from a config file.
 */
class KSetting : public KSettingGeneric
{
 Q_OBJECT
 Q_PROPERTY( QString text READ text WRITE setText )
 public:
    KSetting(const QString &group, const QString &key,
             const QVariant &def, KSettingCollection *,
             const QString &text, QVariant::Type);

    /**
     * Associate an object.
     */
    virtual void associate(QObject *object);

    /**
     * Set the label shown to the user.
     */
    void setText(const QString &text);

    /**
     * Set the "proxy" label : the given @QLabel replaces the widget to display
     * the text.
     */
    void setProxyLabel(QLabel *label);

    /**
     * @return the label shown to the user.
     */
    QString text() const { return _text; }

    /**
     * Set the current value.
     */
    virtual void setValue(const QVariant &value);

    /**
     * @return the current value.
     */
    virtual QVariant value() const;

    /**
     * @return the value read from the config file corresponding to the entry.
     * It does not modify the object current value.
     */
    virtual QVariant configValue() const;

    /**
     * @return the managed object.
     */
    QObject *object() const { return _object; }

    /**
     * @return the managed widget. Null if it is not a @ref QWidget.
     */
    QWidget *widget() const;

    /**
     * @return the default value.
     */
    const QVariant &defaultValue() const { return _def; }

 protected:
    struct Data {
        const char *className, *signal, *property, *labelProperty;
        QVariant::Type type;
    };
    virtual const Data &data() const = 0;

    void loadState();
    bool saveState();
    void setDefaultsState();
    bool hasDefaults() const;

    bool checkType(const QVariant &) const;

 private slots:
    void objectDestroyed();

 private:
    QObject       *_object;
    const QString  _group, _key;
    QVariant       _def;
    QString        _text;
    QLabel        *_label;

    KConfigBase *config() const;
};

/**
 * This class manages a simple widget.
 */
class KSimpleSetting : public KSetting
{
 Q_OBJECT
 public:
    /** The type of the associated UI object (default value type in bracket).
     * <ul>
     * <li> QCheckBox (bool) </li>
     * <li> QLineEdit/KLineEdit (QString) </li>
     * <li> KColorButton (QColor) </li>
     * <li> KToggleAction (bool) </li>
     * <li> KColorComboBox (QColor) </li>
     * <li> editable QComboBox/KComboBox (QString) </li>
     * <li> KDatePicker (QDate) </li>
     * <li> KFontAction (QString) </li>
     * <li> KFontSizeAction (int) </li>
     * <li> QDateTimeEdit (QDateTime) </li>
     * <li> QTextEdit (QString) </li>
     * </ul>
     */
     enum Type { CheckBox = 0, LineEdit, ColorButton, ToggleAction,
                 ColorComboBox, EditableComboBox, DatePicker, FontAction,
                 FontSizeAction, DateTimeEdit, TextEdit, NB_TYPES };

    /**
     * Constructor.
     *
     * @param type the type of the associated UI object.
     * @param group the group in the config file.
     * @param key the key in the config file.
     * @param def the default value ; its type should be compatible with the
     *     object (see list above).
     * @param text the text shown to the user.
     *
     * The @ref KConfigBase used to load/save the state is taken from the
     * @ref KSettingCollection. If @collection is null, the application
     * @ref KConfigBase is used.
     */
    KSimpleSetting(Type type, const QString &group, const QString &key,
                   const QVariant &def, KSettingCollection *collection,
                   const QString &text = QString::null);

 private:
    Type _type;

    static const Data DATA[NB_TYPES];
    const Data &data() const { return DATA[_type]; }
};

/**
 * This class manages a ranged widget (KIntNumInput, KDoubleNumInput,
 * QSpinBox/KIntSpinBox, QSlider, QDial, KSelector).
 */
class KRangedSetting : public KSetting
{
 Q_OBJECT
 public:
    /** The type of the associated UI object (default value type in bracket).
     * <ul>
     * <li> KIntNumInput (int) </li>
     * <li> KDoubleNumInput (double) </li>
     * <li> QSpinBox/KIntSpinBox (int) </li>
     * <li> QSlider (int) </li>
     * <li> QDial (int) </li>
     * <li> KSelector (int) </li>
     * </ul>
     */
     enum Type { IntInput = 0, DoubleInput, SpinBox, Slider, Dial,
                 Selector, NB_TYPES };

    /**
     * Constructor.
     *
     * @param type the type of the associated UI object.
     * @param group the group in the config file.
     * @param key the key in the config file.
     * @param def the default value ; its type should be compatible with the
     * type.
     * @param min minimum value.
     * @param max maximum value.
     * @param text the text shown to the user.
     *
     * The @ref KConfigBase used to load/save the state is taken from the
     * @ref KSettingCollection. If @collection is null, the application
     * @ref KConfigBase is used.
     */
    KRangedSetting(Type type, const QString &group, const QString &key,
                   const QVariant &def, const QVariant &min,
                   const QVariant &max, KSettingCollection *collection,
                   const QString &text = QString::null);

    void associate(QObject *object);

    /**
     * Set the range of allowed values.
     */
    void setRange(const QVariant &min, const QVariant &max);

    /**
     * @return the maximum value.
     */
    QVariant maxValue() const;

    /**
     * @return the minimum value.
     */
    QVariant minValue() const;

    /**
     * Bound the given value to the range (see @ref setRange). It does not
     * modify the widget.
     */
    QVariant bound(const QVariant &value) const;

    /**
     * @return the value read from the config file corresponding to the entry.
     * It does not modify the object current value.
     *
     * The returned value is constrained to the range.
     */
    QVariant configValue() const;

 private:
    Type     _type;
    QVariant _min, _max;

    static const Data DATA[NB_TYPES];
    const Data &data() const { return DATA[_type]; }
};

/**
 * This class manages widget that have multiple choices (readonly
 * QComboBox/KComboBox, QButtonGroup, KSelectAction).
 */
class KMultiSetting : public KSetting
{
 Q_OBJECT
 public:
    /**
     * The type of the associated UI object.
     * <ul>
     * <li> read-only QComboBox/KComboBox </li>
     * <li> QButtonGroup : only manages the QRadioButtons </li>
     * <li> KSelectAction </li>
     * </ul>
     */
     enum Type { ReadOnlyComboBox = 0, RadioButtonGroup, SelectAction,
                 NB_TYPES };

    /**
     * Constructor.
     *
     * @param nbItems the number of items
     * @param group the group in the config file.
     * @param key the key in the config file.
     * @param def the default value ; its type should be @ref QVariant::String.
     *            It should be one of the entry associated with an
     *            item id by @ref map method.
     * @param text the text shown to the user.
     *
     * The @ref KConfigBase used to load/save the state is taken from the
     * @ref KSettingCollection. If @collection is null, the application
     * @ref KConfigBase is used.
     */
    KMultiSetting(Type type, uint nbItems,
                  const QString &group, const QString &key,
                  const QVariant &def, KSettingCollection *collection,
                  const QString &text = QString::null);

    /**
     * Set the entry string that will be saved in the config file when the
     * item is selected. This method inserts the appropriate items (for
     * @ref QButtonGroup, you need to create the @ref QRadioButton beforehand)
     * and sets their label.
     *
     * @param id the item index.
     * @param entry the string that will be saved (this string should
     * not be translated).
     * @param text the label shown to the user (shoulg be translated).
     *
     * Note : if the entry from the
     * config file cannot be mapped to the text of an item, it will try to
     * convert it to an index and set the corresponding item.
     */
    void map(int id, const QString &entry, const QString &text);

    void associate(QObject *object);
    void setValue(const QVariant &);
    QVariant value() const;

    /**
     * @return the item id converted from the value read in the config file.
     */
    int configId() const;

 private:
    Type               _type;
    uint               _nbItems;
    QMap<int, QString> _entries;

    static const Data DATA[NB_TYPES];
    const Data &data() const { return DATA[_type]; }

    int mapToId(const QString &entry) const;
    uint findRadioButtonId(const QButtonGroup *) const;
};

//-----------------------------------------------------------------------------
/**
 * This class manages a list of KSetting.
 */
class KSettingCollection : public KSettingList
{
 Q_OBJECT
 public:
    /**
     * Constructor.
     *
     * @param config the @ref KConfigBase object used to load/save entries.
     *               If null, uses kapp->config()
     */
    KSettingCollection(KConfigBase *config = 0, QObject *parent = 0);

    ~KSettingCollection();

    /**
     * @return the @ref KConfigBase object.
     */
    KConfigBase *config() const { return _config; }

    /**
     * @return the @ref KSetting containing the given @ref QObject.
     */
    KSetting *setting(QObject *object) const;

 private:
    KConfigBase  *_config;

    class KSettingCollectionPrivate;
    KSettingCollectionPrivate *d;
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
 *     QCheckBox *cb = new QCheckBox(i18n("Boat with sails"), this);
 *     (void)new KSetting(cb, i18n("Boat with sails"), "Options",
 *                        "with sails", true, settingCollection());
 *     top->addWidget(cb);
 *
 *     KIntNumInput *l = new KIntNumInput(i18n("Length"), this);
 *     KRangedSetting *rs = new KRangedSetting(l, i18n("Length"), Options",
 *                                          "length", 30, settingCollection());
 *     rs->setRange(10, 100);
 *     top->addWidget(l);
 *
 *     QHBox *hbox = new QHBox(top);
 *     (void)new QLabel(i18n("Type"), hbox)
 *     QComboBox *t = new QComboBox(hbox);
 *     KMultipleSetting *ms = new KMultipleSetting(t, i18n("Type"),
 *                         "Options", "type", "catmaran", settingCollection());
 *     ms->map(0, "single_hull", i18n("single hull"));
 *     ms->map(1, "catamaran", 18n("catamaran"));
 *     ms->map(2, "trimaran", i18n("trimaran"));
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
class KSettingWidget : public QWidget
{
 Q_OBJECT
 public:
    /**
     * Constructor.
     *
     * @param title the title used by @ref Dialog.
     * @param icon the icon used by @ref Dialog.
     * @param config the @rf KConfigBase object passed to
     * @ref KSettingCollection.
     */
    KSettingWidget(const QString &title = QString::null,
                   const QString &icon = QString::null,
                   QWidget *parent = 0, const char *name = 0,
                   KConfigBase *config = 0);
    ~KSettingWidget();

    QString title() const { return _title; }
    QString icon() const { return _icon; }

    /**
     * @return the @ref KSettingCollection.
     */
    KSettingCollection *settingCollection() { return _settings; }

 private:
    KSettingCollection *_settings;
    QString             _title, _icon;

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
    ~KSettingDialog();

    /**
     * Append the given @KSettingWidget to the dialog.
     */
    void append(KSettingWidget *widget);

 signals:
    /**
     * Emitted when some settings have been saved (with
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

    class KSettingDialogPrivate;
    KSettingDialogPrivate *d;

    bool apply();
};

#endif
