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
#include <utility>

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
constexpr char closet_img_path[] = "./assets/image/closet.png";
constexpr char chest_img_path[] = "./assets/image/chest_close.png";
constexpr char chest_opened_img_path[] = "./assets/image/chest_opened.png";
constexpr char door_img_path[] = "./assets/image/door_closed.png";
constexpr char door_opened_img_path[] = "./assets/image/door_opened.png";

void
Level::init() {
	level = -1;
	grid_w = -1;
	grid_h = -1;

	ImageCenter *IC = ImageCenter::get_instance();
	block = IC->get(block_img_path);
	closet = IC->get(closet_img_path);
	chest = IC->get(chest_img_path);
	chest_opened = IC->get(chest_opened_img_path);
	door = IC->get(door_img_path);
	door_opened = IC->get(door_opened_img_path);
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
pair<size_t, size_t>
Level::load_level(int lvl) {
	DataCenter *DC = DataCenter::get_instance();
	size_t player_x = 0;
	size_t player_y = 0;

	char buffer[50];
	sprintf(buffer, LevelSetting::level_path_format, lvl);
	FILE *f = fopen(buffer, "r");
	GAME_ASSERT(f != nullptr, "cannot find level.");
	level = lvl;
	memset(DC->map, 0 , sizeof(DC->map));
	// read each grid
	for(int i = 0; i < 12; ++i) {
		for(int j = 0; j < 20; j++){
			fscanf(f, " %c", &DC->map[i][j]);
			if(DC->map[i][j] == 'P'){
				player_x = j;
				player_y = i;
			}
		}
	}
	fclose(f);

	debug_log("<Level> load level %d.\n", lvl);
	debug_log("map content:\n");
	for(int i = 0; i < 12; ++i) {
		for(int j = 0; j < 20; j++){
			debug_log("%c", DC->map[i][j]);
		}
		debug_log("\n");
	}

	return make_pair(player_x, player_y);
}

/**
 * @brief Updates monster_spawn_counter and create monster if needed.
*/


void
Level::draw() {
	/*
	LEVEL file format:
		- 12 rows, 20 columns
		- 1 for block, 0 for empty
		- P for player initial position
		- N for NPC initial position
		- B for box(chest)
		- O for opened chest
		- C for closet
		- D for door
	*/
	if(level == -1) return;

	DataCenter *DC = DataCenter::get_instance();
	for (int i = 0; i < 12; i++) {
		for (int j = 0; j < 20; j++) {
			switch(DC->map[i][j]){
				case '1':
					al_draw_bitmap(block, j * 64, i * 64, 0);
					break;
				case 'B':
					al_draw_bitmap(chest, j * 64, i * 64, 0);
					break;
				case 'C':
					al_draw_bitmap(closet, j * 64, i * 64, 0);
					break;
				case 'O':
					al_draw_bitmap(chest_opened, j * 64, i * 64, 0);
					break;
				case 'D':
					al_draw_bitmap(door, j * 64, i * 64, 0);
					break;
				case 'P':
				case 'N':
				case '0':
					break;
				default:
					debug_log("Invalid format in LEVEL file.\n");
			}
		}
	}
	al_flip_display();
}




