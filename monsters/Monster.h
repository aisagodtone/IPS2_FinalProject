#ifndef MONSTER_H_INCLUDED
#define MONSTER_H_INCLUDED

#include "../Object.h"
#include "../shapes/Rectangle.h"
#include "../shapes/Point.h"
#include <allegro5/allegro.h>
#include <vector>
#include <queue>
#include <map>
#include <string>

enum class MDir{
	LEFT,
	RIGHT,
	FRONT,
	BACK,
	NPCDIR_MAX
};

// fixed settings
enum class MonsterState {
	PATROL, ALERT, HUNT
};

/**
 * @brief The class of a monster (enemies).
 * @details Monster inherits Object and takes Rectangle as its hit box.
 */
class Monster : public Object
{
public:
	//static Monster *create_monster(MonsterType type, const std::vector<Point> &path);
public:
	//Monster(const std::vector<Point> &path, MonsterType type);
	void init(int x, int y, bool hPatrol,bool fPatrol); // NPC's init_x, init_y on map[][]
	void update();
	void draw();
	void patrol();
	//void hunt(double x, double y);
	//void alert(double x, double y);
	double dir_to_angle(MDir direction);
	bool is_in_fov(const Point &player_pos);
	bool is_visible(const Point &player_pos);
	//void find_path(const Point &monster_pos,const Point &player_pos); 
	//~Monster();

	//const int &get_money() const { return money; }
	//int HP;
	//const std::queue<Point> &get_path() const { return path; }
protected:
	/**
	 * @var HP
	 * @brief Health point of a monster.
	 **
	 * @var v
	 * @brief Moving speed of a monster.
	 **
	 * @var money
	 * @brief The amount of money that player will earn when the monster is killed.
	 **
	 * @var bitmap_img_ids
	 * @brief The first vector is the Dir index, and the second vector is image id.
	 * @details `bitmap_img_ids[Dir][<ordered_id>]`
	 **
	 * @var bitmap_switch_counter
	 * @brief Counting down for bitmap_switch_freq.
	 * @see Monster::bitmap_switch_freq
	 **
	 * @var bitmap_switch_freq
	 * @brief Number of frames required to change to the next move pose for the current facing direction.
	 * @details The variable is left for child classes to define.
	 * 
	 * @var bitmap_img_id
	 * @brief Move pose of the current facing direction.
	 **
	 * @var dir
	 * @brief Current facing direction.
	 **
	 * @var path
	 * @brief The walk path of a monster, represented in grid format.
	 * @see Level::grid_to_region(const Point &grid) const
	*/
	//int money;
	//std::vector<std::vector<int>> bitmap_img_ids;
	//int bitmap_switch_counter;
	//int bitmap_switch_freq;
	//int bitmap_img_id;
private:
  	//ALLEGRO_TIMER *alert_timer;
	MonsterState state;
	int speed;
	MDir dir;
	int patrol_line;           // 巡邏行或列（取決於方向）
    bool horizontal_patrol;    // 是否為橫向巡邏（true 為橫向，false 為縱向）
    bool patrol_forward;       // 當前巡邏方向（true 為正向，false 為反向）
	int detection_radius;    // 警戒范围
   // int chase_radius;        // 追击范围
	int init_x, init_y;
	double hitbox_x, hitbox_y;
	double M_PI;
	
	std::map<MDir, std::string> gifPath;
	//std::queue<Point> path;

};

#endif
