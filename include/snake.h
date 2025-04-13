#ifndef SNAKE_H
#define SNAKE_H

#include <vector>
#include <utility>

enum Direction { UP, DOWN, LEFT, RIGHT };

class Snake {
public:
    Snake(int width, int height);
    void move();
    void grow();
    void setDirection(Direction d);
    bool checkCollision();
    bool eats(int fx, int fy);
    const std::vector<std::pair<int, int>>& getBody() const;
    std::pair<int, int> getHead() const;

private:
    std::vector<std::pair<int, int>> body;
    Direction dir;
    int width, height;
    bool growFlag;
};

#endif
