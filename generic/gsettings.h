/*
    This file is part of the KDE library
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

#ifndef KCONFIGITEM_H
#define KCONFIGITEM_H

#include <qvariant.h>
#include <qvaluevector.h>

#include <kdialogbase.h>


//-----------------------------------------------------------------------------
/**
 * Abstract base class for loading and saving some setting.
 * This class keeps track of the modification status of the setting.
 */
class KConfigItemBase : public QObject
{
 Q_OBJECT

 public:
    KConfigItemBase(QObject *parent = 0, const char *name = 0);

    virtual ~KConfigItemBase();

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

    class KConfigItemBasePrivate;
    KConfigItemBasePrivate *d;
};

//-----------------------------------------------------------------------------
/**
 * This class manages a list of @ref KConfigItemBase.
 */
class KConfigItemList : public KConfigItemBase
{
 Q_OBJECT

 public:
    KConfigItemList(QObject *parent = 0, const char *name = 0);

    ~KConfigItemList();

    /**
     * Append the given @ref KConfigItemBase to the list.
     * Note: the list takes ownership of the given @ref KConfigItemBase
     * (it will delete it at destruction time).
     */
    void insert(KConfigItemBase *uiconfig);

    /**
     * Remove the given @ref KSettingGeneric.
     * It deletes the given @ref KConfigItemBase.
     */
    void remove(KConfigItemBase *uiconfig);

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
     * @return the @ref KConfigItemBase list.
     */
    const QPtrList<KConfigItemBase> &list() const { return _list; }

 private slots:
    void itemDestroyed(QObject *object);

 private:
   QPtrList<KConfigItemBase> _list;
};

//-----------------------------------------------------------------------------
class KConfigCollection;
class KConfigBase;
class QLabel;
class QButtonGroup;
class KConfigItemPrivate;

/**
 * Abstract class that manages an UI control that load/save its state from
 * an entry in a @KConfigBase.
 */
class KConfigItem : public KConfigItemBase
{
 Q_OBJECT
 Q_PROPERTY(QString text READ text WRITE setText)
 Q_PROPERTY(QString whatsThis READ whatsThis WRITE setWhatsThis)
 Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip)

 public:
    /**
     * @internal
     */
    enum Type { Simple = 0, Ranged, Multi, NB_ITEM_TYPES };

     /**
     * @internal
     *
     * The @ref KConfigBase used to load/save the state is taken from the
     * collection. If @param collection is null, the application
     * @ref KConfigBase is used.
     */
     KConfigItem(Type type, QVariant::Type valueType, QObject *object,
                 const QString &group, const QString &key,
                 const QVariant &def, const QString &text,
                 KConfigCollection *collection, const char *name);

    ~KConfigItem();

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
     * Set the What's this text.
     */
    void setWhatsThis(const QString &text);
    /**
     * @return the What's this text.
     */
    QString whatsThis() const { return _whatsthis; }

    /**
     * Set the Tool Tip text.
     */
    void setToolTip(const QString &text);
    /**
     * @return the Tool Tip text.
     */
    QString toolTip() const { return _tooltip; }

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
     * @return the default value.
     */
    const QVariant &defaultValue() const { return _def; }

    /**
     * @return the managed object.
     */
    QObject *object() const;

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
     * @reimplemented
     */
    bool hasDefault() const;

    /**
     * @internal
     */
    bool checkType(const QVariant &) const;

 private slots:
    void objectDestroyed();

 protected:
    /**
     * @internal
     */
    uint objectType() const;

 private:
    const QString  _group, _key;
    QVariant       _def;
    QString        _text, _whatsthis, _tooltip;
    QLabel        *_label;

    KConfigItemPrivate *d;

    KConfigBase *config() const;
};

/**
 * This class manages a simple UI control. The supported controls are:
 * <ul>
 * <li> for bool: QCheckBox </li>
 * <li> for QString: Q/KLineEdit, editable Q/KComboBox, QFontAction,
 *      QTextEdit</li>
 * <li> for QColor: KColorButton, KColorComboBox </li>
 * <li> for QDate: KDatePicker </li>
 * <li> for QDateTime: QDateTimeEdit </li>
 * <li> for int : KFontSizeAction </li>
 * </ul>
 */
class KSimpleConfigItem : public KConfigItem
{
 Q_OBJECT
 public:
    /**
     * Constructor.
     *
     * @param type the value type.
     * @param object the managed control.
     * @param group the configuration entry group.
     * @param key the configuration entry key.
     * @param def the default value (should be of type @param type).
     * @param text the text shown to the user.
     */
    KSimpleConfigItem(QVariant::Type type, QObject *object,
                      const QString &group, const QString &key,
                      const QVariant &def, const QString &text = QString::null,
                      KConfigCollection *collection = 0, const char *name = 0);

    ~KSimpleConfigItem();

 private:
    class KSimpleConfigItemPrivate;
    KSimpleConfigItemPrivate *d;
};

/**
 * This class manages a ranged UI control. The supported controls are:
 * <ul>
 * <li> for int: KIntNumInput, Q/KSpinbox, QSlider, QDial, KSelector </li>
 * <li> for double: KDoubleNumInput </li>
 * </ul>
 */
class KRangedConfigItem : public KConfigItem
{
 Q_OBJECT
 public:
    /**
     * Constructor.
     *
     * @param type the value type.
     * @param object the managed control.
     * @param group the configuration entry group.
     * @param key the configuration entry key.
     * @param def the default value (should be of type @param type).
     * @param min the minimum value (should be of type @param type).
     * @param max the maximum value (should be of type @param type).
     * @param text the text shown to the user.
     */
    KRangedConfigItem(QVariant::Type type, QObject *object,
                      const QString &group, const QString &key,
                      const QVariant &def, const QVariant &min,
                      const QVariant &max, const QString &text = QString::null,
                      KConfigCollection *collection = 0, const char *name = 0);

    ~KRangedConfigItem();

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
    QVariant _min, _max;

    class KRangedConfigItemPrivate;
    KRangedConfigItemPrivate *d;
};

/**
 * This class manages an UI control that have multiple choices. The supported
 * controls are: read-only Q/KComboBox, QButtonGroup (only QRadioButtons are
 * managed) and KSelectAction. The value type is QString.
 */
class KMultiConfigItem : public KConfigItem
{
 Q_OBJECT
 public:
    /**
     * Constructor.
     *
     * @param object the managed control.
     * @param nbItems the number of items (@ref map).
     * @param group the configuration entry group.
     * @param key the configuration entry key.
     * @param def the default value (should be of type QString).
     * @param text the text shown to the user.
     */
    KMultiConfigItem(QObject *object, uint nbItems,
                     const QString &group, const QString &key,
                     const QVariant &def, const QString &text = QString::null,
                     KConfigCollection *collection = 0, const char *name = 0);

    ~KMultiConfigItem();

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
    void map(int index, const char *entry, const QString &text);

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
    int configIndex() const;

 private:
    QValueVector<QString> _entries;

    class KMultiConfigItemPrivate;
    KMultiConfigItemPrivate *d;

    int mapToId(const char *entry) const;
    uint findRadioButtonId(const QButtonGroup *) const;
};

//-----------------------------------------------------------------------------
class QDomDocument;

/**
 * This class manages a list of @ref KConfigItem.
 */
class KConfigCollection : public KConfigItemList
{
 Q_OBJECT
 public:
    /**
     * Constructor.
     *
     * @param config the @ref KConfigBase object used to load/save entries.
     * If null, it uses <pre>kapp->config()</pre>.
     */
    KConfigCollection(KConfigBase *config = 0, QObject *parent = 0,
                      const char *name = 0);

    ~KConfigCollection();

    /**
     * @return the @ref KConfigBase object.
     */
    KConfigBase *config() const;

    /**
     * @return the @ref KConfigItem of the given name.
     */
    KConfigItem *configItem(const char *name) const;

    /**
     * @return the value of config item of the given name as stored in the
     * configuration file. If the collection already contains the
     * @ref KConfigItem, it is more efficient to use
     * <pre>configItem(name)->configValue()</pre>
     */
    static QVariant configItemValue(const char *name);

    /**
     * @return the index of the config item of the given name as stored in
     * the configuration file (the config item should be a
     * @ref KMultiConfigItem). If the collection already contains the
     * @ref KConfigItem, it is more efficient to use
     * <pre>static_cast<KMultiConfigItem *>(configItem(name))->configIndex()</pre>
     */
    static uint configItemIndex(const char *name);

    /**
     * Create a @ref KConfigItem from the XML config file and associate it
     * with the given object.
     */
    KConfigItem *createConfigItem(const char *name, QObject *object);

    static KConfigItem *createConfigItem(const char *name, QObject *object,
                                         KConfigCollection *collection);
 private:
    KConfigBase  *_config;
    static QDomDocument *_xml;

    static void readConfigFile();

    class KConfigCollectionPrivate;
    KConfigCollectionPrivate *d;
};

//-----------------------------------------------------------------------------
/**
 * This is a helper class to make widget that contains a collection of
 * UI control (for example a page in a configuration dialog).
 *
 * Example of usage:
 * the XML config file "myappconfig.rc" contains:
 * <pre>
 * <!DOCTYPE kconfig>
 * <kconfig name="myapp">
 * <Group name="Options">
 *   <Entry name="with_sails" key="with sails" vtype="bool"
 *          defaultValue="true">
 *   <text>Boat with sails</text>
 *   </Entry>
 *
 *   <Entry name="boat_length" key="length" type="ranged" vtype="int"
            defaultValue="30" minValue="10" maxValue="100">
 *   <text>Length</text>
 *   </Entry>
 *
 *   <Entry name="boat_type" key="type" type="multi" defaultValue="catamaran">
 *   <text>Type</text>
 *   <Item name="single hull"><text>Single hull</text></Item>
 *   <Item name="catamaran"><text>Catamaran</text></Item>
 *   <Item name="trimaran"><text>Trimaran</text></Item>
 *   </Entry>
 * </Group>
 * </kconfig>
 * </pre>
 *
 * and the corresponding code is:
 * <pre>
 * MyConfigWidget::MyConfigWidget()
 *    : KConfigWidget(i18n("Misc"), "misc")
 * {
 *     QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());
 *
 *     QCheckBox *cb = new QCheckBox(this);
 *     configCollection()->createConfigItem("with_sails", cb);
 *     top->addWidget(cb);
 *
 *     KIntNumInput *l = new KIntNumInput(this);
 *     configCollection()->createConfigItem("boat_length", l);
 *     top->addWidget(l);
 *
 *     QHBox *hbox = new QHBox(top);
 *     QLabel *label = new QLabel(hbox)
 *     KComboBox *t = new KComboBox(hbox);
 *     KMultiConfigItem *mci =
 *         configCollection()->createConfigItem("boat_type", t);
 *     mci->setProxyLabel(label);
 * }
 *
 * ...
 *
 * void configureSettings()
 * {
 *     KConfigDialog dialog(this);
 *     dialog.append(new MyConfigWidget);
 *     // append other config widgets...
 *     dialog.exec();
 * }
 * </pre>
 *
 * That's it : all the loading/saving/defaulting is done automagically.
 *
 */
class KConfigWidget : public QWidget
{
 Q_OBJECT
 public:
    /**
     * Constructor.
     *
     * @param title the title used by @ref KConfigDialog.
     * @param icon the icon used by @ref KConfigDialog.
     * @param config the @rf KConfigBase object passed to
     * @ref KConfigCollection.
     */
    KConfigWidget(const QString &title = QString::null,
                    const QString &icon = QString::null,
                    QWidget *parent = 0, const char *name = 0,
                    KConfigBase *config = 0);

    ~KConfigWidget();

    QString title() const { return _title; }
    QString icon() const { return _icon; }

    /**
     * @return the @ref KConfigCollection.
     */
    KConfigCollection *configCollection() { return _configCollection; }

 private:
    KConfigCollection *_configCollection;
    QString            _title, _icon;

    class KConfigWidgetPrivate;
    KConfigWidgetPrivate *d;
};

//-----------------------------------------------------------------------------
/**
 * This class is used to display one or several @ref KConfigWidget.
 *
 * See @ref KConfigWidget for an example of usage.
 */
class KConfigDialog : public KDialogBase
{
 Q_OBJECT
 public:
	KConfigDialog(QWidget *parent, const char *name = 0);

    ~KConfigDialog();

    /**
     * Append the given @KConfigWidget to the dialog.
     */
    void append(KConfigWidget *widget);

 signals:
    /**
     * Emitted when some @ref KConfigItem have been saved (with
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
    QPtrList<KConfigWidget> _widgets;

    class KConfigDialogPrivate;
    KConfigDialogPrivate *d;

    bool apply();
};

#endif
