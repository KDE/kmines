#ifndef DEFINES_H
#define DEFINES_H

#include <qcolor.h>

enum CaseState { Covered, Uncovered, Uncertain, Marked, Exploded, Error };
struct Case {
	bool      mine;
	CaseState state;
};

enum Level       { Easy = 0, Normal, Expert, NbLevels, Custom = NbLevels };
enum GameState   { Stopped, Playing, Paused };
enum MouseAction { Reveal = 0, AutoReveal, Mark, UMark, None };
enum MouseButton { Left = 0, Mid, Right };

struct LevelData {
	uint        width, height, nbMines;
    Level       level;
    const char *label, *wwLabel, *i18nLabel;
};
extern const LevelData LEVELS[NbLevels];

#define NB_NUMBER_COLORS 8

struct CaseProperties {
	uint size;
	QColor numberColors[NB_NUMBER_COLORS];
	QColor flagColor, explosionColor, errorColor;
};

#define NB_HS_ENTRIES 10

#endif // DEFINES_H
