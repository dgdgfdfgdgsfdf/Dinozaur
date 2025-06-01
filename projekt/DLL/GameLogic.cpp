#define GAMELOGIC_EXPORTS
#include "gamelogic.h"
#include "API.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>


static DinoGame* g_game = nullptr;
static sf::RenderWindow* g_window = nullptr;

// Implementacja klasy DinoGame
DinoGame::DinoGame()
    : window(nullptr), dinoY(0), dinoVelocityY(0), gameSpeed(INITIAL_SPEED),
    nextSpawnScore(BASE_SPAWN_INTERVAL), currentSpawnInterval(BASE_SPAWN_INTERVAL),
    isGameRunning(false), isJumping(false), isCrouching(false),
    score(0), highScore(0), scoreTimer(0), groundOffsetX(0),
    hasGroundTexture(false),
    rng(std::random_device{}()),
    birdHeightDist(350.0f, 400.0f),
    obstacleTypeDist(1, 3),
    entityTypeDist(1, 100),
    spawnVariationDist(-5, 5),
    animationTimer(0), currentDinoFrame(0), currentBirdFrame(0), isPaused(false), showMenu(true)
{
    LoadHighScore();
}

DinoGame::~DinoGame() {
    SaveHighScore();
    Cleanup();
}

bool DinoGame::Initialize(sf::RenderWindow* gameWindow) {
    window = gameWindow;

    if (!window) {
        return false;
    }

    // Ladowanie tekstur
    if (!LoadTextures()) {
        return false;
    }

    // Proba zaladowania tekstury podloza
    if (!LoadGroundTexture("ground.png")) {
        std::cout << "Nie zaladowano tekstury podloza" << std::endl;
    }

    // Ladowanie fontu
    if (!font.loadFromFile("arial.ttf")) {
        // Domyslny font systemu
    }

    // Inicjalizacja tekstu wyniku
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::Black);
    scoreText.setPosition(window->getSize().x - 150, 20);

    highScoreText.setFont(font);
    highScoreText.setCharacterSize(18);
    highScoreText.setFillColor(sf::Color::Black);
    highScoreText.setPosition(window->getSize().x - 150, 50);

    gameOverText.setFont(font);
    gameOverText.setCharacterSize(36);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setString("GAME OVER");
    sf::FloatRect textBounds = gameOverText.getLocalBounds();
    gameOverText.setPosition(
        (window->getSize().x - textBounds.width) / 2,
        (window->getSize().y - textBounds.height) / 2
    );

    startText.setFont(font);
    startText.setCharacterSize(32);
    startText.setFillColor(sf::Color::Black);
    startText.setString("Press SPACE to Start");
    sf::FloatRect startBounds = startText.getLocalBounds();
    startText.setPosition(
        (window->getSize().x - startBounds.width) / 2,
        window->getSize().y / 2 - 50
    );

    instructionsText.setFont(font);
    instructionsText.setCharacterSize(16);
    instructionsText.setFillColor(sf::Color::Black);
    instructionsText.setString("UP/SPACE: Jump | DOWN: Crouch | P: Pause | R: Restart");
    sf::FloatRect instrBounds = instructionsText.getLocalBounds();
    instructionsText.setPosition(
        (window->getSize().x - instrBounds.width) / 2,
        window->getSize().y / 2 + 20
    );

    pauseText.setFont(font);
    pauseText.setCharacterSize(28);
    pauseText.setFillColor(sf::Color::Blue);
    pauseText.setString("PAUSED - Press P to Continue");
    sf::FloatRect pauseBounds = pauseText.getLocalBounds();
    pauseText.setPosition(
        (window->getSize().x - pauseBounds.width) / 2,
        (window->getSize().y - pauseBounds.height) / 2
    );

    menuBackground.setSize(sf::Vector2f(window->getSize().x, window->getSize().y));
    menuBackground.setFillColor(sf::Color(255, 255, 255, 200));

    CreateButton(startButton, "START GAME", window->getSize().x / 2 - 75, window->getSize().y / 2 - 25, 150, 50);
    CreateButton(pauseButton, "PAUSE", 10, 10, 80, 30);
    CreateButton(restartButton, "RESTART", 100, 10, 80, 30);
    CreateButton(exitButton, "EXIT", window->getSize().x / 2 - 50, window->getSize().y / 2 + 50, 100, 40);

    // Inicjalizacja dinozaura
    dinoSprite.setTexture(dinoRunTexture1);

    dinoSprite.setPosition(DINO_X, GROUND_Y - DINO_SIZE);
    dinoY = GROUND_Y - DINO_SIZE;

    // Reset stanu gry
    isGameRunning = false;
    dinoY = GROUND_Y - DINO_SIZE;
    dinoVelocityY = 0;
    dinoSprite.setPosition(DINO_X, dinoY);
    dinoSprite.setTexture(dinoRunTexture1);
    obstacleSprites.clear();
    birdSprites.clear();

    return true;
}

bool DinoGame::LoadTextures() {
    try {
        // Ladowanie tekstur dinozaura
        if (!dinoRunTexture1.loadFromFile("dino_run1.png")) {
            throw std::runtime_error("Nie mozna zaladowac dino_run1.png");
        }

        if (!dinoRunTexture2.loadFromFile("dino_run2.png")) {
            std::cerr << "Nie mozna zaladowac dino_run2.png" << std::endl;
            dinoRunTexture2 = dinoRunTexture1;
        }

        if (!dinoDeadTexture.loadFromFile("dino_dead.png")) {
            std::cerr << "Nie mozna zaladowac dino_dead.png" << std::endl;
            sf::Image img;
            img.create(44, 44, sf::Color::Red);
            dinoDeadTexture.loadFromImage(img);
        }

        if (!dinoCrouchTexture1.loadFromFile("dino_crouch1.png")) {
            std::cerr << "Nie mozna zaladowac dino_crouch1.png" << std::endl;
            dinoCrouchTexture1 = dinoRunTexture1;
        }

        if (!dinoCrouchTexture2.loadFromFile("dino_crouch2.png")) {
            std::cerr << "Nie mozna zaladowac dino_crouch2.png" << std::endl;
            dinoCrouchTexture2 = dinoCrouchTexture1;
        }

        // Ladowanie tekstur ptakow
        if (!birdTexture1.loadFromFile("bird1.png")) {
            std::cerr << "Nie mozna zaladowac bird1.png" << std::endl;
            sf::Image img;
            img.create(80, 70, sf::Color::Blue);
            birdTexture1.loadFromImage(img);
        }

        if (!birdTexture2.loadFromFile("bird2.png")) {
            std::cerr << "Nie mozna zaladowac bird2.png" << std::endl;
            birdTexture2 = birdTexture1;
        }

        // Ladowanie tekstur przeszkod
        if (!obstacleTexture1.loadFromFile("obstacle1.png")) {
            throw std::runtime_error("Nie mozna zaladowac obstacle1.png");
        }

        if (!obstacleTexture2.loadFromFile("obstacle2.png")) {
            throw std::runtime_error("Nie mozna zaladowac obstacle2.png");
        }

        if (!obstacleTexture3.loadFromFile("obstacle3.png")) {
            throw std::runtime_error("Nie mozna zaladowac obstacle3.png");
        }

        std::cout << "Tekstury zostaly zaladowane!" << std::endl;
        return true;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Blad ladowania tekstur: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Nieznany blad podczas ladowania tekstur" << std::endl;
        return false;
    }
}

bool DinoGame::LoadGroundTexture(const std::string& filename) {
    if (groundTexture.loadFromFile(filename)) {
        hasGroundTexture = true;
        groundSprite.setTexture(groundTexture);

        groundSprite.setPosition(0, GROUND_Y-10);

        groundOffsetX = 0;

        return true;
    }

    hasGroundTexture = false;
    return false;
}

void DinoGame::LoadHighScore() {
    std::ifstream file("highscore.txt");
    if (file.is_open()) {
        file >> highScore;
        file.close();
    }
}

void DinoGame::SaveHighScore() {
    std::ofstream file("highscore.txt");
    if (file.is_open()) {
        file << highScore;
        file.close();
    }
}

void DinoGame::Restart() {
    isGameRunning = true;
    isJumping = false;
    isCrouching = false;
    score = 0;
    scoreTimer = 0;
    gameSpeed = INITIAL_SPEED;
    nextSpawnScore = BASE_SPAWN_INTERVAL;
    currentSpawnInterval = BASE_SPAWN_INTERVAL;
    animationTimer = 0;
    currentDinoFrame = 0;
    currentBirdFrame = 0;
    groundOffsetX = 0;

    dinoY = GROUND_Y - DINO_SIZE;
    dinoVelocityY = 0;
    dinoSprite.setPosition(DINO_X, dinoY);
    dinoSprite.setTexture(dinoRunTexture1);

    obstacleSprites.clear();
    birdSprites.clear();

    isPaused = false;
    showMenu = false;
}

bool DinoGame::Update(float deltaTime) {
    if (!isGameRunning && !showMenu) {
        return true;
    }

    if (isPaused || showMenu) {
        return true;
    }

    UpdateAnimations(deltaTime);
    UpdatePhysics(deltaTime);
    UpdateEntities(deltaTime);
    UpdateGround(deltaTime);
    UpdateScore(deltaTime);
    CheckCollisions();

    return true;
}

void DinoGame::UpdateAnimations(float deltaTime) {
    animationTimer += deltaTime;

    if (animationTimer >= 0.1f) {
        if (!isGameRunning) {
            dinoSprite.setTexture(dinoDeadTexture);
            ApplyDinoScaling(DINO_SIZE, DINO_SIZE);
        }
        else if (isCrouching && !isJumping) {
            currentDinoFrame = (currentDinoFrame + 1) % 2;

            if (currentDinoFrame == 0) {
                dinoSprite.setTexture(dinoCrouchTexture1);
            }
            else {
                dinoSprite.setTexture(dinoCrouchTexture2);
            }

            ApplyDinoScaling(DINO_SIZE, DINO_CROUCH_HEIGHT);
        }
        else if (!isJumping) {
            currentDinoFrame = (currentDinoFrame + 1) % 2;

            if (currentDinoFrame == 0) {
                dinoSprite.setTexture(dinoRunTexture1);
            }
            else {
                dinoSprite.setTexture(dinoRunTexture2);
            }

            ApplyDinoScaling(DINO_SIZE, DINO_SIZE);
        }

        currentBirdFrame = (currentBirdFrame + 1) % 2;
        for (auto& bird : birdSprites) {
            if (currentBirdFrame == 0) {
                bird.setTexture(birdTexture1);
            }
            else {
                bird.setTexture(birdTexture2);
            }
        }

        animationTimer = 0;
    }
}

void DinoGame::ApplyDinoScaling(float targetWidth, float targetHeight) {
    dinoSprite.setScale(1.0f, 1.0f);

    sf::Vector2u textureSize = dinoSprite.getTexture()->getSize();

    if (textureSize.x > 0 && textureSize.y > 0) {
        float scaleX = targetWidth / static_cast<float>(textureSize.x);
        float scaleY = targetHeight / static_cast<float>(textureSize.y);
        dinoSprite.setScale(scaleX, scaleY);
    }
}

void DinoGame::UpdatePhysics(float deltaTime) {
    if (isJumping) {
        dinoVelocityY += GRAVITY * deltaTime;
        dinoY += dinoVelocityY * deltaTime;

        float groundLevel = GROUND_Y - DINO_SIZE;
        if (dinoY >= groundLevel) {
            dinoY = groundLevel;
            dinoVelocityY = 0;
            isJumping = false;
        }
    }
    else {
        if (isCrouching) {
            dinoY = GROUND_Y - DINO_CROUCH_HEIGHT;
        }
        else {
            dinoY = GROUND_Y - DINO_SIZE;
        }
    }

    dinoSprite.setPosition(DINO_X, dinoY);
}

bool DinoGame::CheckSpriteCollision(const sf::Sprite& sprite1, const sf::Sprite& sprite2) {

    sf::Vector2f dinoPos = sprite1.getPosition();
    sf::Vector2f obstaclePos = sprite2.getPosition();

    float dinoWidth, dinoHeight;
    if (isCrouching && !isJumping) {
        dinoWidth = DINO_SIZE;
        dinoHeight = DINO_CROUCH_HEIGHT;
    }
    else {
        dinoWidth = DINO_SIZE;
        dinoHeight = DINO_SIZE;
    }

    float obstacleWidth = OBSTACLE_WIDTH;
    float obstacleHeight = OBSTACLE_HEIGHT;

    const sf::Texture* obstacleTexture = sprite2.getTexture();
    if (obstacleTexture == &obstacleTexture2) {
        obstacleWidth = OBSTACLE2_WIDTH;
    }
    else if (obstacleTexture == &obstacleTexture3) {
        obstacleWidth = OBSTACLE3_WIDTH;
    }
    else if (obstacleTexture == &birdTexture1 || obstacleTexture == &birdTexture2) {
        obstacleWidth = BIRD_WIDTH;
        obstacleHeight = BIRD_HEIGHT;
    }

    sf::FloatRect bounds1(dinoPos.x, dinoPos.y, dinoWidth, dinoHeight);
    sf::FloatRect bounds2(obstaclePos.x, obstaclePos.y, obstacleWidth, obstacleHeight);

    
    if (isCrouching && !isJumping) {
        bounds1.left += 4;
        bounds1.top += 4;
        bounds1.width -= 8;
        bounds1.height -= 8;
    }
    else {
        bounds1.left += 4;
        bounds1.top += 4;
        bounds1.width -= 8;
        bounds1.height -= 8;
    }

    bounds2.left += 2;
    bounds2.top += 2;
    bounds2.width -= 4;
    bounds2.height -= 4;

   
    bool collision = bounds1.intersects(bounds2);

    return collision;
}

void DinoGame::UpdateEntities(float deltaTime) {
    // Aktualizowanie pozycji przeszkod
    for (auto& obstacle : obstacleSprites) {
        sf::Vector2f pos = obstacle.getPosition();
        pos.x -= gameSpeed * deltaTime;
        obstacle.setPosition(pos);
    }

    // Aktualizowanie pozycji ptakow
    for (auto& bird : birdSprites) {
        sf::Vector2f pos = bird.getPosition();
        pos.x -= gameSpeed * deltaTime;
        bird.setPosition(pos);
    }

    // Usuwanie przeszkod ktore wyszly poza ekran
    obstacleSprites.erase(
        std::remove_if(obstacleSprites.begin(), obstacleSprites.end(),
            [](const sf::Sprite& obs) {
                return obs.getPosition().x < -100;
            }),
        obstacleSprites.end()
    );

    // Usuwanie ptakow ktore wyszly poza ekran
    birdSprites.erase(
        std::remove_if(birdSprites.begin(), birdSprites.end(),
            [](const sf::Sprite& bird) {
                return bird.getPosition().x < -100;
            }),
        birdSprites.end()
    );

    CheckForSpawn();

    // Zwiekszanie predkosci
    gameSpeed += SPEED_INCREASE * deltaTime;
}

void DinoGame::CheckForSpawn() {
    if (score >= nextSpawnScore) {
        SpawnRandomEntity();

        currentSpawnInterval = CalculateSpawnInterval(score);
        int variation = spawnVariationDist(rng);
        nextSpawnScore = score + currentSpawnInterval + variation;

        std::cout << "Spawn at score: " << score << ", next spawn: " << nextSpawnScore << std::endl;
    }
}

int DinoGame::CalculateSpawnInterval(int currentScore) {
    int interval = BASE_SPAWN_INTERVAL - (currentScore / 20);

    if (interval < MIN_SPAWN_INTERVAL) {
        interval = MIN_SPAWN_INTERVAL;
    }

    return interval;
}

void DinoGame::SpawnRandomEntity() {
    if (score < BIRD_SPAWN_SCORE) {
        SpawnObstacle();
        return;
    }

    // 70% szans na przeszkode, 30% na ptaka
    int randomChoice = entityTypeDist(rng);

    if (randomChoice <= 70) {
        SpawnObstacle();
    }
    else {
        SpawnBird();
    }
}

void DinoGame::UpdateGround(float deltaTime) {
    if (hasGroundTexture) {
        groundOffsetX -= gameSpeed * deltaTime;

        float textureWidth = static_cast<float>(groundTexture.getSize().x);

        if (groundOffsetX <= -textureWidth) {
            groundOffsetX = 0;
        }

        groundSprite.setPosition(groundOffsetX, groundSprite.getPosition().y);
    }
}

void DinoGame::SpawnObstacle() {
    sf::Sprite obstacle;
    float obstacleWidth = OBSTACLE_WIDTH;

    int obstacleType = obstacleTypeDist(rng);

    switch (obstacleType) {
    case 1:
        obstacle.setTexture(obstacleTexture1);
        obstacleWidth = OBSTACLE_WIDTH;
        break;
    case 2:
        obstacle.setTexture(obstacleTexture2);
        obstacleWidth = OBSTACLE2_WIDTH;
        break;
    case 3:
        obstacle.setTexture(obstacleTexture3);
        obstacleWidth = OBSTACLE3_WIDTH;
        break;
    }

    sf::Vector2u textureSize = obstacle.getTexture()->getSize();
    if (textureSize.x > 0 && textureSize.y > 0) {
        float scaleX = obstacleWidth / textureSize.x;
        float scaleY = OBSTACLE_HEIGHT / textureSize.y;
        obstacle.setScale(scaleX, scaleY);
    }

    float groundY = GROUND_Y - OBSTACLE_HEIGHT;
    obstacle.setPosition(window->getSize().x, groundY);

    obstacleSprites.push_back(obstacle);
}

void DinoGame::SpawnBird() {
    sf::Sprite bird;
    bird.setTexture(birdTexture1);

    sf::Vector2u textureSize = birdTexture1.getSize();
    if (textureSize.x > 0 && textureSize.y > 0) {
        float scaleX = BIRD_WIDTH / textureSize.x;
        float scaleY = BIRD_HEIGHT / textureSize.y;
        bird.setScale(scaleX, scaleY);
    }

    float birdY = birdHeightDist(rng);
    bird.setPosition(window->getSize().x, birdY);

    birdSprites.push_back(bird);
}

void DinoGame::UpdateScore(float deltaTime) {
    scoreTimer += deltaTime;
    if (scoreTimer >= 0.1f) {
        score++;
        scoreTimer = 0;

        if (score > highScore) {
            highScore = score;
        }
    }
}

void DinoGame::CheckCollisions() {
    // Sprawdzanie kolizji z przeszkodami
    for (const auto& obstacle : obstacleSprites) {
        if (CheckSpriteCollision(dinoSprite, obstacle)) {
            isGameRunning = false;
            SaveHighScore();
            break;
        }
    }

    // Sprawdzanie kolizji z ptakami
        for (const auto& bird : birdSprites) {
            if (CheckSpriteCollision(dinoSprite, bird)) {
                isGameRunning = false;
                SaveHighScore();
                break;
            }
        }
    
}

void DinoGame::Jump() {
    if (!isJumping && !isCrouching && isGameRunning) {
        dinoVelocityY = JUMP_FORCE;
        isJumping = true;
    }
}

void DinoGame::Crouch(bool crouch) {
    if (isGameRunning) {
        if (crouch) {
            if (!isJumping && dinoY >= GROUND_Y - DINO_SIZE - 5) {
                isCrouching = true;
            }
        }
        else {
            isCrouching = false;
        }
    }
}

void DinoGame::Render() {
    if (!window) return;

    sf::Color bgColor = sf::Color(247, 247, 247);
    window->clear(bgColor);

    // Rysowanie podloza
    if (hasGroundTexture) {
        float textureWidth = static_cast<float>(groundTexture.getSize().x);
        float windowWidth = static_cast<float>(window->getSize().x);

        window->draw(groundSprite);

        sf::Sprite secondGround = groundSprite;
        secondGround.setPosition(groundOffsetX + textureWidth, groundSprite.getPosition().y);
        window->draw(secondGround);

        if (textureWidth < windowWidth) {
            int copies = static_cast<int>(std::ceil(windowWidth / textureWidth)) + 1;

            for (int i = 1; i <= copies; i++) {
                sf::Sprite additionalGround = groundSprite;
                additionalGround.setPosition(groundOffsetX + (textureWidth * i), groundSprite.getPosition().y);
                window->draw(additionalGround);
            }
        }
    }
    else {
        sf::RectangleShape groundLine;
        groundLine.setSize(sf::Vector2f(window->getSize().x, 5));
        groundLine.setPosition(0, GROUND_Y);
        groundLine.setFillColor(sf::Color::Black);
        window->draw(groundLine);
    }

    // Rysuj dinozaura
    window->draw(dinoSprite);

    // Rysuj przeszkody
    for (const auto& obstacle : obstacleSprites) {
        sf::Sprite tempObstacle = obstacle;
        window->draw(tempObstacle);
    }

    // Rysuj ptaki
    for (const auto& bird : birdSprites) {
        sf::Sprite tempBird = bird;
        window->draw(tempBird);
    }

    if (showMenu) {
        // Menu glowne
        window->draw(menuBackground);
        window->draw(startButton.shape);
        window->draw(startButton.text);
        window->draw(exitButton.shape);
        window->draw(exitButton.text);
    }
    else if (isPaused) {
        // Stan pauzy
        window->draw(pauseText);
    }
    else if (!isGameRunning) {
        // Game Over
        window->draw(gameOverText);
    }

    if (!showMenu) {
        sf::Color textColor = sf::Color::Black;
        scoreText.setFillColor(textColor);
        highScoreText.setFillColor(textColor);

        scoreText.setString("Score: " + std::to_string(score));
        highScoreText.setString("High Score: " + std::to_string(highScore));

        window->draw(scoreText);
        window->draw(highScoreText);

        if (isGameRunning) {
            window->draw(pauseButton.shape);
            window->draw(pauseButton.text);
            window->draw(restartButton.shape);
            window->draw(restartButton.text);
        }
    }

    window->display();
}

void DinoGame::Cleanup() {
    obstacleSprites.clear();
    birdSprites.clear();
    window = nullptr;
}

void DinoGame::TogglePause() {
    if (isGameRunning && !showMenu) {
        isPaused = !isPaused;
    }
}

void DinoGame::ShowStartMenu() {
    showMenu = true;
    isPaused = false;
}

void DinoGame::HideStartMenu() {
    showMenu = false;
}

void DinoGame::CreateButton(Button& button, const std::string& text, float x, float y, float width, float height) {
    button.shape.setSize(sf::Vector2f(width, height));
    button.shape.setPosition(x, y);
    button.shape.setFillColor(sf::Color(200, 200, 200));
    button.shape.setOutlineThickness(2);
    button.shape.setOutlineColor(sf::Color::Black);

    button.text.setFont(font);
    button.text.setString(text);
    button.text.setCharacterSize(16);
    button.text.setFillColor(sf::Color::Black);

    sf::FloatRect textBounds = button.text.getLocalBounds();
    button.text.setPosition(
        x + (width - textBounds.width) / 2,
        y + (height - textBounds.height) / 2 - 5
    );
}

void DinoGame::UpdateButtonState(Button& button, sf::Vector2i mousePos) {
    sf::FloatRect bounds = button.shape.getGlobalBounds();
    button.isHovered = bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    if (button.isHovered) {
        button.shape.setFillColor(sf::Color(150, 150, 255));
    }
    else {
        button.shape.setFillColor(sf::Color(200, 200, 200));
    }
}

bool DinoGame::IsButtonClicked(const Button& button, sf::Vector2i mousePos, bool mouseClick) {
    if (!mouseClick) return false;
    sf::FloatRect bounds = button.shape.getGlobalBounds();
    return bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
}

void DinoGame::UpdateGUI(sf::Vector2i mousePosition, bool mouseClick) {
    mousePos = mousePosition;

    if (showMenu) {
        UpdateButtonState(startButton, mousePos);
        UpdateButtonState(exitButton, mousePos);

        if (IsButtonClicked(startButton, mousePos, mouseClick)) {
            HideStartMenu();
            Restart();
        }
        if (IsButtonClicked(exitButton, mousePos, mouseClick)) {
            if (window) {
                window->close();
            }
        }
    }

    if (isGameRunning && !showMenu) {
        UpdateButtonState(pauseButton, mousePos);
        UpdateButtonState(restartButton, mousePos);

        if (IsButtonClicked(pauseButton, mousePos, mouseClick)) {
            TogglePause();
        }
        if (IsButtonClicked(restartButton, mousePos, mouseClick)) {
            Restart();
        }
    }
}

// Implementacja API C
extern "C" {
    GAMELOGIC_API bool InitGame() {
        if (g_game) {
            delete g_game;
        }

        g_window = new sf::RenderWindow(sf::VideoMode(800, 600), "Chrome Dino Game");
        g_game = new DinoGame();

        if (!g_game->Initialize(g_window)) {
            delete g_game;
            delete g_window;
            g_game = nullptr;
            g_window = nullptr;
            return false;
        }

        return true;
    }

    GAMELOGIC_API bool UpdateGame(float deltaTime) {
        if (!g_game || !g_window) {
            return false;
        }

        sf::Event event;
        while (g_window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                return false;
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space || event.key.code == sf::Keyboard::Up) {
                    if (g_game->IsMenuVisible()) {
                        g_game->HideStartMenu();
                        g_game->Restart();
                    }
                    else if (g_game->IsRunning()) {
                        g_game->Jump();
                    }
                    else {
                        g_game->Restart();
                    }
                }
                if (event.key.code == sf::Keyboard::Down) {
                    g_game->Crouch(true);
                }
                if (event.key.code == sf::Keyboard::P) {
                    g_game->TogglePause();
                }
                if (event.key.code == sf::Keyboard::R) {
                    g_game->Restart();
                }
            }
            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::Down) {
                    g_game->Crouch(false);
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(*g_window);
                    g_game->UpdateGUI(mousePos, true);
                }
            }
            if (event.type == sf::Event::MouseMoved) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(*g_window);
                g_game->UpdateGUI(mousePos, false);
            }
        }

        return g_game->Update(deltaTime) && g_window->isOpen();
    }

    GAMELOGIC_API void RenderGame() {
        if (g_game && g_window) {
            g_game->Render();
        }
    }

    GAMELOGIC_API void Jump() {
        if (g_game) {
            g_game->Jump();
        }
    }

    GAMELOGIC_API void RestartGame() {
        if (g_game) {
            g_game->Restart();
        }
    }

    GAMELOGIC_API bool IsGameRunning() {
        return g_game ? g_game->IsRunning() : false;
    }

    GAMELOGIC_API int GetScore() {
        return g_game ? g_game->GetScore() : 0;
    }

    GAMELOGIC_API void CleanupGame() {
        if (g_game) {
            g_game->Cleanup();
            delete g_game;
            g_game = nullptr;
        }

        if (g_window) {
            g_window->close();
            delete g_window;
            g_window = nullptr;
        }
    }
}