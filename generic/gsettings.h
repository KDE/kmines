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
    void append(KSettingGeneric *setting);

    /**
     * Remove the given @ref KSettingGeneric.
     *
     * Note : the given object will be deleted.
     */
    void remove(KSettingGeneric *setting);

    bool hasDefaults() const;

 protected:
    void loadState();
    bool saveState();
    void setDefaultsState();

 private slots:
    void settingDestroyed(QObject *object);

 private:
    QPtrList<KSettingGeneric> _settings;
};

//-----------------------------------------------------------------------------
class KSettingCollectionPrivate;

/**
 * This class manages a list of objects that load/save their states from
 * a config file with @ref KConfig.
 */
class KSettingCollection : public KSettingList
{
 Q_OBJECT
 public:
    KSettingCollection(QObject *parent = 0);

    ~KSettingCollection();

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
     * Unplug an object.
     */
    void unplug(QObject *object);

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
     * QSpinBox, QSlider, QDial, KSelector), the value returned is constrained
     * to its range.
     */
    QVariant readValue(const QObject *o) const;

    /**
     * @return the id converted from the value read in the config file
     * corresponding to the given object. It only makes sense for object that
     * have multiple choices (for eg QComboBox).
     */
    int readId(const QObject *o) const;

 private slots:
    void objectDestroyed(QObject *object);

 private:
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
 *     QCheckBox *ws = new QCheckBox(i18n("Boat with sails"), this);
 *     top->addWidget(ws);
 *     settings().plug(ws, "Options", "with sails", true);
 *
 *     KIntNumInput *l = new KIntNumInput(i18n("Length"), this);
 *     top->addWidget(l);
 *     settings().plug(l, "Options", "length", 30);
 *
 *     QHBox *hbox = new QHBox(top);
 *     (void)new QLabel(i18n("Type"), hbox)
 *     QComboBox *t = new QComboBox(hbox);
 *     settings().plug(t, "Options", "type", "catmaran");
 *     t->insertItem(i18n("single hull"), 0);
 *     settings().map(t, 0, "single hull");
 *     t->insertItem(i18n("catamaran"), 1);
 *     settings().map(t, 1, "catamaran");
 *     t->insertItem(i18n("trimaran", 2);
 *     settings().map(t, 2, "trimaran");
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
     */
    KSettingWidget(const QString &title = QString::null,
                   const QString &icon = QString::null,
                   QWidget *parent = 0, const char *name = 0);
    ~KSettingWidget();

    QString title() const { return _title; }
    QString icon() const { return _icon; }

    /**
     * @return the @ref KSettingCollection.
     */
    KSettingCollection *settings() { return _settings; }

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
