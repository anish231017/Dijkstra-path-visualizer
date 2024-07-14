#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <cmath>
#include <sstream>

const int GRID_SIZE = 20;
const int CELL_SIZE = 30;
const int WINDOW_WIDTH = GRID_SIZE * CELL_SIZE + 300;
const int WINDOW_HEIGHT = GRID_SIZE * CELL_SIZE;

struct VectorCompare {
    bool operator()(const sf::Vector2i& lhs, const sf::Vector2i& rhs) const {
        return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y);
    }
};

enum class CellType { Empty, Wall, Start, End, Path };

struct Cell {
    CellType type = CellType::Empty;
    int distance = std::numeric_limits<int>::max();
    sf::Vector2i parent = {-1, -1};
    int weight = 1;  // Default weight is 1
};

class DijkstraVisualizer {
private:
    sf::RenderWindow window;
    std::vector<std::vector<Cell>> grid;
    sf::Vector2i startPos = {-1, -1};
    sf::Vector2i endPos = {-1, -1};
    bool isRunning = false;
    bool isSettingWeight = false;

    sf::Font font;
    sf::Text instructionsText;
    sf::Text titleText;

public:
    DijkstraVisualizer() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Dijkstra's Algorithm Visualizer") {
        grid.resize(GRID_SIZE, std::vector<Cell>(GRID_SIZE));
        setupText();
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

private:
    void setupText() {
        bool fontLoaded = font.loadFromFile("arial.ttf");
        if (!fontLoaded) {
            std::cerr << "Failed to load font from file. Attempting to load from system..." << std::endl;
            fontLoaded = font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
        }
        if (!fontLoaded) {
            std::cerr << "Failed to load any font. Text will not be displayed." << std::endl;
            return;
        }

        titleText.setFont(font);
        titleText.setCharacterSize(24);
        titleText.setFillColor(sf::Color::White);
        titleText.setPosition(GRID_SIZE * CELL_SIZE + 10, 10);
        titleText.setString("Dijkstra's Algorithm \nVisualizer");

        instructionsText.setFont(font);
        instructionsText.setCharacterSize(18);
        instructionsText.setFillColor(sf::Color::White);
        instructionsText.setPosition(GRID_SIZE * CELL_SIZE + 10, 90);
        instructionsText.setString(
            "Instructions:\n\n"
            "1. Left-click: Set start (green),\n   end (red), and walls (black)\n"
            "2. Right-click: Remove cell\n"
            "3. Middle-click: Set weight\n"
            "4. Space: Run algorithm\n"
            "5. R: Reset grid\n"
            "6. W: Toggle weight setting mode\n\n"
            "Set start and end points,\n"
            "then add walls and weights.\n"
            "Press Space to visualize\n"
            "the algorithm."
        );
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                handleMouseClick(event.mouseButton);
            } else if (event.type == sf::Event::KeyPressed) {
                handleKeyPress(event.key);
            }
        }
    }

    void handleMouseClick(const sf::Event::MouseButtonEvent& mouseButton) {
        int x = mouseButton.x / CELL_SIZE;
        int y = mouseButton.y / CELL_SIZE;

        if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
            if (mouseButton.button == sf::Mouse::Left) {
                if (isSettingWeight) {
                    if (grid[y][x].type == CellType::Empty) {
                        grid[y][x].weight = (grid[y][x].weight % 9) + 1;
                    }
                } else {
                    if (startPos.x == -1) {
                        startPos = {x, y};
                        grid[y][x].type = CellType::Start;
                    } else if (endPos.x == -1) {
                        endPos = {x, y};
                        grid[y][x].type = CellType::End;
                    } else {
                        grid[y][x].type = CellType::Wall;
                    }
                }
            } else if (mouseButton.button == sf::Mouse::Right) {
                if (grid[y][x].type == CellType::Start) {
                    startPos = {-1, -1};
                } else if (grid[y][x].type == CellType::End) {
                    endPos = {-1, -1};
                }
                grid[y][x].type = CellType::Empty;
                grid[y][x].weight = 1;
            } else if (mouseButton.button == sf::Mouse::Middle) {
                if (grid[y][x].type == CellType::Empty) {
                    grid[y][x].weight = (grid[y][x].weight % 9) + 1;
                }
            }
        }
    }

    void handleKeyPress(const sf::Event::KeyEvent& keyEvent) {
        if (keyEvent.code == sf::Keyboard::Space) {
            if (startPos.x != -1 && endPos.x != -1) {
                runDijkstra();
            }
        } else if (keyEvent.code == sf::Keyboard::R) {
            resetGrid();
        } else if (keyEvent.code == sf::Keyboard::W) {
            isSettingWeight = !isSettingWeight;
        }
    }

    void update() {
        // This method can be used for any continuous updates
    }

    void render() {
        window.clear(sf::Color::Black);

        // Draw the grid
        for (int y = 0; y < GRID_SIZE; ++y) {
            for (int x = 0; x < GRID_SIZE; ++x) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
                cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);

                switch (grid[y][x].type) {
                    case CellType::Empty:
                        cell.setFillColor(sf::Color::White);
                        break;
                    case CellType::Wall:
                        cell.setFillColor(sf::Color::Black);
                        break;
                    case CellType::Start:
                        cell.setFillColor(sf::Color::Green);
                        break;
                    case CellType::End:
                        cell.setFillColor(sf::Color::Red);
                        break;
                    case CellType::Path:
                        cell.setFillColor(sf::Color::Yellow);
                        break;
                }

                window.draw(cell);

                // Draw weight text
                if (grid[y][x].type == CellType::Empty && grid[y][x].weight > 1) {
                    sf::Text weightText;
                    weightText.setFont(font);
                    weightText.setCharacterSize(12);
                    weightText.setFillColor(sf::Color::Black);
                    weightText.setString(std::to_string(grid[y][x].weight));
                    weightText.setPosition(x * CELL_SIZE + 5, y * CELL_SIZE + 5);
                    window.draw(weightText);
                }
            }
        }

        // Draw a background for the instructions
        sf::RectangleShape instructionsBg(sf::Vector2f(300, WINDOW_HEIGHT));
        instructionsBg.setPosition(GRID_SIZE * CELL_SIZE, 0);
        instructionsBg.setFillColor(sf::Color(50, 50, 50)); // Dark gray
        window.draw(instructionsBg);

        // Draw the title and instructions only if the font was loaded
        if (font.getInfo().family != "") {
            window.draw(titleText);
            window.draw(instructionsText);
        }

        window.display();
    }

    void runDijkstra() {
        auto cmp = [](const std::pair<int, sf::Vector2i>& left, const std::pair<int, sf::Vector2i>& right) {
            return left.first > right.first || (left.first == right.first && VectorCompare()(left.second, right.second));
        };
        std::priority_queue<std::pair<int, sf::Vector2i>, 
                            std::vector<std::pair<int, sf::Vector2i>>, 
                            decltype(cmp)> pq(cmp);

        grid[startPos.y][startPos.x].distance = 0;
        pq.emplace(0, startPos);

        while (!pq.empty()) {
            auto [dist, current] = pq.top();
            pq.pop();

            if (current == endPos) {
                reconstructPath();
                return;
            }

            if (dist > grid[current.y][current.x].distance) continue;

            const int dx[] = {-1, 1, 0, 0};
            const int dy[] = {0, 0, -1, 1};

            for (int i = 0; i < 4; ++i) {
                int newX = current.x + dx[i];
                int newY = current.y + dy[i];

                if (newX >= 0 && newX < GRID_SIZE && newY >= 0 && newY < GRID_SIZE && 
                    grid[newY][newX].type != CellType::Wall) {
                    int newDist = grid[current.y][current.x].distance + grid[newY][newX].weight;

                    if (newDist < grid[newY][newX].distance) {
                        grid[newY][newX].distance = newDist;
                        grid[newY][newX].parent = current;
                        pq.emplace(newDist, sf::Vector2i(newX, newY));
                    }
                }
            }

            render();
            sf::sleep(sf::milliseconds(50));  // Slow down the visualization
        }
    }

    void reconstructPath() {
        sf::Vector2i current = endPos;
        while (current != startPos) {
            if (grid[current.y][current.x].type != CellType::End) {
                grid[current.y][current.x].type = CellType::Path;
            }
            current = grid[current.y][current.x].parent;
            render();
            sf::sleep(sf::milliseconds(50));
        }
    }

    void resetGrid() {
        for (auto& row : grid) {
            for (auto& cell : row) {
                cell = Cell();
            }
        }
        startPos = {-1, -1};
        endPos = {-1, -1};
    }
};

int main() {
    DijkstraVisualizer visualizer;
    visualizer.run();
    return 0;
}