#pragma once

#ifdef GAMELOGIC_EXPORTS
#define GAMELOGIC_API __declspec(dllexport)
#else
#define GAMELOGIC_API __declspec(dllimport)
#endif

extern "C" {
    // Inicjalizacja gry
    GAMELOGIC_API bool InitGame();

    // Aktualizacja logiki gry 
    GAMELOGIC_API bool UpdateGame(float deltaTime);

    // Renderowanie gry
    GAMELOGIC_API void RenderGame();

    // Skok dinozaura
    GAMELOGIC_API void Jump();

    // Restart gry
    GAMELOGIC_API void RestartGame();

    // Sprawdz czy gra jest uruchomiona
    GAMELOGIC_API bool IsGameRunning();

    // Pobierz aktualny wynik
    GAMELOGIC_API int GetScore();

    // Zwolnij zasoby
    GAMELOGIC_API void CleanupGame();
}