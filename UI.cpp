#include "UI.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include <algorithm>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include "shapes/Point.h"
#include "shapes/Rectangle.h"
#include "Player.h"
#include "towers/Tower.h"
#include "Level.h"
#include "data/SoundCenter.h"

// fixed settings
constexpr char love_img_path[]            = "./assets/image/love.png";
constexpr int  love_img_padding           = 5;
constexpr char key_img_path[]             = "./assets/image/key.png";
constexpr char gate_key1_img_path[]       = "./assets/image/gate_key_1.png";
constexpr char gate_key2_img_path[]       = "./assets/image/gate_key_2.png";
constexpr char no_key_img_path[]          = "./assets/image/no_key.png";
constexpr char menu_background_img_path[] = "./assets/image/MenuBackground.jpg";
constexpr char menu_button_img_path[]     = "./assets/image/menu_button.png";
constexpr char alert_img_path[]           = "./assets/image/alert.png";
constexpr char menu_bgm_sound_path[]      = "./assets/sound/menu_bgm.wav";

Rectangle start_button_area;
Rectangle quit_button_area;
Rectangle post_menu_button_area;
Rectangle post_quit_button_area;
ALLEGRO_BITMAP *key_imgs[3];
double button_shift_y = 100;
extern float g_mask_energy;
extern bool  g_mask_overheated;

// love, no_key, alert, menu_background, menu_button, state, have_key, alert_level
// 假設都在 UI.h 裡已經宣告成成員變數

void UI::init() {
    ImageCenter *IC = ImageCenter::get_instance();

    love = IC->get(love_img_path);
    GAME_ASSERT(love, "UI::init failed: love.png not loaded");

    key_imgs[0] = IC->get(key_img_path);
    GAME_ASSERT(key_imgs[0], "UI::init failed: key.png not loaded");

    key_imgs[1] = IC->get(gate_key1_img_path);
    GAME_ASSERT(key_imgs[1], "UI::init failed: gate_key_1.png not loaded");

    key_imgs[2] = IC->get(gate_key2_img_path);
    GAME_ASSERT(key_imgs[2], "UI::init failed: gate_key_2.png not loaded");

    no_key = IC->get(no_key_img_path);
    GAME_ASSERT(no_key, "UI::init failed: no_key.png not loaded");

    alert = IC->get(alert_img_path);
    GAME_ASSERT(alert, "UI::init failed: alert.png not loaded");

    menu_background = IC->get(menu_background_img_path);
    GAME_ASSERT(menu_background, "UI::init failed: MenuBackground.jpg not loaded");

    menu_button = IC->get(menu_button_img_path);
    GAME_ASSERT(menu_button, "UI::init failed: menu_button.png not loaded");

    debug_log("<UI> state: change to MENU\n");
    state = STATE::MENU;
}

void UI::update() {
    DataCenter  *DC = DataCenter::get_instance();
    SoundCenter *SC = SoundCenter::get_instance();
    const Point &mouse = DC->mouse;

    static ALLEGRO_SAMPLE_INSTANCE *menu_bgm = nullptr;
    static bool menu_bgm_played = false;

    switch (state) {
        case STATE::MENU: {
            // 播放主選單 BGM（只播一次）
            if (!menu_bgm_played) {
                menu_bgm = SC->play(menu_bgm_sound_path, ALLEGRO_PLAYMODE_LOOP);
                menu_bgm_played = true;
            }

            // START
            if (mouse.overlap(start_button_area)) {
                if (DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
                    debug_log("<UI> state: change to INGAME\n");
                    state = STATE::INGAME;
                    SC->toggle_playing(menu_bgm);
                }
            }
            // QUIT
            else if (mouse.overlap(quit_button_area)) {
                if (DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
                    exit(0);
                }
            }
            break;
        }

        case STATE::INGAME: {
            // in-game HUD 更新交給 Game / Hero / Player，本函式不用畫
            break;
        }

        case STATE::POST_GAME: {
            // 結算畫面 MENU
            if (mouse.overlap(post_menu_button_area)) {
                if (DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
                    debug_log("<UI> state: change to MENU\n");
                    state = STATE::MENU;
                    SC->toggle_playing(menu_bgm);
                }
            }
            // 結算畫面 QUIT
            else if (mouse.overlap(post_quit_button_area)) {
                if (DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
                    exit(0);
                }
            }
            break;
        }
    }
}

void
UI::draw() {
    DataCenter *DC = DataCenter::get_instance();
    const int &game_field_length = DC->game_field_length;
    int key_num   = DC->player->max_key;
    int key_width = al_get_bitmap_width(key_imgs[0]);
    // 不需要 alert 寬度了
    // int alert_width = al_get_bitmap_width(alert);

    // 畫鑰匙狀態
    for (int i = 0; i < key_num; i++) {
        if (have_key[i]) {
            al_draw_bitmap(key_imgs[i],
                           game_field_length - (key_width + 20) * (i+1),
                           30 + al_get_bitmap_height(love),
                           0);
        } else {
            al_draw_bitmap(no_key,
                           game_field_length - (key_width + 20) * (i+1),
                           30 + al_get_bitmap_height(love),
                           0);
        }
    }

    // ===== alert 不畫了，直接刪掉這段 =====
    // for(int i = 0; i < alert_level; i++) {
    //     al_draw_bitmap(alert, 10 + alert_width * (i+1),
    //                    30 + al_get_bitmap_height(love), 0);
    // }

    // ------- E mask 能量條（留在這裡畫） -------
    {
        const float BAR_X = 20.0f;
        const float BAR_Y = 20.0f;
        const float BAR_W = 200.0f;
        const float BAR_H = 16.0f;

        al_draw_filled_rectangle(
            BAR_X, BAR_Y,
            BAR_X + BAR_W, BAR_Y + BAR_H,
            al_map_rgb(50, 50, 50)
        );

        float energy = g_mask_energy;
        if (energy < 0.0f) energy = 0.0f;
        if (energy > 1.0f) energy = 1.0f;
        float w = BAR_W * energy;

        ALLEGRO_COLOR c = g_mask_overheated
                          ? al_map_rgb(200, 50, 50)
                          : al_map_rgb(80, 200, 80);

        al_draw_filled_rectangle(
            BAR_X, BAR_Y,
            BAR_X + w, BAR_Y + BAR_H,
            c
        );

        al_draw_rectangle(
            BAR_X, BAR_Y,
            BAR_X + BAR_W, BAR_Y + BAR_H,
            al_map_rgb(255, 255, 255),
            2.0f
        );
    }
}


void UI::draw_menu() {
    DataCenter *DC = DataCenter::get_instance();
    FontCenter *FC = FontCenter::get_instance();

    double button_width  = al_get_bitmap_width(menu_button);
    double button_height = al_get_bitmap_height(menu_button);

    start_button_area = Rectangle(
        DC->window_width  / 2.0 - button_width  / 2.0,
        DC->window_height / 2.0 - button_height / 2.0,
        DC->window_width  / 2.0 + button_width  / 2.0,
        DC->window_height / 2.0 + button_height / 2.0
    );
    quit_button_area = Rectangle(
        start_button_area.x1,
        start_button_area.y1 + button_shift_y,
        start_button_area.x2,
        start_button_area.y2 + button_shift_y
    );

    // 背景（原本下移 24）
    if (menu_background) {
        al_draw_bitmap(menu_background, 0, 24, 0);
    }

    // 按鈕
    al_draw_bitmap(menu_button, start_button_area.x1, start_button_area.y1, 0);
    al_draw_bitmap(menu_button, quit_button_area.x1,  quit_button_area.y1,  0);

    // 文字
    al_draw_text(
        FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
        start_button_area.center_x(), start_button_area.center_y() - 10,
        ALLEGRO_ALIGN_CENTRE, "START");
    al_draw_text(
        FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
        quit_button_area.center_x(), quit_button_area.center_y() - 10,
        ALLEGRO_ALIGN_CENTRE, "QUIT");
}

UI::STATE UI::get_state() {
    return state;
}

void UI::draw_post_menu() {
    DataCenter *DC = DataCenter::get_instance();
    FontCenter *FC = FontCenter::get_instance();

    double button_width  = al_get_bitmap_width(menu_button);
    double button_height = al_get_bitmap_height(menu_button);

    post_menu_button_area = Rectangle(
        DC->window_width  / 2.0 - button_width  / 2.0,
        DC->window_height / 2.0 - button_height / 2.0,
        DC->window_width  / 2.0 + button_width  / 2.0,
        DC->window_height / 2.0 + button_height / 2.0
    );
    post_quit_button_area = Rectangle(
        post_menu_button_area.x1,
        post_menu_button_area.y1 + button_shift_y,
        post_menu_button_area.x2,
        post_menu_button_area.y2 + button_shift_y
    );

    // 半透明背景
    al_draw_filled_rectangle(
        0, 0,
        DC->window_width, DC->window_height,
        al_map_rgba(50, 50, 50, 64)
    );

    // 按鈕
    al_draw_bitmap(menu_button, post_menu_button_area.x1, post_menu_button_area.y1, 0);
    al_draw_bitmap(menu_button, post_quit_button_area.x1, post_quit_button_area.y1, 0);

    // 文字
    al_draw_text(
        FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
        post_menu_button_area.center_x(), post_menu_button_area.center_y() - 10,
        ALLEGRO_ALIGN_CENTRE, "MENU");
    al_draw_text(
        FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
        post_quit_button_area.center_x(), post_quit_button_area.center_y() - 10,
        ALLEGRO_ALIGN_CENTRE, "QUIT");
}
