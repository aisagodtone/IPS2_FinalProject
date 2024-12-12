#include "Level.h"
#include <string>
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/ImageCenter.h"
#include <allegro5/allegro_primitives.h>
#include "shapes/Point.h"
#include "shapes/Rectangle.h"
#include <array>
#include<cstdlib>

using namespace std;

// fixed settings
namespace LevelSetting {
	constexpr char level_path_format[] = "./assets/level/LEVEL%d.txt";
	//! @brief Grid size for each level.
	constexpr array<int, 4> grid_size = {
		40, 40, 40, 40
	};
};
constexpr char block_img_path[] = "./assets/image/block.png";

void
Level::init() {
	level = -1;
	grid_w = -1;
	grid_h = -1;

	ImageCenter *IC = ImageCenter::get_instance();
	block = IC->get(block_img_path);
}

/**
 * @brief Loads level data from input file. The input file is required to follow the format.
 * @param lvl level index. The path format is a fixed setting in code.
 * @details The content of the input file should be formatted as follows:
 *          * Total number of monsters.
 *          * Number of each different number of monsters. The order and number follows the definition of MonsterType.
 *          * Indefinite number of Point (x, y), represented in grid format.
 * @see level_path_format
 * @see MonsterType
 */
void
Level::load_level(int lvl) {
	DataCenter *DC = DataCenter::get_instance();

	char buffer[50];
	sprintf(buffer, LevelSetting::level_path_format, lvl);
	FILE *f = fopen(buffer, "r");
	GAME_ASSERT(f != nullptr, "cannot find level.");
	level = lvl;
	memset(DC->map, 0 , sizeof(DC->map));
	// read each grid
	for(int i = 0; i < 12; ++i) {
		for(int j = 0; j < 20; j++){
			fscanf(f, "%d", &DC->map[i][j]);
		}
	}

	fclose(f);
	debug_log("<Level> load level %d.\n", lvl);
}

/**
 * @brief Updates monster_spawn_counter and create monster if needed.
*/


void
Level::draw() {
	if(level == -1) return;

	// draw block on the map (1 in LEVEL file -> block)
	for (int i = 0; i < 12; i++) {
		for (int j = 0; j < 20; j++) {
			if(DataCenter::get_instance()->map[i][j] == 1){
				al_draw_bitmap(block, j * 64, i * 64, 0);
			}
		}
	}
	al_flip_display();
}




