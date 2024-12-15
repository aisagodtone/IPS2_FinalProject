#include "hero.h"
#include "data/DataCenter.h"
#include "data/GIFCenter.h"
#include <cstdio>
#include "shapes/Rectangle.h"
#include "Utils.h"

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



void Hero::init(){
	int grid = 64;
	hero_posX = 1;
	hero_posY = 3;
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
		1 * grid,
		3 * grid, 
		1 * grid + gif -> width, 
		3 * grid + gif -> height
	));
}

void Hero::draw(){
	GIFCenter *GIFC = GIFCenter::get_instance();
	ALGIF_ANIMATION *gif = GIFC->get(gifPath[state]);
	algif_draw_gif(
		gif,
		shape->center_x() - gif->width / 2,
		shape->center_y() - gif->height  / 2, 0);
}

void Hero::update(){
	DataCenter *DC = DataCenter::get_instance();
	if(DC->key_state[ALLEGRO_KEY_W]){
		debug_log("hero in map x:%d y: %d \n",hero_posY  ,hero_posX );
		if(DC->map[(static_cast<int>(shape->center_y()- speed))/64][hero_posY] == 1){
			return;
		}
		shape->update_center_y(shape->center_y()- speed);
		hero_posX = shape->center_y()/64;
		hero_hitY = shape->center_y();
		debug_log("update hitbox center x:%lf y: %lf \n", shape->center_x() , shape->center_y());
		state = HeroState::BACK;
	}
	else if(DC->key_state[ALLEGRO_KEY_A]){
		debug_log("hero in map x:%d y: %d \n",hero_posY  ,hero_posX );
		if(DC->map[hero_posX][(static_cast<int>(shape->center_x()- speed))/64] == 1){
			return;
		}
		shape->update_center_x(shape->center_x()- speed);
		hero_posY = shape->center_x()/64;
		hero_hitX = shape->center_x();
		debug_log("update hitbox center x:%lf y: %lf \n", shape->center_x() , shape->center_y());
		state = HeroState::LEFT;
	}
	else if(DC->key_state[ALLEGRO_KEY_S]){
		debug_log("hero in map x:%d y: %d \n",hero_posY  ,hero_posX );
		if(DC->map[(static_cast<int>(shape->center_y()+ speed))/64][hero_posY] == 1){
			return;
		}
		shape->update_center_y(shape->center_y()+ speed);
		hero_posX = shape->center_y()/64;
		hero_hitY = shape->center_y();
		debug_log("update hitbox center x:%lf y: %lf \n", shape->center_x() , shape->center_y());
		state = HeroState::FRONT;
	}
	else if(DC->key_state[ALLEGRO_KEY_D]){
		debug_log("hero in map x:%d y: %d \n",hero_posY  ,hero_posX );
		if(DC->map[hero_posX][(static_cast<int>(shape->center_x()+ speed))/64] == 1){
			return;
		}
		shape->update_center_x(shape->center_x()+ speed);
		hero_posY = shape->center_x()/64;
		hero_hitX = shape->center_x();
		debug_log("update hitbox center x:%lf y: %lf \n", shape->center_x() , shape->center_y());
		state = HeroState::RIGHT;
	}
}

const double &Hero::getCenterX() const{
	return hero_hitX;
}

const double &Hero::getCenterY() const{
	return hero_hitY;
}