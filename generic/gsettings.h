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
#include <qptrlist.h>
#include <qasciidict.h>

#include <kdialogbase.h>

class QDomElement;


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
     * The @ref KConfigBase used to load/save the state is application one.
     */
     KConfigItem(Type type, QVariant::Type valueType,
                 const QCString &group, const QCString &key,
                 const QVariant &def, QObject *parent, const char *name);

    ~KConfigItem();

    /**
     * Set the managed object.
     */
    void setObject(QObject *object);

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
     * @return the value read from the config file.
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
     * @internal
     */
    void checkType(const QVariant &) const;

    /**
     * @internal
     */
    virtual void initObject() {}

 protected:
    /**
     * @internal
     */
    uint objectType() const;

 private:
    const QCString _group, _key;
    QVariant       _def;
    QString        _text, _whatsthis, _tooltip;
    QLabel        *_label;

    KConfigItemPrivate *d;
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
     * @param group the configuration entry group.
     * @param key the configuration entry key.
     * @param def the default value (should be of type @param type).
     */
    KSimpleConfigItem(QVariant::Type type,
                      const QCString &group, const QCString &key,
                      const QVariant &def,
                      QObject *parent = 0, const char *name = 0);

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
     * @param group the configuration entry group.
     * @param key the configuration entry key.
     * @param def the default value (should be of type @param type).
     * @param min the minimum value (should be of type @param type).
     * @param max the maximum value (should be of type @param type).
     */
    KRangedConfigItem(QVariant::Type type,
                      const QCString &group, const QCString &key,
                      const QVariant &def, const QVariant &min,
                      const QVariant &max,
                      QObject *parent = 0, const char *name = 0);

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

    void initObject();
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
     * @param nbItems the number of items (@ref map).
     * @param group the configuration entry group.
     * @param key the configuration entry key.
     * @param def the default value (should be of type QString).
     * @param text the text shown to the user.
     */
    KMultiConfigItem(uint nbItems, const QCString &group, const QCString &key,
                     const QVariant &def,
                     QObject *parent = 0, const char *name = 0);

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
    void map(uint index, const char *entry, const QString &text);

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
    QValueVector<QString> _entries, _items;

    class KMultiConfigItemPrivate;
    KMultiConfigItemPrivate *d;

    int mapToId(const char *entry) const;
    int simpleMapToId(const char *entry) const;
    uint findRadioButtonId(const QButtonGroup *) const;
    void initObject();
};

//-----------------------------------------------------------------------------
/**
 * This class manages a list of @ref KConfigItem.
 */
class KConfigCollection : public KConfigItemBase
{
 Q_OBJECT
 public:
    /**
     * Constructor.
     */
    KConfigCollection(QObject *parent = 0, const char *name = 0);

    ~KConfigCollection();

    /**
     * Should be called once for a given application before accessing any
     * @ref KConfigItem. It reads the XML config file and creates the
     * @ref KConfigItem.
     */
    static void init();

    /**
     * Should be called at application destruction. It deletes the @ref
     * KConfigItem.
     */
    static void cleanUp();

    /**
     * @return the @ref KConfigItem of the given name.
     */
    static KConfigItem *item(const char *name);

    /**
     * Insert a @ref KConfigItemBase in the collection. Use this method
     * only if you know what you are doing.
     */
    void insert(KConfigItemBase *item);

    /**
     * Remove a @ref KConfigItemBase from the collection. Use this method
     * only if you know what you are doing.
     */
    void remove(KConfigItemBase *item);

    /**
     * Associate a @ref KConfigItem with this collection and the given
     * @ref QObject.
     */
    KConfigItem *plug(const char *name, QObject *object);

    /**
     * Dissociate a @ref KConfigItem.
     */
    void unplug(const char *name);

    /**
     * Short hand for <code>item(name)->configValue()</code>
     */
    static QVariant configValue(const char *name);

    /**
     * Short hand for
     * <code>static_cast<KMultiConfigItem *>(item(name))->configItem()</code>
     */
    static uint configIndex(const char *name);

    /**
     * Short hand for
     * <code>static_cast<KRangedConfigItem *>(item(name))->maxValue()</code>
     */
    static QVariant maxValue(const char *name);

    /**
     * Short hand for
     * <code>static_cast<KRangedConfigItem *>(item(name))->minValue()</code>
     */
    static QVariant minValue(const char *name);

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

 private:
    QPtrList<KConfigItemBase> _list;

    class KConfigCollectionPrivate;
    KConfigCollectionPrivate *d;

    static QAsciiDict<KConfigItem> *_items;
    static KConfigItem *createItem(QDomElement &group, QDomElement &entry,
                                   const char *name);
    void _remove(KConfigItemBase *item);
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
 *     configCollection()->plug("with_sails", cb);
 *     top->addWidget(cb);
 *
 *     KIntNumInput *l = new KIntNumInput(this);
 *     configCollection()->plug("boat_length", l);
 *     top->addWidget(l);
 *
 *     QHBox *hbox = new QHBox(top);
 *     QLabel *label = new QLabel(hbox)
 *     KComboBox *t = new KComboBox(hbox);
 *     KConfigItem *mci = configCollection()->plug("boat_type", t);
 *     mci->setProxyLabel(label);
 * }
 * </pre>
 * and some slot connected to the configure menu entry :
 * <pre>
 * void configureSettings()
 * {
 *     KConfigDialog dialog(this);
 *     dialog.append(new MyConfigWidget);
 *     // append other config widgets...
 *     dialog.exec();
 * }
 * </pre>
 *
 * Your main widget constructor should also contain
 * <pre>KConfigCollection::init()</pre> and the destructor
 * <pre>KConfigCollection::cleanUp()</pre>
 *
 * That's it : all the loading/saving/defaulting is done automagically.
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
     */
    KConfigWidget(const QString &title = QString::null,
                  const QString &icon = QString::null,
                  QWidget *parent = 0, const char *name = 0);

    ~KConfigWidget();

    void setTitle(const QString &title) { _title = title; }
    QString title() const { return _title; }
    void setIcon(const QString &icon) { _icon = icon; }
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
