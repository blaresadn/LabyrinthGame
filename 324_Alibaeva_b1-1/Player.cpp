#include "Player.h"

bool Player::Moved() const
{
  if(coords.x == old_coords.x && coords.y == old_coords.y)
    return false;
  else
    return true;
}

void Player::ProcessInput(MovementDir dir)
{
  int move_dist = move_speed * 1;
  switch(dir)
  {
    case MovementDir::UP:
      old_coords.y = coords.y;
      coords.y += move_dist;
      prev_where = where;
      where = MovementDir::UP;
      break;
    case MovementDir::DOWN:
      old_coords.y = coords.y;
      coords.y -= move_dist;
      prev_where = where;
      where = MovementDir::DOWN;
      break;
    case MovementDir::LEFT:
      old_coords.x = coords.x;
      coords.x -= move_dist;
      prev_where = where;
      where = MovementDir::LEFT;
      break;
    case MovementDir::RIGHT:
      old_coords.x = coords.x;
      coords.x += move_dist;
      prev_where = where;
      where = MovementDir::RIGHT;
      break;
    default:
        where = MovementDir::NONE;
      break;
  }
}

int Player::Draw(Image &screen, float time)
{
    char **map_scheme = lab.GetRoomScheme();
    Pixel **map = lab.GetMap();

    // если нужно поменять комнату, то строим новую комнату и выходим из функции
    if (change_state) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                screen.PutPixel(x, y, map[x][y]);
            }
        }
        where = MovementDir::NONE;
        for (int i = 0; i < cur_health; i++) {
            for (int y = height - 1; y > height - heart.Height(); y--) {
                for (int x = width - heart.Width() * (i + 1); x < width; x++) {
                    screen.PutPixel(x, y, blend(map[x][y], heart.GetPixel(x % heart.Width(),
                                                                          heart.Height() - y % heart.Height() -
                                                                          1)));
                }
            }
        }
        change_state = false;
        return 0;
    }

    // Если персонаж движется, то меняем номер спрайта по таймеру
    if (time - tile_change >= 0.1) {
        tile_change += 0.1;
        if (where != MovementDir::NONE) {
            cur_player_state++;
            cur_player_state %= player_states_num;
        }
    }
    player_width = (*player_states.at(DirToNum(where) + cur_player_state)).Width();
    player_height = (*player_states.at(DirToNum(where) + cur_player_state)).Height();

    // По таймеру меняем спрайты шипов
    if (time - last_check >= 0.5) {
        for (int i = 0; i < pixel_num_height; i++) {
            for (int j = 0; j < pixel_num_width; j++) {
                if (map_scheme[i][j] == 'T') {
                    for (int y = 0; y < pixel_height; y++) {
                        for (int x = 0; x < pixel_width; x++) {
                            int cur_x = i * pixel_width + x;
                            int cur_y = j * pixel_height + y;
                            map[cur_x][cur_y] = blend(floor.GetPixel(x, y),
                                          (*trap_states.at(cur_trap_state)).GetPixel(x,pixel_height - y - 1));
                            screen.PutPixel(cur_x, cur_y, map[cur_x][cur_y]);
                        }
                    }
                }
            }
        }
        last_check += 0.5;
        cur_trap_state++;
    }
    cur_trap_state %= trap_states_num;
    if (cur_trap_state == 1) {
        wait = false;
    }

    bool change = false; // флаг для перехода в другую комнату или объявления о победе
    int f = 0; // флаг состояний, который зависит от действий персонажа

    // Проверяем, перешел ли игрок в другую комнату, и если да, даем новые координаты
    if (where == MovementDir::UP) {
        if (change = old_coords.y >= height - player_height - 2) {
            new_coords.x = pixel_width * (pixel_num_width - 1) / 2;
            new_coords.y = 0;
        }
    } else if (where == MovementDir::DOWN) {
        if (change = old_coords.y <= 1) {
            new_coords.x = pixel_width * (pixel_num_width - 1) / 2;
            new_coords.y = pixel_height * (pixel_num_height - 1);
        }
    } else if (where == MovementDir::LEFT) {
        if (change = old_coords.x <= 1) {
            new_coords.x = pixel_width * (pixel_num_width - 1);
            new_coords.y = pixel_width * (pixel_num_height - 1) / 2;
        }
    } else if (where == MovementDir::RIGHT) {
        if (change = old_coords.x >= width - player_width - 2) {
            new_coords.x = 0;
            new_coords.y = pixel_height * (pixel_num_height - 1) / 2;
        }
    }

    // Если игрок нашел выход, то возвращаем код победы
    if (change) {
        bool win = lab.CreateNewRoom(where);
        if (win) {
            if (where == MovementDir::LEFT) {
                return 3;
            }
            return 2;
        }
        old_coords = new_coords;
        coords = new_coords;
    } else {
        // Если игрок не выиграл и не перешел в другую комнату, то проверяем его на взаимодействие с объектами
        if (where == MovementDir::UP) {
            char left_up = map_scheme[(coords.x) / pixel_width][(coords.y + player_height - 1) / pixel_height];
            char right_up = map_scheme[(coords.x + player_width - 1) / pixel_width][(coords.y + player_height - 1) / pixel_height];
            if (left_up == '#' || right_up == '#') {
                f = 1;
            } else if (left_up == ' ' || right_up == ' ') {
                f = 2;
            } else if ((left_up == 'T' || right_up == 'T') && cur_trap_state == 0) {
                if (!wait) {
                    cur_health--;
                    hurt = true;
                    if (cur_health == 0) {
                        f = 2;
                    }
                    wait = true;
                }
            }
        } else if (where == MovementDir::DOWN) {
            char left_down = map_scheme[(coords.x) / pixel_width][(coords.y) / pixel_height];
            char right_down = map_scheme[(coords.x + player_width - 1) / pixel_width][(coords.y) / pixel_height];
            if (left_down == '#' || right_down == '#') {
                f = 1;
            } else if (left_down == ' ' || right_down == ' ') {
                f = 2;
            } else if ((left_down == 'T' || right_down == 'T') && cur_trap_state == 0) {
                if (!wait) {
                    cur_health--;
                    hurt = true;
                    if (cur_health == 0) {
                        f = 2;
                    }
                    wait = true;
                }
            }
        } else if (where == MovementDir::LEFT) {
            char left_up = map_scheme[(coords.x) / pixel_width][(coords.y + player_height - 1) / pixel_height];
            char left_down = map_scheme[(coords.x) / pixel_width][(coords.y) / pixel_height];
            if (left_up == '#' || left_down == '#') {
                f = 1;
            } else if (left_up == ' ' || left_down == ' ') {
                f = 2;
            } else if ((left_up == 'T' || left_down == 'T') && cur_trap_state == 0) {
                if (!wait) {
                    cur_health--;
                    hurt = true;
                    if (cur_health == 0) {
                        f = 2;
                    }
                    wait = true;
                }
            }
        } else if (where == MovementDir::RIGHT) {
            char right_up = map_scheme[(coords.x + player_width - 1) / pixel_width][(coords.y + player_height - 1) / pixel_height];
            char right_down = map_scheme[(coords.x + player_width - 1) / pixel_width][(coords.y) / pixel_height];
            if (right_up == '#' || right_down == '#') {
                f = 1;
            } else if (right_up == ' ' || right_down == ' ') {
                f = 2;
            } else if ((right_up == 'T' || right_down == 'T') && cur_trap_state == 0) {
                if (!wait) {
                    cur_health--;
                    hurt = true;
                    if (cur_health == 0) {
                        f = 2;
                    }
                    wait = true;
                }
            }
        } else if (cur_trap_state == 0) {
            // пораниться от шипов можно только начиная с некоторого отступа
            int hurt_board = 5;
            char left_down = map_scheme[(coords.x + hurt_board) / pixel_width][(coords.y + hurt_board) / pixel_height];
            char right_down = map_scheme[(coords.x + player_width - hurt_board - 1) / pixel_width][(coords.y + hurt_board) / pixel_height];
            char left_up = map_scheme[(coords.x + hurt_board) / pixel_width][(coords.y + player_height - hurt_board - 1) / pixel_height];
            char right_up = map_scheme[(coords.x + player_width - hurt_board - 1) / pixel_width][(coords.y + player_height - hurt_board - 1) / pixel_height];
            if (left_down == 'T' || right_down == 'T' || left_up == 'T' || right_up == 'T') {
                if (!wait) {
                    cur_health--;
                    hurt = true;
                    if (cur_health == 0) {
                        f = 2;
                    }
                    wait = true;
                }
            }
        }
    }
    // Если флаг состоний указывает на взаимодействие со стеной, то персонаж стоит на месте
    if (f == 1) {
        coords = old_coords;
    }

    // Если флаг состояний указывает на случай, когда игрок умер (кончились жизни, упал в пропасть),
    // возвращаем код проигрыша
    if (f == 2) {
        return 3;
    }

    // Если игрок поранился, вычитаем одну жизнь
    if (hurt) {
        for (int y = height; y > height - heart.Height(); y--) {
            for (int x = width - heart.Width() * (cur_health + 1); x < width - heart.Width() * cur_health; x++) {
                screen.PutPixel(x, y, map[x][y]);
            }
        }
    }
    hurt = false;

    // Если комната изменилась, возвращаем код смены комнаты
    if (change) {
        change_state = true;
        return 1;
    }

    // Возвращаем на экран пиксели комнаты после предыдущего спрайта
    if (Moved()) {
        for (int y = old_coords.y;
         y < old_coords.y + (*player_states.at(DirToNum(prev_where) + prev_state)).Height(); ++y) {
            for (int x = old_coords.x;
                 x < old_coords.x + (*player_states.at(DirToNum(prev_where) + prev_state)).Width(); ++x) {
                screen.PutPixel(x, y, map[x][y]);
            }
        }
        old_coords = coords;
    } else {
        where = MovementDir::NONE;
    }

    // Выводим на экран новый спрайт
    if (prev_where != where) {
        cur_player_state = 0;
    }
    for (int y = coords.y; y < coords.y + player_height; ++y) {
        for (int x = coords.x; x < coords.x + player_width; ++x) {
            screen.PutPixel(x, y, blend(map[x][y], (*player_states.at(DirToNum(where) + cur_player_state)).GetPixel(x - coords.x,
                                                                       player_height - y + coords.y - 1)));
        }
    }

    // если игрок не движется, не меняем спрайты
    player_states.at(12) = player_states.at(DirToNum(where) + cur_player_state);
    player_states.at(13) = player_states.at(DirToNum(where) + cur_player_state);
    player_states.at(14) = player_states.at(DirToNum(where) + cur_player_state);
    prev_state = cur_player_state;
    return 0;
}