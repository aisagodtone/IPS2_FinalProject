#ifndef UI_H_INCLUDED
#define UI_H_INCLUDED

#include <allegro5/bitmap.h>
#include <vector>
#include <tuple>
#include "./shapes/Point.h"

class UI
{
public:
	enum class STATE {
		MENU,
		INGAME,
		POST_GAME
		// HALT, // -> HOVER
		// HOVER, // -> HALT, SELECT
		// SELECT, // -> HALT, PLACE
		// PLACE // -> HALT
	};
	UI() {}
	void init();
	void update();
	void draw();
	void draw_menu();
	void draw_post_menu();
	STATE get_state();
	bool have_key[3] = {false};
	STATE state;
	int alert_level = 0;

private:
	ALLEGRO_BITMAP *love;
	ALLEGRO_BITMAP *key;
	ALLEGRO_BITMAP *gate_key1;
	ALLEGRO_BITMAP *gate_key2;
	ALLEGRO_BITMAP *no_key;
	ALLEGRO_BITMAP *menu_background;
	ALLEGRO_BITMAP *menu_button;
	ALLEGRO_BITMAP *alert;
	// tower menu bitmap, (top-left x, top-left y), price
	// std::vector<std::tuple<ALLEGRO_BITMAP*, Point, int>> tower_items;
};

#endif