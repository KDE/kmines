#ifndef DEFINES_H
#define DEFINES_H

#include <qcolor.h>

enum CaseState { Covered, Uncovered, Uncertain, Marked, Exploded, Error };
struct Case {
	bool      mine;
	CaseState state;
};

enum GameType    { Easy = 0, Normal, Expert, Custom, NbLevels };
enum GameState   { Stopped, Playing, Paused, GameOver };
enum MouseAction { Reveal = 0, AutoReveal, Mark, UMark };
enum MouseButton { Left = 0, Mid, Right };

struct Level {
	uint     width, height, nbMines;
	GameType type;
};
extern const Level LEVELS[NbLevels-1];

struct Score {
	uint     sec, min;
	GameType type;
};

#define NB_NUMBER_COLORS 8

struct CaseProperties {
	uint size;
	QColor numberColors[NB_NUMBER_COLORS];
	QColor flagColor, explosionColor, errorColor;
};

#endif // DEFINES_H
