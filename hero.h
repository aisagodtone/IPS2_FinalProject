#ifndef HERO_H_INCLUDED
#define HERO_H_INCLUDED

#include "Object.h"
#include <string>
#include <map>
#include <allegro5/allegro.h>

// fixed settings
enum class HeroState {
	LEFT,
	RIGHT,
	FRONT,
	BACK,
	HEROSTATE_MAX
};

/**
 * @brief The class of a monster (enemies).
 * @details Monster inherits Object and takes Rectangle as its hit box.
 */
class Hero : public Object
{

public:
	void init(std::pair<size_t, size_t>);
	void update();
	void draw();
	bool have_key;

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

private:
	HeroState state = HeroState::FRONT;
	double speed = 2.0;
	double run_speed = 10.0;
	int hero_posX, hero_posY;
	std::map< HeroState, std::string> gifPath;
	ALLEGRO_BITMAP *player_mask;
};

#endif
