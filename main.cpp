#include <cassert>
#include <iostream>
#include <cmath>
#include <deque>
#include "raylib.h"

using namespace std;

enum Dir { N = 1, E, S, W };

class Block {
public:
    int x;
    int y;
    int length;
    Dir direction;

    void draw(int width, Color color) {
        if (direction == Dir::E || direction == Dir::W) {
            DrawRectangle(x, y, length, width, color);
        } else if (direction == Dir::N || direction == Dir::S) {
            DrawRectangle(x, y, width, length, color);
        }
    }

    int growTip(int d) {
        if (direction == Dir::N) {
            y -= d;
        } else if (direction == Dir::W) {
            x -= d;
        }
        length += d;

        // verify if snake went outside the window
        if (x < 0 || (x > GetScreenWidth() - length && direction == Dir::E) || y < 0 || (
                y > GetScreenHeight() - length && direction == Dir::S)) {
            return 1;
        }
        return 0;
    }

    void shrinkBase(int d) {
        assert(d < length);
        if (direction == Dir::S) {
            y += d;
        } else if (direction == Dir::E) {
            x += d;
        }
        length -= d;
    }
};

class Snake {
    deque<Block> blocks;
    double deltaTime = 0;

public:
    int width;
    double speed; // pixels per second
    Color color;
    bool dead;

    Snake(int initialLength, int width, double speed, Color color)
        : width{width},
          color{color},
          speed{speed},
          dead{false} {
        blocks.push_back({20, 400, initialLength, Dir::E});
    }

    void draw() {
        for (auto block: blocks) {
            block.draw(width, color);
        }
    }

    void move(double currentTime);

    void updateDir() {
        assert(blocks.back().length >= width);

        if (IsKeyPressed(KEY_RIGHT)) {
            turn(Dir::E);
        }
        if (IsKeyPressed(KEY_UP)) {
            turn(Dir::N);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            turn(Dir::S);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            turn(Dir::W);
        }
    }

private:
    Dir opposite(Dir dir) {
        switch (dir) {
            case Dir::E: return Dir::W;
            case Dir::W: return Dir::E;
            case Dir::N: return Dir::S;
            case Dir::S: return Dir::N;
        }
        assert(false);
    }

    void turn(Dir newDir) {
        Block &head = blocks.back();
        if (head.direction == newDir || head.direction == opposite(newDir)) { return; }

        head.length -= width;
        int newX = (head.direction == Dir::E) ? head.x + head.length : head.x;
        int newY = (head.direction == Dir::S) ? head.y + head.length : head.y;
        if (head.direction == Dir::W) {
            head.x += width;
        } else if (head.direction == Dir::N) {
            head.y += width;
        }
        assert(head.length >= 0);
        if (head.length == 0) { blocks.pop_back(); }
        blocks.push_back({newX, newY, width, newDir});
    }
};

void Snake::move(double currentTime) {
    currentTime += deltaTime;
    int d = int(round(speed * currentTime));
    double updateTime = d / speed;
    deltaTime = currentTime - updateTime;



    if (blocks.back().growTip(d) == 1) {
        dead = true;
        cout << "snake died" << endl;
    }

    while (d > 0) {
        Block &tail = blocks.front();
        assert(tail.length > 0);

        int dd = min(d, tail.length);
        d -= dd;

        if (tail.length <= dd) {
            blocks.pop_front();
        } else {
            tail.shrinkBase(dd);
        }
    }
}

int main() {
    InitWindow(0, 0, "snake");
    SetTargetFPS(30);

    //double speed = 100; // pixels per second

    SetWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    //MaximizeWindow();
    cout << IsWindowMaximized() << endl;
    cout << GetScreenHeight() << " " << GetScreenWidth() << endl;
    cout << GetRenderHeight() << " " << GetRenderWidth() << endl;

    Snake snake{100, 20, 50, DARKGREEN};
    //blocks.push_back({200, 200, 100, Dir::E});
    // deque<Block> snake;
    // Dir direction{Dir::E};

    while (!WindowShouldClose()) {
        double frameTime = GetFrameTime();
        if (!snake.dead) {
            snake.updateDir();
            snake.move(frameTime);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawFPS(20, 20);
        snake.draw();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
