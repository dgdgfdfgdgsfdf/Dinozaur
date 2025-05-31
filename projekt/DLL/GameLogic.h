#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <fstream>

// Stale gry
const float GROUND_Y = 450.0f;
const float DINO_X = 50.0f;
const float DINO_SIZE = 44.0f;
const float DINO_CROUCH_HEIGHT = 30.0f;
const float OBSTACLE_WIDTH = 25.0f;
const float OBSTACLE_HEIGHT = 50.0f;
const float BIRD_WIDTH = 46.0f;
const float BIRD_HEIGHT = 40.0f;
const float JUMP_FORCE = -500.0f;
const float GRAVITY = 1200.0f;
const float INITIAL_SPEED = 200.0f;
const float SPEED_INCREASE = 5.0f;
const int BIRD_SPAWN_SCORE = 300;
const int BASE_SPAWN_INTERVAL = 1;
const int MIN_SPAWN_INTERVAL = 20;
const float GROUND_HEIGHT = 100.0f;

class DinoGame {
private:
    sf::RenderWindow* window;

    // Tekstury dla roznych elementow gry
    sf::Texture dinoRunTexture1;      
    sf::Texture dinoRunTexture2;      
    sf::Texture dinoDeadTexture;      
    sf::Texture birdTexture1;         
    sf::Texture birdTexture2;         
    sf::Texture obstacleTexture1;     
    sf::Texture obstacleTexture2;     
    sf::Texture obstacleTexture3;     

    // Tekstura podloza
    sf::Texture groundTexture;    
    bool hasGroundTexture;            

    // Sprite dinozaura i podloza
    sf::Sprite dinoSprite;
    sf::Sprite groundSprite;

    // Kontenery spriteow
    std::vector<sf::Sprite> obstacleSprites;
    std::vector<sf::Sprite> birdSprites;

    // Animacja
    float animationTimer;
    int currentDinoFrame;
    int currentBirdFrame;
    float groundOffsetX;

    // Fizyka dinozaura
    float dinoY;
    float dinoVelocityY;

    // Stan gry
    float gameSpeed;
    bool isGameRunning;
    bool isJumping;
    bool isCrouching;

    // Spawning oparty na wyniku
    int nextSpawnScore;
    int currentSpawnInterval;

    // Wynik
    int score;
    int highScore;
    float scoreTimer;

    // Losowe generowanie
    std::mt19937 rng;
    std::uniform_real_distribution<float> birdHeightDist;
    std::uniform_int_distribution<int> obstacleTypeDist;
    std::uniform_int_distribution<int> entityTypeDist;
    std::uniform_int_distribution<int> spawnVariationDist;

    // UI
    sf::Font font;
    sf::Text scoreText;
    sf::Text highScoreText;
    sf::Text gameOverText;

public:
    DinoGame();
    ~DinoGame();

    // Inicjalizacja i cleanup
    bool Initialize(sf::RenderWindow* gameWindow);
    void Cleanup();

    // Ladowanie tekstur
    bool LoadTextures();
    bool LoadGroundTexture(const std::string& filename);

    // Glowna petla gry
    bool Update(float deltaTime);
    void Render();

    // Aktualizacje poszczegolnych systemow
    void UpdateAnimations(float deltaTime);
    void UpdatePhysics(float deltaTime);
    void UpdateEntities(float deltaTime);
    void UpdateGround(float deltaTime);
    void UpdateScore(float deltaTime);

    // Spawning obiektow
    void CheckForSpawn();
    void SpawnObstacle();
    void SpawnBird();
    void SpawnRandomEntity();
    int CalculateSpawnInterval(int currentScore);

    // Kolizje
    void CheckCollisions();
    bool CheckCollision(const sf::RectangleShape& rect1, const sf::RectangleShape& rect2);
    bool CheckSpriteCollision(const sf::Sprite& sprite1, const sf::Sprite& sprite2);

    // Kontrola gracza
    void Jump();
    void Crouch(bool crouch);
    void Restart();

    // Zapis/odczyt wyniku
    void LoadHighScore();
    void SaveHighScore();

    // Gettery
    bool IsRunning() const { return isGameRunning; }
    int GetScore() const { return score; }
    int GetHighScore() const { return highScore; }
};