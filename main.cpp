#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>
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


class Apple {
    float r;
    Vector2 pos;

public:
    Apple(float x, float y) : r{10}, pos{x, y} {
    }

    [[nodiscard]] bool intersects(Rectangle rect) const {
        return CheckCollisionCircleRec(pos, r, rect);
    }

    void draw() const {
        DrawCircle(pos.x, pos.y, r, RED);
        int stemLength = r / 2;
        int stemWidth = r / 5;
        DrawRectangle(pos.x - stemWidth / 2, pos.y - r - stemLength + r / 5, stemWidth, stemLength, BROWN);
        DrawCircle(pos.x + 2 * stemWidth, pos.y - r + r / 5, r / 3, DARKGREEN);
    }
};

using Apples = vector<Apple>;

class Snake {
    deque<Block> blocks;
    double deltaTime = 0;
    int initialLength = 0;

public:
    int width;
    double speed; // pixels per second
    Color color;
    bool dead;
    int score = 0;

    Snake(int initialLength, int width, double speed, Color color)
        : initialLength{initialLength},
          width{width},
          color{color},
          speed{speed},
          dead{false} {
        blocks.push_back({20, 100, initialLength, width, Dir::E});
    }

    void draw() {
        for (auto block: blocks) {
            block.draw(color);
        }
    }

    void drawScore(int x, int y, int size, Color col) {
        DrawText(("score: "s + to_string(score)).c_str(), x, y, size, col);
    }

    Rectangle move(double currentTime, Apples& apples);

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

    int revive(int highscore) {
        if (score > highscore) {
            highscore = score;
        }

        blocks.clear();
        deltaTime = 0;
        dead = false;
        score = 0;
        blocks.push_back({20, 120, initialLength, width, Dir::E});
        return highscore;
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

    bool collidedBorder(int screenWidth, int screenHeight, Rectangle deltaRect) {
        if (deltaRect.x < 0 || deltaRect.x > screenWidth - deltaRect.width || deltaRect.y < 0 ||
            deltaRect.y > screenHeight - deltaRect.height) { return true; }
        return false;
    }

    bool collidedSnake(deque<Block> &bl, Rectangle deltaRect) {
        for (int i = 0; i < blocks.size() - 1; i++) {
            Rectangle r = bl[i].getRect();
            if (CheckCollisionRecs(deltaRect, r)) {
                cout << deltaRect.x << " " << deltaRect.y << " " << deltaRect.width << " " << deltaRect.height << endl;
                cout << r.x << " " << r.y << " " << r.width << " " << r.height << endl;
                return true;
            }
        }
        return false;
    }

    bool ateApple(Apple a, Rectangle deltaRect) {
        return a.intersects(deltaRect);
    }
};

Rectangle Snake::move(double currentTime, Apples& apples) {
    currentTime += deltaTime;
    int d = int(round(speed * currentTime));
    double updateTime = d / speed;
    deltaTime = currentTime - updateTime;

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

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
    Rectangle deltaRect = getDeltaRect(blocks.back(), d);

    for (int i = 0; i < apples.size(); i++) {
        if (ateApple(apples[i], deltaRect)) {
            cout << "apple was eaten" << endl;
            ++score;

            apples[i] = apples.back();
            apples.pop_back();
            --i;

            float newX = rand() % (screenWidth - 20) + 10;
            float newY = rand() % (screenHeight - 20) + 10;
            apples.push_back({newX, newY});
        }
    }
    if (collidedBorder(screenWidth, screenHeight, deltaRect)) {
        dead = true;
        return deltaRect;
    }
    if (collidedSnake(blocks, deltaRect)) {
        dead = true;
        cout << "snake collided" << endl;
    }
    return deltaRect;
}

int main() {
    cout << "Monitor: " << GetMonitorWidth(0) << " x " << GetMonitorHeight(0) << endl;

    InitWindow(GetMonitorWidth(0) / 2, GetMonitorHeight(0) / 2, "snake");
    SetTargetFPS(30);

    int highscore = 0;

    // SetWindowState(FLAG_WINDOW_UNDECORATED);
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    MaximizeWindow();
    cout << IsWindowMaximized() << endl;
    cout << "Screen:  " << GetScreenWidth() << " x " << GetScreenHeight() << endl;
    cout << "Render:  " << GetRenderWidth() << " x " << GetRenderHeight() << endl;

    Snake snake{300, 20, 120, DARKGREEN};
    vector<Apple> apples;
    apples.push_back(Apple(GetScreenWidth() / 2, GetScreenHeight() / 2));

    while (!WindowShouldClose()) {
        double frameTime = GetFrameTime();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawFPS(10, 15);

        if (IsKeyPressed(KEY_SPACE) && snake.dead) {
            highscore = snake.revive(highscore);
        }

        Rectangle deltaRect{};
        if (!snake.dead) {
            snake.updateDir();
            deltaRect = snake.move(frameTime, apples);
        }
        snake.draw();
        //DrawRectangleRec(deltaRect, RED);
        DrawText(("highscore: "s + to_string(highscore)).c_str(), 10, 40, 25, BLUE);
        snake.drawScore(10, 70, 25, PINK);

        for (const Apple &a: apples) {
            a.draw();
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
