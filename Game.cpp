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

// fixed settings
constexpr char game_icon_img_path[]             = "./assets/image/game_icon.png";
constexpr char game_start_sound_path[]          = "./assets/sound/growl.wav";
constexpr char background_img_path[]            = "./assets/image/StartBackground.jpg";
constexpr char ingame_background_sound_path[]   = "./assets/sound/ingame_bgm.mp3";
constexpr char menu_button_img_path[]           = "./assets/image/menu_button.png";

// === black mask 設定 ===
constexpr char mask_img_path[] = "./assets/image/mask.png";
// 這兩個是「mask.png 的中心點對應 hero 中心」時的偏移
#define MASK_SHIFT_X 1280
#define MASK_SHIFT_Y 768

ALLEGRO_BITMAP *player_mask = nullptr;
bool mask = true;
// ======================

Rectangle        menu_button_area;
ALLEGRO_BITMAP  *pause_menu_button = nullptr;

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
 * @brief Initialize all allegro addons and the game body.
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

    // alpha blending（確保 PNG 透明度正常）
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    GAME_ASSERT(
        display = al_create_display(DC->window_width, DC->window_height),
        "failed to create display.");
    GAME_ASSERT(
        timer = al_create_timer(1.0 / DC->FPS),
        "failed to create timer.");
    GAME_ASSERT(
        event_queue = al_create_event_queue(),
        "failed to create event queue.");

    debug_log("Game initialized.\n");
    game_init();
}

/**
 * @brief Initialize all auxiliary resources.
 */
void Game::game_init() {
    DataCenter  *DC = DataCenter::get_instance();
    SoundCenter *SC = SoundCenter::get_instance();
    ImageCenter *IC = ImageCenter::get_instance();
    FontCenter  *FC = FontCenter::get_instance();
    (void)FC;

    // 載入圖像資源 + 檢查
    game_icon = IC->get(game_icon_img_path);
    GAME_ASSERT(game_icon, "failed to load game_icon image");

    pause_menu_button = IC->get(menu_button_img_path);
    GAME_ASSERT(pause_menu_button, "failed to load menu_button image");

    player_mask = IC->get(mask_img_path);
    GAME_ASSERT(player_mask, "failed to load mask image");

    background = IC->get(background_img_path);
    GAME_ASSERT(background, "failed to load background image");

    // 視窗 icon
    al_set_display_icon(display, game_icon);

    // 註冊事件
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

    // 關卡 + 怪物
    DC->level->init();
    DC->monsters[0]->init(10, 3, true, false, true);
    DC->monsters[1]->init(10, 2, true, false, false);

    // 初始狀態
    debug_log("Game state: change to MENU\n");
    state = STATE::MENU;
    mask  = true;

    al_start_timer(timer);
}

/**
 * @brief Game update (logic).
 */
bool Game::game_update() {
    DataCenter      *DC = DataCenter::get_instance();
    OperationCenter *OC = OperationCenter::get_instance();
    SoundCenter     *SC = SoundCenter::get_instance();

    static ALLEGRO_SAMPLE_INSTANCE *bgm_ingame = nullptr;
    static bool   ingame_bgm_played = false;
    static int    lvl        = 1;
    static long long spot_time  = 0;
    static int    warn_level = 0;
    static bool   god_mode   = false;

    // 切換 mask：按下 BACKSLASH 反轉
    if (DC->key_state[ALLEGRO_KEY_BACKSLASH] &&
        !DC->prev_key_state[ALLEGRO_KEY_BACKSLASH]) {
        mask = !mask;
    }

    switch (state) {
        case STATE::MENU: {
            // UI 負責處理 START / QUIT
            ui->update();
            if (ui->get_state() == UI::STATE::INGAME) {
                debug_log("<Game> state: change to START\n");
                state = STATE::START;
            }
            break;
        }

        case STATE::START: {
            if (lvl > 2) {
                state     = STATE::POST_GAME;
                ui->state = UI::STATE::POST_GAME;
                break;
            }

            // 根據關卡設定 max_key
            if (lvl == 1)      DC->player->max_key = 1;
            else if (lvl == 2) DC->player->max_key = 3;

            // 載入關卡、初始化 hero
            player_init_pos = DC->level->load_level(lvl);
            DC->hero->init(player_init_pos);
            DC->hero->cur_lvl = lvl;

            warn_level      = 0;
            spot_time       = 0;
            ui->alert_level = 0;

            lvl++;
            debug_log("<Game> state: change to LEVEL\n");
            state = STATE::LEVEL;
            break;
        }

        case STATE::LEVEL: {
            // 開 BGM（只開一次）
            if (!ingame_bgm_played) {
                bgm_ingame = SC->play(ingame_background_sound_path, ALLEGRO_PLAYMODE_LOOP);
                ingame_bgm_played = true;
            }

            // 更新 UI 的鑰匙顯示
            for (int i = 0; i < 3; i++) {
                ui->have_key[i] = DC->hero->have_key[i];
            }

            // P / ESC 暫停
            if ((DC->key_state[ALLEGRO_KEY_P]      && !DC->prev_key_state[ALLEGRO_KEY_P]) ||
                (DC->key_state[ALLEGRO_KEY_ESCAPE] && !DC->prev_key_state[ALLEGRO_KEY_ESCAPE])) {
                if (bgm_ingame) SC->toggle_playing(bgm_ingame);
                debug_log("<Game> state: change to PAUSE\n");
                state = STATE::PAUSE;
            }

            // 玩家死亡 → POST_GAME
            if (DC->player->HP == 0) {
                debug_log("<Game> state: change to POST_GAME\n");
                state     = STATE::POST_GAME;
                ui->state = UI::STATE::POST_GAME;
            }

            // God mode 切換
            if (DC->key_state[ALLEGRO_KEY_TILDE] &&
                !DC->prev_key_state[ALLEGRO_KEY_TILDE]) {
                god_mode = !god_mode;
            }

            // 被怪物看到計算警戒
            for (auto &mon : DC->monsters) {
                if (mon->is_visible({DC->hero->shape->center_x(), DC->hero->shape->center_y()}) &&
                    mon->is_in_fov({DC->hero->shape->center_x(), DC->hero->shape->center_y()}) &&
                    !god_mode) {
                    spot_time++;
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

        case STATE::PAUSE: {
            // 解除暫停
            if ((DC->key_state[ALLEGRO_KEY_P]      && !DC->prev_key_state[ALLEGRO_KEY_P]) ||
                (DC->key_state[ALLEGRO_KEY_ESCAPE] && !DC->prev_key_state[ALLEGRO_KEY_ESCAPE])) {
                if (bgm_ingame) SC->toggle_playing(bgm_ingame);
                debug_log("<Game> state: change to LEVEL\n");
                state = STATE::LEVEL;
            }

            // 用滑鼠點 MENU 按鈕回主選單
            if (DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
                if (menu_button_area.overlap(DC->mouse)) {
                    debug_log("<Game> state: change to MENU (from PAUSE)\n");
                    state     = STATE::MENU;
                    ui->state = UI::STATE::MENU;
                }
            }
            break;
        }

        case STATE::POST_GAME: {
            ui->update();
            if (ui->get_state() == UI::STATE::MENU) {
                debug_log("<Game> state: change to MENU (from POST_GAME)\n");
                state = STATE::MENU;
                lvl   = 1;
            }
            break;
        }

        case STATE::END: {
            return false;
        }
    }

    // 關卡全破 → POST_GAME
    if (state == STATE::LEVEL && DC->hero->cur_lvl > 2) {
        debug_log("<Game> state: change to POST_GAME (all levels cleared)\n");
        state     = STATE::POST_GAME;
        ui->state = UI::STATE::POST_GAME;
    }

    // 一般邏輯更新（非 PAUSE / END）
    if (state != STATE::PAUSE &&
        state != STATE::END  &&
        state != STATE::MENU &&
        state != STATE::POST_GAME) {
        DC->player->update();
        SC->update();
        ui->update();

        if (state != STATE::START) {
            DC->hero->update();
            DC->monsters[0]->update();
            DC->monsters[1]->update();
            OC->update();
        }

        if (DC->hero->cur_lvl != DC->level->level && DC->hero->cur_lvl < 3) {
            debug_log("<Game> state: change to START (level changed)\n");
            state = STATE::START;
        }
    }

    // 存前一幀鍵盤 / 滑鼠狀態
    std::memcpy(DC->prev_key_state,   DC->key_state,   sizeof(DC->key_state));
    std::memcpy(DC->prev_mouse_state, DC->mouse_state, sizeof(DC->mouse_state));

    return true;
}

/**
 * @brief Draw the whole game and objects.
 */
void Game::game_draw() {
    DataCenter *DC = DataCenter::get_instance();
    FontCenter *FC = FontCenter::get_instance();

    // 1. 每一幀先清畫面（全程唯一 clear）
    al_clear_to_color(al_map_rgb(100, 100, 100));

    // 2. 根據 state 畫畫面
    if (state == STATE::MENU) {
        // 主選單交給 UI 畫
        ui->draw_menu();
    }
    else if (state == STATE::POST_GAME) {
        // 結算畫面
        ui->draw_post_menu();
    }
    else if (state != STATE::END) {
        // START / LEVEL / PAUSE：遊戲內畫面
        if (background) {
            al_draw_bitmap(background, 0, 0, 0);
        }

        if (state != STATE::START) {
            DC->level->draw();
            DC->monsters[0]->draw();
            DC->monsters[1]->draw();
            DC->hero->draw();
            // 黑遮罩畫在最上層
            if (mask && player_mask) {
                double hx = DC->hero->shape->center_x();
                double hy = DC->hero->shape->center_y();
                al_draw_bitmap(
                    player_mask,
                    hx - MASK_SHIFT_X,
                    hy - MASK_SHIFT_Y,
                    0
                );
            }
		
			ui->draw();   // 只在 INGAME 畫 HUD（UI::draw 裡有判斷）
        }

        if (state == STATE::PAUSE) {
            // 疊一層 PAUSE 畫面
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
        // END 狀態
        al_draw_text(
            FC->courier_new[FontSize::LARGE], al_map_rgb(255, 255, 255),
            DC->window_width / 2., DC->window_height / 2.,
            ALLEGRO_ALIGN_CENTRE, "GAME END"
        );
    }

    // 3. 一幀只 flip 一次（全程唯一 flip）
    al_flip_display();
}

Game::~Game() {
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
}
