﻿#include "game_sm.h"
#include "avr/eeprom.h"
#include "defaults.h"
#include "operators.h"

#undef TRANSITION
#define TRANSITION(s) {\
	setState(STATE_CAST(&GameSM::s)); \
	process_criterium_ = 0;\
	process(ON_ENTRY); \
	}

#define LOAD_EFFECT_STANDART(s) {\
	load_following_state_ = STATE_CAST(&GameSM::s);\
	TRANSITION(stateLoadEffect);\
	return;\
	}

#define LOAD_EFFECT_BEGIN(s) {\
	static bool load_passed = false;\
	if(load_passed){\
	load_passed = false;\
	}\
	else{\
	load_passed = true;\
	LOAD_EFFECT_STANDART(s);\
	}\
	}


byte EE_speed EEMEM = DEFAULT_SPEED;
byte EE_language EEMEM = DEFAULT_LANGUAGE;

GameSM::GameSM(Display *display)
   :StateMachine(STATE_CAST(&GameSM::stateDefault)), display_(display), process_timer1_ (0), process_timer2_(0), language_(EN)
{
	display_->text1_.setShiftSpeed(5);
	display_->text2_.setShiftSpeed(5);
	speed_ = eeprom_read_byte(&EE_speed);
	language_ = (eeprom_read_byte(&EE_language) ? DE : EN);
	process(ON_ENTRY);
}

void GameSM::processStateMaschine(byte event)
{
	bool process = false;
	if((process_criterium_ & EVER) || (process_criterium_ & ProcessCriterum::PCINT && event & CHANGE)){
		process = true;
	}
	unsigned long now = millis();
	if (process_criterium_ & ProcessCriterum::TIMER1 && process_timer1_ && process_timer1_ <= now){
		event |= TIMEOUT1;
		process_timer1_ = 0;
		process = true;
	}

	if (process_criterium_ & ProcessCriterum::TIMER2 && process_timer2_ && process_timer2_ <= now){
		event |= TIMEOUT2;
		process_timer2_ = 0;
		process = true;
	}
	if(process)
		this->process(event);
}

GameSM::MenuItem::Button GameSM::MenuItem::advance(byte event, char& item, const char num, const char min) {

	switch (event & INPUT_MASK) {
	case BTN_LEFT:
		if (--item < min)
			item = num-1;
		break;
	case BTN_RIGHT:
		if (++item >= num)
			item = min;
		break;
	case BTN_DOWN:
		return DOWN_BTN;
	case BTN_UP:
		return UP_BTN;
	}
	return NO_BTN;
}

void GameSM::stateDefault(byte event)
{

	const char * texts[2][5] = {{"Tetris", "Snake", "Running Julian", "highscore", "setting"}, {"Tetris","Snake","Rennender Julian", "Highscore","Einstellungen"}};
	static MenuItem item;
	if(event & ON_ENTRY){
		item.init(5);
		process_criterium_ |= PCINT;
		display_->loadMenuConfig();
	}

	else if(event & INPUT_MASK && event & CHANGE){
		if(item.advance(event) == MenuItem::DOWN_BTN){
			switch(item.value_){
			case 0:
				TRANSITION(stateTetris);
				break;
			case 1:
				TRANSITION(stateSnake);
				break;
			case 2:
				TRANSITION(stateRunningMan);
				break;
			case 3:
				TRANSITION(stateHighscoreMenu);
				break;
			case 4:
				TRANSITION(stateSettingsMenu);
				break;
			default:
				break;
			}
			return;
		}
	}

	switch(item.value_){
	case 0:
		display_->setIcon(0xfffff7e300081c00);
		break;
	case 1:
		display_->setIcon(0x3c20203c04045c00);
		break;
	case 2:
		display_->setIcon(0x60600a040e040000);
		break;
	case 3:
		display_->setIcon(0x00081c2018043810);
		break;
	case 4:
		display_->setIcon(0x00003c3c3c3c0000);
		break;
	default:
		break;
	}
	display_->text1_.setText(texts[language_][item.value_]);
}


void GameSM::stateTetris(byte event)
{
//	static bool btn_down_state	= false;
//	static bool btn_left_state = false;
//	static bool btn_right_state = false;
//	static unsigned long step_interval;

//	static int general_step_interval;
//	static int general_down_interval;
//	static int general_first_move_interval;
//	static int general_move_interval;

//	unsigned long now = millis();
	if(event & ON_ENTRY){
		display_->loadsGameCofig();
		if(game_ != nullptr){
			delete game_ ;
			game_ = nullptr;
		}
		game_ = new Tetris(display_,&process_timer1_,&process_timer2_);
		game_->setSpeed(speed_);
		game_->reset();
		game_->start();
//		btn_down_state = false;
		process_criterium_ |= PCINT | TIMER1 | TIMER2;
//		step_interval = general_step_interval;
//		process_timer1_ = now + step_interval;
		return;
	}

	if(game_->process(event)){
		TRANSITION(stateGameOver);
		return;
	}
}

void GameSM::stateSnake(byte event)
{
	static Snake::Direction dir;
	static int interval;
	if(event & ON_ENTRY){
		display_->loadsGameCofig();
		if(game_ != nullptr){
			delete game_ ;
			game_ = nullptr;
		}
		game_ = new Snake(display_);
		game_->start();
		process_criterium_ |= PCINT | TIMER1;
		switch (speed_) {
		case 0:
			interval = 600;
			break;
		case 1:
			interval = 500;
			return;
		case 3:
			interval = 300;
			break;
		case 4:
			interval = 200;
			break;
		case 2:
		default:
			interval = 400;
			break;
		}
		process_timer1_ = millis() + interval;
		dir = Snake::START;
		return;
	}
	bool finished = false;
	if(event & CHANGE){
		if(event & INPUT_MASK){
			bool button_set = false;
			if(event & BTN_UP){
				if(dir != Snake::UP && dir != Snake::DOWN){
					dir = Snake::UP;
					button_set = true;
					finished = game_->up();
				}
			}
			else if(event & BTN_LEFT){
				if(dir != Snake::LEFT && dir != Snake::RIGHT){
					dir = Snake::LEFT;
					button_set = true;
					finished = game_->left();
				}
			}

			else if(event & BTN_RIGHT){
				if(dir != Snake::LEFT && dir != Snake::RIGHT){
					dir = Snake::RIGHT;
					button_set = true;
					finished = game_->right();
				}
			}

			else if(event & BTN_DOWN){
				if(dir != Snake::DOWN && dir != Snake::UP){
					dir = Snake::DOWN;
					button_set = true;
					finished = game_->down();
				}
			}
			if(button_set)
				process_timer1_ = millis() + interval;
		}
	}
	if(event & TIMEOUT1){
		process_timer1_ = millis() + interval;
		if(game_->process(event)){ // game ends
			finished = true;
		}
	}
	if(finished){
		TRANSITION(stateGameOver);
		return;
	}
}

void GameSM::stateRunningMan(byte event)
{
	if(event & ON_ENTRY){
		display_->loadsGameCofig();
		if(game_ != nullptr){
			delete game_ ;
			game_ = nullptr;
		}
		game_ = new RunningMan(display_,&process_timer1_, &process_timer2_);
		game_->setSpeed(speed_);
		game_->reset();
		game_->start();
		process_criterium_ |= PCINT | TIMER1 | TIMER2;
		return;
	}

	if(game_->process(event)){
		TRANSITION(stateGameOver);
	}
}

void GameSM::stateGameOver(byte event){
	if(event & ON_ENTRY){
		LOAD_EFFECT_BEGIN(stateGameOver);
		display_->text1_.clear();
		display_->text2_.clear();
		process_criterium_ = PCINT;
		if(game_ != nullptr){
			process_criterium_ = PCINT;
			display_->loadMenuConfig();
			if(game_->isNewHighscore()){
				display_->text1_.setText((language_ == EN ? "new highscore!" : "neuer Highscore!"));
			}
			else {
				display_->text1_.setText("Game Over");
			}
			display_->text2_.setNumber(game_->points());
			// delete game
			delete game_;
			game_ = nullptr;
		}
		return;
	}

	if(event & CHANGE && event & INPUT_MASK){
		TRANSITION(stateDefault);
		return;
	}
}

void GameSM::stateSettingsMenu(byte event)
{
	static MenuItem item;
	const char *menu_text[2][2] = {{"speed", "language"},{"Geschwindigkeit", "Sprache"}};
	if(event & ON_ENTRY){
		display_->loadMenuConfig();
		item.init(2);
		process_criterium_ = PCINT;
	}
	else if(event & CHANGE && event & INPUT_MASK){
		switch(item.advance(event)){
		case MenuItem::DOWN_BTN:
			switch (item.value_) {
			case 0:
				TRANSITION(stateSpeedMenu);
				break;
			case 1:
				TRANSITION(stateLanguageMenu);
				break;
			default:
				break;
			}
			return;
		case MenuItem::UP_BTN:
			TRANSITION(stateDefault);
			return;
		default:
			break;
		}
	}

	switch (item.value_) {
	case 0:
		display_->setIcon(0x0000122448241200);
		break;
	case 1:
		display_->setIcon(0x2060ff818181ff00);
		break;
	default:
		break;
	}
	display_->text1_.setText(menu_text[language_][item.value_]);
}

void GameSM::stateSpeedMenu(byte event)
{
	static MenuItem item;
	if(event & ON_ENTRY){
		display_->loadMenuConfig();
		item.init(5,speed_);
		process_criterium_ = PCINT;
	}

	else if(event & CHANGE && event & INPUT_MASK){
		switch(item.advance(event)){
		case MenuItem::DOWN_BTN:
			speed_ = item.value_;
			eeprom_write_byte(&EE_speed,speed_);
			LOAD_EFFECT_STANDART(stateSettingsMenu);
		case MenuItem::UP_BTN:
			TRANSITION(stateSettingsMenu);
			return;
		default:
			break;
		}
	}
	display_->clear();
	display_->text1_.setText(display_->formatInt(number_buffer_,8,item.value_+1));
	byte cols = display_->cols() / 5.0 * (item.value_+1);
	for(int col = 0; col < cols; col++){
		display_->setColumn(col,0xFF);
	}
}

void GameSM::stateLanguageMenu(byte event)
{
	const char * menu_text[2][2] = {{"english","Deutsch"},{"english","Deutsch"}};
	static MenuItem item;
	if(event & ON_ENTRY){
		display_->loadMenuConfig();
		item.init(2,(language_==DE?0:1));
		process_criterium_ = PCINT;
	}
	else if(event & CHANGE && event & INPUT_MASK){
		switch(item.advance(event)){ // enter pressed
		case MenuItem::DOWN_BTN:
			language_ = (item.value_ == 0?EN:DE);
			eeprom_write_byte(&EE_language,byte(language_));
			LOAD_EFFECT_STANDART(stateSettingsMenu);
			return;
		case MenuItem::UP_BTN:
			TRANSITION(stateSettingsMenu);
			return;
		default:
			break;
		}
	}

	switch (item.value_) {
	case 0:
		display_->text2_.setText("E");
		break;
	case 1:
		display_->text2_.setText("D");
		break;
	default:
		break;
	}
	display_->text1_.setText(menu_text[language_][item.value_]);
}

void GameSM::stateLoadEffect(byte event)
{
	static byte count = 0;
	if(event & ON_ENTRY){
		count = 0;
		display_->text1_.clear();
		display_->text2_.clear();
		process_criterium_ = TIMER1;
		process_timer1_ = millis() + 50;
	}
	if(event & TIMEOUT1){
		if(count >= display_->rows()){
			if(load_following_state_){
				setState(load_following_state_);
				load_following_state_ = nullptr;
				process_criterium_ = 0;
				process(ON_ENTRY);
			}
			else{
				TRANSITION(stateDefault);
			}
			return;
		}
		display_->setRow(count,0xFF);
		process_timer1_ = millis() + 50;
		count++;
		return;
	}
}

void GameSM::stateHighscoreMenu(byte event)
{
	static MenuItem item;
	if(event & ON_ENTRY){
		item.init(4,0);
		process_criterium_ |= PCINT;
	}
	else if(event & CHANGE && event & INPUT_MASK){
		byte advanced = item.advance(event);
		if(advanced){
			if(item.value_ == 3){
				if(advanced == MenuItem::DOWN_BTN){
					TRANSITION(stateResetMenu);
				}
				else{
					TRANSITION(stateDefault);
				}
				return;
			}
			else{
				TRANSITION(stateDefault);
				return;
			}
		}
	}
	switch (item.value_) {
	case 0:
		display_->setIcon(0xfffff7e300081c00);
		display_->text1_.setNumber(Tetris::highscore());
		break;
	case 1:
		display_->setIcon(0x3c20203c04045c00);
		display_->text1_.setNumber(Snake::highscore());
		break;
	case 2:
		display_->setIcon(0x60600a040e040000);
		display_->text1_.setNumber(RunningMan::highscore());
		break;
	case 3:
		display_->setIcon(0xbd42a59999a542bd);
		display_->text1_.setText(language_ == EN ? "reset" : "Zur""\x1c""cksetzen");
	default:
		break;
	}
}

void GameSM::stateResetMenu(byte event){
	if(event & ON_ENTRY){
		display_->loadMenuConfig();
		process_criterium_ = PCINT;
	}

	else if(event & CHANGE && event & INPUT_MASK){
		switch (event & INPUT_MASK) {
		case BTN_DOWN: // reset
			Tetris::resetHighscore();
			Snake::resetHighscore();
			RunningMan::resetHighscore();
			LOAD_EFFECT_STANDART(stateDefault);
			return;
			break;
		case BTN_UP:
			TRANSITION(stateHighscoreMenu);
			return;
		default:
			break;
		}
	}
	display_->text1_.setText((language_ == EN ? "reset scores" : "Highscores zur""\x1c""cksetzen"));
	display_->setIcon(0x00040a1120408000);
}

GameSM::MenuItem::Button GameSM::MenuItem::advance(byte event)
{
	advance(event,value_,num_);
}

