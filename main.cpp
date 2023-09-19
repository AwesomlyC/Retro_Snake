#include <iostream>
#include <deque>
#include <raylib.h>
#include <raymath.h>

using namespace std;

// Raylib have a built-in Color Struct
Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};

// Cell of the window
int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

bool eventTriggered(double interval){
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval){
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

// Prevent Food Object to appear in the Snake Body
bool ElementInDeque(Vector2 element, std::deque<Vector2> snakeBody){
    for (unsigned int i = 0; i < snakeBody.size(); i++){
        if (Vector2Equals(element, snakeBody[i])){
            return true;
        }
    }
    return false;
}

// Creating the Food Class
class Food{
    public:
        // Vector2 is part of the Raylib module
        // It takes in 2 values, the x and y coordinates/values
        Vector2 position;
        Texture2D texture;

        // Constructor
        Food(std::deque<Vector2> snakeBody){
            Image image = LoadImage("Graphics/food.png");
            texture = LoadTextureFromImage(image); // Utilize the GPU
            UnloadImage(image);     // Free up some memory
            position = GenerateRandomPosition(snakeBody);
        }

        // Raylib have several draw functions i.e. DrawRectangle, DrawCircle, DrawLine, and DrawPoly
        // For the Food Object, we can use the DrawRectangle method and replace it with a Food image
        void Draw(){
            // DrawRectangle(x, y, w, h, color)
            // DrawRectangle(position.x * cellSize, position.y * cellSize, cellSize, cellSize, darkGreen);

            // Draw an object with an image
            DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
        }

        // Generates a random cell for the Food Object using Raylib random function
        Vector2 GenerateRandomCell(){
            float x = GetRandomValue(0, cellCount - 1);
            float y = GetRandomValue(0, cellCount - 1);
            return Vector2{x, y};
        }
        // Generate a random position and checks if it is a legal position
        Vector2 GenerateRandomPosition(std::deque<Vector2> snakeBody){

            Vector2 position = GenerateRandomCell();
            while (ElementInDeque(position, snakeBody))
            {
            position = GenerateRandomCell();
            }
            return position;
        }
        // Destructor
        ~Food(){
            UnloadTexture(texture);
        }
};

// Creating the Snake Class
class Snake{
    public:
        std::deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        Vector2 direction = {1,0};
        bool addSegment = false;

        void Draw()
        {
            for (unsigned int i = 0; i < body.size(); i++){
                float x = body[i].x;
                float y = body[i].y;
                // DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, darkGreen);

                Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize};
                // Possibly change 0.5 to 1 for more "roundedness" and convert the squares into circles
                DrawRectangleRounded(segment, 0.5, 6, darkGreen);
            }
        }

        // Main Idea of the Update Function
        // Pop the last "body" and insert a new "body" in the front to
        // imitate movement
        void Update(){
            body.push_front(Vector2Add(body[0], direction));
            if (addSegment){
                addSegment = false;
            }
            else{
                body.pop_back();
            }
        }

        void Reset(){
            body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
            direction = {1, 0};
        }
};

// Creating the Game Class
class Game{
    public:
        Snake snake = Snake();
        Food food = Food(snake.body);
        bool running = true;
        int score = 0;
        
        Sound eatSound;
        Sound wallSound;

        // Constructor
        Game(){
            InitAudioDevice();
            eatSound = LoadSound("Sounds/eat.mp3");
            wallSound = LoadSound("Sounds/wall.mp3");

        }

        void Draw()
        {
            food.Draw();
            snake.Draw();
        }

        void Update(){
            if (running){
                snake.Update();
                CheckCollisionWithFood();
                CheckCollisionWithEdges();
                CheckCollisionWithTail();
            }
        }

        // Check if the snake's head collides with the Food object
        void CheckCollisionWithFood(){
            if (Vector2Equals(snake.body[0], food.position)){
                // cout << "Eating Food" << endl;
                food.position = food.GenerateRandomPosition(snake.body);
                snake.addSegment = true;
                score++;
                PlaySound(eatSound);
            }
        }

        // Checks if the snake goes out of the window screen
        // Resulting in a "Defeat" Screen
        void CheckCollisionWithEdges(){
            
            if (snake.body[0].x == cellCount || snake.body[0].x == -1){
                // cout << "Snake hit the edge of the screen" << endl;
                GameOver();
            }

            if (snake.body[0].y == cellCount || snake.body[0].y == -1){
                GameOver();
            }
        }
        void CheckCollisionWithTail(){
            std::deque<Vector2> headlessBody = snake.body;
            headlessBody.pop_front();
            if (ElementInDeque(snake.body[0], headlessBody)){
                GameOver();
            }
        }
        void GameOver(){
            // cout << "GAME OVER" << endl;
            snake.Reset();
            score = 0;
            food.position = food.GenerateRandomPosition(snake.body);
            running = false;
            PlaySound(wallSound);
        }
        // Destructor
        ~Game(){
            UnloadSound(eatSound);
            UnloadSound(wallSound);
            CloseAudioDevice();
        }
};

// Implementation of keyboard functionality
void keyboardControls(Game &game){
    if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1){
        game.snake.direction = {0, -1};
        game.running = true;
    }

    else if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1){
        game.snake.direction = {0, 1};
        game.running = true;
    }

    else if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1){
        game.snake.direction = {-1, 0};
        game.running = true;
    }

    else if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1){
        game.snake.direction = {1, 0};
        game.running = true;
    }
}

int main()
{
    // cout << "Starting Game..." << endl;

    const int screenWidth = 2 * offset + cellSize * cellCount, screenHeight = 2 * offset + cellSize * cellCount;

    // Creates the Window 
    // Parameters ==> Width, Height, Title of Window
    InitWindow(screenWidth, screenHeight, "Retro Snake!");

    
    // Set Limit FPS, Without it, FPS is unlimited
    SetTargetFPS(60);

    Game game = Game();

    // Game Loop
    while (!WindowShouldClose())
    {      
        BeginDrawing();
        // Cause the snake to move every 150 ms
        if (eventTriggered(0.15)){
            game.Update();    
        }
        keyboardControls(game);

        // Drawing
        // Setting the background color
        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{(float)offset-5, (float)offset-5, (float)cellSize*cellCount + 10, (float)cellSize*cellCount+10}, 5, darkGreen);
        DrawText("Retro Snake", offset - 5 , 20, 40, darkGreen);
        DrawText(TextFormat("Score: %i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
        game.Draw();

        EndDrawing();
    }

    // Required to close the window automatically
    CloseWindow();

    return 0;
}

// #include <iostream>
// #include <deque>
// #include <raylib.h>
// #include <raymath.h>
// int main(){
//     InitWindow(750, 750, "Retro Snake!");
//     SetTargetFPS(60);
//     std::cout << "HELLO" << std::endl;
//     while (!WindowShouldClose()){

//         BeginDrawing();
//         EndDrawing();
//     }
//     CloseWindow();
//     return 0;
// }