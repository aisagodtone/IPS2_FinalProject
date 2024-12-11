#include "UI.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include <algorithm>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include "shapes/Point.h"
#include "shapes/Rectangle.h"
#include "Player.h"
#include "towers/Tower.h"
#include "Level.h"

// fixed settings
constexpr char love_img_path[] = "./assets/image/love.png";
constexpr int love_img_padding = 5;
constexpr int tower_img_left_padding = 30;
constexpr int tower_img_top_padding = 30;

void
UI::init() {
	DataCenter *DC = DataCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();
	love = IC->get(love_img_path);
	int tl_x = DC->game_field_length + tower_img_left_padding;
	int tl_y = tower_img_top_padding;
	int max_height = 0;
	// arrange tower shop
	for(size_t i = 0; i < (size_t)(TowerType::TOWERTYPE_MAX); ++i) {
		ALLEGRO_BITMAP *bitmap = IC->get(TowerSetting::tower_menu_img_path[i]);
		int w = al_get_bitmap_width(bitmap);
		int h = al_get_bitmap_height(bitmap);
		if(tl_x + w > DC->window_width) {
			tl_x = DC->game_field_length + tower_img_left_padding;
			tl_y += max_height + tower_img_top_padding;
			max_height = 0;
		}
		tower_items.emplace_back(bitmap, Point{tl_x, tl_y}, TowerSetting::tower_price[i]);
		tl_x += w + tower_img_left_padding;
		max_height = std::max(max_height, h);
	}
	debug_log("<UI> state: change to HALT\n");
	state = STATE::HALT;
	on_item = -1;
}

void
UI::update() {
	DataCenter *DC = DataCenter::get_instance();
	const Point &mouse = DC->mouse;

	switch(state) {
		case STATE::HALT: {
			for(size_t i = 0; i < tower_items.size(); ++i) {
				auto &[bitmap, p, price] = tower_items[i];
				int w = al_get_bitmap_width(bitmap);
				int h = al_get_bitmap_height(bitmap);
				// hover on a shop tower item
				if(mouse.overlap(Rectangle{p.x, p.y, p.x+w, p.y+h})) {
					on_item = i;
					debug_log("<UI> state: change to HOVER\n");
					state = STATE::HOVER;
					break;
				}
			}
			break;
		} case STATE::HOVER: {
			auto &[bitmap, p, price] = tower_items[on_item];
			int w = al_get_bitmap_width(bitmap);
			int h = al_get_bitmap_height(bitmap);
			if(!mouse.overlap(Rectangle{p.x, p.y, p.x+w, p.y+h})) {
				on_item = -1;
				debug_log("<UI> state: change to HALT\n");
				state = STATE::HALT;
				break;
			}
			// click mouse left button
			if(DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
				// no money
				if(price > DC->player->coin) {
					debug_log("<UI> Not enough money to buy tower %d.\n", on_item);
					break;
				}
				debug_log("<UI> state: change to SELECT\n");
				state = STATE::SELECT;
			}
			break;
		} case STATE::SELECT: {
			// click mouse left button: place
			if(DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
				debug_log("<UI> state: change to PLACE\n");
				state = STATE::PLACE;
			}
			// click mouse right button: cancel
			if(DC->mouse_state[2] && !DC->prev_mouse_state[2]) {
				on_item = -1;
				debug_log("<UI> state: change to HALT\n");
				state = STATE::HALT;
			}
			break;
		} case STATE::PLACE: {
			// check placement legality
			ALLEGRO_BITMAP *bitmap = Tower::get_bitmap(static_cast<TowerType>(on_item));
			int w = al_get_bitmap_width(bitmap);
			int h = al_get_bitmap_height(bitmap);
			Rectangle place_region{mouse.x - w / 2, mouse.y - h / 2, DC->mouse.x + w / 2, DC->mouse.y + h / 2};
			debug_log("<UI> state: change to HALT\n");
			state = STATE::HALT;
			break;
		}
	}
}

void
UI::draw() {
	DataCenter *DC = DataCenter::get_instance();
	// draw HP
	const int &game_field_length = DC->game_field_length;
	const int &player_HP = DC->player->HP;
	int love_width = al_get_bitmap_width(love);
	for(int i = 1; i <= player_HP; ++i) {
		al_draw_bitmap(love, game_field_length - (love_width + love_img_padding) * i, love_img_padding, 0);
	}
}