#ifndef G_SETTINGS_H
#define G_SETTINGS_H

#include <qptrvector.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <kcolorbtn.h>
#include <kconfig.h>


//-----------------------------------------------------------------------------
class SettingsColorButton : public KColorButton
{
 Q_OBJECT
 public:
    SettingsColorButton(QWidget *parent, const char *name = 0)
        : KColorButton(parent, name) { setFixedWidth(100); }
};

//-----------------------------------------------------------------------------
class BaseSettings
{
 public:
    BaseSettings(const QString &name, const QString &icon,
				 const QString &group = QString::null)
		: _name(name), _icon(icon), _group(group) {}

    QString name() const { return _name; }
    QString icon() const { return _icon; }
    KConfig *config() const;

 private:
	const QString _name, _icon, _group;
};

//-----------------------------------------------------------------------------
class BaseSettingsWidget : public QWidget
{
 Q_OBJECT
 public:
    BaseSettingsWidget(BaseSettings *, QWidget *parent, const char *name);
    virtual ~BaseSettingsWidget();

    const BaseSettings *settings() const { return _settings; }

    virtual void readConfig() {}
    virtual bool writeConfig() { return true; } // return true if ok

 public slots:
    virtual void setDefault() {}

 signals:
    void showPage();

 protected:
    KConfig *config() const { return _settings->config(); }

 private:
    BaseSettings *_settings;
};

class BaseAppearanceSettingsWidget : public BaseSettingsWidget
{
 Q_OBJECT
 public:
    BaseAppearanceSettingsWidget(QWidget *parent)
       : BaseSettingsWidget(new BaseSettings(i18n("Appearance"), "appearance"),
                            parent, "appearance_settings") {}
};

class BaseGameSettingsWidget : public BaseSettingsWidget
{
 Q_OBJECT
 public:
    BaseGameSettingsWidget(QWidget *parent)
        : BaseSettingsWidget(new BaseSettings(i18n("Game"), "misc"),
                             parent, "game_settings") {}
};


//-----------------------------------------------------------------------------
class BaseSettingsDialog : public KDialogBase
{
 Q_OBJECT
 public:
	BaseSettingsDialog(QWidget *parent);

 protected:
    void swallow(BaseSettingsWidget *);

 private slots:
    void accept();
    void showPage();

 private:
    QPtrVector<BaseSettingsWidget> _widgets;
};

#endif
