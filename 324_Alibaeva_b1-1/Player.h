#ifndef MAIN_PLAYER_H
#define MAIN_PLAYER_H

#include "Image.h"
#include "labyrinth.h"

struct Point
{
  int x;
  int y;
} typedef Point;

struct Player
{
    explicit Player(Point pos = {.x = 0, .y = 0}): coords(pos), old_coords(coords) {
        // для удобства храним спрайты шипов в словаре
        trap_states.emplace(std::make_pair(0, &trap1));
        trap_states.emplace(std::make_pair(1, &trap2));
        trap_states.emplace(std::make_pair(2, &trap3));

        // для удобства храним спрайты персонажа в словаре
        player_states.emplace(std::make_pair(0, &up1));
        player_states.emplace(std::make_pair(1, &up2));
        player_states.emplace(std::make_pair(2, &up3));

        player_states.emplace(std::make_pair(3, &down1));
        player_states.emplace(std::make_pair(4, &down2));
        player_states.emplace(std::make_pair(5, &down3));

        player_states.emplace(std::make_pair(6, &left1));
        player_states.emplace(std::make_pair(7, &left2));
        player_states.emplace(std::make_pair(8, &left3));

        player_states.emplace(std::make_pair(9, &right1));
        player_states.emplace(std::make_pair(10, &right2));
        player_states.emplace(std::make_pair(11, &right3));

        player_states.emplace(std::make_pair(12, &right1));
        player_states.emplace(std::make_pair(13, &right2));
        player_states.emplace(std::make_pair(14, &right3));
    }

    bool Moved() const;
    void ProcessInput(MovementDir dir);
    int Draw(Image &screen, float time);
    MovementDir GetDir() const {return where;};

    // функция, позволяющая отрисовывать png-картинки
    static Pixel blend(Pixel oldPixel, Pixel newPixel)
    {
        newPixel.r = newPixel.a / 255.0 * (newPixel.r - oldPixel.r) + oldPixel.r;
        newPixel.g = newPixel.a / 255.0 * (newPixel.g - oldPixel.g) + oldPixel.g;
        newPixel.b = newPixel.a / 255.0 * (newPixel.b - oldPixel.b) + oldPixel.b;
        newPixel.a = 255;

        return newPixel;
    }

    // функция, ставящая каждому направлению в соответствие порядковое число для работы со словарем
    int DirToNum(MovementDir dir)
    {
        if (dir == MovementDir::UP) {
            return 0;
        }
        if (dir == MovementDir::DOWN) {
            return 3;
        }
        if (dir == MovementDir::LEFT) {
            return 6;
        }
        if (dir == MovementDir::RIGHT) {
            return 9;
        }
        if (dir == MovementDir::NONE) {
            return 12;
        }
    }

private:

    Point coords {.x = 10, .y = 10};
    Point old_coords {.x = 10, .y = 10};
    Pixel color {.r = 255, .g = 255, .b = 0, .a = 255};
    int move_speed = 4;

    MovementDir where = MovementDir::NONE;
    MovementDir prev_where = MovementDir::NONE;
    int prev_state = 0;

    Image floor = Image("resources/floor.jpg");

    Image trap1 = Image("resources/trap1.png");
    Image trap2 = Image("resources/trap2.png");
    Image trap3 = Image("resources/trap3.png");
    int trap_states_num = 3;
    std::map<int, Image*> trap_states;

    Image heart = Image("resources/heart.png");
    int lives = 3;
    int cur_trap_state = 0;

    Point new_coords;

    float last_check = 0.;
    float tile_change = 0.;

    int player_states_num = 3;
    std::map<int, Image*> player_states;
    int cur_player_state = 0;

    Image up1 = Image("resources/player/up1.png");
    Image up2 = Image("resources/player/up2.png");
    Image up3 = Image("resources/player/up3.png");

    Image down1 = Image("resources/player/down1.png");
    Image down2 = Image("resources/player/down2.png");
    Image down3 = Image("resources/player/down3.png");

    Image left1 = Image("resources/player/left1.png");
    Image left2 = Image("resources/player/left2.png");
    Image left3 = Image("resources/player/left3.png");

    Image right1 = Image("resources/player/right1.png");
    Image right2 = Image("resources/player/right2.png");
    Image right3 = Image("resources/player/right3.png");

    int player_width = right1.Width();
    int player_height = right2.Height();

    int cur_health = 3;
    bool hurt = false;
    bool wait = false;
    bool fade = false;
    bool change_state = false;

    Labyrinth lab;

    int pixel_width = lab.PixelWidth();
    int pixel_height = lab.PixelHeight();
    int pixel_num_width = lab.PixelNumWidth();
    int pixel_num_height = lab.PixelNumHeight();
    int width = lab.Width();
    int height = lab.Height();
};

#endif //MAIN_PLAYER_H
