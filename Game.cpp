#include "Game.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/OperationCenter.h"
#include "data/SoundCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include "Player.h"
#include "Level.h"
#include "hero.h"
#include "monsters/Monster.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_acodec.h>
#include <vector>
#include <cstring>
#include <utility>

// fixed settings
constexpr char game_icon_img_path[] = "./assets/image/game_icon.png";
constexpr char game_start_sound_path[] = "./assets/sound/growl.wav";
constexpr char background_img_path[] = "./assets/image/StartBackground.jpg";
constexpr char ingame_background_sound_path[] = "./assets/sound/ingame_bgm.mp3";
constexpr char menu_button_img_path[] = "./assets/image/menu_button.png";

Rectangle menu_button_area;
ALLEGRO_BITMAP *pause_menu_button;

/**
 * @brief Game entry.
 * @details The function processes all allegro events and update the event state to a generic data storage (i.e. DataCenter).
 * For timer event, the game_update and game_draw function will be called if and only if the current is timer.
 */
void
Game::execute() {
	DataCenter *DC = DataCenter::get_instance();
	// main game loop
	bool run = true;
	while(run) {
		// process all events here
		al_wait_for_event(event_queue, &event);
		switch(event.type) {
			case ALLEGRO_EVENT_TIMER: {
				run &= game_update();
				game_draw();
				break;
			} case ALLEGRO_EVENT_DISPLAY_CLOSE: { // stop game
				run = false;
				break;
			} case ALLEGRO_EVENT_KEY_DOWN: {
				DC->key_state[event.keyboard.keycode] = true;
				break;
			} case ALLEGRO_EVENT_KEY_UP: {
				DC->key_state[event.keyboard.keycode] = false;
				break;
			} case ALLEGRO_EVENT_MOUSE_AXES: {
				DC->mouse.x = event.mouse.x;
				DC->mouse.y = event.mouse.y;
				break;
			} case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: {
				DC->mouse_state[event.mouse.button] = true;
				break;
			} case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
				DC->mouse_state[event.mouse.button] = false;
				break;
			} default: break;
		}
	}
}

/**
 * @brief Initialize all allegro addons and the game body.
 * @details Only one timer is created since a game and all its data should be processed synchronously.
 */
Game::Game() {
	DataCenter *DC = DataCenter::get_instance();
	GAME_ASSERT(al_init(), "failed to initialize allegro.");

	// initialize allegro addons
	bool addon_init = true;
	addon_init &= al_init_primitives_addon();
	addon_init &= al_init_font_addon();
	addon_init &= al_init_ttf_addon();
	addon_init &= al_init_image_addon();
	addon_init &= al_init_acodec_addon();
	GAME_ASSERT(addon_init, "failed to initialize allegro addons.");

	// initialize events
	bool event_init = true;
	event_init &= al_install_keyboard();
	event_init &= al_install_mouse();
	event_init &= al_install_audio();
	GAME_ASSERT(event_init, "failed to initialize allegro events.");

	// initialize game body
	GAME_ASSERT(
		display = al_create_display(DC->window_width, DC->window_height),
		"failed to create display.");
	GAME_ASSERT(
		timer = al_create_timer(1.0 / DC->FPS),
		"failed to create timer.");
	GAME_ASSERT(
		event_queue = al_create_event_queue(),
		"failed to create event queue.");

	debug_log("Game initialized.\n");
	game_init();
}

/**
 * @brief Initialize all auxiliary resources.
 */
void
Game::game_init() {
	DataCenter *DC = DataCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	// set window icon
	game_icon = IC->get(game_icon_img_path);
	pause_menu_button = IC->get(menu_button_img_path);
	al_set_display_icon(display, game_icon);

	// register events to event_queue
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

	// init sound setting
	SC->init();

	// init font setting
	FC->init();

	ui = new UI();
	ui->init();

	DC->level->init();
	DC->monsters[0]->init(10,3,true, false, true);
	DC->monsters[1]->init(10,2,true, false, false);
	// game start
	background = IC->get(background_img_path);
	debug_log("Game state: change to MENU\n");
	state = STATE::MENU;	// initial state
	al_start_timer(timer);
}

/**
 * @brief The function processes all data update.
 * @details The behavior of the whole game body is determined by its state.
 * @return Whether the game should keep running (true) or reaches the termination criteria (false).
 * @see Game::STATE
 */
bool
Game::game_update() {
	DataCenter *DC = DataCenter::get_instance();
	OperationCenter *OC = OperationCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	static ALLEGRO_SAMPLE_INSTANCE *background = nullptr;
	static int lvl = 1;
	static long long spot_time = 0;
	static int warn_level = 0;
	static bool god_mode = false;

	switch(state) {
		case STATE::MENU: {
			break;
		}
		case STATE::START: {
			if(lvl > 2){
				state = STATE::POST_GAME;
				ui->state = UI::STATE::POST_GAME;
			}
			// static bool is_played = false;
			// static ALLEGRO_SAMPLE_INSTANCE *instance = nullptr;
			// if(!is_played) {
			// 	instance = SC->play(game_start_sound_path, ALLEGRO_PLAYMODE_ONCE);
			 	if(lvl == 1) {
					DC->player->max_key = 1;
				}
				else if(lvl == 2) {
					DC->player->max_key = 3;
				}
				player_init_pos = DC->level->load_level(lvl);	// load level() returns player's initial position (x,y)
				DC->hero->init(player_init_pos);
				DC->hero->cur_lvl = lvl;
				warn_level = 0;
				spot_time = 0;
				ui->alert_level = 0;
			// 	is_played = true;
			// }

			// if(!SC->is_playing(instance)) {
				lvl++;
				debug_log("<Game> state: change to LEVEL\n");
				state = STATE::LEVEL;
			// }
			break;
		
		} case STATE::LEVEL: {
			static bool BGM_played = false;
			if(!BGM_played) {
				background = SC->play(ingame_background_sound_path, ALLEGRO_PLAYMODE_LOOP);
				BGM_played = true;
			}
			for(int i = 0; i < 3; i++){
				if(DC->hero->have_key[i]){
					ui->have_key[i] = true;
				}
				else{
					ui->have_key[i] = false;
				}
			}
			if( (DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P]) 
				|| (DC->key_state[ALLEGRO_KEY_ESCAPE] && !DC->prev_key_state[ALLEGRO_KEY_ESCAPE]) ) {
				// pause the game
				SC->toggle_playing(background);
				debug_log("<Game> state: change to PAUSE\n");
				state = STATE::PAUSE;
			}
			/*if(DC->level->remain_monsters() == 0 && DC->monsters.size() == 0) {
				debug_log("<Game> state: change to END\n");
				state = STATE::END;
			}*/
			if(DC->player->HP == 0) {
				debug_log("<Game> state: change to END\n");
				state = STATE::POST_GAME;
			}

			if(DC->key_state[ALLEGRO_KEY_TILDE] && !DC->prev_key_state[ALLEGRO_KEY_TILDE]) {
				god_mode = !god_mode;	// toggle god mode
			}
			for(auto &mon : DC->monsters){
				if(mon->is_visible({DC->hero->shape->center_x(),DC->hero->shape->center_y()}) 
				&& mon->is_in_fov({DC->hero->shape->center_x(),DC->hero->shape->center_y()})
				&& !god_mode)
				{
					spot_time++;
					debug_log("Player is found\n");
				}
			}
			debug_log("spot time: %lld\n", spot_time);
			warn_level += (spot_time/60);
			if (spot_time/60 > 0){
				spot_time = 0;
			}
			ui->alert_level = warn_level;
			if(warn_level > 2){
				debug_log("<Game> state: change to POST_GAME\n");
				state = STATE::POST_GAME;
				ui->state = UI::STATE::POST_GAME;
				SC->toggle_playing(background);
				warn_level = 0;
				ui->alert_level = 3;
				ui->draw();
			}

			break;
		} case STATE::PAUSE: {
			if( (DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P]) 
				|| (DC->key_state[ALLEGRO_KEY_ESCAPE] && !DC->prev_key_state[ALLEGRO_KEY_ESCAPE]) ) {
				SC->toggle_playing(background);
				debug_log("<Game> state: change to LEVEL\n");
				state = STATE::LEVEL;
			}
			if(DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
				if(menu_button_area.overlap(DC->mouse)) {
					debug_log("<Game> state: change to MENU\n");
					state = STATE::MENU;
					ui->state = UI::STATE::MENU;
				}
			}
			break;
		}case STATE::POST_GAME: {
			break;
		} case STATE::END: {
			return false;
		}
	}
	if(state == STATE::LEVEL && DC->hero->cur_lvl > 2) {
		debug_log("<Game> state: change to POST_GAME\n");
		state = STATE::POST_GAME;
		ui->state = UI::STATE::POST_GAME;
	}
	if(state == STATE::MENU) {
		ui->update();	// load menu
		if(ui->get_state() == UI::STATE::INGAME) {
			debug_log("<Game> state: change to START\n");
			state = STATE::START;
		}
	}
	else if(state == STATE::POST_GAME) {
		ui->update();
		if(ui->get_state() == UI::STATE::MENU) {
			debug_log("<Game> state: change to MENU\n");
			state = STATE::MENU;
			lvl = 1;
		}
	}
	// If the game is not paused, we should progress update.
	else if(state != STATE::PAUSE && state != STATE::END) {
		DC->player->update();
		SC->update();
		ui->update();
		if(state != STATE::START) {
			DC->hero->update();
			DC->monsters[0]->update();
			DC->monsters[1]->update();
			OC->update();
		}
		if(DC->hero->cur_lvl != DC->level->level && DC->hero->cur_lvl < 3) {
			debug_log("<Game> state: change to START\n");
			state = STATE::START;
		}
	}
	// game_update is finished. The states of current frame will be previous states of the next frame.
	memcpy(DC->prev_key_state, DC->key_state, sizeof(DC->key_state));
	memcpy(DC->prev_mouse_state, DC->mouse_state, sizeof(DC->mouse_state));
	return true;
}

/**
 * @brief Draw the whole game and objects.
 */
void
Game::game_draw() {
	DataCenter *DC = DataCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();

	// Flush the screen first.
	if(state != STATE::END && state != STATE::MENU && state != STATE::POST_GAME) {
		al_clear_to_color(al_map_rgb(100, 100, 100));
		// in-game background
		al_draw_bitmap(background, 0, 0, 0);
		// user interface
		if(state != STATE::START) {
			DC->level->draw();
			DC->monsters[0]->draw();
			DC->monsters[1]->draw();
			DC->hero->draw();
			ui->draw();
			// OC->draw();
		}
		al_flip_display();
	}
	switch(state) {
		case STATE::POST_GAME:{
		}case STATE::MENU:{
		}case STATE::START: {
		} case STATE::LEVEL: {
			break;
		} case STATE::PAUSE: {
			// game layout cover
			al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(20, 20, 20, 64));
			al_draw_text(
				FC->courier_new[FontSize::LARGE], al_map_rgb(255, 255, 255),
				DC->window_width/2., DC->window_height/4.,
				ALLEGRO_ALIGN_CENTRE, "GAME PAUSED");

			menu_button_area = Rectangle(
				DC->window_width/2. - al_get_bitmap_width(pause_menu_button)/2.,	// upper-left x
				DC->window_height/2. - al_get_bitmap_height(pause_menu_button)/2.,	// upper-left y
				DC->window_width/2. + al_get_bitmap_width(pause_menu_button)/2.,	// lower-right x
				DC->window_height/2. + al_get_bitmap_height(pause_menu_button)/2.);	// lower-right y
			al_draw_bitmap(pause_menu_button, menu_button_area.x1, menu_button_area.y1, 0);
			al_draw_text(
				FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
				menu_button_area.center_x(), menu_button_area.center_y() - 10,
				ALLEGRO_ALIGN_CENTRE, "MENU");
			break;
		} case STATE::END: {
		}
	}
	al_flip_display();
}

Game::~Game() {
	al_destroy_display(display);
	al_destroy_timer(timer);
	al_destroy_event_queue(event_queue);
}