#include "snake.h"
#include "stddef.h"

Snake::Snake(int width, int height) : width(width), height(height), dir(RIGHT), growFlag(false) {
    int y = height / 2;
    int x = width / 2;
    body.emplace_back(y, x);       // head
    body.emplace_back(y, x - 1);
    body.emplace_back(y, x - 2);   // tail
}
void Snake::move() {
    auto head = body.front();
    switch (dir) {
        case UP:    head.first--; break;
        case DOWN:  head.first++; break;
        case LEFT:  head.second--; break;
        case RIGHT: head.second++; break;
    }

    if (head.first < 0) head.first = height - 1;
    if (head.first >= height) head.first = 0;
    if (head.second < 0) head.second = width - 1;
    if (head.second >= width) head.second = 0;

    body.insert(body.begin(), head); // new head
    if (!growFlag)
        body.pop_back();             
    else
        growFlag = false;            

void Snake::grow() {
    growFlag = true;
}

void Snake::setDirection(Direction d) {
    // avoid reverse direction
    if ((dir == UP && d != DOWN) || (dir == DOWN && d != UP) ||
        (dir == LEFT && d != RIGHT) || (dir == RIGHT && d != LEFT)) {
        dir = d;
    }
}

bool Snake::checkCollision() {
    auto head = body.front();
    for (size_t i = 1; i < body.size(); ++i)
        if (body[i] == head) return true;
    return false;
}


bool Snake::eats(int fx, int fy) {
    int hx = body.front().second;
    int hy = body.front().first;
    return hx == fx && hy == fy;
}


const std::vector<std::pair<int, int>>& Snake::getBody() const {
    return body;
}

std::pair<int, int> Snake::getHead() const {
    return body.front();
}
