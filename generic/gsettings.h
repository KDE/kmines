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
 * Abstract base class for loading and saving some setting.
 * This class keeps track of the modification status of the setting.
 */
class KUIConfigBase : public QObject
{
 Q_OBJECT

 public:
    KUIConfigBase(QObject *parent = 0);

    virtual ~KUIConfigBase();

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
    void setDefault();

    /**
     * @return true if the setting has its default state.
     */
    virtual bool hasDefault() const = 0;

 public slots:
    /**
     * Should be called when the setting state has been modified.
     */
    void modifiedSlot();

 signals:
    /**
     * Emitted when the setting state has been modified.
     */
    void modified();

    /**
     * Emitted when the setting state has been saved.
     */
    void saved();

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
    virtual void setDefaultState() = 0;

 private:
    bool _modified;

    class KUIConfigBasePrivate;
    KUIConfigBasePrivate *d;
};

//-----------------------------------------------------------------------------
/**
 * This class manages a list of @ref KUIConfigBase.
 */
class KUIConfigList : public KUIConfigBase
{
 Q_OBJECT

 public:
    KUIConfigList(QObject *parent = 0);

    ~KUIConfigList();

    /**
     * Append the given @ref KUIConfigBase to the list.
     * Note: the list takes ownership of the given @ref KUIConfigBase
     * (it will delete it at destruction time).
     */
    void insert(KUIConfigBase *uiconfig);

    /**
     * Remove the given @ref KSettingGeneric.
     * It deletes the given @ref KUIConfigBase.
     */
    void remove(KUIConfigBase *uiconfig);

    /**
     * @reimplemented
     */
    bool hasDefault() const;

 protected:
    /**
     * @reimplemented
     */
    void loadState();

    /**
     * @reimplemented
     */
    bool saveState();

    /**
     * @reimplemented
     */
    void setDefaultState();

    /**
     * @return the @ref KUIConfigBase list.
     */
    const QPtrList<KUIConfigBase> &list() const { return _list; }

 private slots:
    void UIConfigDestroyed(QObject *object);

 private:
   QPtrList<KUIConfigBase> _list;
};

//-----------------------------------------------------------------------------
class KUIConfigCollection;
class KConfigBase;
class QLabel;
class QButtonGroup;

/**
 * Abstract class that manages an UI control that load/save its state from
 * an entry in a @KConfigBase.
 */
class KUIConfig : public KUIConfigBase
{
 Q_OBJECT
 Q_PROPERTY(QString text READ text WRITE setText)

 public:
    /**
     * Constructor.
     *
     * @param group the group of the configuration entry
     * @param key the key of the configuration entry
     * @param def the default value
     * @param collection the parent collection
     * @param text the text shown in the UI
     * @param type the type of the value of the configuration entry
     *
     * The @ref KConfigBase used to load/save the state is taken from the
     * collection. If @param collection is null, the application
     * @ref KConfigBase is used.
     */
     KUIConfig(const QString &group, const QString &key,
               const QVariant &def, KUIConfigCollection *collection,
               const QString &text, QVariant::Type type);

    /**
     * Associate the UI control that will be managed by this class.
     */
    virtual void associate(QObject *object);

    /**
     * Set the text shown to the user.
     */
    void setText(const QString &text);

    /**
     * Set a "proxy" label : the given @QLabel replaces the widget to display
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
     * @return the value read from the @ref KConfigBase.
     * It does not modify the current value.
     */
    virtual QVariant configValue() const;

    /**
     * @return the managed object.
     */
    QObject *object() const { return _object; }

    /**
     * @return the default value.
     */
    const QVariant &defaultValue() const { return _def; }

 protected:
    struct Data {
        const char *className, *signal, *property, *labelProperty;
        QVariant::Type type;
    };
    /**
     * @internal
     */
    virtual const Data &data() const = 0;

    /**
     * @reimplemented
     */
    void loadState();

    /**
     * @reimplemented
     */
    bool saveState();

    /**
     * @reimplemented
     */
    void setDefaultState();

    /**
     * @reimplemented
     */
    bool hasDefault() const;

    /**
     * @internal
     */
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

    class KUIConfigPrivate;
    KUIConfigPrivate *d;
};

/**
 * This class manages a simple UI control (@see Type).
 */
class KSimpleUIConfig : public KUIConfig
{
 Q_OBJECT
 public:
    /** The type of the associated UI control (default value type in bracket).
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
                 FontSizeAction, DateTimeEdit, TextEdit,
                 Reserved1, Reserved2, Reserved3, Reserved4, Reserved5,
                 NB_TYPES };

    /**
     * Constructor.
     *
     * @param type the type of the associated UI control.
     * @param group the group of the configuration entry.
     * @param key the key of the configuration entry.
     * @param def the default value ; its type should be compatible with the
     * object (see @ref Type).
     * @param text the text shown to the user.
     *
     * The @ref KConfigBase used to load/save the state is taken from the
     * @ref KUIConfigCollection. If @param collection is null, the application
     * @ref KConfigBase is used.
     */
    KSimpleUIConfig(Type type, const QString &group, const QString &key,
                    const QVariant &def, KUIConfigCollection *collection,
                    const QString &text = QString::null);

 private:
    Type _type;

    static const Data DATA[NB_TYPES];
    const Data &data() const { return DATA[_type]; }
};

/**
 * This class manages a ranged UI control (@see Type).
 */
class KRangedUIConfig : public KUIConfig
{
 Q_OBJECT
 public:
    /** The type of the associated UI control (default value type in bracket).
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
                 Selector,
                 Reserved1, Reserved2, Reserved3, Reserved4, Reserved5,
                 NB_TYPES };

    /**
     * Constructor.
     *
     * @param type the type of the associated UI control.
     * @param group the group of the configuration entry.
     * @param key the key of the configuration entry.
     * @param def the default value ; its type should be compatible with the
     * type (see @ref Type).
     * @param min the minimal value.
     * @param max the maximal value.
     * @param text the text shown to the user.
     *
     * The @ref KConfigBase used to load/save the state is taken from the
     * @ref KUIConfigCollection. If @param collection is null, the application
     * @ref KConfigBase is used.
     */
    KRangedUIConfig(Type type, const QString &group, const QString &key,
                    const QVariant &def, const QVariant &min,
                    const QVariant &max, KUIConfigCollection *collection,
                    const QString &text = QString::null);

    /**
     * @reimplemented
     */
    void associate(QObject *object);

    /**
     * Set the range of allowed values.
     */
    void setRange(const QVariant &min, const QVariant &max);

    /**
     * @return the maximal value.
     */
    QVariant maxValue() const;

    /**
     * @return the minimal value.
     */
    QVariant minValue() const;

    /**
     * Bound the given value to the range (see @ref setRange). It does not
     * modify the current value.
     */
    QVariant bound(const QVariant &value) const;

    /**
     * @reimplemented
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
 * This class manages an UI control that have multiple choices (@see Type).
 */
class KMultiUIConfig : public KUIConfig
{
 Q_OBJECT
 public:
    /**
     * The type of the associated UI control.
     * <ul>
     * <li> read-only QComboBox/KComboBox </li>
     * <li> QButtonGroup : only manages the QRadioButtons </li>
     * <li> KSelectAction </li>
     * </ul>
     */
     enum Type { ReadOnlyComboBox = 0, RadioButtonGroup, SelectAction,
                 Reserved1, Reserved2, Reserved3, Reserved4, Reserved5,
                 NB_TYPES };

    /**
     * Constructor.
     *
     * @param type the type of the associated UI control.
     * @param nbItems the number of items.
     * @param group the group of the configuration entry.
     * @param key the key of the configuration entry.
     * @param def the default value; its type should be @ref QVariant::String.
     * It should be one of the entries associated with item index by @ref map.
     * @param text the text shown to the user.
     *
     * The @ref KConfigBase used to load/save the state is taken from the
     * @ref KUIConfigCollection. If @param collection is null, the application
     * @ref KConfigBase is used.
     */
    KMultiUIConfig(Type type, uint nbItems,
                   const QString &group, const QString &key,
                   const QVariant &def, KUIConfigCollection *collection,
                   const QString &text = QString::null);

    /**
     * Set the entry string that will be saved in the config file when the
     * item is selected. This method inserts the appropriate items (for
     * @ref QButtonGroup, you need to create the @ref QRadioButton beforehand).
     *
     * @param index the item index.
     * @param entry the string that will be saved (this string should
     * not be translated).
     * @param text the text shown to the user (should be translated).
     *
     * Note: if the value read from @ref KConfigBase is not one of the
     * given entries, the class will try to convert it to an index and set the
     * corresponding item.
     */
    void map(int index, const QString &entry, const QString &text);

    /**
     * @reimplemented
     */
    void associate(QObject *object);

    /**
     * @reimplemented
     */
    void setValue(const QVariant &);

    /**
     * @reimplemented
     */
    QVariant value() const;

    /**
     * @return the item index converted from the value read from the
     * @KConfigBase. It does not modify the current value.
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
 * This class manages a list of @ref KUIConfig.
 */
class KUIConfigCollection : public KUIConfigList
{
 Q_OBJECT
 public:
    /**
     * Constructor.
     *
     * @param config the @ref KConfigBase object used to load/save entries.
     * If null, it uses <pre>kapp->config()</pre>.
     */
    KUIConfigCollection(KConfigBase *config = 0, QObject *parent = 0);

    ~KUIConfigCollection();

    /**
     * @return the @ref KConfigBase object.
     */
    KConfigBase *config() const;

    /**
     * @return the @ref KUIConfig containing the given @ref QObject.
     */
    KUIConfig *UIConfig(QObject *object) const;

 private:
    KConfigBase  *_config;

    class KUIConfigCollectionPrivate;
    KUIConfigCollectionPrivate *d;
};

//-----------------------------------------------------------------------------
/**
 * This is a helper class to make widget that contains a collection of
 * UI control (for example a page in a configuration dialog).
 *
 * Example of usage :
 * <pre>
 * MyConfigWidget::MyConfigWidget()
 *    : KUIConfigWidget(i18n("Misc"), "misc")
 * {
 *     QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());
 *
 *     QCheckBox *cb = new QCheckBox(this);
 *     KSimpleUIConfig *suc =
 *         new KSimpleUIConfig(KSimpleUIConfig::CheckBox,
 *                             "Options", "with sails", true,
 *                             UIConfigCollection(), i18n("Boat with sails"));
 *     uc->associate(cb);
 *     top->addWidget(cb);
 *
 *     KIntNumInput *l = new KIntNumInput(this);
 *     KRangedUIConfig *ruc =
 *         new KRangedUIConfig(KRangedUIConfig::IntInput,
 *                             "Options", "length", 30, 10, 100,
 *                             UIConfigCollection(), i18n("Length"));
 *     ruc->associate(l);
 *     top->addWidget(l);
 *
 *     QHBox *hbox = new QHBox(top);
 *     QLabel *label = new QLabel(hbox)
 *     KComboBox *t = new KComboBox(hbox);
 *     KMultiUIConfig *muc =
 *         new KMultiUIConfig(KMultiUIConfig::ReadOnlyComboBox, 3,
 *                            "Options", "type", "catamaran",
 *                            UIConfigCollection(), i18n("Type"));
 *     muc->associate(t);
 *     muc->setProxyLabel(label);
 *     muc->map(0, "single_hull", i18n("single hull"));
 *     muc->map(1, "catamaran", 18n("catamaran"));
 *     muc->map(2, "trimaran", i18n("trimaran"));
 * }
 *
 * ...
 *
 * void configureSettings()
 * {
 *     KUIConfigDialog dialog(this);
 *     dialog.append(new MyConfigWidget);
 *     // append other config widgets...
 *     dialog.exec();
 * }
 * </pre>
 *
 * That's it : all the loading/saving/defaulting is done automagically.
 *
 */
class KUIConfigWidget : public QWidget
{
 Q_OBJECT
 public:
    /**
     * Constructor.
     *
     * @param title the title used by @ref KUIConfigDialog.
     * @param icon the icon used by @ref KUIConfigDialog.
     * @param config the @rf KConfigBase object passed to
     * @ref KUIConfigCollection.
     */
    KUIConfigWidget(const QString &title = QString::null,
                    const QString &icon = QString::null,
                    QWidget *parent = 0, const char *name = 0,
                    KConfigBase *config = 0);

    ~KUIConfigWidget();

    QString title() const { return _title; }
    QString icon() const { return _icon; }

    /**
     * @return the @ref KUIConfigCollection.
     */
    KUIConfigCollection *UIConfigCollection() { return _UIConfigCollection; }

 private:
    KUIConfigCollection *_UIConfigCollection;
    QString             _title, _icon;

    class KUIConfigWidgetPrivate;
    KUIConfigWidgetPrivate *d;
};

//-----------------------------------------------------------------------------
/**
 * This class is used to display one or several @ref KUIConfigWidget.
 *
 * See @ref KUIConfigWidget for an example of usage.
 */
class KUIConfigDialog : public KDialogBase
{
 Q_OBJECT
 public:
	KUIConfigDialog(QWidget *parent, const char *name = 0);

    ~KUIConfigDialog();

    /**
     * Append the given @KUIConfigWidget to the dialog.
     */
    void append(KUIConfigWidget *widget);

 signals:
    /**
     * Emitted when some @ref KUIConfig have been saved (with
     * Apply button or with Ok button).
     */
    void saved();

 private slots:
    void accept();
    void slotDefault();
    void slotApply();
    void modified();
    void aboutToShowPageSlot(QWidget *page);

 private:
    QPtrList<KUIConfigWidget> _widgets;

    class KUIConfigDialogPrivate;
    KUIConfigDialogPrivate *d;

    bool apply();
};

#endif
