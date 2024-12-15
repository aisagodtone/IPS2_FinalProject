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
    detection_radius = 256.0; // 默认警戒范围
    chase_radius = 128.0;     // 默认追捕范围
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
	DataCenter *DC = DataCenter::get_instance();
	//debug_log("NPC postion x: %lf, y: %lf\n", hitbox_x, hitbox_y);
	debug_log("NPC state: %d\n", static_cast<int>(state));
    // 假設玩家位置和場景牆壁
    Point player_pos{DC->hero->getCenterX(), DC->hero->getCenterY()};
    Point monster_pos{hitbox_x, hitbox_y};

    // 扇形視野檢測參數
    double fov_angle = M_PI / 3;  // 60 度視野
    double alert_distance = detection_radius; // 警戒距離
    double hunt_distance = chase_radius; // 追捕距離

    // 狀態邏輯判斷
    if (is_in_fov(player_pos, fov_angle, alert_distance) && is_visible(player_pos) && Point::dist(player_pos, monster_pos) <= alert_distance) {
        if (Point::dist(player_pos, monster_pos) <= hunt_distance) {
            state = MonsterState::HUNT;
			speed = 2.2; // 近距離追捕
        } 
		else {
            state = MonsterState::ALERT; // 遠距離警戒
        }
    } else if (state == MonsterState::HUNT) {
        state = MonsterState::PATROL; // 如果看不到玩家且在追捕狀態，回到巡邏
    }
	 switch (state) {
        case MonsterState::PATROL:
            patrol();
            break;

        case MonsterState::ALERT:
            alert(DC->hero->getCenterX(), DC->hero->getCenterY());
            break;

        case MonsterState::HUNT:
            //hunt(DC->hero->getCenterX(), DC->hero->getCenterY());
            break;
    }
	
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
    if (DC->map[next_y][next_x] == 1) {
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

void Monster::alert(double x, double y) {

    Point monster_pos = {hitbox_x, hitbox_y};
    Point target_pos = {x, y};
    find_path(monster_pos, target_pos);  // 直接更新 path 佇列
	if (path.empty()) {
        debug_log("Path not found!\n");
    }
    // 使用 path 來移動怪物
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
}

bool Monster::is_in_fov(const Point &player_pos, double fov_angle, double max_distance) {
    // 計算怪物和玩家的相對位置向量
    double dx = player_pos.x - shape->center_x();
    double dy = player_pos.y - shape->center_y();
    double distance = std::sqrt(dx * dx + dy * dy);

    // 判斷是否超出最大檢測距離
    if (distance > max_distance) {
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

void Monster::find_path(const Point& start, const Point& end) {
    DataCenter* DC = DataCenter::get_instance();
    // 使用 map 或 bool 陣列來紀錄已訪問的點
    std::map<int, std::map<int, bool>> visited;
    std::map<int, std::map<int, Point>> previous_map;  // 用來記錄每個點的前一個點

    // 初始化起始點和終點
    Point current = start;

    // BFS 設定
    std::queue<Point> q;
    q.push(start);  // 將起點加入隊列
    visited[start.center_x() / 64][start.center_y() / 64] = true; // 標記起點為已訪問

    // 設定四個方向移動
    const int dx[] = {1, 0, -1, 0};
    const int dy[] = {0, 1, 0, -1};

    while (!q.empty()) {
        current = q.front();
        q.pop();

        // 如果到達終點，則開始回溯路徑
        if (current.center_x() == end.center_x() && current.center_y() == end.center_y()) {
            // 回溯路徑並將每個點加入 path
            while (current.center_x() != start.center_x() || current.center_y() != start.center_y()) {
                path.push(current);  // 將當前點加入路徑
                current = previous_map[current.center_x() / 64][current.center_y() / 64]; // 使用記錄的上一個點
            }
            path.push(start);  // 把起點也加入路徑
            break;
        }

        // 檢查四個方向的鄰居
        for (int i = 0; i < 4; ++i) {
            int nx = current.center_x() + dx[i] * 64;
            int ny = current.center_y() + dy[i] * 64;
            Point next(nx, ny);

            // 判斷是否越界、是否已經訪問過、是否是障礙物
            if (nx >= 0 && ny >= 0 && 
                !visited[nx / 64][ny / 64] && 
                DC->map[ny / 64][nx / 64] != 1) {
                visited[nx / 64][ny / 64] = true;  // 標記為已訪問
                previous_map[nx / 64][ny / 64] = current;  // 記錄來自當前點的鄰居
                q.push(next);  // 將新位置加入隊列
            }
        }
    }
}



/*Monster::~Monster(){
	 if (alert_timer) {
        al_destroy_timer(alert_timer);
    }
}*/


/*
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
}*/

/*void
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
