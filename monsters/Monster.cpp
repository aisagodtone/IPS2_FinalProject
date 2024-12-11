#include "Monster.h"
#include "MonsterWolf.h"
#include "MonsterCaveMan.h"
#include "MonsterWolfKnight.h"
#include "MonsterDemonNinja.h"
#include "../data/DataCenter.h"
#include "../data/ImageCenter.h"
#include "../Level.h"
#include "../shapes/Point.h"
#include "../shapes/Rectangle.h"
#include "../Utils.h"
#include <allegro5/allegro_primitives.h>

/*using namespace std;

// fixed settings
enum class Dir {
	UP, DOWN, LEFT, RIGHT
};
namespace MonsterSetting {
	static constexpr char monster_imgs_root_path[static_cast<int>(MonsterType::MONSTERTYPE_MAX)][40] = {
		"./assets/image/monster/Wolf",
		"./assets/image/monster/CaveMan",
		"./assets/image/monster/WolfKnight",
		"./assets/image/monster/DemonNinja"
	};
	static constexpr char dir_path_prefix[][10] = {
		"UP", "DOWN", "LEFT", "RIGHT"
	};
}


Monster *Monster::create_monster(MonsterType type, const vector<Point> &path) {
	switch(type) {
		case MonsterType::WOLF: {
			return new MonsterWolf{path};
		}
		case MonsterType::CAVEMAN: {
			return new MonsterCaveMan{path};
		}
		case MonsterType::WOLFKNIGHT: {
			return new MonsterWolfKnight{path};
		}
		case MonsterType::DEMONNIJIA: {
			return new MonsterDemonNinja{path};
		}
		case MonsterType::MONSTERTYPE_MAX: {}
	}
	GAME_ASSERT(false, "monster type error.");
}


Dir convert_dir(const Point &v) {
	if(v.y < 0 && abs(v.y) >= abs(v.x))
		return Dir::UP;
	if(v.y > 0 && abs(v.y) >= abs(v.x))
		return Dir::DOWN;
	if(v.x < 0 && abs(v.x) >= abs(v.y))
		return Dir::LEFT;
	if(v.x > 0 && abs(v.x) >= abs(v.y))
		return Dir::RIGHT;
	return Dir::RIGHT;
}

Monster::Monster(const vector<Point> &path, MonsterType type) {
	DataCenter *DC = DataCenter::get_instance();

	shape.reset(new Rectangle{0, 0, 0, 0});
	this->type = type;
	dir = Dir::RIGHT;
	bitmap_img_id = 0;
	bitmap_switch_counter = 0;
	for(const Point &p : path)
		this->path.push(p);
	if(!path.empty()) {
		const Point &grid = this->path.front();
		const Rectangle &region = DC->level->grid_to_region(grid);
		// Temporarily set the bounding box to the center (no area) since we haven't got the hit box of the monster.
		shape.reset(new Rectangle{region.center_x(), region.center_y(), region.center_x(), region.center_y()});
		this->path.pop();
	}
}


void Monster::update() {
	DataCenter *DC = DataCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();

	// After a period, the bitmap for this monster should switch from (i)-th image to (i+1)-th image to represent animation.
	if(bitmap_switch_counter) --bitmap_switch_counter;
	else {
		bitmap_img_id = (bitmap_img_id + 1) % (bitmap_img_ids[static_cast<int>(dir)].size());
		bitmap_switch_counter = bitmap_switch_freq;
	}
	// v (velocity) divided by FPS is the actual moving pixels per frame.
	double movement = v / DC->FPS;
	// Keep trying to move to next destination in "path" while "path" is not empty and we can still move.
	while(!path.empty() && movement > 0) {
		const Point &grid = this->path.front();
		const Rectangle &region = DC->level->grid_to_region(grid);
		const Point &next_goal = Point{region.center_x(), region.center_y()};

		// Extract the next destination as "next_goal". If we want to reach next_goal, we need to move "d" pixels.
		double d = Point::dist(Point{shape->center_x(), shape->center_y()}, next_goal);
		Dir tmpdir;
		if(d < movement) {
			// If we can move more than "d" pixels in this frame, we can directly move onto next_goal and reduce "movement" by "d".
			movement -= d;
			tmpdir = convert_dir(Point{next_goal.x - shape->center_x(), next_goal.y - shape->center_y()});
			shape.reset(new Rectangle{
				next_goal.x, next_goal.y,
				next_goal.x, next_goal.y
		});
			path.pop();
		} else {
			// Otherwise, we move exactly "movement" pixels.
			double dx = (next_goal.x - shape->center_x()) / d * movement;
			double dy = (next_goal.y - shape->center_y()) / d * movement;
			tmpdir = convert_dir(Point{dx, dy});
			shape->update_center_x(shape->center_x() + dx);
			shape->update_center_y(shape->center_y() + dy);
			movement = 0;
		}
		// Update facing direction.
		dir = tmpdir;
	}
	// Update real hit box for monster.
	char buffer[50];
	sprintf(
		buffer, "%s/%s_%d.png",
		MonsterSetting::monster_imgs_root_path[static_cast<int>(type)],
		MonsterSetting::dir_path_prefix[static_cast<int>(dir)],
		bitmap_img_ids[static_cast<int>(dir)][bitmap_img_id]);
	ALLEGRO_BITMAP *bitmap = IC->get(buffer);
	const double &cx = shape->center_x();
	const double &cy = shape->center_y();
	// We set the hit box slightly smaller than the actual bounding box of the image because there are mostly empty spaces near the edge of a image.
	const int &h = al_get_bitmap_width(bitmap) * 0.8;
	const int &w = al_get_bitmap_height(bitmap) * 0.8;
	shape.reset(new Rectangle{
		(cx - w / 2.), (cy - h / 2.),
		(cx - w / 2. + w), (cy - h / 2. + h)
	});
}

void
Monster::draw() {
	ImageCenter *IC = ImageCenter::get_instance();
	char buffer[50];
	sprintf(
		buffer, "%s/%s_%d.png",
		MonsterSetting::monster_imgs_root_path[static_cast<int>(type)],
		MonsterSetting::dir_path_prefix[static_cast<int>(dir)],
		bitmap_img_ids[static_cast<int>(dir)][bitmap_img_id]);
	ALLEGRO_BITMAP *bitmap = IC->get(buffer);
	al_draw_bitmap(
		bitmap,
		shape->center_x() - al_get_bitmap_width(bitmap) / 2,
		shape->center_y() - al_get_bitmap_height(bitmap) / 2, 0);
}*/
