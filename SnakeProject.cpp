//In the first step, we will call the libraries that allow us to use the functions we will need to manage the game.
#include <iostream>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
using namespace std;

//Here we used enum to know the movement directions of the snake.
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

//In this class we started by defining the coordinates of the snake's head, the map dimensions, and a vector showing the location of the tail. We prepared the initial image of the snake where it starts from the middle. We used void move to change the location of the tail. Then we made the conditional functions responsible for wrapping the snake around the edges. Finally, void grow to add a piece to the snake each time. The logical function checks for head and tail collisions.
class Snake {
private:
    int x, y;
    int width, height;
    vector<pair<int, int>> tail;

public:
    Snake(int w, int h) : width(w), height(h) {
        reset();
    }

    void reset() {
        x = width / 2;
        y = height / 2;
        tail.clear();
    }

    void move(int dx, int dy) {
        if (!tail.empty()) {
            for (int i = tail.size() - 1; i > 0; i--)
                tail[i] = tail[i - 1];
            tail[0] = { x, y };
        }
        x += dx;
        y += dy;
        if (x >= width) x = 0;
        if (x < 0) x = width - 1;
        if (y >= height) y = 0;
        if (y < 0) y = height - 1;
    }

    void grow() {
        tail.push_back({ x, y });
    }

    int getX() { return x; }
    int getY() { return y; }
    vector<pair<int, int>> getTail() { return tail; }

    bool checkCollision() {
        for (auto& seg : tail)
            if (seg.first == x && seg.second == y)
                return true;
        return false;
    }

    bool isOnPosition(int px, int py) {
        if (x == px && y == py) return true;
        for (auto& seg : tail)
            if (seg.first == px && seg.second == py)
                return true;
        return false;
    }
};

//Here we also started defining the coordinates of the food location and the dimensions of the snake network, then we used the constructor to create the food and spawn to generate food in random locations.
class Food {
private:
    int x, y;
    int width, height;

public:
    Food(int w, int h) : width(w), height(h) {
        spawn();
    }

    void spawn() {
        x = rand() % width;
        y = rand() % height;
    }

    int getX() { return x; }
    int getY() { return y; }
};

//This class is the most important part of the game as well as the code because it contains the logic on which the game works
class Game {
private:
    int width, height;
    bool gameOver, paused;
    int score, highScore;
    Direction dir;
    Snake snake;
    Food food;
    int speed;
    vector<pair<int, int>> obstacles;

public:
    Game(int w, int h) : width(w), height(h), snake(w, h), food(w, h) {
        LoadHighScore();
        gameOver = false;
        paused = false;
        dir = STOP;
        score = 0;
        speed = 100;
        GenerateObstacles();
    }

    void GenerateObstacles() {
        obstacles.clear();
        for (int i = 0; i < 10; i++) {
            int ox = rand() % width;
            int oy = rand() % height;
            if (!snake.isOnPosition(ox, oy) && !(ox == food.getX() && oy == food.getY())) {
                obstacles.push_back({ ox, oy });
            }
        }
    }

    void Draw() {
        system("cls");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
        cout << "Score: " << score << "   High Score: " << highScore;
        if (paused) cout << "   [PAUSED]";
        cout << "\n";

        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << "\n";

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (x == 0) cout << "#";

                if (snake.getX() == x && snake.getY() == y)
                    cout << "O";
                else if (food.getX() == x && food.getY() == y)
                    cout << "*";
                else {
                    bool printed = false;

                    for (auto& seg : snake.getTail()) {
                        if (seg.first == x && seg.second == y) {
                            cout << "o";
                            printed = true;
                            break;
                        }
                    }

                    for (auto& obs : obstacles) {
                        if (obs.first == x && obs.second == y) {
                            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12); // red
                            cout << "X";
                            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10); // green back
                            printed = true;
                            break;
                        }
                    }

                    if (!printed) cout << " ";
                }

                if (x == width - 1) cout << "#";
            }
            cout << "\n";
        }

        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << "\n";

        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); // reset
    }

    void Input() {
        if (_kbhit()) {
            char ch = _getch();
            switch (ch) {
            case 'a': case 'A': if (dir != RIGHT) dir = LEFT; break;
            case 'd': case 'D': if (dir != LEFT) dir = RIGHT; break;
            case 'w': case 'W': if (dir != DOWN) dir = UP; break;
            case 's': case 'S': if (dir != UP) dir = DOWN; break;
            case 'p': case 'P':
                paused = !paused;
                break;
            case 'x': case 'X':
                gameOver = true;
                break;
            }
        }
    }

    bool isCollisionWithObstacle(int x, int y) {
        for (auto& obs : obstacles)
            if (obs.first == x && obs.second == y)
                return true;
        return false;
    }

    void Logic() {
        if (paused) return;

        int dx = 0, dy = 0;
        if (dir == LEFT) dx = -1;
        if (dir == RIGHT) dx = 1;
        if (dir == UP) dy = -1;
        if (dir == DOWN) dy = 1;

        snake.move(dx, dy);

        if (snake.checkCollision() || isCollisionWithObstacle(snake.getX(), snake.getY())) {
            gameOver = true;
            return;
        }

        if (snake.getX() == food.getX() && snake.getY() == food.getY()) {
            score += 10;
            if (score > highScore) highScore = score;
            snake.grow();
            food.spawn();
            if (speed > 30) speed -= 5;
        }
    }

    void LoadHighScore() {
        ifstream f("highscore.txt");
        highScore = 0;
        if (f.is_open()) {
            f >> highScore;
            f.close();
        }
    }

    void SaveHighScore() {
        ofstream f("highscore.txt");
        if (f.is_open()) {
            f << highScore;
            f.close();
        }
    }

    void Run() {
        while (!gameOver) {
            Draw();
            Input();
            Logic();
            Sleep(speed);
        }

        SaveHighScore();
        system("cls");
        cout << "💀 Game Over! Your score: " << score << "\n";
        cout << "🎯 High Score: " << highScore << "\n";
        cout << "Press any key to exit...";
        _getch();
    }
};

//Here is the main function that contains the run function to execute.
int main() {
    srand(time(0));
    Game game(40, 20);
    game.Run();
    return 0;
}