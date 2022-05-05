#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <random>
#include <string>

#define DEAD "\033[48;2;0;0;0m  "
#define ALIVE "\033[48;2;255;255;255m  "
#define BACKGROUND "\033[0m  "

// https://hotnews8.net/programming/tricky-code/c-code03
int kbhit() {
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt); // 端末の属性
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO); // local mode flagに関するビット演算
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF) {
      ungetc(ch, stdin);
      return 1;
  }

  return 0;
}

int count_cells_arround_cell(const std::vector<std::vector<std::string>> expand_game_display, int x, int y, std::string state) {
  std::vector<std::string> cnt = {
    expand_game_display[y-1][x-1],
    expand_game_display[y-1][x],
    expand_game_display[y-1][x+1],
    expand_game_display[y][x-1],
    expand_game_display[y][x+1],
    expand_game_display[y+1][x-1],
    expand_game_display[y+1][x],
    expand_game_display[y+1][x+1],
  };

  return std::count(cnt.begin(), cnt.end(), state);
}

void update_game(const int game_size, std::vector<std::vector<std::string>>& game_display) {
  std::vector<std::vector<std::string>> expand_game_display = game_display;
  expand_game_display.insert(expand_game_display.begin(), std::vector<std::string>(game_size, DEAD));
  expand_game_display.push_back(std::vector<std::string>(game_size, DEAD));

  for (auto& line: expand_game_display) {
    line.insert(line.begin(), DEAD);
    line.push_back(DEAD);
  }

  for (int y=0; y<game_size; y++) {
    for (int x=0; x<game_size; x++) {
      if (expand_game_display[y+1][x+1] == DEAD) {
        if (count_cells_arround_cell(expand_game_display, x+1, y+1, DEAD) == 3)
          game_display[y][x] = ALIVE;
      }
      else {
        int arrond_ALIVE_cell_cnt = count_cells_arround_cell(expand_game_display, x+1, y+1, ALIVE);
        if (arrond_ALIVE_cell_cnt < 2 || arrond_ALIVE_cell_cnt > 3) {
          // std::cout << "(" << y << ", " << x << "): " << arrond_ALIVE_cell_cnt<< " " << game_display[y][x] << BACKGROUND << std::endl;
          game_display[y][x] = DEAD;
          // std::cout << "(" << y << ", " << x << "): " << arrond_ALIVE_cell_cnt<< " " << game_display[y][x] << BACKGROUND << std::endl;
        }
      }
    }
  }
}

int main(){
  std::random_device rd;
  std::mt19937 mt(rd());
  std::bernoulli_distribution dist(0.3);

  struct winsize ws;

  if (~ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)) {
    std::cout << "(" << ws.ws_row << ", " << ws.ws_col << ")" << std::endl;
  }
  else {
    perror("ioctl");
  }

  int game_size = std::min(ws.ws_row-1, ws.ws_col-1);
  std::vector<std::vector<std::string>> display(game_size, std::vector<std::string>(game_size, DEAD));

  for (auto& line: display) {
    for (auto& chara: line) {
      if (dist(mt)) {
        chara = ALIVE;
      }
      else {
        chara = DEAD;
      }
    }
  }

  for (auto line: display) {
    for (auto chara: line) {
      std::cout << chara;
    }
    std::cout << BACKGROUND << std::endl;
  }

  while (1) {
    if (kbhit()) {
      if ('q' == getchar()) {
        break;
      }
    }

    update_game(game_size, display);
    sleep(0.1);

    std::cout << "\033[" << game_size << 'A';
    for (auto line: display) {
      for (auto chara: line) {
        std::cout << chara;
      }
      std::cout << BACKGROUND << std::endl;
    }
  }
  return 0;
}