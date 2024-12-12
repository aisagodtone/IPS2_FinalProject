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
		INGAME
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
	STATE get_state();

private:
	STATE state;
	ALLEGRO_BITMAP *love;
	ALLEGRO_BITMAP *menu_background;
	ALLEGRO_BITMAP *menu_button;
	// tower menu bitmap, (top-left x, top-left y), price
	std::vector<std::tuple<ALLEGRO_BITMAP*, Point, int>> tower_items;
	int on_item;
};

#endif