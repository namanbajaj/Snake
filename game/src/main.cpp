#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <map>
#include <sstream>
#include <vector>
#include <algorithm>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

int cellSize = 30;
int cellCount = 25;
float sliderCellCount = (float) cellCount;

int offset = 100;
int settingsOffset = 400;

int width = cellSize * cellCount;
int height = cellSize * cellCount;

const char* game = "snake";

int gameRunning = 0;

std::string keyboardBuffer = "";
int tMode = 0;
std::map<std::string, std::string> backgrounds;
std::string currentBackground = "0";
int changeBackgroundFlag = 0;

std::string Vector2AsString(Vector2 v) {
    std::ostringstream s;
    s << "(" << v.x << ", " << v.y << ")";
    return s.str();
}

void stringToCharArray(std::string str, char* arr) {
    std::vector<char> vec(str.begin(), str.end());
    vec.push_back('\0');
    std::copy(vec.begin(), vec.end(), arr);
}

double EASY = 0.2;
double MEDIUM = 0.1;
double HARD = 0.05;
double lastUpdateTime = 0;
int eventTriggered(double interval) {
    double currentTime = GetTime();
    if(currentTime - lastUpdateTime >= interval){
        lastUpdateTime = currentTime;
        return 1;
    }

    return 0;
}

double userDifficulty = EASY;

int infiniteBoundaries = 0;
bool infBounds = false;

struct Color lightgreen = {173, 204, 96, 255};
struct Color darkgreen = {43, 51, 24, 255};

Vector2 UP = Vector2{0, -1};
Vector2 DOWN = Vector2{0, 1};
Vector2 LEFT = Vector2{-1, 0};
Vector2 RIGHT = Vector2{1, 0};

std::map<int, Vector2> translateUserInput = {
    {KEY_UP, UP},
    {KEY_DOWN, DOWN},
    {KEY_LEFT, LEFT},
    {KEY_RIGHT, RIGHT}
};

class Snake 
{
    public:
        std::deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        int addSegment = 0;

        void Draw() {
            for(int i = 0; i < body.size(); i++) {
                Vector2 coord = body[i];
                Rectangle rec = Rectangle{offset + coord.x * cellSize, offset + coord.y * cellSize, (float) cellSize, (float) cellSize};
                DrawRectangleRounded(rec, 0.5, 6, darkgreen);
            }
        }

        void Update() {
            body.push_front(Vector2Add(body[0], currentDirection));

            if(addSegment) {
                addSegment = 0;
            }
            else {
                body.pop_back();
            }
        }

        void changeDirection(int key) {
            if(key >= 262 and key <= 265){
                if(!gameRunning) {
                    gameRunning = 1;
                    return;
                }

                Vector2 desiredDirection = translateUserInput[key];
                // checks for going the opposite way
                if(
                    (Vector2Equals(currentDirection, UP) && Vector2Equals(desiredDirection, DOWN)) ||
                    (Vector2Equals(currentDirection, DOWN) && Vector2Equals(desiredDirection, UP)) ||
                    (Vector2Equals(currentDirection, LEFT) && Vector2Equals(desiredDirection, RIGHT)) ||
                    (Vector2Equals(currentDirection, RIGHT) && Vector2Equals(desiredDirection, LEFT))
                ) return;

                currentDirection = desiredDirection;
            }
            else if(key >= 65 && key <= 90) {
                // std::cout << "KEY PRESSED: " << key << std::endl;
                keyboardBuffer.push_back(key);
                transform(keyboardBuffer.begin(), keyboardBuffer.end(), keyboardBuffer.begin(), ::tolower);
                // std::cout << "KBB SIZE:" << keyboardBuffer.size() << std::endl;
                // std::cout << "KBB SIZE:" << keyboardBuffer << std::endl;

                std::vector<std::string> keys;
                for(auto const& imap: backgrounds){
                    keys.push_back(imap.first);
                }

                for(auto bgName : keys) {
                    // std::cout << bgName << "|";
                    if(keyboardBuffer.find(bgName) != std::string::npos) {
                        // std::cout << "MATCH FOUND" << std::endl;

                        currentBackground = backgrounds[bgName];
                        changeBackgroundFlag = 1;

                        keyboardBuffer.clear();
                    }
                }
            }
        }

        // used to check if random generation of food conflicts with position of snake body
        int conflictsWithBody(Vector2 loc) {
            for(int i = 0; i < body.size(); i++) {
                if(Vector2Equals(body[i], loc)) {
                    std::cout << "Conflict found, food generated at " << Vector2AsString(loc) << std::endl;
                    return true;
                }
            }
            return false;
        }

    private:
        Vector2 currentDirection = RIGHT;
};

class Food 
{
    public:
        Vector2 position;
        Texture2D texture;

        Food() {
            Image image = LoadImage("game/src/assets/food.png");
            texture = LoadTextureFromImage(image);
            UnloadImage(image);
            // position = generateRandomPos();
        }
        
        ~Food() {
            std::cout << "Food destroyed" << std::endl;
        }
        
        void Draw(){
            // DrawRectangle(position.x * cellSize, position.y * cellSize, cellSize, cellSize, darkgreen);
            DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
        }

        Vector2 generateRandomPos() {
            float x = GetRandomValue(0, width / cellSize - 1);
            float y = GetRandomValue(0, height / cellSize - 1);

            return Vector2{x, y};
        }

        int checkFoodCollision(Vector2 head) {
            return Vector2Equals(position, head);
        }
};

class Game
{
    public:
        Snake snake = Snake();
        Food food = Food();
        int score;
        Sound eat;
        Sound wall;
        Texture2D customBg;

        Game() {
            score = 0;
            InitFood();
            InitAudioDevice();
            eat = LoadSound("game/src/Sounds/eat.mp3");
            wall = LoadSound("game/src/Sounds/wall.mp3");            
        }

        ~Game() {
            CloseAudioDevice();
            UnloadSound(eat);
            UnloadSound(wall);
        }

        void InitFood() {
            Vector2 foodPosition;
            do
            {
                foodPosition = food.generateRandomPos();
            } while (snake.conflictsWithBody(foodPosition));
            food.position = foodPosition;
        }

        void Draw() {
            // Draw tiles
            for(int i = 0; i < cellCount; i++) {
                for(int j = 0; j < cellCount; j++) {
                    DrawTexture(gridTile, cellSize * i, cellSize * j, WHITE);
                }
            }
            
            if(currentBackground.compare("0")) {
                // std::cout << "DRAWING CUSTOM BACKGROUND" << std::endl;
                DrawTexture(customBg, offset, offset, WHITE);
            }
            else {
                // std::cout << "DRAWING PLAIN BACKGROUND" << std::endl;
            }

            // Draw gridlines
            for(int i = 0; i < cellCount; i++){
                DrawLine(offset + 0, offset + cellSize * i, offset + width, offset + cellSize * i, WHITE);
                DrawLine(offset + cellSize * i, offset + 0, offset + cellSize * i, offset + height, WHITE);
            }

            // Draw border
            DrawRectangleLinesEx(Rectangle{(float) offset-5, (float) offset-5, (float) cellSize*cellCount+10, (float) cellSize*cellCount+10}, 5, darkgreen);

            // Text
            DrawText("Retro Snake", offset - 5, 20, 40, darkgreen);
            DrawText(TextFormat("Score: %d", score), offset - 5, offset + width + 10, 40, darkgreen);

            // Settings
            DrawText("Settings", width + offset*2, 20, 40, darkgreen);

            // Limitless Bounds
            GuiCheckBox(Rectangle{(float) width + offset*2, (float) offset, 20.0f, 20.0f}, "Enable Limitless Bounds", &infBounds);

            // Cell Count Slider
            char* sliderCellCountString = (char *) malloc(10);
            stringToCharArray(std::to_string((int) sliderCellCount), sliderCellCountString);
            DrawText("Cell Count: ", width + offset*2, offset*1.8f, 20, darkgreen);
            GuiTextBox(Rectangle{(float) width + offset*3.2f, (float) offset*1.8f, 30.0f, 20.0f}, sliderCellCountString, 100, false);
            GuiSlider(Rectangle{(float) width + offset*2, (float) offset*2, 200.0f, 20.0f}, "10", "50", &sliderCellCount, 10, 50);
            
            snake.Draw();
            food.Draw();
        }

        void Update() {
            if(changeBackgroundFlag) {
                changeBackground();
                changeBackgroundFlag = 0;
            }

            snake.changeDirection(GetKeyPressed());

            if(infBounds != infiniteBoundaries) {
                infiniteBoundaries = infBounds;
                GameOver();
            }

            if(GuiButton(Rectangle{(float) width + offset*2, (float) offset*2.3f, 100.0f, 20.0f}, "Apply")) {
                cellCount = sliderCellCount;
                width = cellCount * cellSize;
                height = cellCount * cellSize;
                SetWindowSize(width + offset*2 + settingsOffset, height + offset*2);
                GameOver();
            }

            if(GuiButton(Rectangle{(float) width + offset*3, (float) offset*2.3f, 100.0f, 20.0f}, "Cancel")) {
                sliderCellCount = cellCount;
            }


            if(gameRunning) {
                if(eventTriggered(userDifficulty)) {
                    snake.Update();
                    
                    int foodCollision = food.checkFoodCollision(snake.body[0]);
                    if(foodCollision){
                        std::cout << "FOOD COLLISION" << std::endl;
                        score++;
                        InitFood();
                        snake.addSegment = 1;
                        PlaySound(eat);
                    }

                    // check for collision with edge
                    if(snake.body[0].x >= cellCount || snake.body[0].y >= cellCount || snake.body[0].x < 0 || snake.body[0].y < 0){
                        std::cout << "EDGE HIT" << std::endl;
                        if(!infiniteBoundaries) {
                            GameOver();
                        }
                        else {
                            if(snake.body[0].x < 0) {
                                snake.body[0].x = 24;
                            }
                            if(snake.body[0].x >= 25) {
                                snake.body[0].x = 0;
                            }
                            if(snake.body[0].y < 0) {
                                snake.body[0].y = 24;
                            }
                            if(snake.body[0].y >= 25) {
                                snake.body[0].y = 0;
                            }                        
                        }
                    }

                    // check for collision with body
                    Vector2 head = snake.body[0];
                    for(int i = 1; i < snake.body.size(); i++){
                        if(Vector2Equals(head, snake.body[i])) {
                            GameOver();
                        }
                    }

                    // Check for game over
                    if(snake.body.size() == cellCount * cellCount) {
                        std::cout << "GAME OVER!" << std::endl;
                    }
                }
            }
        }

        void GameOver() {
            snake = Snake();
            food = Food();
            InitFood();
            gameRunning = 0;

            //TODO, add max score functionality
            score = 0;

            PlaySound(wall);
        }

        void changeBackground() {
            if(currentBackground.compare("0")) {
                char* path = (char*) malloc(100); 
                stringToCharArray("game/src/assets/backgrounds/" + currentBackground, path);
                Image customImageBg = LoadImage(path);
                ImageResize(&customImageBg, cellSize * cellCount, cellSize * cellCount);
                customBg = LoadTextureFromImage(customImageBg);
                UnloadImage(customImageBg);
            }
        }

    private:
        Image gridImage = LoadImage("game/src/assets/grid.png");
        Texture gridTile = LoadTextureFromImage(gridImage);
};

int main () {
    std::cout << "Starting game" << std::endl;
    reinit:
    InitWindow(width + offset*2 + settingsOffset, height + offset*2, game);
    SetTargetFPS(60);
    Game game = Game();

    // initialize backgrounds
    backgrounds["plain"] = "0";
    backgrounds["tiny"] = "t.png";
    backgrounds["dirt"] = "d.png";

    while(!WindowShouldClose()) {
        BeginDrawing();
        
        ClearBackground(lightgreen);

        game.Draw();
        game.Update();

        // std::cout << "MouseX: " << GetMouseX() << std::endl;
        // std::cout << "MouseY: " << GetMouseY() << std::endl;

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

