
#define _CRT_SECURE_NO_WARNINGS 1
#include <graphics.h>      
#include <conio.h>
#include <stdio.h>
#include <easyx.h>
#include <string>
#include <stdlib.h>
#include <windows.h>
#include <memory.h>
#include <vector>
#include <cmath>
#pragma comment(lib,"MSIMG32.LIB")

//宏定义
#define SCREEN_HEIGHT 800
#define SCREEN_WIDTH 1300
#define PLAYER_SIZE 100
#define FPS 60
#define MAX_PLAYER_HP 10
#define DEMON_MAX_HP 5

//结构体定义

// 玩家账号信息（链表节点）
typedef struct PlayerAccount {
    char name[45];
    char password[45];
    int score;
    PlayerAccount* next;
} PlayerAccount;

// 怪物Demon状态结构体
typedef struct Demon {
    int x, y;
    int hp;
    int mood;  // 1=行走, 2=攻击, 3=死亡
    int attack_frame;  // 攻击动画帧计数器
    int death_frame;   // 死亡动画帧计数器
    bool is_attacking; // 是否正在攻击
} Demon;

// 怪物ufo状态结构体
typedef struct Ufo {
    int x, y;
    int hp;
    bool alive;
    int attack_frame;  // 攻击动画帧计数器
    int death_frame;   // 死亡动画帧计数器
} Ufo;

// 子弹结构体
typedef struct Bullet {
    int x, y;
    float dx, dy;  // 移动方向向量
    bool friendly;
} Bullet;

// 图片资源结构体（集中管理所有图片）
typedef struct GameResources {
    // 玩家动画
    IMAGE player_walk_left[9];
    IMAGE player_walk_right[9];
    IMAGE player_walk_up[9];
    IMAGE player_walk_down[9];
    IMAGE player_idle_left[2];
    IMAGE player_idle_right[2];
    IMAGE player_idle_up[2];
    IMAGE player_idle_down[2];

    // 怪物动画
    IMAGE demon_floating[10];
    IMAGE demon_attack[12];
    IMAGE demon_death[13];
    IMAGE ufo_idle[4];
    IMAGE ufo_death[4];

    // 通用图片
    IMAGE bg_begin;
    IMAGE bg_spellcast_talk;
    IMAGE bg_player_talk;
    IMAGE img_wall;
    IMAGE img_ground;
    IMAGE img_portal;
    IMAGE spellcast_anime[7];
    IMAGE link[6];
    IMAGE signs;
    IMAGE pvp;

    // 资源加载标记
    bool is_loaded;
} GameResources;

// 全局游戏状态
typedef struct GameState {
    // 玩家状态
    POINT player_pos;
    int player_hp;
    int player_speed;
    int player_direction;  // 0=下,1=左,2=上,3=右
    bool player_walk_flag;
    int time_cost;
    int wall_num;

    // 动画帧计数器
    int counter;
    int idle_anime_frame;
    int walk_anime_frame;
    int demon_floating_frame;
    int ufo_idle_frame;
    int link_frame;

    // 输入状态
    bool key_w;
    bool key_a;
    bool key_s;
    bool key_d;

    // 游戏逻辑标记
    bool can_wall;
    bool if_new;
    bool have_game;
    bool login_success;
    bool quit_hall;
    bool game_win;
    int bg_mode;
    int sleep_flag;

    // 动态数据
    PlayerAccount* current_player;
    std::vector<Bullet> bullets;
    std::vector<Demon> demons;
    std::vector<Ufo> ufo;
    int wall_map[16][26];
} GameState;

//函数声明

// 工具函数模块
inline void putimage_alpha(int x, int y, IMAGE* img);
inline std::wstring str_to_wstr(const char* str);
void free_player_list(PlayerAccount* head);
void control_fps(DWORD start_time);

// 资源管理模块
bool init_game_resources(GameResources* res);

// 文件操作模块
PlayerAccount* load_players_from_file();
void save_players_to_file(PlayerAccount* head, GameState* state);
PlayerAccount* add_new_player(PlayerAccount* head, const char* name, const char* password);
bool check_password(PlayerAccount* head, const char* name, const char* password, PlayerAccount** current_player);
void save_singleplayer_game(GameState* state);
bool load_singleplayer_game(GameState* state, const char* player_name);
void resume_singleplayer_game(GameState* state, GameResources* res);

// 碰撞检测模块
bool check_player_collision(POINT player_pos, int map[][26]);
bool check_bullet_demon_collision(Bullet* bullet, Demon* demon);
bool check_player_demon_collision(POINT player_pos, Demon* demon);
bool check_bullet_ufo_collision(Bullet* bullet, Ufo* ufo);
bool check_player1_bullet_collision(POINT player1_pos, Bullet* bullet);
bool check_player2_bullet_collision(POINT player2_pos, Bullet* bullet);

// UI渲染模块
void draw_start_screen(GameResources* res);
void draw_hall_screen(GameResources* res, bool login_success, char* player_name, int score);
void draw_game_background(POINT player_pos, bool up_or_down, int map[][26], GameResources* res);
void draw_player_hp_bar(int hp);
void draw_player1_hp_bar(int hp);
void draw_player2_hp_bar(int hp);
void draw_demon_hp_bar(Demon* demon);
void draw_player_animation(GameState* state, GameResources* res);
void draw_demon_animation(GameState* state, GameResources* res);
void draw_bullets(GameState* state);
void draw_new_or_old(GameState* state, GameResources* res);
void draw_stop(GameState* state);
void draw_ufo_hp_bar(Ufo* ufo);
void draw_game_over(GameState* state);
void draw_time_cost(GameState* state);
void draw_ranking(GameState* state);
void draw_pvp_over(GameState* state1, GameState* state2);

// 输入处理模块
void handle_start_input(GameState* state);
bool handle_text_input(IMAGE* bg, int input_type, PlayerAccount* player_list,
    PlayerAccount** current_player, int is_old_user);
bool handle_login_process(GameResources* res, PlayerAccount* player_list, PlayerAccount** current_player, GameState* state);
void handle_hall_logic(GameState* state, GameResources* res, PlayerAccount* player_list);
void handle_game_level(GameState* state, GameResources* res);
void handle_new_or_old(GameState* state, GameResources* res);
void handle_stop(GameState* state);
void handle_game_level2(GameState* state, GameResources* res);
void handle_game_over(GameState* state);
void handle_ranking(GameState* state);
void handle_pvp(GameState* state, GameResources* res);

//主函数
int main() {
    // 初始化图形界面
    initgraph(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetWindowText(GetHWnd(), _T("Game"));

    // 初始化游戏资源和状态
    GameResources res = { 0 };
    GameState state = { 0 };
    init_game_resources(&res);

    // 加载玩家数据
    PlayerAccount* player_list = load_players_from_file();
    state.current_player = NULL;

    // 显示开始界面
    draw_start_screen(&res);
    handle_start_input(&state);

    // 检查是否退出
    if (state.sleep_flag == 1) {
        free_player_list(player_list);
        closegraph();
        return 0;
    }
    save_players_to_file(player_list, &state);

    // 进入大厅
    cleardevice();
    handle_hall_logic(&state, &res, player_list);

    // 释放资源
    free_player_list(player_list);
    closegraph();
    return 0;
}

//工具函数模块

// 透明贴图函数
inline void putimage_alpha(int x, int y, IMAGE* img) {
    if (img == NULL) return;
    int w = img->getwidth();
    int h = img->getheight();
    AlphaBlend(GetImageHDC(NULL), x, y, w, h, GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

// 多字节字符串转宽字符
inline std::wstring str_to_wstr(const char* str) {
    int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    std::wstring wstr(len, 0);
    MultiByteToWideChar(CP_ACP, 0, str, -1, &wstr[0], len);
    return wstr;
}

// 安全释放玩家链表
void free_player_list(PlayerAccount* head) {
    PlayerAccount* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

// 帧率控制函数
void control_fps(DWORD start_time) {
    DWORD end_time = GetTickCount();
    DWORD delta = end_time - start_time;
    if (delta < 1000 / FPS) {
        Sleep(1000 / FPS - delta);
    }
}

//资源管理模块

// 初始化游戏资源
bool init_game_resources(GameResources* res) {
    if (res->is_loaded) return true;

    // 初始化标记
    res->is_loaded = false;

    // 加载开始背景
    loadimage(&res->bg_begin, _T("begin_background.png"), SCREEN_WIDTH, SCREEN_HEIGHT);

    // 加载对话背景
    loadimage(&res->bg_spellcast_talk, _T("spellcast_talk.png"), SCREEN_WIDTH, SCREEN_HEIGHT);
    loadimage(&res->bg_player_talk, _T("player_talk.png"), SCREEN_WIDTH, SCREEN_HEIGHT);

    // 加载通用场景图片
    loadimage(&res->img_wall, _T("wall.png"), 50, 75);
    loadimage(&res->img_ground, _T("ground.png"), 50, 50);
    loadimage(&res->img_portal, _T("portal.png"), 200, 200);
    loadimage(&res->signs, _T("signs.png"), 100, 100);
    loadimage(&res->pvp, _T("pvp.png"), 100, 100);

    // 加载施法动画
    for (int i = 1; i <= 7; i++) {
        std::wstring path = L"spellcast\\" + std::to_wstring(i) + L".png";
        loadimage(&res->spellcast_anime[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);
    }

    // 加载玩家行走动画
    for (int i = 1; i <= 9; i++) {
        std::wstring path = L"walk\\left\\l" + std::to_wstring(i) + L".png";
        loadimage(&res->player_walk_left[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);

        path = L"walk\\right\\r" + std::to_wstring(i) + L".png";
        loadimage(&res->player_walk_right[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);

        path = L"walk\\up\\u" + std::to_wstring(i) + L".png";
        loadimage(&res->player_walk_up[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);

        path = L"walk\\down\\d" + std::to_wstring(i) + L".png";
        loadimage(&res->player_walk_down[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);
    }

    // 加载玩家Idle动画
    for (int i = 1; i <= 2; i++) {
        std::wstring path = L"idle\\left\\l" + std::to_wstring(i) + L".png";
        loadimage(&res->player_idle_left[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);

        path = L"idle\\right\\r" + std::to_wstring(i) + L".png";
        loadimage(&res->player_idle_right[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);

        path = L"idle\\up\\u" + std::to_wstring(i) + L".png";
        loadimage(&res->player_idle_up[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);

        path = L"idle\\down\\d" + std::to_wstring(i) + L".png";
        loadimage(&res->player_idle_down[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);
    }

    // 加载怪物Demon动画
    for (int i = 1; i <= 10; i++) {
        std::wstring path = L"Floating_Demon\\" + std::to_wstring(i) + L".png";
        loadimage(&res->demon_floating[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);
    }

    for (int i = 1; i <= 12; i++) {
        std::wstring path = L"Attacking_Demon\\" + std::to_wstring(i) + L".png";
        loadimage(&res->demon_attack[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);
    }

    for (int i = 1; i <= 13; i++) {
        std::wstring path = L"Death_Demon\\" + std::to_wstring(i) + L".png";
        loadimage(&res->demon_death[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);
    }

    // 加载怪物ufo动画
    for (int i = 1; i <= 4; i++) {
        std::wstring path = L"ufo\\idle\\" + std::to_wstring(i) + L".png";
        loadimage(&res->ufo_idle[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);
    }
    for (int i = 1; i <= 4; i++) {
        std::wstring path = L"ufo\\die\\" + std::to_wstring(i) + L".png";
        loadimage(&res->ufo_death[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);
    }

    //加载林克动画
    for (int i = 1; i <= 6; i++) {
        std::wstring path = L"link\\" + std::to_wstring(i) + L".png";
        loadimage(&res->link[i - 1], path.c_str(), PLAYER_SIZE, PLAYER_SIZE);
    }

    res->is_loaded = true;
    return true;
}

//文件操作模块

// 从文件加载玩家列表
PlayerAccount* load_players_from_file() {
    PlayerAccount* head = NULL;
    PlayerAccount* tail = NULL;
    FILE* fp = fopen("player_list.txt", "r");

    // 读取玩家数量
    int player_num = 0;
    fscanf(fp, "%d", &player_num);

    // 读取每个玩家数据
    for (int i = 0; i < player_num; i++) {
        PlayerAccount* new_node = (PlayerAccount*)malloc(sizeof(PlayerAccount));

        fscanf(fp, "%s %s %d", new_node->name, new_node->password, &new_node->score);
        new_node->next = NULL;

        if (head == NULL) {
            head = new_node;
            tail = new_node;
        }
        else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    fclose(fp);
    return head;
}

// 保存玩家列表到文件
void save_players_to_file(PlayerAccount* head, GameState* state) {
    _fcloseall(); // 关闭所有打开的文件
    FILE* fp = fopen("player_list.txt", "w");

    char n[100];
    if (state->current_player != NULL) {
        strcpy(n, state->current_player->name);
    }

    PlayerAccount pc[100];
    // 统计玩家数量
    int count = 0;
    PlayerAccount* p = head;
    while (p != NULL) {
        pc[count] = *p;
        count++;
        p = p->next;
    }

    //排序
    PlayerAccount temp;
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - 1 - i; j++) {
            if (pc[j].score < pc[j + 1].score) {
                strcpy(temp.name, pc[j].name);
                strcpy(temp.password, pc[j].password);
                temp.score = pc[j].score;

                strcpy(pc[j].name, pc[j + 1].name);
                strcpy(pc[j].password, pc[j + 1].password);
                pc[j].score = pc[j + 1].score;

                strcpy(pc[j + 1].name, temp.name);
                strcpy(pc[j + 1].password, temp.password);
                pc[j + 1].score = temp.score;
            }
        }
    }

    //重写链表
    p = head;
    count = 0;
    while (p != NULL) {
        if (state->current_player != NULL) {
            if (strcmp(n, pc[count].name) == 0) {
                state->current_player = p;
            }
        }
        strcpy(p->name, pc[count].name);
        strcpy(p->password, pc[count].password);
        p->score = pc[count].score;

        count++;
        p = p->next;
    }

    // 写入玩家数量
    fprintf(fp, "%d\n", count);

    // 写入玩家数据
    p = head;
    while (p != NULL) {
        fprintf(fp, "%s %s %d\n", p->name, p->password, p->score);
        fflush(fp); // 强制刷新缓冲区
        p = p->next;
    }

    fclose(fp);
}

// 添加新玩家到链表
PlayerAccount* add_new_player(PlayerAccount* head, const char* name, const char* password) {
    PlayerAccount* new_node = (PlayerAccount*)malloc(sizeof(PlayerAccount));

    strcpy(new_node->name, name);
    strcpy(new_node->password, password);
    new_node->score = 0;
    new_node->next = NULL;

    if (head == NULL) {
        return new_node;
    }

    // 找到链表尾部
    PlayerAccount* p = head;
    while (p->next != NULL) {
        p = p->next;
    }
    p->next = new_node;

    return new_node;
}

// 验证密码
bool check_password(PlayerAccount* head, const char* name, const char* password, PlayerAccount** current_player) {
    PlayerAccount* p = head;
    while (p != NULL) {
        if (strcmp(p->name, name) == 0) {
            *current_player = p;
            return strcmp(p->password, password) == 0;
        }
        p = p->next;
    }
    return false;
}

//碰撞检测模块

// 检测玩家与地图碰撞
bool check_player_collision(POINT player_pos, int map[][26]) {
    int bx = player_pos.x + PLAYER_SIZE / 4;
    int by = player_pos.y + PLAYER_SIZE * 2 / 3;
    int x = bx / 50;
    int y = by / 50;

    if (map[y][x]) return true;

    x = (bx + PLAYER_SIZE * 5 / 12) / 50;
    if (map[y][x]) return true;

    y = (by + PLAYER_SIZE / 3) / 50;
    if (map[y][x]) return true;

    x = bx / 50;
    if (map[y][x]) return true;

    return false;
}

// 检测子弹与怪物demon碰撞
bool check_bullet_demon_collision(Bullet* bullet, Demon* demon) {
    return (bullet->x + 10 > demon->x) &&
        (bullet->x - 10 < demon->x + PLAYER_SIZE) &&
        (bullet->y + 10 > demon->y) &&
        (bullet->y - 10 < demon->y + PLAYER_SIZE);
}

// 检测子弹与怪物ufo碰撞
bool check_bullet_ufo_collision(Bullet* bullet, Ufo* ufo) {
    return (bullet->x + 10 > ufo->x) &&
        (bullet->x - 10 < ufo->x + PLAYER_SIZE) &&
        (bullet->y + 10 > ufo->y) &&
        (bullet->y - 10 < ufo->y + PLAYER_SIZE);
}

// 检测玩家与怪物碰撞
bool check_player_demon_collision(POINT player_pos, Demon* demon) {
    return (player_pos.x + PLAYER_SIZE * 2 / 3 > demon->x) &&
        (player_pos.x + PLAYER_SIZE / 4 < demon->x + PLAYER_SIZE) &&
        (player_pos.y + PLAYER_SIZE > demon->y) &&
        (player_pos.y + PLAYER_SIZE * 2 / 3 < demon->y + PLAYER_SIZE);
}

// 检测玩家与子弹碰撞
bool check_player_bullet_collision(POINT player_pos, Bullet* bullet) {
    return (player_pos.x + PLAYER_SIZE * 2 / 3 > bullet->x) &&
        (player_pos.x + PLAYER_SIZE / 4 < bullet->x + PLAYER_SIZE) &&
        (player_pos.y + PLAYER_SIZE > bullet->y) &&
        (player_pos.y + PLAYER_SIZE * 2 / 3 < bullet->y + PLAYER_SIZE);
}

// 检测玩家一与玩家二子弹碰撞
bool check_player1_bullet_collision(POINT player1_pos, Bullet* bullet) {
    return (player1_pos.x + PLAYER_SIZE * 2 / 3 > bullet->x) &&
        (player1_pos.x + PLAYER_SIZE / 4 < bullet->x + 10) && // 子弹半径10，适配原有子弹大小
        (player1_pos.y + PLAYER_SIZE > bullet->y) &&
        (player1_pos.y + PLAYER_SIZE * 2 / 3 < bullet->y + 10);
}

// 检测玩家二与玩家一子弹碰撞
bool check_player2_bullet_collision(POINT player2_pos, Bullet* bullet) {
    return (player2_pos.x + PLAYER_SIZE * 2 / 3 > bullet->x) &&
        (player2_pos.x + PLAYER_SIZE / 4 < bullet->x + 10) &&
        (player2_pos.y + PLAYER_SIZE > bullet->y) &&
        (player2_pos.y + PLAYER_SIZE * 2 / 3 < bullet->y + 10);
}

//UI渲染模块

// 绘制开始界面
void draw_start_screen(GameResources* res) {
    putimage(0, 0, &res->bg_begin);

    setcolor(BLACK);
    setfillcolor(YELLOW);
    settextcolor(BLACK);
    setbkmode(TRANSPARENT);
    settextstyle(50, 0, _T("Consolas"));

    fillrectangle(540, 350, 740, 450);
    outtextxy(560, 380, _T("wake up"));

    fillrectangle(540, 500, 740, 600);
    outtextxy(580, 530, _T("sleep"));
}

// 绘制大厅界面
void draw_hall_screen(GameResources* res, bool login_success, char* player_name, int score) {

    // 绘制施法动画和传送门
    static int counter = 0, spellcast_frame = 0;
    counter++;
    if (counter % 3 == 0) spellcast_frame++;
    if (spellcast_frame >= 7) spellcast_frame = 0;
    if (counter >= 21) counter = 0;

    putimage_alpha(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 100, &res->spellcast_anime[spellcast_frame]);
    putimage(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 100, &res->img_portal);
    putimage(SCREEN_WIDTH / 2 - 400, SCREEN_HEIGHT / 2 - 100, &res->signs);
    putimage(SCREEN_WIDTH / 2 + 300, SCREEN_HEIGHT / 2 - 100, &res->pvp);

    // 绘制玩家名称或操作提示
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    settextstyle(50, 0, _T("Consolas"));
    if (login_success && player_name != NULL) {
        TCHAR name_buf[45] = { 0 };
        MultiByteToWideChar(CP_ACP, 0, player_name, -1, name_buf, 45);
        outtextxy(50, 10, _T("Name:"));
        outtextxy(170, 10, name_buf);
        TCHAR s[5];
        _stprintf(s, _T("%d"), score);
        outtextxy(SCREEN_WIDTH / 2 - 300, 10, _T("Score:"));
        outtextxy(SCREEN_WIDTH / 2 - 160, 10, s);
    }
    else {
        outtextxy(250, 10, _T("W A S D control up,left,down,right"));
    }

}

// 绘制游戏背景
void draw_game_background(POINT player_pos, bool up_or_down, int map[][26], GameResources* res) {
    int y = (player_pos.y + PLAYER_SIZE * 2 / 3) / 50;

    if (up_or_down) {
        // 绘制上半部分
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 26; j++) {
                if (map[i][j] == 0 || map[i][j] == 2) putimage(j * 50, i * 50, &res->img_ground);
            }
        }
        for (int i = 0; i <= y; i++) {
            for (int j = 0; j < 26; j++) {
                if (map[i][j] == 1) putimage(j * 50, i * 50 - 25, &res->img_wall);
            }
        }
    }
    else {
        // 绘制下半部分
        for (int i = y + 1; i < 16; i++) {
            for (int j = 0; j < 26; j++) {
                if (map[i][j] == 1) putimage(j * 50, i * 50 - 25, &res->img_wall);
            }
        }
    }
}

// 绘制玩家血条
void draw_player_hp_bar(int hp) {
    // 血条背景
    int hp_x = 50, hp_y = 20;
    int hp_width = 200, hp_height = 30;
    setfillcolor(RGB(50, 50, 50));
    setlinecolor(RGB(20, 20, 20));
    fillrectangle(hp_x, hp_y, hp_x + hp_width, hp_y + hp_height);
    rectangle(hp_x, hp_y, hp_x + hp_width, hp_y + hp_height);

    // 血条进度
    float hp_ratio = (float)hp / MAX_PLAYER_HP;
    int current_width = hp_width * hp_ratio;
    if (current_width < 0) current_width = 0;

    setfillcolor(hp_ratio <= 0.3 ? RGB(255, 50, 50) : RGB(50, 255, 50));
    fillrectangle(hp_x + 2, hp_y + 2, hp_x + current_width - 2, hp_y + hp_height - 2);

    // 血量文字
    settextcolor(BLACK);
    setbkmode(TRANSPARENT);
    settextstyle(20, 0, _T("Consolas"));
    TCHAR hp_text[20];
    wsprintf(hp_text, _T("HP: %d/%d"), hp, MAX_PLAYER_HP);
    int text_x = hp_x + (hp_width - textwidth(hp_text)) / 2;
    int text_y = hp_y + (hp_height - textheight(hp_text)) / 2;
    outtextxy(text_x, text_y, hp_text);
}

void draw_player1_hp_bar(int hp) {
    // 血条背景
    int hp_x = 50, hp_y = 20;
    int hp_width = 200, hp_height = 30;
    setfillcolor(RGB(50, 50, 50));
    setlinecolor(RGB(20, 20, 20));
    fillrectangle(hp_x, hp_y, hp_x + hp_width, hp_y + hp_height);
    rectangle(hp_x, hp_y, hp_x + hp_width, hp_y + hp_height);

    // 血条进度
    float hp_ratio = (float)hp / MAX_PLAYER_HP;
    int current_width = hp_width * hp_ratio;
    if (current_width < 0) current_width = 0;

    setfillcolor(hp_ratio <= 0.3 ? RGB(255, 50, 50) : RGB(50, 255, 50));
    fillrectangle(hp_x + 2, hp_y + 2, hp_x + current_width - 2, hp_y + hp_height - 2);

    // 血量文字（标注Player 1）
    settextcolor(BLACK);
    setbkmode(TRANSPARENT);
    settextstyle(20, 0, _T("Consolas"));
    TCHAR hp_text[20];
    wsprintf(hp_text, _T("Player1 HP: %d/%d"), hp, MAX_PLAYER_HP);
    int text_x = hp_x + (hp_width - textwidth(hp_text)) / 2;
    int text_y = hp_y + (hp_height - textheight(hp_text)) / 2;
    outtextxy(text_x, text_y, hp_text);
}

void draw_player2_hp_bar(int hp) {
    // 血条背景
    int hp_x = SCREEN_WIDTH - 250, hp_y = 20;
    int hp_width = 200, hp_height = 30;
    setfillcolor(RGB(50, 50, 50));
    setlinecolor(RGB(20, 20, 20));
    fillrectangle(hp_x, hp_y, hp_x + hp_width, hp_y + hp_height);
    rectangle(hp_x, hp_y, hp_x + hp_width, hp_y + hp_height);

    // 血条进度
    float hp_ratio = (float)hp / MAX_PLAYER_HP;
    int current_width = hp_width * hp_ratio;
    if (current_width < 0) current_width = 0;

    setfillcolor(hp_ratio <= 0.3 ? RGB(255, 50, 50) : RGB(50, 50, 255));
    fillrectangle(hp_x + 2, hp_y + 2, hp_x + current_width - 2, hp_y + hp_height - 2);

    // 血量文字（标注Player 2）
    settextcolor(BLACK);
    setbkmode(TRANSPARENT);
    settextstyle(20, 0, _T("Consolas"));
    TCHAR hp_text[20];
    wsprintf(hp_text, _T("Player2 HP: %d/%d"), hp, MAX_PLAYER_HP);
    int text_x = hp_x + (hp_width - textwidth(hp_text)) / 2;
    int text_y = hp_y + (hp_height - textheight(hp_text)) / 2;
    outtextxy(text_x, text_y, hp_text);
}

// 绘制怪物demon血条
void draw_demon_hp_bar(Demon* demon) {
    int hp_x = demon->x;
    int hp_y = demon->y - 15;
    int hp_width = PLAYER_SIZE;
    int hp_height = 8;

    // 血条背景
    setfillcolor(RGB(80, 80, 80));
    setlinecolor(RGB(50, 50, 50));
    fillrectangle(hp_x, hp_y, hp_x + hp_width, hp_y + hp_height);
    rectangle(hp_x, hp_y, hp_x + hp_width, hp_y + hp_height);

    // 血条进度
    float hp_ratio = (float)demon->hp / DEMON_MAX_HP;
    int current_width = hp_width * hp_ratio;
    if (current_width < 0) current_width = 0;

    setfillcolor(RGB(255, 50, 50));
    fillrectangle(hp_x + 1, hp_y + 1, hp_x + current_width - 1, hp_y + hp_height - 1);

    // 血量文字
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    settextstyle(12, 0, _T("Consolas"));
    TCHAR hp_text[10];
    wsprintf(hp_text, _T("%d/5"), demon->hp);
    int text_x = hp_x + (hp_width - textwidth(hp_text)) / 2;
    int text_y = hp_y - 2;
    outtextxy(text_x, text_y, hp_text);
}

//绘制怪物ufo血条
void draw_ufo_hp_bar(Ufo* ufo) {
    int hp_x = ufo->x;
    int hp_y = ufo->y - 15;
    int hp_width = PLAYER_SIZE;
    int hp_height = 8;

    // 血条背景
    setfillcolor(RGB(80, 80, 80));
    setlinecolor(RGB(50, 50, 50));
    fillrectangle(hp_x, hp_y, hp_x + hp_width, hp_y + hp_height);
    rectangle(hp_x, hp_y, hp_x + hp_width, hp_y + hp_height);

    // 血条进度
    float hp_ratio = (float)ufo->hp / DEMON_MAX_HP;
    int current_width = hp_width * hp_ratio;
    if (current_width < 0) current_width = 0;

    setfillcolor(RGB(255, 50, 50));
    fillrectangle(hp_x + 1, hp_y + 1, hp_x + current_width - 1, hp_y + hp_height - 1);

    // 血量文字
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    settextstyle(12, 0, _T("Consolas"));
    TCHAR hp_text[10];
    wsprintf(hp_text, _T("%d/5"), ufo->hp);
    int text_x = hp_x + (hp_width - textwidth(hp_text)) / 2;
    int text_y = hp_y - 2;
    outtextxy(text_x, text_y, hp_text);
}

// 绘制玩家动画
void draw_player_animation(GameState* state, GameResources* res) {
    if (state->player_walk_flag) {
        // 行走动画
        switch (state->player_direction) {
        case 0: putimage_alpha(state->player_pos.x, state->player_pos.y, &res->player_walk_down[state->walk_anime_frame]); break;
        case 1: putimage_alpha(state->player_pos.x, state->player_pos.y, &res->player_walk_left[state->walk_anime_frame]); break;
        case 2: putimage_alpha(state->player_pos.x, state->player_pos.y, &res->player_walk_up[state->walk_anime_frame]); break;
        case 3: putimage_alpha(state->player_pos.x, state->player_pos.y, &res->player_walk_right[state->walk_anime_frame]); break;
        }
    }
    else {
        // Idle动画
        switch (state->player_direction) {
        case 0: putimage_alpha(state->player_pos.x, state->player_pos.y, &res->player_idle_down[state->idle_anime_frame]); break;
        case 1: putimage_alpha(state->player_pos.x, state->player_pos.y, &res->player_idle_left[state->idle_anime_frame]); break;
        case 2: putimage_alpha(state->player_pos.x, state->player_pos.y, &res->player_idle_up[state->idle_anime_frame]); break;
        case 3: putimage_alpha(state->player_pos.x, state->player_pos.y, &res->player_idle_right[state->idle_anime_frame]); break;
        }
    }
}

// 绘制怪物demon动画
void draw_demon_animation(GameState* state, GameResources* res) {
    for (int i = 0; i < state->demons.size(); i++) {
        Demon* demon = &state->demons[i];

        if (demon->mood == 3) {
            // 死亡动画
            int frame_idx = demon->death_frame / 5;
            if (frame_idx >= 13) frame_idx = 12;
            putimage_alpha(demon->x, demon->y, &res->demon_death[frame_idx]);
        }
        else if (demon->is_attacking) {
            // 攻击动画
            int frame_idx = demon->attack_frame / 5;
            if (frame_idx >= 12) frame_idx = 11;
            putimage_alpha(demon->x, demon->y, &res->demon_attack[frame_idx]);
            draw_demon_hp_bar(demon);
        }
        else {
            // 漂浮动画
            putimage_alpha(demon->x, demon->y, &res->demon_floating[state->demon_floating_frame]);
            draw_demon_hp_bar(demon);
        }
    }
}

// 绘制怪物ufo动画
void draw_ufo_animation(GameState* state, GameResources* res) {
    for (int i = 0; i < state->ufo.size(); i++) {
        Ufo* ufo = &state->ufo[i];

        // Use hp to determine alive/dead display to avoid mismatches between
        // the saved `alive` flag and `hp` after loading.
        if (ufo->hp <= 0) {
            // 死亡动画
            int frame_idx = ufo->death_frame / 5;
            if (frame_idx >= 4) frame_idx = 3;
            putimage_alpha(ufo->x, ufo->y, &res->ufo_death[frame_idx]);
        }
        else {
            // 漂浮动画
            putimage_alpha(ufo->x, ufo->y, &res->ufo_idle[state->ufo_idle_frame]);
            draw_ufo_hp_bar(ufo);
        }
    }
}

// 绘制子弹
void draw_bullets(GameState* state) {
    for (int i = 0; i < state->bullets.size(); i++) {
        if (state->bullets[i].friendly) {
            setfillcolor(YELLOW);
        }
        else {
            setfillcolor(RED);
        }
        fillcircle(state->bullets[i].x, state->bullets[i].y, 10);
    }
}

//绘制新旧游戏选择界面
void draw_new_or_old(GameState* state, GameResources* res) {
    setcolor(WHITE);
    setfillcolor(BLACK);
    settextstyle(50, 0, _T("Consolas"));
    fillrectangle(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 150, SCREEN_WIDTH / 2 + 150, SCREEN_HEIGHT / 2 - 150);
    outtextxy(SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 130, _T("Choose"));
    if (state->have_game) {//有旧档
        fillrectangle(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 70, SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2);
        outtextxy(SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 - 60, _T("New Game"));
        fillrectangle(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 30, SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2 + 100);
        outtextxy(SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 40, _T("Continue"));
    }
    else {//无旧档
        fillrectangle(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 35, SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2 + 35);
        outtextxy(SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 - 25, _T("New Game"));
    }
}

//绘制暂停界面
void draw_stop(GameState* state) {
    setcolor(WHITE);
    setfillcolor(BLACK);
    settextstyle(40, 0, _T("Consolas"));
    fillrectangle(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 150, SCREEN_WIDTH / 2 + 150, SCREEN_HEIGHT / 2 - 150);
    outtextxy(SCREEN_WIDTH / 2 - 45, SCREEN_HEIGHT / 2 - 130, _T("Pause"));
    fillrectangle(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 70, SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2);
    outtextxy(SCREEN_WIDTH / 2 - 75, SCREEN_HEIGHT / 2 - 55, _T("Continue"));
    fillrectangle(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 30, SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2 + 100);
    outtextxy(SCREEN_WIDTH / 2 - 85, SCREEN_HEIGHT / 2 + 45, _T("Quit Hall"));
}

//绘制游戏结束界面
void draw_game_over(GameState* state) {
    setcolor(WHITE);
    setfillcolor(BLACK);
    settextstyle(40, 0, _T("Consolas"));
    fillrectangle(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 150, SCREEN_WIDTH / 2 + 150, SCREEN_HEIGHT / 2 - 150);
    if (state->game_win) {
        outtextxy(SCREEN_WIDTH / 2 - 65, SCREEN_HEIGHT / 2 - 130, _T("Victory"));
        setcolor(GREEN);
        TCHAR t[5];
        _stprintf(t, _T("%d"), 100-state->time_cost);
        outtextxy(SCREEN_WIDTH / 2 - 15, SCREEN_HEIGHT / 2 - 40, t);
        setcolor(WHITE);
        fillrectangle(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 30, SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2 + 100);
        outtextxy(SCREEN_WIDTH / 2 - 85, SCREEN_HEIGHT / 2 + 45, _T("Quit Hall"));
        if (state->current_player->score < 100-state->time_cost) {
            state->current_player->score = 100-state->time_cost;
        }
    }
    else {
        setcolor(RED);
        outtextxy(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 70, _T("Defeat"));
        setcolor(WHITE);
        fillrectangle(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 30, SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2 + 100);
        outtextxy(SCREEN_WIDTH / 2 - 85, SCREEN_HEIGHT / 2 + 45, _T("Quit Hall"));
    }
}

//绘制耗时
void draw_time_cost(GameState* state) {
    TCHAR t[100];
    _stprintf(t, _T("%d"), state->time_cost);
    setcolor(WHITE);
    settextstyle(50, 0, _T("Consolas"));
    outtextxy(300, 10, _T("Time:"));
    outtextxy(415, 10, t);
}

//绘制排行榜
void draw_ranking(GameState* state) {
    setcolor(WHITE);
    setfillcolor(BLACK);
    settextstyle(40, 0, _T("Consolas"));
    fillrectangle(SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2 + 275, SCREEN_WIDTH / 2 + 300, SCREEN_HEIGHT / 2 - 250);

    // 标题：Ranking List 居中
    settextcolor(WHITE);
    settextstyle(50, 0, _T("Consolas"));
    const TCHAR title[] = _T("Ranking List");
    int titleX = (SCREEN_WIDTH - textwidth(title)) / 2;
    int titleY = SCREEN_HEIGHT / 2 - 200;
    outtextxy(titleX, titleY, title);

    // 读取玩家列表（文件已排好序，直接读前5个）
    char names[5][45] = { 0 };
    int scores[5] = { 0 };
    int count = 0;

    FILE* fp = fopen("player_list.txt", "r");
    if (fp != NULL) {
        int total = 0;
        fscanf(fp, "%d", &total);

        for (int i = 0; i < total && count < 5; ++i) {
            char name[45] = { 0 };
            char pwd[45] = { 0 };
            int sc = 0;
            if (fscanf(fp, "%s %s %d", name, pwd, &sc) == 3) {
                strcpy(names[count], name);
                scores[count] = sc;
                count++;
            }
        }
        fclose(fp);
    }

    // 绘制前5名（等间距）
    settextstyle(35, 0, _T("Consolas"));
    int startY = SCREEN_HEIGHT / 2 - 120;
    int lineH = 60;

    for (int i = 0; i < 5; ++i) {
        TCHAR line[100] = { 0 };
        if (i < count) {
            TCHAR wname[45] = { 0 };
            MultiByteToWideChar(CP_ACP, 0, names[i], -1, wname, 45);
            wsprintf(line, _T("%d. %s  ——  %d"), i + 1, wname, scores[i]);
        }
        else {
            wsprintf(line, _T("%d. ------"), i + 1);
        }

        int x = (SCREEN_WIDTH - textwidth(line)) / 2;
        int y = startY + i * lineH;
        outtextxy(x, y, line);
    }

    // 退出按钮
    int btnW = 200, btnH = 60;
    int btnX = (SCREEN_WIDTH - btnW) / 2;
    int btnY = SCREEN_HEIGHT / 2 + 200;
    setfillcolor(RGB(70, 70, 70));
    fillrectangle(btnX, btnY, btnX + btnW, btnY + btnH);
    settextcolor(WHITE);
    const TCHAR quit[] = _T("Quit");
    int tx = btnX + (btnW - textwidth(quit)) / 2;
    int ty = btnY + (btnH - textheight(quit)) / 2;
    outtextxy(tx, ty, quit);
}

// 绘制PVP对战结束界面（显示胜负结果）
void draw_pvp_over(GameState* state1, GameState* state2) {
    setcolor(WHITE);
    setfillcolor(BLACK);
    settextstyle(40, 0, _T("Consolas"));
    fillrectangle(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 150, SCREEN_WIDTH / 2 + 150, SCREEN_HEIGHT / 2 - 150);

    // 判定胜负（生命值先清零者输）
    TCHAR result[50] = { 0 };
    if (state1->player_hp <= 0) {
        setcolor(RGB(50, 50, 255)); // 玩家二胜利，蓝色字体
        wsprintf(result, _T("Player 2 Win!"));
    }
    else {
        setcolor(RGB(50, 255, 50)); // 玩家一胜利，绿色字体
        wsprintf(result, _T("Player 1 Win!"));
    }

    // 绘制胜负文字（居中）
    int textX = (SCREEN_WIDTH - textwidth(result)) / 2;
    int textY = SCREEN_HEIGHT / 2 - 70;
    outtextxy(textX, textY, result);

    // 退出按钮（返回大厅）
    setfillcolor(RGB(70, 70, 70));
    fillrectangle(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 30, SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2 + 100);
    setcolor(WHITE);
    const TCHAR quit[] = _T("Quit Hall");
    int btnTextX = SCREEN_WIDTH / 2 - (textwidth(quit) / 2);
    int btnTextY = SCREEN_HEIGHT / 2 + 45;
    outtextxy(btnTextX, btnTextY, quit);
}


//输入处理模块

// 处理开始界面输入
void handle_start_input(GameState* state) {
    ExMessage mouse;
    while (1) {
        peekmessage(&mouse, EX_MOUSE, true);
        if (mouse.message == WM_LBUTTONDOWN) {
            // Sleep按钮
            if (mouse.x >= 540 && mouse.x <= 740 && mouse.y >= 500 && mouse.y <= 600) {
                state->sleep_flag = 1;
                break;
            }
            // Wake up按钮
            if (mouse.x >= 540 && mouse.x <= 740 && mouse.y >= 350 && mouse.y <= 450) {
                state->bg_mode = 1;
                break;
            }
        }
    }
}

// 处理文本输入（用户名/密码）
bool handle_text_input(IMAGE* bg, int input_type, PlayerAccount* player_list,
    PlayerAccount** current_player, int is_old_user) {
    settextcolor(BLACK);
    bool input_finish = false;
    char input_buf[45] = { 0 };
    int cursor_pos = 0;
    int char_width = textwidth(_T("A")) + 2;
    static char temp_name[45] = { 0 };

    while (1) {
        DWORD start_time = GetTickCount();
        ExMessage msg;

        // 处理输入
        while (peekmessage(&msg, EX_KEY | EX_MOUSE, true)) {
            if (msg.message == WM_KEYDOWN) {
                // 退格键
                if (msg.vkcode == VK_BACK && cursor_pos > 0) {
                    cursor_pos--;
                    memmove(&input_buf[cursor_pos], &input_buf[cursor_pos + 1], strlen(input_buf) - cursor_pos);
                }
                // 回车键确认
                else if (msg.vkcode == VK_RETURN && strlen(input_buf) > 0) {
                    input_finish = true;
                }
                // 可打印字符
                else if (msg.vkcode >= 32 && msg.vkcode <= 126 && cursor_pos < 44) {
                    memmove(&input_buf[cursor_pos + 1], &input_buf[cursor_pos], strlen(input_buf) - cursor_pos + 1);
                    input_buf[cursor_pos] = msg.vkcode;
                    cursor_pos++;
                }
            }
            // 点击确认按钮
            else if (msg.message == WM_LBUTTONDOWN) {
                if (msg.x >= 1040 && msg.x <= 1220 && msg.y >= SCREEN_HEIGHT - 100 && msg.y <= SCREEN_HEIGHT - 50 && strlen(input_buf) > 0) {
                    input_finish = true;
                }
            }
        }

        // 处理输入完成
        if (input_finish) {
            if (input_type == 0) {
                // 用户名输入
                strcpy(temp_name, input_buf);
                PlayerAccount* p = player_list;
                while (p != NULL) {
                    if (strcmp(p->name, input_buf) == 0) {
                        *current_player = p;
                        return true; // 老用户
                    }
                    p = p->next;
                }
                // 新用户
                *current_player = add_new_player(player_list, input_buf, "");
                return false;
            }
            else {
                // 密码输入
                if (is_old_user) {
                    // 验证密码
                    return check_password(player_list, temp_name, input_buf, current_player);
                }
                else {
                    // 设置新密码
                    strcpy((*current_player)->password, input_buf);
                    return true;
                }
            }
        }

        // 绘制输入界面
        putimage(0, 0, bg);
        setfillcolor(DARKGRAY);
        fillrectangle(1040, SCREEN_HEIGHT - 100, 1220, SCREEN_HEIGHT - 50);
        settextcolor(WHITE);
        outtextxy(1050, SCREEN_HEIGHT - 100, _T("confirm"));
        settextcolor(BLACK);

        if (input_type == 0) {
            outtextxy(50, SCREEN_HEIGHT - 220, _T("My name is:"));
        }
        else {
            outtextxy(50, SCREEN_HEIGHT - 220, _T("My password is:"));
        }

        // 绘制输入内容
        int x = 100;
        for (int i = 0; input_buf[i] != '\0'; i++) {
            if (x + char_width > 1100) break;
            TCHAR ch[2] = { 0 };
            ch[0] = input_buf[i];
            outtextxy(x, SCREEN_HEIGHT - 160, ch);
            x += char_width;
        }

        // 帧率控制
        FlushBatchDraw();
        control_fps(start_time);
    }
}

// 处理登录/注册流程
bool handle_login_process(GameResources* res, PlayerAccount* player_list, PlayerAccount** current_player, GameState* state) {
    ExMessage msg;
    char input_name[45] = { 0 };
    char input_pwd[45] = { 0 };

    // 第一步：请求输入用户名
    putimage(0, 0, &res->bg_spellcast_talk);
    settextcolor(BLACK);
    setbkmode(TRANSPARENT);
    outtextxy(50, SCREEN_HEIGHT - 220, _T("Tell me your name,my lord."));
    outtextxy(750, SCREEN_HEIGHT - 80, _T("(press F to continue)"));
    FlushBatchDraw();

    // 等待F键
    while (1) {
        if (peekmessage(&msg, EX_KEY, true) && msg.message == WM_KEYDOWN && msg.vkcode == 'F') {
            break;
        }
    }

    // 输入用户名
    putimage(0, 0, &res->bg_player_talk);
    outtextxy(50, SCREEN_HEIGHT - 220, _T("My name is:"));
    setcolor(BLACK);
    setfillcolor(DARKGRAY);
    fillrectangle(1040, SCREEN_HEIGHT - 100, 1220, SCREEN_HEIGHT - 50);
    settextcolor(WHITE);
    outtextxy(1050, SCREEN_HEIGHT - 100, _T("confirm"));
    FlushBatchDraw();

    // 处理用户名输入
    bool is_old_user = handle_text_input(&res->bg_player_talk, 0, player_list, current_player, 0);
    strcpy(input_name, (*current_player)->name);

    if (is_old_user) {
        // 老用户：验证密码
        putimage(0, 0, &res->bg_spellcast_talk);
        outtextxy(50, SCREEN_HEIGHT - 220, _T("I remember you,tell me your password,my lord."));
        outtextxy(750, SCREEN_HEIGHT - 80, _T("(press F to continue)"));
        FlushBatchDraw();

        // 等待F键
        while (1) {
            if (peekmessage(&msg, EX_KEY, true) && msg.message == WM_KEYDOWN && msg.vkcode == 'F') {
                break;
            }
        }

        // 输入密码
        putimage(0, 0, &res->bg_player_talk);
        outtextxy(50, SCREEN_HEIGHT - 220, _T("My password is:"));
        fillrectangle(1040, SCREEN_HEIGHT - 100, 1220, SCREEN_HEIGHT - 50);
        outtextxy(1050, SCREEN_HEIGHT - 100, _T("confirm"));
        FlushBatchDraw();

        // 验证密码
        bool pwd_correct = handle_text_input(&res->bg_player_talk, 1, player_list, current_player, 1);
        if (!pwd_correct) {
            putimage(0, 0, &res->bg_spellcast_talk);
            outtextxy(50, SCREEN_HEIGHT - 220, _T("Wrong password! Try again later."));
            FlushBatchDraw();
            Sleep(2000);
            return false;
        }
        else {
            putimage(0, 0, &res->bg_spellcast_talk);
            outtextxy(50, SCREEN_HEIGHT - 220, _T("Welcome back, my lord!"));
            FlushBatchDraw();
            Sleep(2000);
            // 登录成功后检查是否存在存档
            if (state != NULL && *current_player != NULL) {
                char savefile[128];
                sprintf(savefile, "save_%s.dat", (*current_player)->name);
                FILE* sf = fopen(savefile, "r");
                if (sf) { fclose(sf); state->have_game = true; }
                else state->have_game = false;
            }
            return true;
        }
    }
    else {
        // 新用户：设置密码
        putimage(0, 0, &res->bg_spellcast_talk);
        outtextxy(50, SCREEN_HEIGHT - 220, _T("It's our first meeting,give me a password,my lord."));
        outtextxy(750, SCREEN_HEIGHT - 80, _T("(press F to continue)"));
        FlushBatchDraw();

        // 等待F键
        while (1) {
            if (peekmessage(&msg, EX_KEY, true) && msg.message == WM_KEYDOWN && msg.vkcode == 'F') {
                break;
            }
        }

        // 输入密码
        putimage(0, 0, &res->bg_player_talk);
        outtextxy(50, SCREEN_HEIGHT - 220, _T("My password is:"));
        fillrectangle(1040, SCREEN_HEIGHT - 100, 1220, SCREEN_HEIGHT - 50);
        outtextxy(1050, SCREEN_HEIGHT - 100, _T("confirm"));
        FlushBatchDraw();

        // 设置密码
        bool pwd_ok = handle_text_input(&res->bg_player_talk, 1, player_list, current_player, 0);
        if (pwd_ok) {
            save_players_to_file(player_list, state);
            putimage(0, 0, &res->bg_spellcast_talk);
            outtextxy(50, SCREEN_HEIGHT - 220, _T("Welcome to our world, my lord!"));
            FlushBatchDraw();
            Sleep(2000);
            // 新用户刚创建，不存在存档
            if (state != NULL) state->have_game = false;
            return true;
        }
    }

    return false;
}

// 处理大厅游戏逻辑
void handle_hall_logic(GameState* state, GameResources* res, PlayerAccount* player_list) {
    // 初始化地图
    int map[16][26] = { 0 };
    for (int i = 2; i < 15; i++) {
        for (int j = 1; j < 25; j++) {
            if (i == 2 || i == 14 || j == 1 || j == 24) map[i][j] = 1;
        }
    }
    // 传送门区域
    for (int i = 6; i < 10; i++) {
        for (int j = 11; j < 15; j++) {
            map[i][j] = 1;
        }
    }
    // 魔法师区域
    for (int i = 6; i < 8; i++) {
        for (int j = 9; j < 11; j++) {
            map[i][j] = 1;
        }
    }

    //排行榜区域
    for (int i = 6; i < 8; i++) {
        for (int j = 5; j < 7; j++) {
            map[i][j] = 1;
        }
    }

    //pvp区域
    for (int i = 6; i < 8; i++) {
        for (int j = 19; j < 21; j++) {
            map[i][j] = 1;
        }
    }

    // 初始化玩家位置
    state->player_pos = { SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2 - 100 };
    state->player_speed = 5;
    state->player_direction = 0;

    BeginBatchDraw();
    while (1) {
        DWORD start_time = GetTickCount();
        ExMessage msg;

        // 处理输入
        while (peekmessage(&msg, EX_KEY | EX_MOUSE, true)) {
            if (msg.message == WM_KEYDOWN) {
                switch (msg.vkcode) {
                case 'W': state->key_w = true; state->player_direction = 2; break;
                case 'A': state->key_a = true; state->player_direction = 1; break;
                case 'S': state->key_s = true; state->player_direction = 0; break;
                case 'D': state->key_d = true; state->player_direction = 3; break;
                case 'F':
                    // 对话检测
                    bool talk_pos = (state->player_pos.x >= SCREEN_WIDTH / 2 - 220 &&
                        state->player_pos.x <= SCREEN_WIDTH / 2 - 180 &&
                        state->player_pos.y >= SCREEN_HEIGHT / 2 - 100 &&
                        state->player_pos.y <= SCREEN_HEIGHT / 2 - 60 &&
                        state->player_direction == 2);

                    // 传送检测
                    bool transmit_pos = (state->login_success && !talk_pos &&
                        state->player_pos.y >= SCREEN_HEIGHT / 2 + 30 &&
                        state->player_pos.y <= SCREEN_HEIGHT / 2 + 70 &&
                        state->player_pos.x > SCREEN_WIDTH / 2 - 170 &&
                        state->player_pos.x < SCREEN_WIDTH / 2 + 70);

                    //排行榜检测
                    bool signs_pos = (state->player_pos.x >= SCREEN_WIDTH / 2 - 420 &&
                        state->player_pos.x <= SCREEN_WIDTH / 2 - 380 &&
                        state->player_pos.y >= SCREEN_HEIGHT / 2 - 100 &&
                        state->player_pos.y <= SCREEN_HEIGHT / 2 - 60 &&
                        state->player_direction == 2);

                    //pvp检测
                    bool pvp_pos = (state->player_pos.x >= SCREEN_WIDTH / 2 + 280 &&
                        state->player_pos.x <= SCREEN_WIDTH / 2 + 320 &&
                        state->player_pos.y >= SCREEN_HEIGHT / 2 - 100 &&
                        state->player_pos.y <= SCREEN_HEIGHT / 2 - 60 &&
                        state->player_direction == 2);

                    if (talk_pos) {
                        // 处理登录
                        state->login_success = handle_login_process(res, player_list, &state->current_player, state);
                        cleardevice();
                    }
                    else if (transmit_pos) {
                        // 进入游戏关卡
                        EndBatchDraw();
                        handle_new_or_old(state, res);
                        save_players_to_file(player_list, state);
                        BeginBatchDraw();  // 重新启动大厅的批次绘制
                        state->counter = 0; // 重置动画计数器
                        state->idle_anime_frame = 0;
                        state->walk_anime_frame = 0;
                        state->demon_floating_frame = 0;
                        state->player_pos = { SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2 - 100 };
                        state->player_direction = 0;
                        cleardevice();     // 清理残留画面
                    }
                    else if (signs_pos) {
                        handle_ranking(state);
                    }
                    else if (pvp_pos) {
                        EndBatchDraw();
                        handle_pvp(state,res);
                        BeginBatchDraw();  // 重新启动大厅的批次绘制
                        state->counter = 0; // 重置动画计数器
                        state->idle_anime_frame = 0;
                        state->walk_anime_frame = 0;
                        state->demon_floating_frame = 0;
                        state->player_pos = { SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2 - 100 };
                        state->player_direction = 0;
                        cleardevice();     // 清理残留画面
                    }
                    break;
                }
            }
            else if (msg.message == WM_KEYUP) {
                switch (msg.vkcode) {
                case 'W': state->key_w = false; break;
                case 'A': state->key_a = false; break;
                case 'S': state->key_s = false; break;
                case 'D': state->key_d = false; break;
                }
            }
        }

        // 玩家移动
        state->player_walk_flag = state->key_w || state->key_a || state->key_s || state->key_d;

        if (state->key_w) {
            state->player_pos.y -= state->player_speed;
            if (check_player_collision(state->player_pos, map)) {
                state->player_pos.y += state->player_speed;
            }
        }
        if (state->key_a) {
            state->player_pos.x -= state->player_speed;
            if (check_player_collision(state->player_pos, map)) {
                state->player_pos.x += state->player_speed;
            }
        }
        if (state->key_s) {
            state->player_pos.y += state->player_speed;
            if (check_player_collision(state->player_pos, map)) {
                state->player_pos.y -= state->player_speed;
            }
        }
        if (state->key_d) {
            state->player_pos.x += state->player_speed;
            if (check_player_collision(state->player_pos, map)) {
                state->player_pos.x -= state->player_speed;
            }
        }

        // 更新动画帧
        state->counter++;
        if (state->counter % 5 == 0) {
            state->idle_anime_frame = (state->idle_anime_frame + 1) % 2;
            state->demon_floating_frame = (state->demon_floating_frame + 1) % 10;
        }
        if (state->counter % 2 == 0) {
            state->walk_anime_frame = (state->walk_anime_frame + 1) % 9;
        }
        if (state->counter > 20) state->counter = 0;

        // 绘制界面
        cleardevice();
        int score = 0;
        if (state->current_player != NULL) {  // 增加空指针判断
            score = state->current_player->score;
        }
        // 绘制墙体和地面
        for (int i = 2; i < 15; i++) {
            for (int j = 1; j < 25; j++) {
                if (i == 2) putimage(j * 50, i * 50 - 25, &res->img_wall);
                else if (j == 1 || j == 24) putimage(j * 50, i * 50 - 25, &res->img_wall);
                else putimage(j * 50, i * 50, &res->img_ground);
            }
        }
        draw_hall_screen(res, state->login_success,
            state->current_player ? state->current_player->name : NULL, score);
        draw_player_animation(state, res);
        // 绘制下墙体
        for (int i = 1; i < 25; i++) {
            putimage(i * 50, 14 * 50 - 25, &res->img_wall);
        }

        // 绘制交互提示
        bool talk_pos = (state->player_pos.x >= SCREEN_WIDTH / 2 - 220 &&
            state->player_pos.x <= SCREEN_WIDTH / 2 - 180 &&
            state->player_pos.y >= SCREEN_HEIGHT / 2 - 100 &&
            state->player_pos.y <= SCREEN_HEIGHT / 2 - 60 &&
            state->player_direction == 2);

        bool transmit_pos = (state->login_success && !talk_pos &&
            state->player_pos.y >= SCREEN_HEIGHT / 2 + 30 &&
            state->player_pos.y <= SCREEN_HEIGHT / 2 + 70 &&
            state->player_pos.x > SCREEN_WIDTH / 2 - 170 &&
            state->player_pos.x < SCREEN_WIDTH / 2 + 70);

        bool signs_pos = (state->player_pos.x >= SCREEN_WIDTH / 2 - 420 &&
            state->player_pos.x <= SCREEN_WIDTH / 2 - 380 &&
            state->player_pos.y >= SCREEN_HEIGHT / 2 - 100 &&
            state->player_pos.y <= SCREEN_HEIGHT / 2 - 60 &&
            state->player_direction == 2);

        bool pvp_pos = (state->player_pos.x >= SCREEN_WIDTH / 2 + 280 &&
            state->player_pos.x <= SCREEN_WIDTH / 2 + 320 &&
            state->player_pos.y >= SCREEN_HEIGHT / 2 - 100 &&
            state->player_pos.y <= SCREEN_HEIGHT / 2 - 60 &&
            state->player_direction == 2);

        if (talk_pos) {
            settextstyle(50, 0, _T("Consolas"));
            outtextxy(250, SCREEN_HEIGHT - 50, _T("Press the F to start a conversation"));
        }
        else if (transmit_pos) {
            settextstyle(50, 0, _T("Consolas"));
            outtextxy(400, SCREEN_HEIGHT - 50, _T("Press the F to transmit"));
        }
        else if (signs_pos) {
            settextstyle(50, 0, _T("Consolas"));
            outtextxy(220, SCREEN_HEIGHT - 50, _T("Press the F to check the rangking list"));
        }
        else if (pvp_pos) {
            settextstyle(50, 0, _T("Consolas"));
            outtextxy(380, SCREEN_HEIGHT - 50, _T("Press the F to start PVP game"));
        }

        // 帧率控制
        FlushBatchDraw();
        control_fps(start_time);
    }
    EndBatchDraw();
}

// 处理游戏关卡逻辑
void handle_game_level(GameState* state, GameResources* res) {
    // 初始化地图
    // 标记当前关卡为关卡1（用于保存/读档时区分）
    state->bg_mode = 1;
    int map[16][26] = {
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1},
        {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
        {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
        {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
        { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
        {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
        {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
        {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
        {-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
    };

    // 初始化游戏状态
    state->quit_hall = false;
    state->have_game = true;
    state->player_pos = { 0, SCREEN_HEIGHT / 2 - 75 };
    state->player_hp = MAX_PLAYER_HP;
    state->player_speed = 5;
    state->player_direction = 3;
    state->bullets.clear();
    state->demons.clear();
    state->can_wall = false;
    state->game_win = false;
    state->time_cost = 0;
    state->wall_num = 0;
    bool next_level_pos = false;
    DWORD delta_time_cost = 0;
    DWORD start_time_cost = 0;
    DWORD end_time_cost = 0;

    // 初始化怪物
    Demon demon1 = { SCREEN_WIDTH / 2, 4 * 50, DEMON_MAX_HP, 1, 0, 0, false };
    Demon demon2 = { SCREEN_WIDTH / 2, 12 * 50, DEMON_MAX_HP, 1, 0, 0, false };
    Demon demon3 = { SCREEN_WIDTH - 200, SCREEN_HEIGHT / 2 - 50, DEMON_MAX_HP, 1, 0, 0, false };
    state->demons.push_back(demon1);
    state->demons.push_back(demon2);
    state->demons.push_back(demon3);

    // 保存初始地图到 state->wall_map
    for (int i = 0; i < 16; ++i) for (int j = 0; j < 26; ++j) state->wall_map[i][j] = map[i][j];

    BeginBatchDraw();
    while (1) {
        DWORD start_time = GetTickCount();
        start_time_cost = GetTickCount();
        ExMessage msg;

        // 处理输入
        while (peekmessage(&msg, EX_KEY | EX_MOUSE, true)) {
            if (msg.message == WM_KEYDOWN) {
                switch (msg.vkcode) {
                case 'W': state->key_w = true;  break;
                case 'A': state->key_a = true;  break;
                case 'S': state->key_s = true;  break;
                case 'D': state->key_d = true;  break;
                case 'F':
                    if (state->can_wall && next_level_pos) {
                        handle_game_level2(state, res);
                    }
                    if (state->quit_hall) {
                        EndBatchDraw();
                        return;
                    }
                    break;
                }
            }
            else if (msg.message == WM_KEYUP) {
                switch (msg.vkcode) {
                case 'W': state->key_w = false; break;
                case 'A': state->key_a = false; break;
                case 'S': state->key_s = false; break;
                case 'D': state->key_d = false; break;
                }
            }
            else if (msg.message == WM_LBUTTONDOWN) {
                if (msg.x >= SCREEN_WIDTH - 60 && msg.x <= SCREEN_WIDTH - 10 && msg.y >= 10 && msg.y <= 60) {
                    end_time_cost = GetTickCount();
                    delta_time_cost += end_time_cost - start_time_cost;
                    handle_stop(state);
                    if (state->quit_hall) {
                        EndBatchDraw();
                        return;
                    }
                    start_time_cost = GetTickCount();
                }
                else {
                    // 发射子弹
                    Bullet bullet;
                    bullet.x = state->player_pos.x + PLAYER_SIZE / 2;
                    bullet.y = state->player_pos.y + PLAYER_SIZE * 2 / 3;

                    float dx = msg.x - bullet.x;
                    float dy = msg.y - bullet.y;
                    float dist = sqrt(dx * dx + dy * dy);

                    if (dist > 0) {
                        bullet.dx = (dx / dist) * 10;
                        bullet.dy = (dy / dist) * 10;
                        state->bullets.push_back(bullet);
                    }
                }
            }
            else if (msg.message == WM_RBUTTONDOWN && state->can_wall == true) {
                map[msg.y / 50][msg.x / 50] = 1;
                state->wall_map[msg.y / 50][msg.x / 50] = 1;
                state->wall_num++;
            }
        }

        // 玩家移动
        state->player_walk_flag = state->key_w || state->key_a || state->key_s || state->key_d;
        if (state->key_w) {
            state->player_direction = 2;
            state->player_pos.y -= state->player_speed;
            if (check_player_collision(state->player_pos, map)) {
                state->player_pos.y += state->player_speed;
            }
        }
        if (state->key_a) {
            state->player_direction = 1;
            state->player_pos.x -= state->player_speed;
            if (check_player_collision(state->player_pos, map)) {
                state->player_pos.x += state->player_speed;
            }
        }
        if (state->key_s) {
            state->player_direction = 0;
            state->player_pos.y += state->player_speed;
            if (check_player_collision(state->player_pos, map)) {
                state->player_pos.y -= state->player_speed;
            }
        }
        if (state->key_d) {
            state->player_direction = 3;
            state->player_pos.x += state->player_speed;
            if (check_player_collision(state->player_pos, map)) {
                state->player_pos.x -= state->player_speed;
            }
        }
        next_level_pos = (state->player_pos.x + PLAYER_SIZE / 2 >= SCREEN_WIDTH - 100 &&
            state->player_pos.x + PLAYER_SIZE / 2 <= SCREEN_WIDTH &&
            state->player_pos.y + PLAYER_SIZE * 5 / 6 >= SCREEN_HEIGHT / 2 - 50 &&
            state->player_pos.y + PLAYER_SIZE * 5 / 6 <= SCREEN_HEIGHT / 2 + 50);

        // 怪物移动和攻击
        for (int i = 0; i < state->demons.size(); i++) {
            Demon* demon = &state->demons[i];

            if (demon->hp > 0 && demon->mood != 3 && !demon->is_attacking) {
                // 怪物移动
                float dx = state->player_pos.x - demon->x;
                float dy = state->player_pos.y - demon->y;
                float dist = sqrt(dx * dx + dy * dy);

                if (dist > 1) {
                    demon->x += dx / dist * 2;
                    demon->y += dy / dist * 2;
                }

                // 怪物攻击检测
                if (check_player_demon_collision(state->player_pos, demon) && state->player_hp > 0) {
                    demon->is_attacking = true;
                    demon->attack_frame = 0;
                    state->player_hp--;
                    if (state->player_hp < 0) state->player_hp = 0;
                }
            }

            // 更新怪物攻击动画
            if (demon->is_attacking) {
                demon->attack_frame++;
                if (demon->attack_frame >= 12 * 5) {
                    demon->is_attacking = false;
                    demon->attack_frame = 0;
                }
            }

            // 更新怪物死亡动画
            if (demon->mood == 3) {
                demon->death_frame++;
            }
        }

        // 移除播放完死亡动画的怪物
        for (int i = state->demons.size() - 1; i >= 0; i--) {
            if (state->demons[i].mood == 3 && state->demons[i].death_frame >= 13 * 5) {
                state->demons.erase(state->demons.begin() + i);
            }
        }

        // 子弹移动和碰撞
        for (int i = state->bullets.size() - 1; i >= 0; i--) {
            Bullet* bullet = &state->bullets[i];
            bullet->x += bullet->dx;
            bullet->y += bullet->dy;

            // 子弹碰墙
            int x = bullet->x / 50;
            int y = bullet->y / 50;
            if (x >= 0 && x < 26 && y >= 0 && y < 16 && map[y][x]) {
                state->bullets.erase(state->bullets.begin() + i);
                continue;
            }

            // 子弹碰怪物
            bool hit = false;
            for (int j = 0; j < state->demons.size(); j++) {
                Demon* demon = &state->demons[j];
                if (demon->hp <= 0) continue;

                if (check_bullet_demon_collision(bullet, demon)) {
                    demon->hp--;
                    hit = true;

                    if (demon->hp <= 0) {
                        demon->mood = 3;
                        demon->death_frame = 0;
                    }
                    break;
                }
            }

            if (hit) {
                state->bullets.erase(state->bullets.begin() + i);
            }
        }

        // 更新动画帧
        state->counter++;
        if (state->counter % 5 == 0) {
            state->idle_anime_frame = (state->idle_anime_frame + 1) % 2;
            state->demon_floating_frame = (state->demon_floating_frame + 1) % 10;
        }
        if (state->counter % 2 == 0) {
            state->walk_anime_frame = (state->walk_anime_frame + 1) % 9;
        }
        if (state->counter > 200) state->counter = 0;

        // 绘制游戏界面
        cleardevice();
        draw_game_background(state->player_pos, 1, map, res);
        draw_player_hp_bar(state->player_hp);
        draw_time_cost(state);
        draw_player_animation(state, res);
        draw_game_background(state->player_pos, 0, map, res);
        draw_demon_animation(state, res);
        draw_bullets(state);
        setcolor(WHITE);//暂停键
        setfillcolor(BLACK);
        fillrectangle(SCREEN_WIDTH - 60, 10, SCREEN_WIDTH - 10, 60);
        settextstyle(40, 0, _T("Consolas"));
        outtextxy(SCREEN_WIDTH - 53, 15, _T("||"));
        if (state->can_wall) {
            settextstyle(50, 0, _T("Consolas"));
            if (next_level_pos) {
                outtextxy(350, SCREEN_HEIGHT - 50, _T("Press F to go next level"));
            }
            else {
                outtextxy(230, SCREEN_HEIGHT - 50, _T("You can press RButton to build a wall"));
            }
        }

        // 游戏结束检测（玩家血量为0）
        if (state->player_hp <= 0) {
            handle_game_over(state);
            if (state->quit_hall) {
                EndBatchDraw();
                return;
            }
        }

        // 所有怪物死亡
        if (state->demons.empty()) {
            if (!state->can_wall) {
                cleardevice();
                putimage(0, 0, &res->bg_player_talk);
                settextstyle(50, 0, _T("Consolas"));
                setcolor(BLACK);
                outtextxy(50, SCREEN_HEIGHT - 220, _T("I feel like I've mastered something new"));
                FlushBatchDraw();
                Sleep(3000);
                state->can_wall = true;
            }
        }

        // 帧率控制
        FlushBatchDraw();

        control_fps(start_time);
        end_time_cost = GetTickCount();
        delta_time_cost += end_time_cost - start_time_cost;
        state->time_cost = delta_time_cost / 1000;
    }
}

void handle_new_or_old(GameState* state, GameResources* res) {
    ExMessage msg;
    draw_new_or_old(state, res);
    while (1) {
        while (peekmessage(&msg, EX_MOUSE, true)) {
            if (msg.message == WM_LBUTTONDOWN) {
                if (state->have_game) {
                    //新游戏
                    if (msg.x >= SCREEN_WIDTH / 2 - 100 && msg.x <= SCREEN_WIDTH / 2 + 100
                        && msg.y >= SCREEN_HEIGHT / 2 - 70 && msg.y <= SCREEN_HEIGHT / 2) {
                        state->if_new = true;
                        handle_game_level(state, res);
                        return;
                    }
                    // 继续游戏
                    if (msg.x >= SCREEN_WIDTH / 2 - 100 && msg.x <= SCREEN_WIDTH / 2 + 100
                        && msg.y >= SCREEN_HEIGHT / 2 + 30 && msg.y <= SCREEN_HEIGHT / 2 + 100) {
                        // 尝试加载存档并恢复
                        if (state->current_player != NULL && load_singleplayer_game(state, state->current_player->name)) {
                            // 成功加载，恢复并进入游戏循环
                            resume_singleplayer_game(state, res);
                            return;
                        }
                        else {
                            // 无存档则新游戏
                            state->if_new = true;
                            handle_game_level(state, res);
                            return;
                        }
                    }
                    //旧游戏

                }
                else {
                    //新游戏
                    if (msg.x >= SCREEN_WIDTH / 2 - 100 && msg.x <= SCREEN_WIDTH / 2 + 100
                        && msg.y >= SCREEN_HEIGHT / 2 - 35 && msg.y <= SCREEN_HEIGHT / 2 + 35) {
                        state->if_new = true;
                        handle_game_level(state, res);
                        return;
                    }
                }
            }
        }
    }
}

void handle_stop(GameState* state) {
    ExMessage msg;
    // 增加“保存并退出”按钮支持
    draw_stop(state);
    // 在暂停界面增加第三个按钮（保存并退出）
    setfillcolor(RGB(100, 100, 100));
    fillrectangle(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 110, SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 2 + 170);
    settextcolor(WHITE);
    settextstyle(24, 0, _T("Consolas"));
    outtextxy(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 + 120, _T("Save && Quit"));
    FlushBatchDraw();

    while (1) {
        while (peekmessage(&msg, EX_MOUSE, true)) {
            if (msg.message == WM_LBUTTONDOWN) {
                // Continue
                if (msg.x >= SCREEN_WIDTH / 2 - 100 && msg.x <= SCREEN_WIDTH / 2 + 100
                    && msg.y >= SCREEN_HEIGHT / 2 - 70 && msg.y <= SCREEN_HEIGHT / 2) {
                    state->quit_hall = false;
                    return;
                }
                // Quit without saving
                else if (msg.x >= SCREEN_WIDTH / 2 - 100 && msg.x <= SCREEN_WIDTH / 2 + 100
                    && msg.y >= SCREEN_HEIGHT / 2 + 30 && msg.y <= SCREEN_HEIGHT / 2 + 100) {
                    state->quit_hall = true;
                    return;
                }
                // Save and Quit
                else if (msg.x >= SCREEN_WIDTH / 2 - 100 && msg.x <= SCREEN_WIDTH / 2 + 100
                    && msg.y >= SCREEN_HEIGHT / 2 + 110 && msg.y <= SCREEN_HEIGHT / 2 + 170) {
                    // 保存当前游戏到文件（按当前登录玩家）
                    if (state->current_player != NULL) {
                        save_singleplayer_game(state);
                    }
                    state->quit_hall = true;
                    return;
                }
            }
        }
    }
}

// 将单人游戏状态保存到以玩家名为名的文件中（文本格式）
void save_singleplayer_game(GameState* state) {
    if (state == NULL || state->current_player == NULL) return;
    char filename[128];
    sprintf(filename, "save_%s.dat", state->current_player->name);
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) return;

    // 保存基础信息：level(1/2)
    // 使用 state->bg_mode 来确定当前关卡，避免在 ufo 被清空时误判为关卡1
    int level = (state->bg_mode == 2) ? 2 : 1;
    fprintf(fp, "%d\n", level);
    fprintf(fp, "%d %d\n", state->player_pos.x, state->player_pos.y);
    fprintf(fp, "%d %d %d %d %d\n", state->player_hp, state->player_direction, state->can_wall ? 1 : 0, state->time_cost, state->wall_num);

    // 保存demons
    fprintf(fp, "%d\n", (int)state->demons.size());
    for (size_t i = 0; i < state->demons.size(); ++i) {
        Demon* d = &state->demons[i];
        fprintf(fp, "%d %d %d %d %d %d %d\n", d->x, d->y, d->hp, d->mood, d->attack_frame, d->death_frame, d->is_attacking ? 1 : 0);
    }

    // 保存ufo
    fprintf(fp, "%d\n", (int)state->ufo.size());
    for (size_t i = 0; i < state->ufo.size(); ++i) {
        Ufo* u = &state->ufo[i];
        fprintf(fp, "%d %d %d %d %d %d\n", u->x, u->y, u->hp, u->alive ? 1 : 0, u->attack_frame, u->death_frame);
    }

    // 保存墙体地图（16x26）
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 26; ++j) {
            fprintf(fp, "%d ", state->wall_map[i][j]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

// 从以玩家名为名的文件读取并恢复单人游戏状态
// helper: parse saved data (common for both levels)
static bool parse_saved_state_from_file(GameState* state, FILE* fp) {
    if (state == NULL || fp == NULL) return false;

    fscanf(fp, "%d %d", &state->player_pos.x, &state->player_pos.y);
    int can_wall_int = 0;
    fscanf(fp, "%d %d %d %d %d", &state->player_hp, &state->player_direction, &can_wall_int, &state->time_cost, &state->wall_num);
    state->can_wall = can_wall_int ? true : false;

    // load demons
    int demons_count = 0;
    fscanf(fp, "%d", &demons_count);
    state->demons.clear();
    for (int i = 0; i < demons_count; ++i) {
        Demon d = {0};
        int is_att = 0;
        fscanf(fp, "%d %d %d %d %d %d %d", &d.x, &d.y, &d.hp, &d.mood, &d.attack_frame, &d.death_frame, &is_att);
        d.is_attacking = is_att ? true : false;
        state->demons.push_back(d);
    }

    // load ufo
    int ufo_count = 0;
    fscanf(fp, "%d", &ufo_count);
    state->ufo.clear();
    for (int i = 0; i < ufo_count; ++i) {
        Ufo u = {0};
        int alive_int = 1;
        fscanf(fp, "%d %d %d %d %d %d", &u.x, &u.y, &u.hp, &alive_int, &u.attack_frame, &u.death_frame);
        // Normalize alive flag based on hp to avoid inconsistent saved state
        // If hp>0 we consider the ufo alive; otherwise dead.
        u.alive = (u.hp > 0) ? true : false;
        if (u.alive) {
            // reset death_frame if still alive
            u.death_frame = 0;
            // clamp attack_frame to reasonable range
            if (u.attack_frame < 0) u.attack_frame = 0;
        }
        state->ufo.push_back(u);
    }

    // load wall_map (16x26)
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 26; ++j) {
            int v = 0;
            fscanf(fp, "%d", &v);
            state->wall_map[i][j] = v;
        }
    }

    // 清理可能遗留的子弹，防止读档时立刻触发碰撞
    state->bullets.clear();
    return true;
}

// load helpers for level-specific handling
bool load_singleplayer_game_level1(GameState* state, FILE* fp) {
    // level1 currently uses the common parser
    return parse_saved_state_from_file(state, fp);
}

bool load_singleplayer_game_level2(GameState* state, FILE* fp) {
    // level2 currently uses the same layout; keep separate for future differences
    return parse_saved_state_from_file(state, fp);
}

bool load_singleplayer_game(GameState* state, const char* player_name) {
    if (state == NULL || player_name == NULL) return false;
    char filename[128];
    sprintf(filename, "save_%s.dat", player_name);
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) return false;

    int level = 1;
    if (fscanf(fp, "%d", &level) != 1) { fclose(fp); return false; }
    state->bg_mode = level;

    bool ok = false;
    if (level == 2) {
        ok = load_singleplayer_game_level2(state, fp);
    }
    else {
        ok = load_singleplayer_game_level1(state, fp);
    }

    fclose(fp);
    return ok;
}

// 恢复并运行已保存的单人游戏
void resume_singleplayer_game(GameState* state, GameResources* res) {
    // 根据 level (state->bg_mode) 加载地图
    int map[16][26];
    if (state->bg_mode == 2) {
        FILE* fp = fopen("map2.txt", "r");
        if (fp) {
            for (int i = 0; i < 16; i++) for (int j = 0; j < 26; j++) fscanf(fp, "%d", &map[i][j]);
            fclose(fp);
        }
        else memset(map, 0, sizeof(map));
    }
    else {
        // level1 默认地图
        int tmp[16][26] = {
            {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
            {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
            {-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1},
            {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
            {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
            {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
            { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
            {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
            {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
            {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
            {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
            {-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1},
            {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
        };
        memcpy(map, tmp, sizeof(map));
    }

    // 恢复保存的墙体到本地 map（如果有）
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 26; ++j) {
            if (state->wall_map[i][j]) map[i][j] = state->wall_map[i][j];
        }
    }

    // 确保从存档恢复的 Ufo 状态一致：hp>0 则为 alive 并重置死亡动画计数
    for (int i = 0; i < state->ufo.size(); ++i) {
        Ufo &u = state->ufo[i];
        if (u.hp > 0) {
            u.alive = true;
            u.death_frame = 0;
            if (u.attack_frame < 0) u.attack_frame = 0;
        }
        else {
            u.alive = false;
            if (u.death_frame < 0) u.death_frame = 0;
            if (u.attack_frame < 0) u.attack_frame = 0;
        }
    }

    // (debug dump removed)

    // 时间跟踪基线（继续时保留已有的 time_cost）
    DWORD delta_time_cost = state->time_cost*1000;
    DWORD start_time_cost = GetTickCount();
    DWORD end_time_cost = 0;

    // 进入绘制循环，使用 state 中已有的数据（demons/ufo/bullets/player_pos等）
    bool next_level_pos = false;
    BeginBatchDraw();
    while (1) {
        DWORD start_time = GetTickCount();
        start_time_cost = GetTickCount();
        ExMessage msg;

        while (peekmessage(&msg, EX_KEY | EX_MOUSE, true)) {
            if (msg.message == WM_KEYDOWN) {
                switch (msg.vkcode) {
                case 'W': state->key_w = true;  break;
                case 'A': state->key_a = true;  break;
                case 'S': state->key_s = true;  break;
                case 'D': state->key_d = true;  break;
                case 'F':
                    // 关卡2：如果站在林克处按 F 触发通关逻辑
                    if (state->bg_mode == 2) {
                        bool link_pos_now = (state->player_pos.x + PLAYER_SIZE / 2 >= 18 * 50 &&
                            state->player_pos.x + PLAYER_SIZE / 2 <= 19 * 50 &&
                            state->player_pos.y + PLAYER_SIZE * 5 / 6 >= 8 * 50 &&
                            state->player_pos.y + PLAYER_SIZE * 5 / 6 <= 9 * 50);
                        if (link_pos_now) {
                            state->game_win = true;
                            if (state->wall_num == 0) {
                                cleardevice();
                                putimage(0, 0, &res->bg_player_talk);
                                settextstyle(50, 0, _T("Consolas"));
                                setcolor(BLACK);
                                outtextxy(50, SCREEN_HEIGHT - 220, _T("emmmmmm,wall? useless!"));
                                FlushBatchDraw();
                                Sleep(3000);
                            }
                            handle_game_over(state);
                            if (state->quit_hall) { EndBatchDraw(); return; }
                        }
                    }
                    // 如果已经获得建墙技能并站在传送处，按 F 进入下一关（关卡1 使用）
                    if (state->can_wall && next_level_pos) {
                        handle_game_level2(state, res);
                    }
                    if (state->quit_hall) { EndBatchDraw(); return; }
                    break;
                }
            }
            else if (msg.message == WM_KEYUP) {
                switch (msg.vkcode) {
                case 'W': state->key_w = false; break;
                case 'A': state->key_a = false; break;
                case 'S': state->key_s = false; break;
                case 'D': state->key_d = false; break;
                }
            }
            else if (msg.message == WM_LBUTTONDOWN) {
                if (msg.x >= SCREEN_WIDTH - 60 && msg.x <= SCREEN_WIDTH - 10 && msg.y >= 10 && msg.y <= 60) {
                    end_time_cost = GetTickCount();
                    delta_time_cost += end_time_cost - start_time_cost;
                    handle_stop(state);
                    if (state->quit_hall) { EndBatchDraw(); return; }
                    start_time_cost = GetTickCount();
                }
                else {
                    // 发射子弹（点击方向）
                    Bullet bullet;
                    bullet.x = state->player_pos.x + PLAYER_SIZE / 2;
                    bullet.y = state->player_pos.y + PLAYER_SIZE * 2 / 3;
                    float dx = msg.x - bullet.x;
                    float dy = msg.y - bullet.y;
                    float dist = sqrt(dx * dx + dy * dy);
                    if (dist > 0) {
                        bullet.dx = (dx / dist) * 10;
                        bullet.dy = (dy / dist) * 10;
                        state->bullets.push_back(bullet);
                    }
                }
            }
            else if (msg.message == WM_RBUTTONDOWN && state->can_wall == true) {
                map[msg.y / 50][msg.x / 50] = 1;
                state->wall_map[msg.y / 50][msg.x / 50] = 1;
                state->wall_num++;
            }
        }

        // 玩家移动
        state->player_walk_flag = state->key_w || state->key_a || state->key_s || state->key_d;
        if (state->key_w) {
            state->player_direction = 2;
            state->player_pos.y -= state->player_speed;
            if (check_player_collision(state->player_pos, map)) state->player_pos.y += state->player_speed;
        }
        if (state->key_a) {
            state->player_direction = 1;
            state->player_pos.x -= state->player_speed;
            if (check_player_collision(state->player_pos, map)) state->player_pos.x += state->player_speed;
        }
        if (state->key_s) {
            state->player_direction = 0;
            state->player_pos.y += state->player_speed;
            if (check_player_collision(state->player_pos, map)) state->player_pos.y -= state->player_speed;
        }
        if (state->key_d) {
            state->player_direction = 3;
            state->player_pos.x += state->player_speed;
            if (check_player_collision(state->player_pos, map)) state->player_pos.x -= state->player_speed;
        }

        // 计算是否在传送处（进入下一关的触发点）
        next_level_pos = (state->player_pos.x + PLAYER_SIZE / 2 >= SCREEN_WIDTH - 100 &&
            state->player_pos.x + PLAYER_SIZE / 2 <= SCREEN_WIDTH &&
            state->player_pos.y + PLAYER_SIZE * 5 / 6 >= SCREEN_HEIGHT / 2 - 50 &&
            state->player_pos.y + PLAYER_SIZE * 5 / 6 <= SCREEN_HEIGHT / 2 + 50);
        // 每帧规范化 Ufo 状态，保证 hp 为真值时不会播放死亡动画或被错误移除
        for (int _k = 0; _k < state->ufo.size(); ++_k) {
            Ufo* _u = &state->ufo[_k];
            if (_u->hp > 0) {
                _u->alive = true;
                _u->death_frame = 0;
                if (_u->attack_frame < 0) _u->attack_frame = 0;
            }
            else {
                _u->alive = false;
                if (_u->death_frame < 0) _u->death_frame = 0;
                if (_u->attack_frame < 0) _u->attack_frame = 0;
            }
        }


        // ---- 先让 demon 移动并攻击，再处理子弹碰撞（确保读档后怪物行为正常） ----
        for (int i = 0; i < state->demons.size(); i++) {
            Demon* demon = &state->demons[i];

            // 如果怪物生命已为0，确保处于死亡状态
            if (demon->hp <= 0) {
                demon->mood = 3;
            }

            if (demon->hp > 0 && demon->mood != 3 && !demon->is_attacking) {
                float dx = state->player_pos.x - demon->x;
                float dy = state->player_pos.y - demon->y;
                float dist = sqrt(dx * dx + dy * dy);
                if (dist > 1) {
                    demon->x += (int)(dx / dist * 2);
                    demon->y += (int)(dy / dist * 2);
                }

                if (check_player_demon_collision(state->player_pos, demon) && state->player_hp > 0) {
                    demon->is_attacking = true;
                    demon->attack_frame = 0;
                    state->player_hp--;
                    if (state->player_hp < 0) state->player_hp = 0;
                }
            }

            if (demon->is_attacking) {
                demon->attack_frame++;
                if (demon->attack_frame >= 12 * 5) {
                    demon->is_attacking = false;
                    demon->attack_frame = 0;
                }
            }

            if (demon->mood == 3) {
                demon->death_frame++;
            }
        }

        // 移除播放完死亡动画的 demon
        for (int i = state->demons.size() - 1; i >= 0; i--) {
            if (state->demons[i].mood == 3 && state->demons[i].death_frame >= 13 * 5) {
                state->demons.erase(state->demons.begin() + i);
            }
        }

        for (int i = 0; i < state->ufo.size(); i++) {
            Ufo* ufo = &state->ufo[i];

        // Use hp as source of truth for firing behavior; saved `alive` flag
        // may be inconsistent. If hp>0 the ufo should act alive.
        if (ufo->hp > 0) {
                // 发射子弹
                Bullet bullet;
                bullet.friendly = false;
                bullet.x = ufo->x + PLAYER_SIZE / 2;
                bullet.y = ufo->y + PLAYER_SIZE / 2;

                float dx = 0, dy;
                if (ufo->y > SCREEN_HEIGHT / 2)dy = -10;
                else dy = 10;
                bullet.dx = dx;
                bullet.dy = dy;
                state->bullets.push_back(bullet);
            }
            // 更新怪物死亡动画（以 hp 为准）
            if (ufo->hp <= 0) {
                ufo->death_frame++;
            }
        }

        // 移除播放完死亡动画的怪物
        for (int i = state->ufo.size() - 1; i >= 0; i--) {
            if (state->ufo[i].hp <= 0 && state->ufo[i].death_frame >= 4 * 5) {
                state->ufo.erase(state->ufo.begin() + i);
            }
        }

        // 处理子弹移动与碰撞
        for (int i = state->bullets.size() - 1; i >= 0; i--) {
            Bullet* bullet = &state->bullets[i];
            bullet->x += bullet->dx;
            bullet->y += bullet->dy;

            // 子弹碰墙
            int x = bullet->x / 50;
            int y = bullet->y / 50;
            if (x >= 0 && x < 26 && y >= 0 && y < 16 && map[y][x]) {
                state->bullets.erase(state->bullets.begin() + i);
                continue;
            }

            if (bullet->friendly) {
                // 子弹碰 demon
                bool hit = false;
                for (int j = 0; j < state->demons.size(); j++) {
                    Demon* demon = &state->demons[j];
                    if (demon->hp <= 0) continue;
                    if (check_bullet_demon_collision(bullet, demon)) {
                        demon->hp--;
                        hit = true;
                        if (demon->hp <= 0) {
                            demon->mood = 3;
                            demon->death_frame = 0;
                        }
                        break;
                    }
                }
                if (hit) { state->bullets.erase(state->bullets.begin() + i); continue; }

                // 子弹碰 ufo
                for (int j = 0; j < state->ufo.size(); j++) {
                    Ufo* u = &state->ufo[j];
                    if (u->hp <= 0) continue;
                    if (check_bullet_ufo_collision(bullet, u)) {
                        u->hp--;
                        if (u->hp <= 0) { u->alive = false; u->death_frame = 0; }
                        state->bullets.erase(state->bullets.begin() + i);
                        break;
                    }
                }
            }
        }

        // 更新动画帧
        state->counter++;
        if (state->counter % 5 == 0) {
            state->idle_anime_frame = (state->idle_anime_frame + 1) % 2;
            state->demon_floating_frame = (state->demon_floating_frame + 1) % 10;
            state->ufo_idle_frame = (state->ufo_idle_frame + 1) % 4;
            state->link_frame = (state->link_frame + 1) % 6;
        }
        if (state->counter % 2 == 0) state->walk_anime_frame = (state->walk_anime_frame + 1) % 9;
        if (state->counter > 200) state->counter = 0;

        // 绘制
        cleardevice();
        draw_game_background(state->player_pos, 1, map, res);
        draw_player_hp_bar(state->player_hp);
        draw_time_cost(state);
        draw_player_animation(state, res);
        draw_game_background(state->player_pos, 0, map, res);
        draw_demon_animation(state, res);
        draw_ufo_animation(state, res);
        // Level2: draw Link NPC and show prompt when nearby
        if (state->bg_mode == 2) {
            // draw link sprite at same position as in fresh level2
            putimage_alpha(19 * 50, 7 * 50, &res->link[state->link_frame]);
        }
        draw_bullets(state);

        // 暂停键
        setcolor(WHITE); setfillcolor(BLACK);
        fillrectangle(SCREEN_WIDTH - 60, 10, SCREEN_WIDTH - 10, 60);
        settextstyle(40, 0, _T("Consolas")); outtextxy(SCREEN_WIDTH - 53, 15, _T("||"));

        if (state->can_wall && state->bg_mode==1) {
            settextstyle(50, 0, _T("Consolas"));
            if (next_level_pos) {
                outtextxy(350, SCREEN_HEIGHT - 50, _T("Press F to go next level"));
            }
            else {
                outtextxy(230, SCREEN_HEIGHT - 50, _T("You can press RButton to build a wall"));
            }
        }

        // Level2: show prompt to save Link when player is at link position
        if (state->bg_mode == 2) {
            bool link_pos = (state->player_pos.x + PLAYER_SIZE / 2 >= 18 * 50 &&
                state->player_pos.x + PLAYER_SIZE / 2 <= 19 * 50 &&
                state->player_pos.y + PLAYER_SIZE * 5 / 6 >= 8 * 50 &&
                state->player_pos.y + PLAYER_SIZE * 5 / 6 <= 9 * 50);
            if (link_pos) {
                setcolor(WHITE);
                settextstyle(50, 0, _T("Consolas"));
                outtextxy(500, SCREEN_HEIGHT - 50, _T("Press F to save Link"));
            }
        }

        // 游戏死亡判定
        if (state->player_hp <= 0) {
            handle_game_over(state);
            if (state->quit_hall) { EndBatchDraw(); return; }
        }

        // (safety window removed)

        // 所有怪物死亡
        if (state->demons.empty()) {
            if (!state->can_wall) {
                cleardevice();
                putimage(0, 0, &res->bg_player_talk);
                settextstyle(50, 0, _T("Consolas"));
                setcolor(BLACK);
                outtextxy(50, SCREEN_HEIGHT - 220, _T("I feel like I've mastered something new"));
                FlushBatchDraw();
                Sleep(3000);
                state->can_wall = true;
            }
        }

        FlushBatchDraw();
        control_fps(start_time);
        end_time_cost = GetTickCount();
        delta_time_cost += end_time_cost - start_time_cost;
        state->time_cost = delta_time_cost / 1000;
    }
    EndBatchDraw();
}

void handle_game_over(GameState* state) {
    ExMessage msg;
    draw_game_over(state);
    FlushBatchDraw();
    while (1) {
        while (peekmessage(&msg, EX_MOUSE, true)) {
            if (msg.message == WM_LBUTTONDOWN) {
                if (msg.x >= SCREEN_WIDTH / 2 - 100 && msg.x <= SCREEN_WIDTH / 2 + 100
                    && msg.y >= SCREEN_HEIGHT / 2 + 30 && msg.y <= SCREEN_HEIGHT / 2 + 100) {
                    state->quit_hall = true;
                    return;
                }
            }
        }
    }
}

void handle_game_level2(GameState* state, GameResources* res) {
    // 初始化地图
    // 标记当前关卡为关卡2（用于保存/读档时区分）
    state->bg_mode = 2;
    int map[16][26];
    FILE* fp = fopen("map2.txt", "r");
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 26; j++) {
            fscanf(fp, "%d",&map[i][j]);
        }
    }
    fclose(fp);

    // 保存初始地图到 state->wall_map
    for (int i = 0; i < 16; ++i) for (int j = 0; j < 26; ++j) state->wall_map[i][j] = map[i][j];

    //map[16][26] = {
    //    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    //    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    //    {-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1},
    //    {-1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
    //    {-1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
    //    {-1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
    //    { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
    //    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 1,-1},
    //    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 1,-1},
    //    { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
    //    {-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
    //    {-1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
    //    {-1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
    //    {-1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,-1},
    //    {-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1},
    //    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
    //};

    // 初始化游戏状态
    state->quit_hall = false;
    state->have_game = true;
    state->player_pos = { 0, SCREEN_HEIGHT / 2 - 75 };
    state->player_hp = MAX_PLAYER_HP;
    state->player_speed = 5;
    state->player_direction = 3;
    state->bullets.clear();
    state->ufo.clear();
    state->game_win = false;
    bool link_pos = false;
    DWORD delta_time_cost = 0;
    DWORD start_time_cost = 0;
    DWORD end_time_cost = 0;
    DWORD basic_time = state->time_cost;

    // 初始化怪物
    Ufo ufo1 = { 6 * 50, 3 * 50, DEMON_MAX_HP, true, 0, 0 };
    Ufo ufo2 = { 10 * 50, 12 * 50, DEMON_MAX_HP, true, 0, 0 };
    Ufo ufo3 = { 14 * 50, 3 * 50, DEMON_MAX_HP, true, 0, 0 };
    state->ufo.push_back(ufo1);
    state->ufo.push_back(ufo2);
    state->ufo.push_back(ufo3);

    BeginBatchDraw();
    while (1) {
        DWORD start_time = GetTickCount();
        start_time_cost = GetTickCount();
        ExMessage msg;

        // 处理输入
        while (peekmessage(&msg, EX_KEY | EX_MOUSE, true)) {
            if (msg.message == WM_KEYDOWN) {
                switch (msg.vkcode) {
                case 'W': state->key_w = true;  break;
                case 'A': state->key_a = true;  break;
                case 'S': state->key_s = true;  break;
                case 'D': state->key_d = true;  break;
                case 'F':
                    if (link_pos) {
                        state->game_win = true;
                        if (state->wall_num == 0) {
                            cleardevice();
                            putimage(0, 0, &res->bg_player_talk);
                            settextstyle(50, 0, _T("Consolas"));
                            setcolor(BLACK);
                            outtextxy(50, SCREEN_HEIGHT - 220, _T("emmmmmm,wall? useless!"));
                            FlushBatchDraw();
                            Sleep(3000);
                        }
                        handle_game_over(state);
                        if (state->quit_hall) {
                            EndBatchDraw();
                            return;
                        }
                    }
                }
            }
            else if (msg.message == WM_KEYUP) {
                switch (msg.vkcode) {
                case 'W': state->key_w = false; break;
                case 'A': state->key_a = false; break;
                case 'S': state->key_s = false; break;
                case 'D': state->key_d = false; break;
                }
            }
            else if (msg.message == WM_LBUTTONDOWN) {
                if (msg.x >= SCREEN_WIDTH - 60 && msg.x <= SCREEN_WIDTH - 10 && msg.y >= 10 && msg.y <= 60) {
                    end_time_cost = GetTickCount();
                    delta_time_cost += end_time_cost - start_time_cost;
                    handle_stop(state);
                    if (state->quit_hall) {
                        EndBatchDraw();
                        return;
                    }
                    start_time_cost = GetTickCount();
                }
                else {
                    
                        Bullet bullet;
                        bullet.friendly = true;
                        bullet.x = state->player_pos.x + PLAYER_SIZE / 2;
                        bullet.y = state->player_pos.y + PLAYER_SIZE * 2 / 3;

                        float dx = msg.x - bullet.x;
                        float dy = msg.y - bullet.y;
                        float dist = sqrt(dx * dx + dy * dy);

                        if (dist > 0) {
                            bullet.dx = (dx / dist) * 10;
                            bullet.dy = (dy / dist) * 10;
                            state->bullets.push_back(bullet);
                        }
                    
                }
            }
            else if (msg.message == WM_RBUTTONDOWN && state->can_wall == true) {
                map[msg.y / 50][msg.x / 50] = 1;
                state->wall_map[msg.y / 50][msg.x / 50] = 1;
                state->wall_num++;
            }
        }

        // 玩家移动
        state->player_walk_flag = state->key_w || state->key_a || state->key_s || state->key_d;
        if (state->key_w) {
            state->player_direction = 2;
            state->player_pos.y -= state->player_speed;
            if (check_player_collision(state->player_pos, map)) {
                state->player_pos.y += state->player_speed;
            }
        }
        if (state->key_a) {
            state->player_direction = 1;
            state->player_pos.x -= state->player_speed;
            if (check_player_collision(state->player_pos, map)) {
                state->player_pos.x += state->player_speed;
            }
        }
        if (state->key_s) {
            state->player_direction = 0;
            state->player_pos.y += state->player_speed;
            if (check_player_collision(state->player_pos, map)) {
                state->player_pos.y -= state->player_speed;
            }
        }
        if (state->key_d) {
            state->player_direction = 3;
            state->player_pos.x += state->player_speed;
            if (check_player_collision(state->player_pos, map)) {
                state->player_pos.x -= state->player_speed;
            }
        }

        link_pos = (state->player_pos.x + PLAYER_SIZE / 2 >= 18 * 50 &&
            state->player_pos.x + PLAYER_SIZE / 2 <= 19 * 50 &&
            state->player_pos.y + PLAYER_SIZE * 5 / 6 >= 8 * 50 &&
            state->player_pos.y + PLAYER_SIZE * 5 / 6 <= 9 * 50);

        // 怪物攻击
        for (int i = 0; i < state->ufo.size(); i++) {
            Ufo* ufo = &state->ufo[i];
            // Ensure hp is the source of truth for activity
            if (ufo->hp > 0) {
                // 发射子弹
                Bullet bullet;
                bullet.friendly = false;
                bullet.x = ufo->x + PLAYER_SIZE / 2;
                bullet.y = ufo->y + PLAYER_SIZE / 2;

                float dx = 0, dy;
                if (ufo->y > SCREEN_HEIGHT / 2)dy = -10;
                else dy = 10;
                bullet.dx = dx;
                bullet.dy = dy;
                state->bullets.push_back(bullet);
            }
            // 更新怪物死亡动画（以 hp 为准）
            if (ufo->hp <= 0) {
                ufo->death_frame++;
            }
        }

        // 移除播放完死亡动画的怪物（以 hp 为准）
        for (int i = state->ufo.size() - 1; i >= 0; i--) {
            if (state->ufo[i].hp <= 0 && state->ufo[i].death_frame >= 4 * 5) {
                state->ufo.erase(state->ufo.begin() + i);
            }
        }

        // 子弹移动和碰撞
        for (int i = state->bullets.size() - 1; i >= 0; i--) {
            Bullet* bullet = &state->bullets[i];
            bullet->x += bullet->dx;
            bullet->y += bullet->dy;

            // 子弹碰墙
            int x = bullet->x / 50;
            int y = bullet->y / 50;
            if (x >= 0 && x < 26 && y >= 0 && y < 16 && map[y][x]) {
                state->bullets.erase(state->bullets.begin() + i);
                continue;
            }

            // 子弹碰怪物ufo
            if (bullet->friendly) {
                bool hit = false;
                for (int j = 0; j < state->ufo.size(); j++) {
                    Ufo* ufo = &state->ufo[j];
                    if (ufo->hp <= 0) continue;
                    if (check_bullet_ufo_collision(bullet, ufo)) {
                        ufo->hp--;
                        hit = true;
                        if (ufo->hp <= 0) {
                            ufo->alive = false;
                            ufo->death_frame = 0;
                        }
                        break;
                    }
                }
                if (hit) {
                    state->bullets.erase(state->bullets.begin() + i);
                }
            }
            else {
                bool hit = false;
                if (check_player_bullet_collision(state->player_pos, bullet)) {
                    state->player_hp--;
                    hit = true;
                }
                if (hit) {
                    state->bullets.erase(state->bullets.begin() + i);
                }

            }

        }

        // 更新动画帧
        state->counter++;
        if (state->counter % 5 == 0) {
            state->idle_anime_frame = (state->idle_anime_frame + 1) % 2;
            state->demon_floating_frame = (state->demon_floating_frame + 1) % 10;
            state->ufo_idle_frame = (state->ufo_idle_frame + 1) % 4;
            state->link_frame = (state->link_frame + 1) % 6;
        }
        if (state->counter % 2 == 0) {
            state->walk_anime_frame = (state->walk_anime_frame + 1) % 9;
        }
        if (state->counter > 200) state->counter = 0;

        // 绘制游戏界面
        cleardevice();
        draw_game_background(state->player_pos, 1, map, res);
        draw_player_hp_bar(state->player_hp);
        draw_time_cost(state);
        draw_player_animation(state, res);
        draw_game_background(state->player_pos, 0, map, res);
        draw_ufo_animation(state, res);
        //绘制林克
        putimage_alpha(19 * 50, 7 * 50, &res->link[state->link_frame]);
        draw_bullets(state);
        //绘制暂停键
        setcolor(WHITE);
        setfillcolor(BLACK);
        fillrectangle(SCREEN_WIDTH - 60, 10, SCREEN_WIDTH - 10, 60);
        settextstyle(40, 0, _T("Consolas"));
        outtextxy(SCREEN_WIDTH - 53, 15, _T("||"));
        if (link_pos) {
            setcolor(WHITE);
            settextstyle(50, 0, _T("Consolas"));
            outtextxy(500, SCREEN_HEIGHT - 50, _T("Press F to save Link"));
        }

        // 游戏结束检测（玩家血量为0）
        if (state->player_hp <= 0) {
            handle_game_over(state);
            if (state->quit_hall) {
                EndBatchDraw();
                return;
            }
        }

        // 帧率控制
        FlushBatchDraw();
        control_fps(start_time);
        end_time_cost = GetTickCount();
        delta_time_cost += end_time_cost - start_time_cost;
        state->time_cost = delta_time_cost / 1000 + basic_time;
    }
}

void handle_ranking(GameState* state) {
    draw_ranking(state);
    FlushBatchDraw();

    ExMessage msg;
    while (true) {
        while (peekmessage(&msg, EX_MOUSE)) {
            if (msg.message == WM_LBUTTONDOWN) {
                // 退出按钮区域
                int btnX = (SCREEN_WIDTH - 200) / 2;
                int btnY = SCREEN_HEIGHT / 2 + 200;
                if (msg.x >= btnX && msg.x <= btnX + 200 &&
                    msg.y >= btnY && msg.y <= btnY + 60) {
                    return; // 返回大厅
                }
            }
        }
        FlushBatchDraw();
        Sleep(10);
    }
}

void handle_pvp(GameState* state, GameResources* res) {
    // 初始化PVP地图（从文件读取，与原有逻辑一致）
    int map[16][26];
    FILE* fp = fopen("pvp_map.txt", "r");
    if (fp == NULL) {
        // 地图文件读取失败时，使用默认边界地图
        memset(map, 0, sizeof(map));
        for (int i = 2; i < 15; i++) {
            for (int j = 1; j < 25; j++) {
                if (i == 2 || i == 14 || j == 1 || j == 24) map[i][j] = 1;
            }
        }
    }
    else {
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 26; j++) {
                fscanf(fp, "%d", &map[i][j]);
            }
        }
        fclose(fp);
    }

    // 初始化两名玩家状态（玩家一：WASD移动，F发射右向子弹；玩家二：方向键移动，L发射左向子弹）
    GameState player1 = { 0 }, player2 = { 0 };

    // 玩家一初始化（左侧出生，绿色血条）
    player1.player_pos = { 100, SCREEN_HEIGHT / 2 - 75 };
    player1.player_hp = MAX_PLAYER_HP;
    player1.player_speed = 5;
    player1.player_direction = 3; // 初始朝右
    player1.bullets.clear();
    player1.player_walk_flag = false;
    player1.key_w = player1.key_a = player1.key_s = player1.key_d = false;

    // 玩家二初始化（右侧出生，蓝色血条）
    player2.player_pos = { SCREEN_WIDTH - 200, SCREEN_HEIGHT / 2 - 75 };
    player2.player_hp = MAX_PLAYER_HP;
    player2.player_speed = 5;
    player2.player_direction = 1; // 初始朝左
    player2.bullets.clear();
    player2.player_walk_flag = false;
    player2.key_w = player2.key_a = player2.key_s = player2.key_d = false;

    // 对战状态标记
    bool pvp_over = false;
    state->quit_hall = false;

    BeginBatchDraw();
    while (1) {
        DWORD start_time = GetTickCount();
        ExMessage msg;

        // 处理双人输入（区分玩家一和玩家二按键）
        while (peekmessage(&msg, EX_KEY | EX_MOUSE, true)) {
            if (msg.message == WM_KEYDOWN) {
                // 玩家一操作：WASD移动，F发射右向子弹
                switch (msg.vkcode) {
                case 'W': player1.key_w = true;  break;
                case 'A': player1.key_a = true;  break;
                case 'S': player1.key_s = true;  break;
                case 'D': player1.key_d = true;  break;
                case 'F': {
                    // 玩家一发射子弹：水平向右，速度10
                    Bullet bullet;
                    bullet.friendly = true; // 标记为玩家一子弹
                    bullet.x = player1.player_pos.x + PLAYER_SIZE / 2;
                    bullet.y = player1.player_pos.y + PLAYER_SIZE * 2 / 3;
                    bullet.dx = 10; // 水平向右
                    bullet.dy = 0;
                    player1.bullets.push_back(bullet);
                    break;
                }
                        // 玩家二操作：方向键移动，L发射左向子弹
                case VK_UP: player2.key_w = true;  break;
                case VK_LEFT: player2.key_a = true;  break;
                case VK_DOWN: player2.key_s = true;  break;
                case VK_RIGHT: player2.key_d = true;  break;
                case 'L': {
                    // 玩家二发射子弹：水平向左，速度10
                    Bullet bullet;
                    bullet.friendly = false; // 标记为玩家二子弹（与玩家一区分）
                    bullet.x = player2.player_pos.x + PLAYER_SIZE / 2;
                    bullet.y = player2.player_pos.y + PLAYER_SIZE * 2 / 3;
                    bullet.dx = -10; // 水平向左
                    bullet.dy = 0;
                    player2.bullets.push_back(bullet);
                    break;
                }
                }
            }
            else if (msg.message == WM_KEYUP) {
                // 玩家一按键松开
                switch (msg.vkcode) {
                case 'W': player1.key_w = false; break;
                case 'A': player1.key_a = false; break;
                case 'S': player1.key_s = false; break;
                case 'D': player1.key_d = false; break;
                }
                // 玩家二按键松开
                switch (msg.vkcode) {
                case VK_UP: player2.key_w = false;  break;
                case VK_LEFT: player2.key_a = false;  break;
                case VK_DOWN: player2.key_s = false;  break;
                case VK_RIGHT: player2.key_d = false;  break;
                }
            }
            // 暂停按钮（右上角，与原有关卡一致）
            else if (msg.message == WM_LBUTTONDOWN) {
                if (msg.x >= SCREEN_WIDTH - 60 && msg.x <= SCREEN_WIDTH - 10 && msg.y >= 10 && msg.y <= 60) {
                    handle_stop(state);
                    if (state->quit_hall) {
                        EndBatchDraw();
                        return;
                    }
                }
            }
        }

        // 玩家一移动逻辑（与原有玩家移动一致，增加碰撞检测）
        player1.player_walk_flag = player1.key_w || player1.key_a || player1.key_s || player1.key_d;
        if (player1.key_w) {
            player1.player_direction = 2;
            player1.player_pos.y -= player1.player_speed;
            if (check_player_collision(player1.player_pos, map)) {
                player1.player_pos.y += player1.player_speed;
            }
        }
        if (player1.key_a) {
            player1.player_direction = 1;
            player1.player_pos.x -= player1.player_speed;
            if (check_player_collision(player1.player_pos, map)) {
                player1.player_pos.x += player1.player_speed;
            }
        }
        if (player1.key_s) {
            player1.player_direction = 0;
            player1.player_pos.y += player1.player_speed;
            if (check_player_collision(player1.player_pos, map)) {
                player1.player_pos.y -= player1.player_speed;
            }
        }
        if (player1.key_d) {
            player1.player_direction = 3;
            player1.player_pos.x += player1.player_speed;
            if (check_player_collision(player1.player_pos, map)) {
                player1.player_pos.x -= player1.player_speed;
            }
        }

        // 玩家二移动逻辑（与玩家一一致，方向键控制）
        player2.player_walk_flag = player2.key_w || player2.key_a || player2.key_s || player2.key_d;
        if (player2.key_w) {
            player2.player_direction = 2;
            player2.player_pos.y -= player2.player_speed;
            if (check_player_collision(player2.player_pos, map)) {
                player2.player_pos.y += player2.player_speed;
            }
        }
        if (player2.key_a) {
            player2.player_direction = 1;
            player2.player_pos.x -= player2.player_speed;
            if (check_player_collision(player2.player_pos, map)) {
                player2.player_pos.x += player2.player_speed;
            }
        }
        if (player2.key_s) {
            player2.player_direction = 0;
            player2.player_pos.y += player2.player_speed;
            if (check_player_collision(player2.player_pos, map)) {
                player2.player_pos.y -= player2.player_speed;
            }
        }
        if (player2.key_d) {
            player2.player_direction = 3;
            player2.player_pos.x += player2.player_speed;
            if (check_player_collision(player2.player_pos, map)) {
                player2.player_pos.x -= player2.player_speed;
            }
        }

        // 玩家一子弹移动与碰撞检测
        for (int i = player1.bullets.size() - 1; i >= 0; i--) {
            Bullet* bullet = &player1.bullets[i];
            bullet->x += bullet->dx;
            bullet->y += bullet->dy;

            // 子弹碰墙（移除）
            int x = bullet->x / 50;
            int y = bullet->y / 50;
            if (x >= 0 && x < 26 && y >= 0 && y < 16 && map[y][x]) {
                player1.bullets.erase(player1.bullets.begin() + i);
                continue;
            }

            // 子弹碰玩家二（玩家二掉血，子弹移除）
            if (check_player2_bullet_collision(player2.player_pos, bullet)) {
                player2.player_hp--;
                if (player2.player_hp < 0) player2.player_hp = 0;
                player1.bullets.erase(player1.bullets.begin() + i);
            }
        }

        // 玩家二子弹移动与碰撞检测
        for (int i = player2.bullets.size() - 1; i >= 0; i--) {
            Bullet* bullet = &player2.bullets[i];
            bullet->x += bullet->dx;
            bullet->y += bullet->dy;

            // 子弹碰墙（移除）
            int x = bullet->x / 50;
            int y = bullet->y / 50;
            if (x >= 0 && x < 26 && y >= 0 && y < 16 && map[y][x]) {
                player2.bullets.erase(player2.bullets.begin() + i);
                continue;
            }

            // 子弹碰玩家一（玩家一掉血，子弹移除）
            if (check_player1_bullet_collision(player1.player_pos, bullet)) {
                player1.player_hp--;
                if (player1.player_hp < 0) player1.player_hp = 0;
                player2.bullets.erase(player2.bullets.begin() + i);
            }
        }

        // 更新动画帧（与原有逻辑一致，同步两名玩家动画）
        state->counter++;
        if (state->counter % 5 == 0) {
            player1.idle_anime_frame = (player1.idle_anime_frame + 1) % 2;
            player2.idle_anime_frame = (player2.idle_anime_frame + 1) % 2;
        }
        if (state->counter % 2 == 0) {
            player1.walk_anime_frame = (player1.walk_anime_frame + 1) % 9;
            player2.walk_anime_frame = (player2.walk_anime_frame + 1) % 9;
        }
        if (state->counter > 200) state->counter = 0;

        // 绘制PVP对战界面（模仿handle_game_level2绘制逻辑）
        cleardevice();
        // 绘制地图背景
        draw_game_background(player1.player_pos, 1, map, res);
        // 绘制两名玩家血条
        draw_player1_hp_bar(player1.player_hp);
        draw_player2_hp_bar(player2.player_hp);
        // 绘制两名玩家动画
        draw_player_animation(&player1, res);
        draw_player_animation(&player2, res);
        draw_game_background(player1.player_pos, 0, map, res);
        // 绘制双方子弹
        draw_bullets(&player1);
        draw_bullets(&player2);
        // 绘制暂停键
        setcolor(WHITE);
        setfillcolor(BLACK);
        fillrectangle(SCREEN_WIDTH - 60, 10, SCREEN_WIDTH - 10, 60);
        settextstyle(40, 0, _T("Consolas"));
        outtextxy(SCREEN_WIDTH - 53, 15, _T("||"));

        // 判定对战结束（任意玩家生命值清零）
        if (player1.player_hp <= 0 || player2.player_hp <= 0) {
            pvp_over = true;
            draw_pvp_over(&player1, &player2);
            FlushBatchDraw();
            // 等待点击退出，返回大厅
            ExMessage exit_msg;
            while (true) {
                while (peekmessage(&exit_msg, EX_MOUSE)) {
                    if (exit_msg.message == WM_LBUTTONDOWN) {
                        // 点击退出按钮，返回大厅
                        int btnX = SCREEN_WIDTH / 2 - 100;
                        int btnY = SCREEN_HEIGHT / 2 + 30;
                        if (exit_msg.x >= btnX && exit_msg.x <= btnX + 200 &&
                            exit_msg.y >= btnY && exit_msg.y <= btnY + 70) {
                            EndBatchDraw();
                            return;
                        }
                    }
                }
                FlushBatchDraw();
                Sleep(10);
            }
        }

        // 帧率控制（与原有逻辑一致）
        FlushBatchDraw();
        control_fps(start_time);
    }
    EndBatchDraw();
}