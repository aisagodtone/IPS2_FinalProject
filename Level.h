#ifndef LEVEL_H_INCLUDED
#define LEVEL_H_INCLUDED

#include <vector>
#include <utility>
#include <tuple>
#include "./shapes/Rectangle.h"

/**
 * @brief The class manages data of each level.
 * @details The class could load level with designated input file and record. The level itself will decide when to create next monster.
 * @see DataCenter::level
 */
class Level
{
public:
	Level() {}
	void init();
	void load_level(int lvl);
	void draw();
	
private:
	/**
	 * @brief Stores the monster's attack route, whose Point is represented in grid format.
	 */
	/**
	 * @brief The index of current level.
	 */
	int level;
	/**
	 * @brief Number of grid in x-direction.
	 */
	int grid_w;
	/**
	 * @brief Number of grid in y-direction.
	 */
	int grid_h;
	/**
	 * @brief Time remaining for the next monster to spawn.
	 */
	/**
	 * @brief Number of each different type of monsters.
	 */
};

#endif
