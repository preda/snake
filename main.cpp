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
    int width;
    Dir direction;

    Rectangle getRect() {
        float w = 0, h = 0;
        if (direction == Dir::E || direction == Dir::W) {
            w = length;
            h = width;
        } else if (direction == Dir::N || direction == Dir::S) {
            h = length;
            w = width;
        }
        return {float(x), float(y), w, h};
    }

    void draw(Color color) {
        DrawRectangleRec(getRect(), color);
    }

    void growTip(int d) {
        if (direction == Dir::N) {
            y -= d;
        } else if (direction == Dir::W) {
            x -= d;
        }
        length += d;
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
    Rectangle deltaRect;

public:
    int width;
    double speed; // pixels per second
    Color color;
    bool dead;

    Snake(int initialLength, int width, double speed, Color color)
        : width{width},
          color{color},
          speed{speed},
          dead{false},
          deltaRect{0, 0, 0, 0} {
        blocks.push_back({20, 400, initialLength, width, Dir::E});
    }

    void draw() {
        for (auto block: blocks) {
            block.draw(color);
        }
    }

    void drawDeltaRect() {
        DrawRectangleRec(deltaRect, RED);
        //cout << deltaRect.x << " " << deltaRect.y << " " << deltaRect.width << " " << deltaRect.height << endl;
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
        blocks.push_back({newX, newY, width, width, newDir});
    }

    Rectangle getDeltaRect(const Block &head, int d) {
        float newX = 0, newY = 0, w = 0, h = 0;
        if (head.direction == Dir::E || head.direction == Dir::W) {
            newX = (head.direction == Dir::E) ? head.x + head.length - d : head.x;
            newY = head.y;
            w = d;
            h = head.width;
        } else if (head.direction == Dir::N || head.direction == Dir::S) {
            newX = head.x;
            newY = (head.direction == Dir::S) ? head.y + head.length - d : head.y;
            w = head.width;
            h = d;
        }
        return {newX, newY, w, h};
    }

    bool collidedBorder(int screenWidth, int screenHeight) {
        if (deltaRect.x < 0 || deltaRect.x > screenWidth - deltaRect.width || deltaRect.y < 0 ||
            deltaRect.y > screenHeight - deltaRect.height) { return true; }
        return false;
    }

    bool collidedSnake(deque<Block> &bl) {
        for (int i = 0; i < blocks.size()-1; i++) {
            Rectangle r = bl[i].getRect();
            if (CheckCollisionRecs(deltaRect, r)) {
                cout << deltaRect.x << " " << deltaRect.y << " " << deltaRect.width << " " << deltaRect.height << endl;
                cout << r.x << " " << r.y << " " << r.width << " " << r.height << endl;
                return true;
            }
        }
        return false;
    }
};

void Snake::move(double currentTime) {
    currentTime += deltaTime;
    int d = int(round(speed * currentTime));
    double updateTime = d / speed;
    deltaTime = currentTime - updateTime;

    blocks.back().growTip(d);

    int cd = d;
    while (cd > 0) {
        Block &tail = blocks.front();
        assert(tail.length > 0);

        int dd = min(cd, tail.length);
        cd -= dd;

        if (tail.length <= dd) {
            blocks.pop_front();
        } else {
            tail.shrinkBase(dd);
        }
    }
    deltaRect = getDeltaRect(blocks.back(), d);
    if (collidedBorder(GetScreenWidth(), GetScreenHeight())) {
        dead = true;
        return;
    }
    if (collidedSnake(blocks)) {
        dead = true;
        cout << "snake collided" << endl;
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

    Snake snake{300, 20, 50, DARKGREEN};
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
        DrawFPS(50, 50);
        snake.draw();
        snake.drawDeltaRect();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
