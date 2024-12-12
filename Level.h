#ifndef LEVEL_H_INCLUDED
#define LEVEL_H_INCLUDED

#include <vector>
#include <utility>
#include <tuple>
#include "./shapes/Rectangle.h"
#include <allegro5/bitmap.h>

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

	// bitmap of block
	ALLEGRO_BITMAP *block;
};

#endif
