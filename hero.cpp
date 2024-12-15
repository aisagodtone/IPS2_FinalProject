#include "hero.h"
#include "data/DataCenter.h"
#include "data/GIFCenter.h"
#include <cstdio>
#include "shapes/Rectangle.h"
#include "Utils.h"
#include "data/SoundCenter.h"
#include <allegro5/allegro_primitives.h>
#include "data/ImageCenter.h"

#define MASK_SHIFT_X 1280
#define MASK_SHIFT_Y 768

using namespace std;

// fixed settings
enum class Dir {
	UP, DOWN, LEFT, RIGHT
};

namespace HeroSetting {
	static constexpr char gif_root_path[40] ="./assets/gif/Player";

	static constexpr char dir_path_prefix[][10] = {
		"left", "right", "down", "up"
	};
}

constexpr char chest_sound_path[] = "./assets/sound/chest_open.wav";
constexpr char key_sound_path[] = "./assets/sound/get_key.wav";
constexpr char door_sound_path[] = "./assets/sound/door_open.wav";
constexpr char door_locked_sound_path[] = "./assets/sound/door_locked.wav";
constexpr char closet_open_sound_path[] = "./assets/sound/closet_open.mp3";
constexpr char closet_close_sound_path[] = "./assets/sound/closet_close.mp3";
constexpr char mask_img_path[] = "./assets/image/mask.png";
bool mask = true;
bool in_closet = false;

void Hero::init(pair<size_t, size_t> pos){
	ImageCenter *IC = ImageCenter::get_instance();
	int grid = 64;
	hero_posX = pos.first;
	hero_posY = pos.second;
	have_key = false;
	player_mask = IC->get(mask_img_path);
	for(size_t i = 0; i < static_cast<size_t>(HeroState::HEROSTATE_MAX); ++i){
		char buffer[50];
		sprintf(
			buffer, "%s/walk_%s.gif",
			HeroSetting::gif_root_path,
			HeroSetting::dir_path_prefix[static_cast<int>(i)]
		);
		gifPath[static_cast<HeroState>(i)] = std::string(buffer);
	}
	GIFCenter *GIFC = GIFCenter::get_instance();
	ALGIF_ANIMATION *gif = GIFC->get(gifPath[HeroState::FRONT]);
	shape.reset(new Rectangle(
		hero_posX * grid,	// initial top-left
		hero_posY * grid, 
		hero_posX * grid + gif -> width, 	// initial bottom-right
		hero_posY * grid + gif -> height
	));
}

void Hero::draw(){
	GIFCenter *GIFC = GIFCenter::get_instance();
	ALGIF_ANIMATION *gif = GIFC->get(gifPath[state]);
	if(!in_closet){
		algif_draw_gif(
			gif,
			shape->center_x() - gif->width / 2,
			shape->center_y() - gif->height / 2, 0);
	}
	if(mask){
		// draw mask
		al_draw_bitmap(player_mask, shape->center_x() - MASK_SHIFT_X, shape->center_y() - MASK_SHIFT_Y, 0);
	}
}

void Hero::update(){
	DataCenter *DC = DataCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();

	// toggle mask
	if(DC->key_state[ALLEGRO_KEY_BACKSLASH] && !DC->prev_key_state[ALLEGRO_KEY_BACKSLASH]){
		mask = !mask;
	}

	// WASD movements, LSHIFT for running, and blocking objects handling
	if(DC->key_state[ALLEGRO_KEY_W] && !in_closet){
		debug_log("Player in map x:%d y: %d \n",hero_posY  ,hero_posX );
		if(DC->map[(static_cast<int>(shape->center_y() - run_speed))/64][hero_posY] == '1'
			|| DC->map[(static_cast<int>(shape->center_y() - run_speed))/64][hero_posY] == 'D'
			|| DC->map[(static_cast<int>(shape->center_y() - run_speed))/64][hero_posY] == 'C'
			|| DC->map[(static_cast<int>(shape->center_y() - run_speed))/64][hero_posY] == 'B'
			|| DC->map[(static_cast<int>(shape->center_y() - run_speed))/64][hero_posY] == 'O'
			|| DC->map[(static_cast<int>(shape->center_y() - run_speed))/64][hero_posY] == 'K'){
			return;
		}
		if(DC->key_state[ALLEGRO_KEY_LSHIFT]){
			shape->update_center_y(shape->center_y() - run_speed);
		}
		else{
			shape->update_center_y(shape->center_y()- speed);
		}
		hero_posX = shape->center_y()/64;
		// debug_log("update hitbox center x:%lf y: %lf \n", shape->center_x() , shape->center_y());
		state = HeroState::BACK;
	}
	else if(DC->key_state[ALLEGRO_KEY_A] && !in_closet){
		debug_log("Player in map x:%d y: %d \n",hero_posY  ,hero_posX );
		if(DC->map[hero_posX][(static_cast<int>(shape->center_x()- run_speed))/64] == '1'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() - run_speed))/64] == 'D'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() - run_speed))/64] == 'C'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() - run_speed))/64] == 'B'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() - run_speed))/64] == 'O'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() - run_speed))/64] == 'K'){
			return;
		}
		if(DC->key_state[ALLEGRO_KEY_LSHIFT]){
			shape->update_center_x(shape->center_x() - run_speed);
		}
		else{
			shape->update_center_x(shape->center_x()- speed);
		}
		hero_posY = shape->center_x()/64;
		// debug_log("update hitbox center x:%lf y: %lf \n", shape->center_x() , shape->center_y());
		state = HeroState::LEFT;
	}
	else if(DC->key_state[ALLEGRO_KEY_S] && !in_closet){
		debug_log("Player in map x:%d y: %d \n",hero_posY  ,hero_posX );
		if(DC->map[(static_cast<int>(shape->center_y()+ run_speed))/64][hero_posY] == '1'
			|| DC->map[(static_cast<int>(shape->center_y() + run_speed))/64][hero_posY] == 'D'
			|| DC->map[(static_cast<int>(shape->center_y() + run_speed))/64][hero_posY] == 'C'
			|| DC->map[(static_cast<int>(shape->center_y() + run_speed))/64][hero_posY] == 'B'
			|| DC->map[(static_cast<int>(shape->center_y() + run_speed))/64][hero_posY] == 'O'
			|| DC->map[(static_cast<int>(shape->center_y() + run_speed))/64][hero_posY] == 'K'){
			return;
		}
		if(DC->key_state[ALLEGRO_KEY_LSHIFT]){
			shape->update_center_y(shape->center_y() + run_speed);
		}
		else{
			shape->update_center_y(shape->center_y()+ speed);
		}
		hero_posX = shape->center_y()/64;
		// debug_log("update hitbox center x:%lf y: %lf \n", shape->center_x() , shape->center_y());
		state = HeroState::FRONT;
	}
	else if(DC->key_state[ALLEGRO_KEY_D] && !in_closet){
		debug_log("Player in map x:%d y: %d \n",hero_posY  ,hero_posX );
		if(DC->map[hero_posX][(static_cast<int>(shape->center_x() + run_speed))/64] == '1'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() + run_speed))/64] == 'D'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() + run_speed))/64] == 'C'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() + run_speed))/64] == 'B'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() + run_speed))/64] == 'O'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() + run_speed))/64] == 'K'){
			return;
		}
		if(DC->key_state[ALLEGRO_KEY_LSHIFT]){
			shape->update_center_x(shape->center_x() + run_speed);
		}
		else{
			shape->update_center_x(shape->center_x()+ speed);
		}
		hero_posY = shape->center_x()/64;
		// debug_log("update hitbox center x:%lf y: %lf \n", shape->center_x() , shape->center_y());
		state = HeroState::RIGHT;
	}

	// press ENTER next to a chest/closet/door
	else if(DC->key_state[ALLEGRO_KEY_ENTER] && !DC->prev_key_state[ALLEGRO_KEY_ENTER]){
		debug_log("Enter pressed\n");
		if((hero_posX+1 < 20 && DC->map[hero_posX+1][hero_posY] == 'B')
			|| (hero_posX+1 < 20 && DC->map[hero_posX+1][hero_posY] == 'K')){
			// chest or key chest at right side
			if(DC->map[hero_posX+1][hero_posY] == 'K'){
				SC->play(key_sound_path, ALLEGRO_PLAYMODE_ONCE);
				have_key = true;
				debug_log("Fonud key\n");
			}
			SC->play(chest_sound_path, ALLEGRO_PLAYMODE_ONCE);
			DC->map[hero_posX+1][hero_posY] = 'O';
		} 
		else if((hero_posX-1 >= 0 && DC->map[hero_posX-1][hero_posY] == 'B')
			|| (hero_posX-1 >= 0 && DC->map[hero_posX-1][hero_posY] == 'K')){
			// chest or key chest at left side
			if(DC->map[hero_posX-1][hero_posY] == 'K'){
				SC->play(key_sound_path, ALLEGRO_PLAYMODE_ONCE);
				have_key = true;
				debug_log("Fonud key\n");
			}
			SC->play(chest_sound_path, ALLEGRO_PLAYMODE_ONCE);
			DC->map[hero_posX-1][hero_posY] = 'O';
		}
		else if((hero_posY+1 < 12 && DC->map[hero_posX][hero_posY+1] == 'B')
			|| (hero_posY+1 < 12 && DC->map[hero_posX][hero_posY+1] == 'K')){
			// chest or key chest at bottom
			if(DC->map[hero_posX][hero_posY+1] == 'K'){
				SC->play(key_sound_path, ALLEGRO_PLAYMODE_ONCE);
				have_key = true;
				debug_log("Fonud key\n");
			}
			SC->play(chest_sound_path, ALLEGRO_PLAYMODE_ONCE);
			DC->map[hero_posX][hero_posY+1] = 'O';
		}
		else if((hero_posY-1 >= 0 && DC->map[hero_posX][hero_posY-1] == 'B')
			|| (hero_posY-1 >= 0 && DC->map[hero_posX][hero_posY-1] == 'K')){
			// chest or key chest at top
			if(DC->map[hero_posX][hero_posY-1] == 'K'){
				SC->play(key_sound_path, ALLEGRO_PLAYMODE_ONCE);
				have_key = true;
				debug_log("Fonud key\n");
			}
			SC->play(chest_sound_path, ALLEGRO_PLAYMODE_ONCE);
			DC->map[hero_posX][hero_posY-1] = 'O';
		}

		// press ENTER next to a door
		else if((hero_posX+1 < 20 && DC->map[hero_posX+1][hero_posY] == 'D')){
			// door at right side
			if(have_key){
				SC->play(door_sound_path, ALLEGRO_PLAYMODE_ONCE);
				DC->map[hero_posX + 1][hero_posY] = '@';
				debug_log("key used\n");
				have_key = false;
			}
			else{
				SC->play(door_locked_sound_path, ALLEGRO_PLAYMODE_ONCE);
			}
		}
		else if((hero_posX-1 >= 0 && DC->map[hero_posX-1][hero_posY] == 'D')){
			// door at left side
			if(have_key){
				SC->play(door_sound_path, ALLEGRO_PLAYMODE_ONCE);
				DC->map[hero_posX - 1][hero_posY] = '@';
				debug_log("key used\n");
				have_key = false;
			}
			else{
				SC->play(door_locked_sound_path, ALLEGRO_PLAYMODE_ONCE);
			}
		}
		else if((hero_posY+1 < 12 && DC->map[hero_posX][hero_posY+1] == 'D')){
			// door at bottom
			if(have_key){
				SC->play(door_sound_path, ALLEGRO_PLAYMODE_ONCE);
				DC->map[hero_posX][hero_posY + 1] = '@';
				debug_log("key used\n");
				have_key = false;
			}
			else{
				SC->play(door_locked_sound_path, ALLEGRO_PLAYMODE_ONCE);
			}
		}
		else if((hero_posY-1 >= 0 && DC->map[hero_posX][hero_posY-1] == 'D')){
			if(have_key){
				// door at top
				SC->play(door_sound_path, ALLEGRO_PLAYMODE_ONCE);
				DC->map[hero_posX][hero_posY - 1] = '@';
				debug_log("key used\n");
				have_key = false;
			}
			else{
				SC->play(door_locked_sound_path, ALLEGRO_PLAYMODE_ONCE);
			}
		}

		// press ENTER next to a closet
		else if( (hero_posX+1 < 20 && DC->map[hero_posX+1][hero_posY] == 'C') 
			|| (hero_posX-1 >= 0 && DC->map[hero_posX-1][hero_posY] == 'C')
			|| (hero_posY+1 < 12 && DC->map[hero_posX][hero_posY+1] == 'C') 
			|| (hero_posY-1 >= 0 && DC->map[hero_posX][hero_posY-1] == 'C')
			){
			static ALLEGRO_SAMPLE_INSTANCE *instance = nullptr;
			if(!in_closet){
				instance = SC->play(closet_open_sound_path, ALLEGRO_PLAYMODE_ONCE);
			}
			else{
				instance = SC->play(closet_close_sound_path, ALLEGRO_PLAYMODE_ONCE);
			}
			while(SC->is_playing(instance)){
				// wait until the sound is finished
			}
			in_closet = !in_closet;
		}
	}
}