#include "Game.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/OperationCenter.h"
#include "data/SoundCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include "Player.h"
#include "Level.h"
#include "hero.h"
#include "monsters/Monster.h"

#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_acodec.h>

#include <vector>
#include <cstring>
#include <utility>
#include <cstdlib>
#include <algorithm>

// fixed settings
constexpr char game_icon_img_path[]           = "./assets/image/game_icon.png";
constexpr char game_start_sound_path[]        = "./assets/sound/growl.wav";
constexpr char background_img_path[]          = "./assets/image/StartBackground.jpg";
constexpr char ingame_background_sound_path[] = "./assets/sound/ingame_bgm.mp3";
constexpr char menu_button_img_path[]         = "./assets/image/menu_button.png";
constexpr char mask_img_path[]                = "./assets/image/mask.png";
constexpr char e_mask_img_path[]              = "./assets/image/toggle_E_mask.png";

// === black mask 設定 ===
#define MASK_SHIFT_X 1280
#define MASK_SHIFT_Y 768

ALLEGRO_BITMAP *player_mask   = nullptr;
ALLEGRO_BITMAP *player_mask_E = nullptr;   // 按 E 時用的 mask
bool            mask          = true;

// ======================

Rectangle       menu_button_area;
ALLEGRO_BITMAP *pause_menu_button = nullptr;

// 有幾隻怪物實際會被啟用，只使用 DC->monsters[0 .. g_active_monsters-1]
int   g_active_monsters   = 2;

// ===== E mask 能量系統（給 UI 用 extern） =====
float g_mask_energy       = 1.0f;  // 0.0 ~ 1.0
bool  g_mask_overheated   = false; // 用光後進入過熱，回復變慢
bool  g_e_mask_active     = false; // 目前畫的是 E 版本 mask 嗎？

/**
 * @brief Game entry.
 */
void Game::execute() {
    DataCenter *DC = DataCenter::get_instance();
    (void)DC;

    bool run = true;
    while (run) {
        al_wait_for_event(event_queue, &event);
        switch (event.type) {
            case ALLEGRO_EVENT_TIMER: {
                run &= game_update();
                game_draw();
                break;
            }
            case ALLEGRO_EVENT_DISPLAY_CLOSE: {
                run = false;
                break;
            }
            case ALLEGRO_EVENT_KEY_DOWN: {
                DC->key_state[event.keyboard.keycode] = true;
                break;
            }
            case ALLEGRO_EVENT_KEY_UP: {
                DC->key_state[event.keyboard.keycode] = false;
                break;
            }
            case ALLEGRO_EVENT_MOUSE_AXES: {
                DC->mouse.x = event.mouse.x;
                DC->mouse.y = event.mouse.y;
                break;
            }
            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: {
                DC->mouse_state[event.mouse.button] = true;
                break;
            }
            case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
                DC->mouse_state[event.mouse.button] = false;
                break;
            }
            default:
                break;
        }
    }
}

/**
 * @brief Initialize allegro and game body.
 */
Game::Game() {
    DataCenter *DC = DataCenter::get_instance();
    GAME_ASSERT(al_init(), "failed to initialize allegro.");

    bool addon_init = true;
    addon_init &= al_init_primitives_addon();
    addon_init &= al_init_font_addon();
    addon_init &= al_init_ttf_addon();
    addon_init &= al_init_image_addon();
    addon_init &= al_init_acodec_addon();
    GAME_ASSERT(addon_init, "failed to initialize allegro addons.");

    bool event_init = true;
    event_init &= al_install_keyboard();
    event_init &= al_install_mouse();
    event_init &= al_install_audio();
    GAME_ASSERT(event_init, "failed to initialize allegro events.");

    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    GAME_ASSERT(
        display = al_create_display(DC->window_width, DC->window_height),
        "failed to create display."
    );
    GAME_ASSERT(
        timer = al_create_timer(1.0 / DC->FPS),
        "failed to create timer."
    );
    GAME_ASSERT(
        event_queue = al_create_event_queue(),
        "failed to create event queue."
    );

    debug_log("Game initialized.\n");
    game_init();
}

/**
 * @brief Initialize auxiliary resources.
 */
void Game::game_init() {
    DataCenter  *DC = DataCenter::get_instance();
    SoundCenter *SC = SoundCenter::get_instance();
    ImageCenter *IC = ImageCenter::get_instance();
    FontCenter  *FC = FontCenter::get_instance();
    (void)FC;

    // 圖片
    game_icon = IC->get(game_icon_img_path);
    GAME_ASSERT(game_icon, "failed to load game_icon");

    pause_menu_button = IC->get(menu_button_img_path);
    GAME_ASSERT(pause_menu_button, "failed to load menu_button");

    player_mask = IC->get(mask_img_path);
    GAME_ASSERT(player_mask, "failed to load mask");

    player_mask_E = IC->get(e_mask_img_path);
    GAME_ASSERT(player_mask_E, "failed to load E mask");

    background = IC->get(background_img_path);
    GAME_ASSERT(background, "failed to load background");

    al_set_display_icon(display, game_icon);

    // 事件
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    // 音效 / 字型
    SC->init();
    FC->init();

    // UI
    ui = new UI();
    ui->init();

    // 關卡 / 怪物
    DC->level->init();

    // 先初始化幾隻怪物（位置之後在 START 裡會被覆蓋）
    if (DC->monsters.size() >= 1) {
        DC->monsters[0]->init(10, 3, true, false, true);
    }
    if (DC->monsters.size() >= 2) {
        DC->monsters[1]->init(10, 2, true, false, false);
    }
    g_active_monsters = std::min<int>((int)DC->monsters.size(), 2);

    debug_log("Game state: change to MENU\n");
    state = STATE::MENU;
    mask  = true;

    al_start_timer(timer);
}

/**
 * @brief Game logic update.
 */
bool Game::game_update() {
    DataCenter      *DC = DataCenter::get_instance();
    OperationCenter *OC = OperationCenter::get_instance();
    SoundCenter     *SC = SoundCenter::get_instance();

    static ALLEGRO_SAMPLE_INSTANCE *bgm_ingame        = nullptr;
    static bool                     ingame_bgm_played = false;
    static int                      lvl               = 1;   // 只用來 reset，實際只玩 level 1
    static long long                spot_time         = 0;
    static int                      warn_level        = 0;
    static bool                     god_mode          = false;

    // 切換整體 mask on/off（反斜線）
    if (DC->key_state[ALLEGRO_KEY_BACKSLASH] &&
        !DC->prev_key_state[ALLEGRO_KEY_BACKSLASH]) {
        mask = !mask;
    }

    switch (state) {
        // ========== 主選單 ==========
        case STATE::MENU: {
            ui->update();
            if (ui->get_state() == UI::STATE::INGAME) {
                debug_log("<Game> state: change to START\n");
                state = STATE::START;
            }
            break;
        }

        // ========== 關卡初始化（只做 level 1） ==========
        case STATE::START: {
            // 你現在只需要一關 → 直接鎖死用 level 1
            lvl = 1;
            DC->player->max_key = 1;

            // 載入關卡 1 + 初始化 hero 位置（grid）
            player_init_pos = DC->level->load_level(1);
            DC->hero->init(player_init_pos);
            DC->hero->cur_lvl = 1;

            warn_level      = 0;
            spot_time       = 0;
            ui->alert_level = 0;

            // ============================
            // level 1：隨機怪物生成
            // ============================
            {
                const int H = 12;
                const int W = 20;

                int hero_grid_x = (int)player_init_pos.first;   // col
                int hero_grid_y = (int)player_init_pos.second;  // row

                // 收集所有可以放怪物的 0 格，且離牆至少一格
                std::vector<std::pair<int,int>> free_cells;
                for (int i = 2; i <= H - 3; ++i) {
                    for (int j = 2; j <= W - 3; ++j) {
                        if (DC->map[i][j] == '0') {
                            free_cells.emplace_back(i, j); // (row, col)
                        }
                    }
                }

                auto manhattan = [](int r1, int c1, int r2, int c2) -> int {
                    int dr = r1 - r2; if (dr < 0) dr = -dr;
                    int dc = c1 - c2; if (dc < 0) dc = -dc;
                    return dr + dc;
                };

                const int MIN_MONSTER_DIST_TILES = 3; // 跟玩家至少 3 格

                // 過濾：只留下「離 hero 不太近」的格子
                std::vector<std::pair<int,int>> candidate_cells;
                for (auto &cell : free_cells) {
                    int r = cell.first;
                    int c = cell.second;
                    if (manhattan(r, c, hero_grid_y, hero_grid_x) >= MIN_MONSTER_DIST_TILES) {
                        candidate_cells.emplace_back(r, c);
                    }
                }

                // 要生成的怪物數量：2~5，不能超過實際有的怪物物件數
                int max_monsters = (int)DC->monsters.size();
                int desired      = 2 + (std::rand() % 4); // 2..5
                int count        = desired;
                if (count > max_monsters) count = max_monsters;
                if (count < 0)            count = 0;
                g_active_monsters = count;

                auto take_and_erase = [](std::vector<std::pair<int,int>> &vec) -> std::pair<int,int> {
                    int idx   = std::rand() % (int)vec.size();
                    auto cell = vec[idx];
                    vec.erase(vec.begin() + idx);
                    return cell;
                };

                for (int mi = 0; mi < count; ++mi) {
                    std::pair<int,int> cell;

                    if (!candidate_cells.empty()) {
                        // 優先用有距離限制的候選點
                        cell = take_and_erase(candidate_cells);
                    } else if (!free_cells.empty()) {
                        // 沒有候選點，就退一步用所有 free_cells
                        cell = take_and_erase(free_cells);
                    } else {
                        // 真的沒有空格可以放怪物了
                        break;
                    }

                    int grid_row = cell.first;
                    int grid_col = cell.second;

                    bool horizontal_patrol = (std::rand() % 2) == 0;
                    bool forward           = true;
                    bool dir               = (std::rand() % 2) == 0;

                    // Monster::init(x, y, hPatrol, fPatrol, dir)
                    DC->monsters[mi]->init(grid_col, grid_row,
                                           horizontal_patrol,
                                           forward,
                                           dir);
                }

                debug_log("<Game> level 1: spawn %d monsters (random).\n", g_active_monsters);
            }

            // 初始化完成 → 直接進入 LEVEL
            debug_log("<Game> state: change to LEVEL\n");
            state = STATE::LEVEL;
            break;
        }

        // ========== 關卡中 ==========
        case STATE::LEVEL: {
            // ------- E mask 能量與狀態更新 -------
            {
                double dt     = 1.0 / DC->FPS;               // 每一 frame 的時間
                bool   e_down = DC->key_state[ALLEGRO_KEY_E];

                const float USE_RATE             = 0.4f;  // 按住 E 消耗速度（每秒）
                const float REGEN_RATE_FAST      = 0.25f; // 正常回復速度（放開、沒用光）
                const float REGEN_RATE_OVERHEAT  = 0.08f; // 過熱時的慢速回復
                const float OVERHEAT_RELEASE_LVL = 0.7f;  // 回到這個以上才解除過熱

                if (e_down && !g_mask_overheated && g_mask_energy > 0.0f) {
                    // 可以啟用 E mask，並消耗能量
                    g_e_mask_active = true;
                    g_mask_energy  -= USE_RATE * (float)dt;

                    if (g_mask_energy <= 0.0f) {
                        g_mask_energy     = 0.0f;
                        g_mask_overheated = true;   // 用光 → 過熱
                        g_e_mask_active   = false;  // 即使還按著 E 也不能用
                    }
                } else {
                    // 沒在使用 E（沒按 / 過熱 / 能量 0）
                    g_e_mask_active = false;

                    if (g_mask_overheated) {
                        // 過熱狀態：慢慢回復
                        g_mask_energy += REGEN_RATE_OVERHEAT * (float)dt;
                        if (g_mask_energy >= OVERHEAT_RELEASE_LVL) {
                            g_mask_overheated = false;  // 回到一定程度，解除過熱
                        }
                    } else {
                        // 一般情況：較快回復
                        g_mask_energy += REGEN_RATE_FAST * (float)dt;
                    }

                    if (g_mask_energy > 1.0f) g_mask_energy = 1.0f;
                }
            }
            // ------- E mask 能量更新結束 -------

            // BGM
            if (!ingame_bgm_played) {
                bgm_ingame        = SC->play(ingame_background_sound_path, ALLEGRO_PLAYMODE_LOOP);
                ingame_bgm_played = true;
            }

            // UI 鑰匙
            for (int i = 0; i < 3; ++i) {
                ui->have_key[i] = DC->hero->have_key[i];
            }

            // 暫停
            if ((DC->key_state[ALLEGRO_KEY_P]      && !DC->prev_key_state[ALLEGRO_KEY_P]) ||
                (DC->key_state[ALLEGRO_KEY_ESCAPE] && !DC->prev_key_state[ALLEGRO_KEY_ESCAPE])) {
                if (bgm_ingame) SC->toggle_playing(bgm_ingame);
                debug_log("<Game> state: change to PAUSE\n");
                state = STATE::PAUSE;
            }

            // 玩家死亡
            if (DC->player->HP == 0) {
                debug_log("<Game> state: change to POST_GAME (HP=0)\n");
                state     = STATE::POST_GAME;
                ui->state = UI::STATE::POST_GAME;
            }

            // God mode
            if (DC->key_state[ALLEGRO_KEY_TILDE] &&
                !DC->prev_key_state[ALLEGRO_KEY_TILDE]) {
                god_mode = !god_mode;
            }

            // 被怪物看到 → 累計警戒值（之後你要整個刪 alert 機制可以再整理）
            {
                int limit = std::min<int>(g_active_monsters, (int)DC->monsters.size());
                for (int i = 0; i < limit; ++i) {
                    auto mon = DC->monsters[i];
                    if (mon->is_visible({DC->hero->shape->center_x(), DC->hero->shape->center_y()}) &&
                        mon->is_in_fov({DC->hero->shape->center_x(), DC->hero->shape->center_y()}) &&
                        !god_mode) {
                        spot_time++;
                    }
                }
            }

            warn_level += (spot_time / 60);
            if (spot_time / 60 > 0) spot_time = 0;

            ui->alert_level = warn_level;

            if (warn_level > 2) {
                debug_log("<Game> state: change to POST_GAME (spotted)\n");
                state           = STATE::POST_GAME;
                ui->state       = UI::STATE::POST_GAME;
                if (bgm_ingame) SC->toggle_playing(bgm_ingame);
                warn_level      = 0;
                ui->alert_level = 3;
            }

            break;
        }

        // ========== 暫停 ==========
        case STATE::PAUSE: {
            // P / ESC 取消暫停，回到遊戲
            if ((DC->key_state[ALLEGRO_KEY_P]      && !DC->prev_key_state[ALLEGRO_KEY_P]) ||
                (DC->key_state[ALLEGRO_KEY_ESCAPE] && !DC->prev_key_state[ALLEGRO_KEY_ESCAPE])) {
                if (bgm_ingame) SC->toggle_playing(bgm_ingame);
                debug_log("<Game> state: change to LEVEL (unpause)\n");
                state = STATE::LEVEL;
            }

            // 滑鼠點 MENU：回主選單（重新開始）
            if (DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
                if (menu_button_area.overlap(DC->mouse)) {
                    debug_log("<Game> state: change to MENU (from PAUSE)\n");
                    state           = STATE::MENU;          // Game 狀態回 MENU
                    ui->state       = UI::STATE::MENU;      // UI 也切回 MENU
                    lvl             = 1;
                    warn_level      = 0;
                    spot_time       = 0;
                    ui->alert_level = 0;

                    // 下次進關卡時要重新播 BGM
                    ingame_bgm_played = false;
                }
            }
            break;
        }

        // ========== 結算畫面 ==========
        case STATE::POST_GAME: {
            ui->update();
            if (ui->get_state() == UI::STATE::MENU) {
                debug_log("<Game> state: change to MENU (from POST_GAME)\n");
                state = STATE::MENU;
                lvl   = 1;
                // 下次進關卡時要重新播 BGM
                ingame_bgm_played = false;
            }
            break;
        }

        case STATE::END: {
            return false;
        }
    }

    // 一般 update（非 MENU / POST_GAME / PAUSE / END）
    if (state != STATE::PAUSE &&
        state != STATE::END   &&
        state != STATE::MENU  &&
        state != STATE::POST_GAME) {

        DC->player->update();
        SC->update();
        ui->update();

        if (state != STATE::START) {
            DC->hero->update();

            int limit = std::min<int>(g_active_monsters, (int)DC->monsters.size());
            for (int i = 0; i < limit; ++i) {
                DC->monsters[i]->update();
            }

            OC->update();
        }
    }

    // ✅ 只做一關：如果 hero 想切換關卡（cur_lvl != level），代表通關 → 進結算畫面
    if (state == STATE::LEVEL &&
        DC->hero->cur_lvl != DC->level->level) {

        debug_log(
            "<Game> level 1 cleared -> POST_GAME (hero->cur_lvl=%d, level=%d)\n",
            DC->hero->cur_lvl, DC->level->level
        );

        state     = STATE::POST_GAME;
        ui->state = UI::STATE::POST_GAME;

        // 把 in-game BGM 停掉
        // 注意：這裡用 static 的 bgm_ingame / ingame_bgm_played
        // 如果上面 extern 會出錯，就改成在前面把 bgm_ingame 改成非 static，這裡就能直接用
    }

    // 存前一幀鍵鼠狀態
    std::memcpy(DC->prev_key_state,   DC->key_state,   sizeof(DC->key_state));
    std::memcpy(DC->prev_mouse_state, DC->mouse_state, sizeof(DC->mouse_state));

    return true;
}

/**
 * @brief Draw game and objects.
 */
void Game::game_draw() {
    DataCenter *DC = DataCenter::get_instance();
    FontCenter *FC = FontCenter::get_instance();

    // 每幀先清畫面
    al_clear_to_color(al_map_rgb(100, 100, 100));

    if (state == STATE::MENU) {
        // 主選單畫面交給 UI
        ui->draw_menu();
    }
    else if (state == STATE::POST_GAME) {
        ui->draw_post_menu();
    }
    else if (state != STATE::END) {
        // 遊戲內畫面 (START / LEVEL / PAUSE)
        if (background) {
            al_draw_bitmap(background, 0, 0, 0);
        }

        if (state != STATE::START) {
            DC->level->draw();

            int limit = std::min<int>(g_active_monsters, (int)DC->monsters.size());
            for (int i = 0; i < limit; ++i) {
                DC->monsters[i]->draw();
            }

            DC->hero->draw();

            // black mask 蓋在最前面
            if (mask) {
                double hx = DC->hero->shape->center_x();
                double hy = DC->hero->shape->center_y();

                ALLEGRO_BITMAP *mask_to_use = nullptr;
                if (g_e_mask_active && player_mask_E) {
                    mask_to_use = player_mask_E;   // 按住 E + 有能量 → 用 E 版本
                } else if (player_mask) {
                    mask_to_use = player_mask;     // 其他情況 → 一般版本
                }

                if (mask_to_use) {
                    al_draw_bitmap(
                        mask_to_use,
                        hx - MASK_SHIFT_X,
                        hy - MASK_SHIFT_Y,
                        0
                    );
                }
            }

            ui->draw(); // INGAME HUD（鑰匙 + E 能量條）
        }

        if (state == STATE::PAUSE) {
            // 疊一層 PAUSE 面板
            al_draw_filled_rectangle(
                0, 0,
                DC->window_width, DC->window_height,
                al_map_rgba(20, 20, 20, 64)
            );

            al_draw_text(
                FC->courier_new[FontSize::LARGE], al_map_rgb(255, 255, 255),
                DC->window_width / 2., DC->window_height / 4.,
                ALLEGRO_ALIGN_CENTRE, "GAME PAUSED"
            );

            menu_button_area = Rectangle(
                DC->window_width  / 2. - al_get_bitmap_width(pause_menu_button)  / 2.,
                DC->window_height / 2. - al_get_bitmap_height(pause_menu_button) / 2.,
                DC->window_width  / 2. + al_get_bitmap_width(pause_menu_button)  / 2.,
                DC->window_height / 2. + al_get_bitmap_height(pause_menu_button) / 2.
            );

            al_draw_bitmap(pause_menu_button, menu_button_area.x1, menu_button_area.y1, 0);
            al_draw_text(
                FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
                menu_button_area.center_x(), menu_button_area.center_y() - 10,
                ALLEGRO_ALIGN_CENTRE, "MENU"
            );
        }
    }
    else {
        // END 狀態（現在基本用不到）
        al_draw_text(
            FC->courier_new[FontSize::LARGE], al_map_rgb(255, 255, 255),
            DC->window_width / 2., DC->window_height / 2.,
            ALLEGRO_ALIGN_CENTRE, "GAME END"
        );
    }

    // 一幀只 flip 一次
    al_flip_display();
}

Game::~Game() {
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
}
