#include "gsettings.h"
#include "gsettings.moc"

#include <qlayout.h>

#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>


//-----------------------------------------------------------------------------
KConfig *BaseSettings::config() const
{
    KConfig *c = kapp->config();
    c->setGroup(_group);
    return c;
}

//-----------------------------------------------------------------------------
BaseSettingsWidget::BaseSettingsWidget(BaseSettings *settings,
                                       QWidget *parent, const char *name)
    : QWidget(parent, name), _settings(settings)
{
    Q_ASSERT(settings);
}

BaseSettingsWidget::~BaseSettingsWidget()
{
    delete _settings;
}

//-----------------------------------------------------------------------------
BaseSettingsDialog::BaseSettingsDialog(QWidget *parent)
: KDialogBase(IconList, i18n("Configure"), Ok|Cancel|Default, Cancel,
			  parent, "option_dialog", true, true)
{
    setIconListAllVisible(true);
}

void BaseSettingsDialog::swallow(BaseSettingsWidget *w)
{
    w->readConfig();
    QFrame *page = addPage(w->settings()->name(), QString::null,
                           BarIcon(w->settings()->icon(), KIcon::SizeLarge));
    w->reparent(page, 0, QPoint());
    QVBoxLayout *vbox = new QVBoxLayout(page);
    vbox->addWidget(w);
    vbox->addStretch(1);

    connect(w, SIGNAL(showPage()), SLOT(showPage()));
    connect(this, SIGNAL(defaultClicked()), w, SLOT(setDefault()));
    uint s = _widgets.size();
    _widgets.resize(s+1);
    _widgets.insert(s, w);
}

void BaseSettingsDialog::accept()
{
    for (uint i=0; i<_widgets.size(); i++)
        if ( !_widgets[i]->writeConfig() ) return;
    KDialogBase::accept();
}

void BaseSettingsDialog::showPage()
{
    QWidget *page = (QWidget *)sender()->parent();
    qDebug("showPage %i", pageIndex(page));
    KDialogBase::showPage( pageIndex(page) );
}
