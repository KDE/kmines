#include "dialogs.h"
#include "dialogs.moc"

#include <qpixmap.h>
#include <qvgroupbox.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qgrid.h>
#include <qlabel.h>

#include <klocale.h>

#include "bitmaps/smile"
#include "bitmaps/smile_happy"
#include "bitmaps/smile_ohno"
#include "bitmaps/smile_stress"
#include "bitmaps/smile_sleep"


//-----------------------------------------------------------------------------
const char **Smiley::XPM_NAMES[Smiley::NbPixmaps] = {
    smile_xpm, smile_stress_xpm, smile_happy_xpm, smile_ohno_xpm,
    smile_sleep_xpm
};

void Smiley::setMood(Mood mood)
{
    QPixmap p(XPM_NAMES[mood]);
    setPixmap(p);
}

//-----------------------------------------------------------------------------
LCDNumber::LCDNumber(QWidget *parent, const char *name)
: QLCDNumber(parent, name)
{
	setFrameStyle( QFrame::Panel | QFrame::Sunken );
	setSegmentStyle(Flat);
    setColor(white);
}

void LCDNumber::setColor(QColor color)
{
	QPalette p = palette();
	p.setColor(QColorGroup::Background, black);
    p.setColor(QColorGroup::Foreground, color);
	setPalette(p);
}

//-----------------------------------------------------------------------------
DigitalClock::DigitalClock(QWidget *parent)
: LCDNumber(parent, "digital_clock")
{}

KExtHighscores::Score DigitalClock::score() const
{
    KExtHighscores::Score score(KExtHighscores::Won);
    score.setData("score", 3600 - (_min*60 + _sec));
    score.setData("nb_actions", _nbActions);
    return score;
}

void DigitalClock::timerEvent(QTimerEvent *)
{
 	if (_stop) return;

    if ( _min==59 && _sec==59 ) return; // waiting an hour do not restart timer
    _sec++;
    if (_sec==60) {
        _min++;
        _sec = 0;
    }
    showTime();

    if ( _first<score() ) setColor(red);
    else if ( _last<score() ) setColor(blue);
    else setColor(white);
}

void DigitalClock::showTime()
{
	char s[6] = "00:00";
	if (_min>=10) s[0] += _min / 10;
	s[1] += _min % 10;
	if (_sec>=10) s[3] += _sec / 10;
	s[4] += _sec % 10;

	display(s);
}

void DigitalClock::reset(const KExtHighscores::Score &first,
                         const KExtHighscores::Score &last)
{
	killTimers();

	_stop = true;
	_sec = 0;
    _min = 0;
    _nbActions = 0;
    _first = first;
    _last = last;
	startTimer(1000); // one second

	setColor(white);
	showTime();
}

//-----------------------------------------------------------------------------
CustomSettings::CustomSettings()
    : KSettingWidget(i18n("Custom game"), "configure")
{
	QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    _width = createWidth(this);
	connect(_width, SIGNAL(valueChanged(int)), SLOT(updateNbMines()));
	top->addWidget(_width);

    _height = createHeight(this);
	connect(_height, SIGNAL(valueChanged(int)), SLOT(updateNbMines()));
	top->addWidget(_height);

    _mines = createMines(this);
	connect(_mines, SIGNAL(valueChanged(int)), SLOT(updateNbMines()));
	top->addWidget(_mines);

    top->addSpacing(2 * KDialog::spacingHint());

    // combo to choose level
    QHBoxLayout *hbox = new QHBoxLayout(top);
    QLabel *label = new QLabel(i18n("Choose level"), this);
    hbox->addWidget(label);
    _gameType = new QComboBox(false, this);
    connect(_gameType, SIGNAL(activated(int)), SLOT(typeChosen(int)));
    for (uint i=0; i<=Level::NbLevels; i++)
        _gameType->insertItem(i18n(Level::data((Level::Type)i).i18nLabel));
    hbox->addWidget(_gameType);
}

void CustomSettings::updateNbMines()
{
    Level level(_width->value(), _height->value(), _mines->value());
	_mines->setRange(1, level.maxNbMines());
	uint nb = level.width() * level.height();
	_mines->setLabel(i18n("Mines (%1%)").arg(100*level.nbMines()/nb));
    _gameType->setCurrentItem(level.type());
}

void CustomSettings::typeChosen(int i)
{
    blockSignals(true);
    Level level((Level::Type)i);
    _width->setValue(level.width());
    _height->setValue(level.height());
    _mines->setValue(level.nbMines());
    blockSignals(false);
    updateNbMines();
}

KIntNumInput *CustomSettings::createWidth(KSettingWidget *sw)
{
    KIntNumInput *w = new KIntNumInput(sw);
    w->setLabel(i18n("Width"));
    w->setRange(Level::MIN_CUSTOM_SIZE, Level::MAX_CUSTOM_SIZE);
    sw->plug(w, OP_GROUP, "custom width", Level::data(Level::Custom).width);
    return w;
}

KIntNumInput *CustomSettings::createHeight(KSettingWidget *sw)
{
    KIntNumInput *h = new KIntNumInput(sw);
    h->setLabel(i18n("Height"));
    h->setRange(Level::MIN_CUSTOM_SIZE, Level::MAX_CUSTOM_SIZE);
    sw->plug(h, OP_GROUP, "custom height", Level::data(Level::Custom).height);
    return h;
}

KIntNumInput *CustomSettings::createMines(KSettingWidget *sw)
{
    KIntNumInput *m = new KIntNumInput(sw);
    sw->plug(m, OP_GROUP, "custom mines", Level::data(Level::Custom).nbMines);
    return m;
}

Level CustomSettings::readLevel()
{
    KSettingWidget sw;
    KIntNumInput *i = createWidth(&sw);
    uint w = sw.readValue(i).toUInt();
    i = createHeight(&sw);
    uint h = sw.readValue(i).toUInt();
    i = createMines(&sw);
    uint n = sw.readValue(i).toUInt();
    return Level(w, h, n);
}

//-----------------------------------------------------------------------------
const char *BINDING_LABELS[3]
    = { I18N_NOOP("Left button"), I18N_NOOP("Mid button"),
        I18N_NOOP("Right button") };
const char *OP_MOUSE_BINDINGS[3] = { "mouse left", "mouse mid", "mouse right"};
const char *ACTION_BINDINGS[4] =
    { I18N_NOOP("reveal"), I18N_NOOP("autoreveal"),
      I18N_NOOP("toggle flag"), I18N_NOOP("toggle ? mark") };

GameSettings::GameSettings()
    : KSettingWidget(i18n("Game"), "misc")
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    QCheckBox *cb = createUMark(this);
    top->addWidget(cb);

    cb = createKeyboard(this);
	top->addWidget(cb);

    cb = createPauseFocus(this);
	top->addWidget(cb);

	top->addSpacing(2 * KDialog::spacingHint());

	QVGroupBox *gb = new QVGroupBox(i18n("Mouse bindings"), this);
	top->addWidget(gb);
	QGrid *grid = new QGrid(2, gb);
	grid->setSpacing(10);
    for (uint i=0; i<3; i++) {
        (void)new QLabel(i18n(BINDING_LABELS[i]), grid);
        createMouseBinding(this, grid, (MouseButton)i);
	}
}

QCheckBox *GameSettings::createUMark(KSettingWidget *sw)
{
    QCheckBox *cb = new QCheckBox(i18n("Enable ? mark"), sw);
    sw->plug(cb, OP_GROUP, "? mark", true);
    return cb;
}

bool GameSettings::readUMark()
{
    KSettingWidget sw;
    QCheckBox *cb = createUMark(&sw);
    return sw.readValue(cb).toBool();
}

QCheckBox *GameSettings::createKeyboard(KSettingWidget *sw)
{
    QCheckBox *cb = new QCheckBox(i18n("Enable keyboard"), sw);
    sw->plug(cb, OP_GROUP, "keyboard game", false);
    return cb;
}

bool GameSettings::readKeyboard()
{
    KSettingWidget sw;
    QCheckBox *cb = createKeyboard(&sw);
    return sw.readValue(cb).toBool();
}

QCheckBox *GameSettings::createPauseFocus(KSettingWidget *sw)
{
    QCheckBox *cb = new QCheckBox(i18n("Pause if window lose focus"), sw);
    sw->plug(cb, OP_GROUP, "paused if lose focus", true);
    return cb;
}

bool GameSettings::readPauseFocus()
{
    KSettingWidget sw;
    QCheckBox *cb = createPauseFocus(&sw);
    return sw.readValue(cb).toBool();
}

QComboBox *GameSettings::createMouseBinding(KSettingCollection *col,
                                            QWidget *parent, MouseButton i)
{
    QComboBox *cb = new QComboBox(parent);
    col->plug(cb, OP_GROUP, OP_MOUSE_BINDINGS[i], ACTION_BINDINGS[i]);
    for (uint j=0; j<4; j++) {
        cb->insertItem(i18n(ACTION_BINDINGS[j]), j);
        col->map(cb, j, ACTION_BINDINGS[j]);
    }
    return cb;
}

KMines::MouseAction GameSettings::readMouseBinding(MouseButton mb)
{
    KSettingWidget sw;
    QComboBox *cb = createMouseBinding(&sw, &sw, mb);
    return (MouseAction)sw.readId(cb);
}

//-----------------------------------------------------------------------------
const uint MIN_CASE_SIZE = 20;
const uint MAX_CASE_SIZE = 100;

struct ColorData {
    const char *entry;
    const char *label;
    QColor      def;
};

const ColorData COLOR_DATA[KMines::NB_COLORS] = {
    { "flag color",      I18N_NOOP("Flag color"),      Qt::red },
    { "explosion color", I18N_NOOP("Explosion color"), Qt::red },
    { "error color",     I18N_NOOP("Error color"),     Qt::red }
};

const QColor DEFAULT_NUMBER_COLOR[KMines::NB_NUMBER_COLORS] = {
    Qt::blue, Qt::darkGreen, Qt::darkYellow, Qt::darkMagenta, Qt::red,
    Qt::darkRed, Qt::black, Qt::black
};

AppearanceSettings::AppearanceSettings()
    : KSettingWidget(i18n("Appearance"), "appearance")
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    KIntNumInput *cs = createCaseSize(this);
    top->addWidget(cs);

    top->addSpacing(2 * KDialog::spacingHint());

    QGrid *grid = new QGrid(2, this);
    top->addWidget(grid);

    for (uint i=0; i<NB_COLORS; i++) {
        (void)new QLabel(i18n(COLOR_DATA[i].label), grid);
        createColor(this, grid, i);
    }

	for (uint i=0; i<NB_NUMBER_COLORS; i++) {
		(void)new QLabel(i==0 ? i18n("1 mine color")
						 : i18n("%1 mines color").arg(i+1), grid);
        createNumberColor(this, grid, i);
	}
}

KIntNumInput *AppearanceSettings::createCaseSize(KSettingWidget *sw)
{
    KIntNumInput *cs = new KIntNumInput(sw);
    cs->setLabel(i18n("Case size"));
    cs->setRange(MIN_CASE_SIZE, MAX_CASE_SIZE);
    sw->plug(cs, OP_GROUP, "case size", MIN_CASE_SIZE);
    return cs;
}

KColorButton *AppearanceSettings::createColor(KSettingCollection *col,
                                              QWidget *parent, uint i)
{
    KColorButton *cb = new KColorButton(parent);
    cb->setFixedWidth(100);
    col->plug(cb, OP_GROUP, COLOR_DATA[i].entry, COLOR_DATA[i].def);
    return cb;
}

KColorButton *AppearanceSettings::createNumberColor(KSettingCollection *col,
                                                    QWidget *parent, uint i)
{
    KColorButton *cb = new KColorButton(parent);
    cb->setFixedWidth(100);
    col->plug(cb, OP_GROUP, QString("color #%1").arg(i),
              DEFAULT_NUMBER_COLOR[i]);
    return cb;
}

KMines::CaseProperties AppearanceSettings::readCaseProperties()
{
    CaseProperties cp;
    KSettingWidget sw;

    KIntNumInput *cs = createCaseSize(&sw);
    cp.size = sw.readValue(cs).toUInt();

    for (uint i=0; i<NB_COLORS; i++) {
        KColorButton *cb = createColor(&sw, &sw, i);
        cp.colors[i] = sw.readValue(cb).toColor();
    }

	for (uint i=0; i<NB_NUMBER_COLORS; i++) {
        KColorButton *cb = createNumberColor(&sw, &sw, i);
		cp.numberColors[i] = sw.readValue(cb).toColor();
    }

	return cp;
}

//-----------------------------------------------------------------------------
SettingsDialog::SettingsDialog(QWidget *parent)
    : KSettingDialog(parent)
{
    append(new GameSettings);
    append(new AppearanceSettings);
    append( kHighscores->createSettingsWidget(this) );
    append(new CustomSettings);

    connect(this, SIGNAL(settingsSaved()), parent, SLOT(settingsChanged()));
}
