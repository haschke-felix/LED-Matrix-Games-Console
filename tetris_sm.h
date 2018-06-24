#pragma once
#include "statemachine.h"
#include "display.h"
#include "game.h"

#define ON_ENTRY bit(7)
#define CHANGE   bit(6)
// these are the actual input pins (of PINC)
#define BTN_LEFT bit(0)
#define BTN_DOWN bit(1)
#define BTN_FLIP  bit(2)
#define BTN_RIGHT bit(3)
// mask of all input pins
#define INPUT_MASK (BTN_DOWN | BTN_LEFT | BTN_RIGHT | BTN_FLIP)

class Display;
class TetrisSM : public StateMachine
{
public:
	TetrisSM(Display * display);

private: // states
	void stateDefault(byte event);

private:
	Display * display_;
	Game * game_;
};
