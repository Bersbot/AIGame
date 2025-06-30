#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace sf;

RenderWindow window(VideoMode(800, 600), "AI_Game");

View camOnMe;

float baseSpeed = 0.5f;

struct Bot {
    CircleShape shape;
    Vector2f position;
    float speed;

    Bot(Vector2f pos) {
        shape.setRadius(10);
        shape.setFillColor(Color::Green);
        shape.setOrigin(10, 10);
        shape.setPosition(pos);
        position = pos;
        speed = baseSpeed + static_cast<float>(rand() % 100) / 1000.0f; // немного рандомной скорости
    }

    void move(Vector2f dir) {
        position += dir * speed;
        shape.setPosition(position);
    }
};

std::vector<Bot> bots;
std::vector<CircleShape> redBots;
std::vector<CircleShape> foods;

float length(Vector2f v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

Vector2f normalize(Vector2f v) {
    float len = length(v);
    return (len != 0) ? v / len : Vector2f(0, 0);
}

void addBot(Vector2f pos) {
    bots.emplace_back(pos);
}

void spawnFood(int count) {
    for (int i = 0; i < count; ++i) {
        CircleShape f(5);
        f.setFillColor(Color::Blue);
        f.setOrigin(5, 5);
        f.setPosition(rand() % window.getSize().x, rand() % window.getSize().y);
        foods.push_back(f);
    }
}

void resetGame() {
    bots.clear();
    redBots.clear();
    foods.clear();

    // Один бот в начале
    addBot(Vector2f(window.getSize().x / 2, window.getSize().y / 2));

    // Много еды
    spawnFood(10);
}

void updateBots() {
    for (size_t i = 0; i < bots.size(); ++i) {
        Bot& bot = bots[i];

        Vector2f dir(0, 0);

        // Найти ближайшую еду
        float minDist = 999999;
        int closestFood = -1;
        for (size_t j = 0; j < foods.size(); ++j) {
            float d = length(foods[j].getPosition() - bot.position);
            if (d < minDist) {
                minDist = d;
                closestFood = j;
            }
        }

        if (closestFood != -1) {
            dir = normalize(foods[closestFood].getPosition() - bot.position);
        }

        // Убегать от красных
        for (const auto& red : redBots) {
            Vector2f fromRed = bot.position - red.getPosition();
            float len = length(fromRed);
            if (len < 100) // зона "опасности"
                dir += normalize(fromRed) * 0.7f;
        }

        bot.move(dir);

        // Столкновение с едой
        for (size_t j = 0; j < foods.size(); ++j) {
            if (bot.shape.getGlobalBounds().intersects(foods[j].getGlobalBounds())) {
                foods.erase(foods.begin() + j);
                spawnFood(1); // сразу заменяем съеденную еду

                float chance = static_cast<float>(rand()) / RAND_MAX;
                if (chance < 0.05f) {
                    // создать красного
                    CircleShape red(15);
                    red.setFillColor(Color::Red);
                    red.setOrigin(15, 15);
                    red.setPosition(bot.position);
                    redBots.push_back(red);
                } else {
                    addBot(bot.position);
                }
                break;
            }
        }
    }
}

void updateRedBots() {
    for (auto& red : redBots) {
        if (bots.empty()) continue;

        float minDist = 1e9;
        Vector2f targetPos;

        for (const auto& bot : bots) {
            float dist = length(bot.position - red.getPosition());
            if (dist < minDist) {
                minDist = dist;
                targetPos = bot.position;
            }
        }

        if (minDist > 0) {
            Vector2f dir = normalize(targetPos - red.getPosition());
            red.move(dir * 0.12f);
        }

        for (size_t i = 0; i < bots.size(); ++i) {
            if (red.getGlobalBounds().intersects(bots[i].shape.getGlobalBounds())) {
                bots.erase(bots.begin() + i);
                break;
            }
        }
    }
}

void drawAll() {
    window.clear(Color(30, 50, 70));

    for (auto& f : foods)
        window.draw(f);

    for (auto& bot : bots)
        window.draw(bot.shape);

    for (auto& red : redBots)
        window.draw(red);

    window.display();
}

int main() {
    srand(static_cast<unsigned>(time(0)));
    resetGame();

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed || Keyboard::isKeyPressed(Keyboard::Escape))
                window.close();
        }
        if (Keyboard::isKeyPressed(Keyboard::A)){camOnMe.move(-1, 0);}
        if (Keyboard::isKeyPressed(Keyboard::D)){camOnMe.move(1, 0);}
        if (Keyboard::isKeyPressed(Keyboard::W)){camOnMe.move(0, -1);}
        if (Keyboard::isKeyPressed(Keyboard::S)){camOnMe.move(0, 1);}
        if (Keyboard::isKeyPressed(Keyboard::R)){resetGame();}
        window.setTitle("Greens: " + std::to_string(bots.size()) + " VS Reds: " + std::to_string(redBots.size()));
        window.setView(camOnMe);
        updateBots();
        updateRedBots();
        drawAll();
    }

    return 0;
}
