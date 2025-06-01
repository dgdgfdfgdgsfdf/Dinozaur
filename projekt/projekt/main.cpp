#include <iostream>
#include <chrono>
#include <thread>
#include "GameLogic.h"
#include "API.h"
#include <stdexcept>


int main() {
    try {
        // Inicjalizacja gry
        if (!InitGame()) {
            throw std::runtime_error("Nie mozna zainicjalizowac gry!");
        }

        // Zmienna do pomiaru czasu
        auto lastTime = std::chrono::high_resolution_clock::now();

        // Glowna petla gry
        bool gameRunning = true;

        while (gameRunning) {
            try {
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
            catch (const std::exception& e) {
                std::cerr << "Blad w petli gry: " << e.what() << std::endl;
                gameRunning = false;
            }
        }

        // Sprzatanie
        CleanupGame();
        return 0;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Blad inicjalizacji: " << e.what() << std::endl;
        CleanupGame();
        return -1;
    }
    catch (const std::exception& e) {
        std::cerr << "Nieoczekiwany blad: " << e.what() << std::endl;
        CleanupGame();
        return -1;
    }
    catch (...) {
        std::cerr << "Nieznany blad krytyczny!" << std::endl;
        CleanupGame();
        return -1;
    }
}