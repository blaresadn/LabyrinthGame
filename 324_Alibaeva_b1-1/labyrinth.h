#ifndef MAIN_LABYRINTH_H
#define MAIN_LABYRINTH_H

// Собственный клас для считывания карт и создания карт

#include "Image.h"
#include "Player.h"

#include <iostream>
#include <fstream>
#include <map>
#include <set>

enum class MovementDir
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
};

class Labyrinth
{
public:
    Labyrinth()
    {
        main = new Pixel*[width_];
        for (int i = 0; i < width_; i++) {
            main[i] = new Pixel[height_];
        }

        for (char c = 'A'; c < 'G'; c++) {
            room_schemes.emplace(std::make_pair(c, room_maker(c)));
        }
        map_maker();

        tiles.emplace(' ', "resources/empt.jpg");
        tiles.emplace('.', "resources/floor.jpg");
        tiles.emplace('#', "resources/wall.jpg");
        tiles.emplace('x', "resources/floor.jpg");
        tiles.emplace('@', "resources/floor.jpg");
        tiles.emplace('Q', "resources/floor.jpg");
        tiles.emplace('T', "resources/floor.jpg");

        CreateNewRoom(MovementDir::RIGHT);
    }

    // метод, создающий новую комнату
    bool CreateNewRoom(MovementDir move)
    {
        // проверяем, не нашел ли игрок выход из лабиринта
        if (move == MovementDir::UP) {
            cur_room_x--;
            if (cur_room_x == -1) {
                return true;
            }
        } else if (move == MovementDir::DOWN) {
            cur_room_x++;
            if (cur_room_x == rooms_on_column) {
                return true;
            }
        } else if (move == MovementDir::LEFT) {
            cur_room_y--;
            if (cur_room_y == -1) {
                return true;
            }
        } else if (move == MovementDir::RIGHT) {
            cur_room_y++;
            if (cur_room_y == rooms_on_row) {
                return true;
            }
        }
        cur_room_scheme = room_schemes.at(map_scheme[cur_room_x][cur_room_y]);
        for (int i = 0; i < pixel_num_width; i++) {
            for (int j = 0; j < pixel_num_height; j++) {
                for (int y = 0; y < pixel_height; y++) {
                    for (int x = 0; x < pixel_width; x++) {
                        main[i * pixel_width + x][j * pixel_height + y] =
                                tiles.at(cur_room_scheme[i][j]).GetPixel(x, pixel_height - y - 1);
                    }
                }
            }
        }
        return false;
    }

    Pixel **GetMap() const
    {
        return main;
    }

    char **GetRoomScheme() const
    {
        return cur_room_scheme;
    }

    int Width() const
    {
        return width_;
    }

    int Height() const
    {
        return height_;
    }

    int PixelWidth() const
    {
        return pixel_width;
    }

    int PixelHeight() const
    {
        return pixel_height;
    }

    int PixelNumWidth() const
    {
        return pixel_num_width;
    }

    int PixelNumHeight() const
    {
        return pixel_num_height;
    }

private:
    int width_ = 32 * 32;
    int height_ = 32 * 32;
    int pixel_width = 32;
    int pixel_height = 32;
    int pixel_num_width = 32;
    int pixel_num_height = 32;

    int rooms_on_row = 5;
    int rooms_on_column = 4;

    Pixel **main;
    char **map_scheme;
    std::map<char, char**> room_schemes;
    char **cur_room_scheme;

    int cur_room_x = 0;
    int cur_room_y = -1;

    std::map<char, Image> tiles;

    // считывает схему комнаты из файла
    char **room_maker(char room_number)
    {
        std::ifstream room_file;
        room_file.open("maps/room" + std::string(1, room_number) + ".txt");
        std::string cur_line;

        char **room_scheme = new char*[pixel_num_width];
        for (int i = 0; i < pixel_num_width; i++) {
            room_scheme[i] = new char[pixel_num_height];
        }
        int line_num = 0;
        while (getline(room_file, cur_line)) {
            for (int i = 0; i < cur_line.size(); i++) {
                room_scheme[i][pixel_num_height - line_num - 1] = cur_line[i];
            }
            line_num++;
        }
        room_file.close();
        return room_scheme;
    }

    // считывает схему карту их файла
    void map_maker()
    {
        std::ifstream map_file;
        map_file.open("maps/whole_map.txt");
        std::string cur_line;

        map_scheme = new char*[rooms_on_column];
        for (int i = 0; i < rooms_on_column; i++) {
            map_scheme[i] = new char[rooms_on_row];
        }
        int line_num = 0;
        while (getline(map_file, cur_line)) {
            for (int i = 0; i < cur_line.size(); i++) {
                map_scheme[line_num][i] = cur_line[i];
            }
            line_num++;
        }
        map_file.close();
    }
};

#endif //MAIN_LABYRINTH_H
