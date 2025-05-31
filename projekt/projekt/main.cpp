#include <iostream>
#include <chrono>
#include <thread>
#include "GameLogic.h"
#include "API.h"

int main() {

    // Inicjalizacja gry
    if (!InitGame()) {
        std::cerr << "Blad: Nie mozna zainicjalizowac gry!" << std::endl;
        return -1;
    }

    // Zmienna do pomiaru czasu
    auto lastTime = std::chrono::high_resolution_clock::now();

    // Glowna petla gry
    bool gameRunning = true;
    int lastScore = 0;
    bool wasGameRunning = true;

    while (gameRunning) {
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime);
        float deltaTime = duration.count() / 1000000.0f;
        lastTime = currentTime;

        // Ograniczenie deltaTime
        if (deltaTime > 0.05f) {
            deltaTime = 0.05f;
        }

        // Aktualizacja gry
        gameRunning = UpdateGame(deltaTime);

        // Renderowanie
        RenderGame();

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }

    // Sprzatanie
    CleanupGame();

    return 0;
}