#include "hero.h"
#include "data/DataCenter.h"
#include "data/GIFCenter.h"
#include <cstdio>
#include "shapes/Rectangle.h"
#include "Utils.h"
#include "data/SoundCenter.h"

using namespace std;

// fixed settings
enum class Dir {
	UP, DOWN, LEFT, RIGHT
};

namespace HeroSetting {
	static constexpr char gif_root_path[40] ="./assets/gif/hero";

	static constexpr char dir_path_prefix[][10] = {
		"left", "right", "front", "back"
	};
}

constexpr char chest_sound_path[] = "./assets/sound/chest_open.wav";

void Hero::init(pair<size_t, size_t> pos){
	int grid = 64;
	hero_posX = pos.first;
	hero_posY = pos.second;
	for(size_t i = 0; i < static_cast<size_t>(HeroState::HEROSTATE_MAX); ++i){
		char buffer[50];
		sprintf(
			buffer, "%s/dragonite_%s.gif",
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
	algif_draw_gif(
		gif,
		shape->center_x() - gif->width / 2,
		shape->center_y() - gif->height / 2, 0);
}

void Hero::update(){
	DataCenter *DC = DataCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	if(DC->key_state[ALLEGRO_KEY_W]){
		debug_log("hero in map x:%d y: %d \n",hero_posY  ,hero_posX );
		if(DC->map[(static_cast<int>(shape->center_y() - speed))/64][hero_posY] == '1'
			|| DC->map[(static_cast<int>(shape->center_y() - speed))/64][hero_posY] == 'D'
			|| DC->map[(static_cast<int>(shape->center_y() - speed))/64][hero_posY] == 'C'
			|| DC->map[(static_cast<int>(shape->center_y() - speed))/64][hero_posY] == 'B'
			|| DC->map[(static_cast<int>(shape->center_y() - speed))/64][hero_posY] == 'O'){
			return;
		}
		shape->update_center_y(shape->center_y()- speed);
		hero_posX = shape->center_y()/64;
		debug_log("update hitbox center x:%lf y: %lf \n", shape->center_x() , shape->center_y());
		state = HeroState::BACK;
	}
	else if(DC->key_state[ALLEGRO_KEY_A]){
		debug_log("hero in map x:%d y: %d \n",hero_posY  ,hero_posX );
		if(DC->map[hero_posX][(static_cast<int>(shape->center_x()- speed))/64] == '1'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() - speed))/64] == 'D'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() - speed))/64] == 'C'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() - speed))/64] == 'B'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() - speed))/64] == 'O'){
			return;
		}
		shape->update_center_x(shape->center_x()- speed);
		hero_posY = shape->center_x()/64;
		debug_log("update hitbox center x:%lf y: %lf \n", shape->center_x() , shape->center_y());
		state = HeroState::LEFT;
	}
	else if(DC->key_state[ALLEGRO_KEY_S]){
		debug_log("hero in map x:%d y: %d \n",hero_posY  ,hero_posX );
		if(DC->map[(static_cast<int>(shape->center_y()+ speed))/64][hero_posY] == '1'
			|| DC->map[(static_cast<int>(shape->center_y() + speed))/64][hero_posY] == 'D'
			|| DC->map[(static_cast<int>(shape->center_y() + speed))/64][hero_posY] == 'C'
			|| DC->map[(static_cast<int>(shape->center_y() + speed))/64][hero_posY] == 'B'
			|| DC->map[(static_cast<int>(shape->center_y() + speed))/64][hero_posY] == 'O'){
			return;
		}
		shape->update_center_y(shape->center_y()+ speed);
		hero_posX = shape->center_y()/64;
		debug_log("update hitbox center x:%lf y: %lf \n", shape->center_x() , shape->center_y());
		state = HeroState::FRONT;
	}
	else if(DC->key_state[ALLEGRO_KEY_D]){
		debug_log("hero in map x:%d y: %d \n",hero_posY  ,hero_posX );
		if(DC->map[hero_posX][(static_cast<int>(shape->center_x() + speed))/64] == '1'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() + speed))/64] == 'D'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() + speed))/64] == 'C'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() + speed))/64] == 'B'
			|| DC->map[hero_posX][(static_cast<int>(shape->center_x() + speed))/64] == 'O'){
			return;
		}
		shape->update_center_x(shape->center_x()+ speed);
		hero_posY = shape->center_x()/64;
		debug_log("update hitbox center x:%lf y: %lf \n", shape->center_x() , shape->center_y());
		state = HeroState::RIGHT;
	}
	else if(DC->key_state[ALLEGRO_KEY_ENTER]){
		// press ENTER next to a chest to open it
		debug_log("Enter pressed\n");
		if(hero_posX+1 < 20 && DC->map[hero_posX+1][hero_posY] == 'B'){
			SC->play(chest_sound_path, ALLEGRO_PLAYMODE_ONCE);
			DC->map[hero_posX+1][hero_posY] = 'O';
		} 
		else if(hero_posX-1 >= 0 && DC->map[hero_posX-1][hero_posY] == 'B'){
			SC->play(chest_sound_path, ALLEGRO_PLAYMODE_ONCE);
			DC->map[hero_posX-1][hero_posY] = 'O';
		}
		else if(hero_posY+1 < 12 && DC->map[hero_posX][hero_posY+1] == 'B'){
			SC->play(chest_sound_path, ALLEGRO_PLAYMODE_ONCE);
			DC->map[hero_posX][hero_posY+1] = 'O';
		}
		else if(hero_posY-1 >= 0 && DC->map[hero_posX][hero_posY-1] == 'B'){
			SC->play(chest_sound_path, ALLEGRO_PLAYMODE_ONCE);
			DC->map[hero_posX][hero_posY-1] = 'O';
		}
	}
}