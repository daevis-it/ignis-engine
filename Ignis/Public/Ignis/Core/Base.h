#pragma once

#include "Ignis/Core/Logger.h"

#include <utility>

// ══════════════════════════════════════════════════════════════════════════════
//  Piattaforma
//
//  Un #error invece di un silenzioso "nessuna delle due": meglio non compilare
//  affatto che compilare su un sistema mai testato e comportarsi in modo strano.
// ══════════════════════════════════════════════════════════════════════════════

#if defined(_WIN32)
    #define IGNIS_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define IGNIS_PLATFORM_LINUX
#else
    #error "Ignis supporta solo Windows e Linux."
#endif

// ══════════════════════════════════════════════════════════════════════════════
//  Configurazione
//
//  IGNIS_DEBUG e IGNIS_RELEASE arrivano da CMake (target_compile_definitions).
//  Il #error non è pedanteria: il 2026-08-25 quella riga di CMake è stata persa per
//  tre task, e il risultato è stato assert disattivati e log di traccia spariti —
//  SENZA UN SEGNALE, perché la mancanza di una macro non è un errore, è solo
//  un #if che va sul ramo sbagliato. Adesso perderla di nuovo non compila.
// ══════════════════════════════════════════════════════════════════════════════

#if !defined(IGNIS_DEBUG) && !defined(IGNIS_RELEASE)
    #error "Ne' IGNIS_DEBUG ne' IGNIS_RELEASE sono definite. Manca target_compile_definitions in Ignis/CMakeLists.txt?"
#endif

// ══════════════════════════════════════════════════════════════════════════════
//  Debug break
//
//  __builtin_trap() ferma il processo nel punto esatto: con GDB o CLion ti trovi
//  sulla riga giusta, che è ciò che serve. Non permette di "continuare" dopo il
//  break come fa __debugbreak() di MSVC — se un giorno darà fastidio, l'alternativa
//  su Linux è raise(SIGTRAP) da <csignal>.
// ══════════════════════════════════════════════════════════════════════════════

#if defined(IGNIS_PLATFORM_WINDOWS)
    #define IGNIS_DEBUGBREAK() __debugbreak()
#else
    #define IGNIS_DEBUGBREAK() __builtin_trap()
#endif

// ══════════════════════════════════════════════════════════════════════════════
//  Assert
//
//  UN ASSERT NON È LA GESTIONE DI UN ERRORE.
//
//  Un assert verifica un'INVARIANTE INTERNA: qualcosa che se è falso significa che
//  abbiamo scritto un bug noi. Per questo può sparire in Release.
//
//  Un file mancante, un glfwInit fallito, un puntatore GL nullo NON sono invarianti:
//  sono condizioni del mondo reale e vanno gestite SEMPRE, anche in Release, con un
//  errore esplicito. Non contraddice "niente default silenziosi": lo precisa.
//
//  ── NIENTE SIDE EFFECT DENTRO UN ASSERT ──
//  IGNIS_ASSERT(Inizializza(), "...") funziona in Debug e in Release non chiama più
//  Inizializza(). Per quei casi esiste IGNIS_VERIFY, che valuta sempre la condizione
//  e controlla solo in Debug.
//
//  Il messaggio è OBBLIGATORIO, e non per pedanteria: "assert fallito" senza contesto
//  costa più tempo di quanto ne faccia risparmiare. Tenerlo obbligatorio evita anche
//  __VA_OPT__, che su MSVC richiede il preprocessore conforme (/Zc:preprocessor) e
//  aprirebbe una divergenza fra i due compilatori proprio nel meccanismo che ci serve
//  quando qualcosa va storto.
// ══════════════════════════════════════════════════════════════════════════════

namespace Ignis::Detail
{
    template<typename... Args>
    void AssertFailed(LogSource source, const char* file, int line,
                      const char* expression,
                      std::format_string<Args...> fmt, Args&&... args)
    {
        Logger::Log(source, LogLevel::Error, file, line,
                    "ASSERT FALLITO:  {}", expression);
        Logger::Log(source, LogLevel::Error, file, line,
                    fmt, std::forward<Args>(args)...);
    }
}

#if defined(IGNIS_DEBUG)

    #define IGNIS_CORE_ASSERT(condition, ...)                                        \
        do {                                                                         \
            if (!(condition)) {                                                      \
                ::Ignis::Detail::AssertFailed(::Ignis::LogSource::Core,               \
                    __FILE__, __LINE__, #condition, __VA_ARGS__);                    \
                IGNIS_DEBUGBREAK();                                                  \
            }                                                                        \
        } while (0)

    #define IGNIS_ASSERT(condition, ...)                                             \
        do {                                                                         \
            if (!(condition)) {                                                      \
                ::Ignis::Detail::AssertFailed(::Ignis::LogSource::Client,             \
                    __FILE__, __LINE__, #condition, __VA_ARGS__);                    \
                IGNIS_DEBUGBREAK();                                                  \
            }                                                                        \
        } while (0)

    // VERIFY: la condizione viene valutata anche in Release, solo il controllo sparisce.
    #define IGNIS_CORE_VERIFY(condition, ...) IGNIS_CORE_ASSERT(condition, __VA_ARGS__)
    #define IGNIS_VERIFY(condition, ...)      IGNIS_ASSERT(condition, __VA_ARGS__)

#else

    #define IGNIS_CORE_ASSERT(condition, ...) ((void)0)
    #define IGNIS_ASSERT(condition, ...)      ((void)0)

    #define IGNIS_CORE_VERIFY(condition, ...) ((void)(condition))
    #define IGNIS_VERIFY(condition, ...)      ((void)(condition))

#endif
