#include "Monster.h"
//#include "MonsterWolf.h"
//#include "MonsterCaveMan.h"
//#include "MonsterWolfKnight.h"
//#include "MonsterDemonNinja.h"
#include "../data/DataCenter.h"
#include "../data/ImageCenter.h"
#include "../data/GIFCenter.h"
#include "../Level.h"
#include "../shapes/Point.h"
#include "../shapes/Rectangle.h"
#include "../Utils.h"
#include <allegro5/allegro_primitives.h>
#include <cstdio>
using namespace std;

// fixed settings

namespace MonsterSetting {
	/*static constexpr char monster_imgs_root_path[static_cast<int>(MonsterType::MONSTERTYPE_MAX)][40] = {
		"./assets/image/monster/Wolf",
		"./assets/image/monster/CaveMan",
		"./assets/image/monster/WolfKnight",
		"./assets/image/monster/DemonNinja"
	};*/
	static constexpr char gif_root_path[40] ="./assets/gif/Npc";

	static constexpr char dir_path_prefix[][10] = {
		"left", "right", "front", "back"
	};
}

void Monster::init(int x, int y, bool hPatrol,bool fPatrol){
	speed = 2;
	init_x = x, init_y = y;
	horizontal_patrol = hPatrol;
	patrol_forward = fPatrol;
    state = MonsterState::PATROL;
	int grid = 64;
    detection_radius = 128; 
	for(size_t i = 0; i < static_cast<size_t>(MDir::NPCDIR_MAX); ++i){
		char buffer[50];
		sprintf(
			buffer, "%s/warrior_%s.gif",
			MonsterSetting::gif_root_path,
			MonsterSetting::dir_path_prefix[static_cast<int>(i)]
		);
		gifPath[static_cast<MDir>(i)] = std::string(buffer);
	}
	GIFCenter *GIFC = GIFCenter::get_instance();
	ALGIF_ANIMATION *gif = GIFC->get(gifPath[MDir::FRONT]);
	shape.reset(new Rectangle(
		init_x * grid,
		init_y * grid, 
		init_x * grid + gif -> width, 
		init_y * grid + gif -> height
	));
	hitbox_x = shape->center_x();
	hitbox_y = shape->center_y();
}


void Monster::update() {
    patrol();
}

void
Monster::draw() {
	GIFCenter *GIFC = GIFCenter::get_instance();
	ALGIF_ANIMATION *gif = GIFC->get(gifPath[dir]);
	algif_draw_gif(
		gif,
		shape->center_x() - gif->width / 2,
		shape->center_y() - gif->height  / 2, 0);
}

void Monster::patrol() {
    DataCenter *DC = DataCenter::get_instance();
    int dx = 0, dy = 0;

    if (horizontal_patrol) {
        dx = patrol_forward ? 1 : -1;
    } else {
        dy = patrol_forward ? 1 : -1;
    }

    // 計算下一個位置
    int next_x = (static_cast<int>(hitbox_x + dx)) / 64 ;
    int next_y = (static_cast<int>(hitbox_y + dy)) / 64 ;

    // 判斷是否碰到牆壁
    if (DC->map[next_y][next_x] != '0') {
        patrol_forward = !patrol_forward; // 遇到牆壁時反向
        // 更新方向
        if (horizontal_patrol) {
            dir = patrol_forward ? MDir::RIGHT : MDir::LEFT;
        } else {
            dir = patrol_forward ? MDir::FRONT : MDir::BACK;
        }
    } else {
        // 更新位置
        hitbox_x += dx * speed;
        hitbox_y += dy * speed;
		shape->update_center_x(hitbox_x);
    	shape->update_center_y(hitbox_y);
    }
}

bool Monster::is_in_fov(const Point &player_pos) {
    // 計算怪物和玩家的相對位置向量
	double fov_angle = M_PI / 3; 
    double dx = player_pos.x - shape->center_x();
    double dy = player_pos.y - shape->center_y();
    double distance = std::sqrt(dx * dx + dy * dy);

    // 判斷是否超出最大檢測距離
    if (distance > detection_radius) {
        return false;
    }

    // 計算相對角度
    double angle_to_player = std::atan2(dy, dx); // 玩家相對於怪物的角度
    double monster_facing_angle = dir_to_angle(dir); // 怪物當前朝向的角度

    // 計算角度差並判斷是否在 FOV 範圍內
    double angle_diff = std::fabs(angle_to_player - monster_facing_angle);
    if (angle_diff > M_PI) angle_diff = 2 * M_PI - angle_diff; // 保證角度在 [0, π] 範圍

    return angle_diff <= (fov_angle / 2); // 在扇形範圍內
}

double Monster::dir_to_angle(MDir direction) {
    switch (direction) {
        case MDir::FRONT: return M_PI / 2;   // 面向前 (90°)
        case MDir::BACK:  return -M_PI / 2;  // 面向後 (-90°)
        case MDir::LEFT:  return M_PI;       // 面向左 (180°)
        case MDir::RIGHT: return 0;          // 面向右 (0°)
        default: return 0;
    }
}

bool Monster::is_visible(const Point &player_pos) {
	DataCenter *DC = DataCenter::get_instance();
  	int x0 = static_cast<int>(hitbox_x/64);
    int y0 = static_cast<int>(hitbox_y/64);
    int x1 = static_cast<int>(player_pos.x /64);
    int y1 = static_cast<int>(player_pos.y)/64;

    // Bresenham's Line Algorithm
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        // 如果當前位置是牆壁，則玩家被遮擋
        if (DC->map[y0][x0] == 1) {
            return false;
        }
        // 如果到達終點，則玩家可見
        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }

    return true;
}

/*void Monster::alert(double x, double y) {

    Point monster_pos = {hitbox_x, hitbox_y};
    Point target_pos = {x, y};
    find_path(monster_pos, target_pos);  // 直接更新 path 佇列
	if (path.empty()) {
        debug_log("Path not found!\n");
    }

    if (!path.empty()) {
        Point next_step = path.front();
        path.pop();

        // 更新怪物位置
        hitbox_x = next_step.x;
        hitbox_y = next_step.y;
		shape->update_center_x(hitbox_x);
		shape->update_center_y(hitbox_y);
    }
}

void Monster::hunt(double x, double y) {
	DataCenter *DC = DataCenter::get_instance();
    Point player_pos{DC->hero->getCenterX(), DC->hero->getCenterY()};

    double dx = player_pos.x - hitbox_x;
    double dy = player_pos.y - hitbox_y;
    double distance = std::sqrt(dx * dx + dy * dy);

    if (distance <= chase_radius) { // 朝玩家移動
        hitbox_x += (dx / distance) * speed;
        hitbox_y += (dy / distance) * speed;
		shape->update_center_x(hitbox_x);
		shape->update_center_y(hitbox_y);
    } 
	else if (!is_visible(player_pos) || distance > detection_radius) {
        state = MonsterState::PATROL; // 玩家不可見時回到巡邏
    }
}*/
