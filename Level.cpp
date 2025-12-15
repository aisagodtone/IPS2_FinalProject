#include "Level.h"
#include <string>
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/ImageCenter.h"
#include <allegro5/allegro_primitives.h>
#include "shapes/Point.h"
#include "shapes/Rectangle.h"

#include <array>
#include <cstdlib>
#include <utility>
#include <cstdio>
#include <cstring>
#include <vector>
#include <ctime>

using namespace std;

namespace LevelSetting {
    constexpr char level_path_format[] = "./assets/level/LEVEL%d.txt";
}

constexpr char block_img_path[]        = "./assets/image/block.png";
constexpr char closet_img_path[]       = "./assets/image/closet.png";
constexpr char chest_img_path[]        = "./assets/image/chest_close.png";
constexpr char chest_opened_img_path[] = "./assets/image/chest_opened.png";
constexpr char door_img_path[]         = "./assets/image/door_closed.png";
constexpr char door_opened_img_path[]  = "./assets/image/door_opened.png";
constexpr char gate1_img_path[]        = "./assets/image/gate1_closed.png";
constexpr char gate2_img_path[]        = "./assets/image/gate2_closed.png";

void Level::init() {
    level = -1;

    ImageCenter *IC = ImageCenter::get_instance();
    block        = IC->get(block_img_path);
    closet       = IC->get(closet_img_path);
    chest        = IC->get(chest_img_path);
    chest_opened = IC->get(chest_opened_img_path);
    door         = IC->get(door_img_path);
    door_opened  = IC->get(door_opened_img_path);
    gate1        = IC->get(gate1_img_path);
    gate2        = IC->get(gate2_img_path);

    // 初始化亂數種子（只需一次）
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

/**
 * lvl == 1：隨機生成地圖（P / D / K / B）
 * 其他 lvl：照原本讀 LEVEL*.txt
 */
std::pair<size_t, size_t> Level::load_level(int lvl) {
    DataCenter *DC = DataCenter::get_instance();

    size_t player_x = 1;
    size_t player_y = 1;

    const int H = 12;
    const int W = 20;

    // =========================
    // level 1：隨機生成
    // =========================
    if (lvl == 1) {
        level = lvl;

        // 先全部清成 0
        std::memset(DC->map, 0, sizeof(DC->map));

        // 四邊牆壁 '1'，內部 '0'
        for (int i = 0; i < H; ++i) {
            for (int j = 0; j < W; ++j) {
                if (i == 0 || i == H - 1 || j == 0 || j == W - 1) {
                    DC->map[i][j] = '1';
                } else {
                    DC->map[i][j] = '0';
                }
            }
        }

        // 收集所有內部可走的格子 (目前都是 '0')
        std::vector<std::pair<int,int>> free_cells;
        for (int i = 1; i < H - 1; ++i) {
            for (int j = 1; j < W - 1; ++j) {
                if (DC->map[i][j] == '0') {
                    free_cells.emplace_back(i, j); // (row, col)
                }
            }
        }

        auto take_random_cell = [&](void) -> std::pair<int,int> {
            int idx = std::rand() % static_cast<int>(free_cells.size());
            auto cell = free_cells[idx];
            free_cells.erase(free_cells.begin() + idx);
            return cell;
        };

        // 1) Hero 出生點 P
        {
            auto rc = take_random_cell();
            int ry = rc.first;
            int cx = rc.second;
            player_x = static_cast<size_t>(cx);
            player_y = static_cast<size_t>(ry);
            DC->map[ry][cx] = 'P';
        }

        // 2) 門 D
        {
            auto rc = take_random_cell();
            int ry = rc.first;
            int cx = rc.second;
            DC->map[ry][cx] = 'D';
        }

        // 3) 唯一鑰匙箱子 K
        {
            auto rc = take_random_cell();
            int ry = rc.first;
            int cx = rc.second;
            DC->map[ry][cx] = 'K';
        }

        // 4) 額外普通箱子 B：總箱子數 3~7（含 K）
        int total_boxes = 3 + (std::rand() % 5);  // 3,4,5,6,7
        int extra_boxes = total_boxes - 1;        // 扣掉 K

        for (int n = 0; n < extra_boxes && !free_cells.empty(); ++n) {
            auto rc = take_random_cell();
            int ry = rc.first;
            int cx = rc.second;
            DC->map[ry][cx] = 'B';
        }

        debug_log("<Level> load random level %d.\n", lvl);
        debug_log("map content:\n");
        for (int i = 0; i < H; ++i) {
            for (int j = 0; j < W; ++j) {
                debug_log("%c", DC->map[i][j]);
            }
            debug_log("\n");
        }

        return std::make_pair(player_x, player_y);
    }

    // =========================
    // 其他關卡：照原本讀 LEVEL*.txt
    // =========================
    char buffer[50];
    std::sprintf(buffer, LevelSetting::level_path_format, lvl);
    FILE *f = std::fopen(buffer, "r");
    GAME_ASSERT(f != nullptr, "cannot find level.");
    level = lvl;

    std::memset(DC->map, 0, sizeof(DC->map));

    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            std::fscanf(f, " %c", &DC->map[i][j]);
            if (DC->map[i][j] == 'P') {
                player_x = j;
                player_y = i;
            }
        }
    }
    std::fclose(f);

    debug_log("<Level> load level %d from file.\n", lvl);
    debug_log("map content:\n");
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            debug_log("%c", DC->map[i][j]);
        }
        debug_log("\n");
    }

    return std::make_pair(player_x, player_y);
}

/**
 * 照 map 畫出地圖
 */
void Level::draw() {
    if (level == -1) return;

    DataCenter *DC = DataCenter::get_instance();

    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 20; ++j) {
            switch (DC->map[i][j]) {
                case '1':
                    al_draw_bitmap(block, j * 64, i * 64, 0);
                    break;
                case 'B':
                case 'K':
                case 'x':
                case 'y':
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
                case '@':
                    al_draw_bitmap(door_opened, j * 64, i * 64, 0);
                    break;
                case 'X':
                    al_draw_bitmap(gate1, j * 64, i * 64, 0);
                    break;
                case 'Y':
                    al_draw_bitmap(gate2, j * 64, i * 64, 0);
                    break;
                case 'P':
                case 'N':
                case '0':
                    // 不畫任何東西
                    break;
                default:
                    debug_log("Invalid format in LEVEL file.\n");
            }
        }
    }
}
