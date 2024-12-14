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
#include "data/SoundCenter.h"

// fixed settings
constexpr char love_img_path[] = "./assets/image/love.png";
constexpr int love_img_padding = 5;
constexpr char key_img_path[] = "./assets/image/key.png";
constexpr char no_key_img_path[] = "./assets/image/no_key.png";
constexpr char menu_background_img_path[] = "./assets/image/MenuBackground.jpg";
constexpr char menu_button_img_path[] = "./assets/image/menu_button.png";
constexpr char menu_bgm_sound_path[] = "./assets/sound/menu_bgm.wav";

bool menu_drew = false;
Rectangle start_button_area;

void
UI::init() {
	ImageCenter *IC = ImageCenter::get_instance();
	love = IC->get(love_img_path);
	key = IC->get(key_img_path);
	no_key = IC->get(no_key_img_path);
	menu_background = IC->get(menu_background_img_path);
	menu_button = IC->get(menu_button_img_path);
	debug_log("<UI> state: change to MENU\n");
	state = STATE::MENU;
}

void
UI::update() {
	DataCenter *DC = DataCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	const Point &mouse = DC->mouse;
	static ALLEGRO_SAMPLE_INSTANCE *menu_bgm = nullptr;

	switch(state) {
		case STATE::MENU:{
			if(!menu_drew){
				UI::draw_menu();
				static bool menu_bgm_played = false;
				if(!menu_bgm_played){
					menu_bgm = SC->play(menu_bgm_sound_path, ALLEGRO_PLAYMODE_LOOP);
					menu_bgm_played = true;
				}
				break;
			}
			if(mouse.overlap(start_button_area)){
				debug_log("mouse overlapping with start button\n");
				if(DC->mouse_state[1] && !DC->prev_mouse_state[1]){
					debug_log("<UI> state: change to INGAME\n");
					state = STATE::INGAME;
					SC->toggle_playing(menu_bgm);
					al_clear_to_color(al_map_rgb(100, 100, 100));
					menu_drew = false;
					break;
				}
			}
			break;
		}
		case STATE::INGAME:{
			// no ui need to be drawn in-game
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
	int key_width = al_get_bitmap_width(key);
	for(int i = 1; i <= player_HP; ++i) {
		al_draw_bitmap(love, game_field_length - (love_width + love_img_padding) * i, love_img_padding, 0);
	}
	if(have_key){
		al_draw_bitmap(key, game_field_length - (key_width + 5), 30 + al_get_bitmap_height(love), 0);
	}
	else{
		al_draw_bitmap(no_key, game_field_length - (key_width + 5), 30 + al_get_bitmap_height(love), 0);
	}
}

void UI::draw_menu(){
	DataCenter *DC = DataCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	double button_width = al_get_bitmap_width(menu_button);
	double button_height = al_get_bitmap_height(menu_button);
	start_button_area = Rectangle(
		DC->window_width/2.0 - button_width/2.0,	// upper-left x
		DC->window_height/2.0 - button_height/2.0,	// upper-left y
		DC->window_width/2.0 - button_width/2.0 + button_width,	// lower-right x
		DC->window_height/2.0 - button_height/2.0 + button_height);	// lower-right y
	al_clear_to_color(al_map_rgb(0, 0, 0));	// set base color to black
	al_draw_bitmap(menu_background, 0, 24, 0);	// height of window is 768, manu background is 720 => shift down 24
	al_draw_bitmap(menu_button, start_button_area.x1, start_button_area.y1, 0);
	al_draw_text(
		FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
		start_button_area.center_x(), start_button_area.center_y() - 10,
		ALLEGRO_ALIGN_CENTRE, "START");
	al_flip_display();
	menu_drew = true;
	debug_log("menu drew\n");
}

UI::STATE UI::get_state(){
	return state;
}