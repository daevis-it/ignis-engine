#pragma once

#include <format>
#include <string_view>
#include <utility>

namespace Ignis
{
    // Livelli in ordine crescente di gravità: il filtro è un semplice confronto.
    enum class LogLevel : int
    {
        Trace = 0,
        Info,
        Warn,
        Error
    };

    // Chi sta parlando. Serve a poter zittire il gioco senza perdere il motore
    // (o l'esatto contrario, mentre si debugga un sottosistema).
    enum class LogSource : int
    {
        Core = 0,   // il motore
        Client      // l'editor o il gioco
    };

    class Logger
    {
    public:
        // Va chiamata una volta all'avvio, PRIMA di qualsiasi log.
        // Lo fa EntryPoint.h: il main appartiene all'engine, quindi non si può dimenticare.
        // Decide se i colori sono usabili e, su Windows, li abilita.
        static void Init();

        static void     SetLevel(LogLevel level) { s_Level = level; }
        static LogLevel GetLevel()               { return s_Level; }

        // Il cuore. NON chiamarla direttamente: usa le macro IGNIS_* qui sotto,
        // che riempiono file e riga per te.
        //
        // std::format_string<Args...> invece di string_view NON è un dettaglio:
        // è ciò che rende il formato verificato dal COMPILATORE. Una stringa
        // costruita a runtime non è più accettata, e un segnaposto in più degli
        // argomenti diventa un errore di compilazione invece di un'eccezione.
        template<typename... Args>
        static void Log(LogSource source, LogLevel level,
                        const char* file, int line,
                        std::format_string<Args...> fmt, Args&&... args)
        {
            if (static_cast<int>(level) < static_cast<int>(s_Level))
                return;

            Write(source, level, file, line,
                  std::format(fmt, std::forward<Args>(args)...));
        }

    private:
        // Tutta la parte che tocca il sistema operativo vive nel .cpp.
        static void Write(LogSource source, LogLevel level,
                          const char* file, int line, std::string_view message);

        // inline (C++17): niente definizione nel .cpp, niente ordine di
        // inizializzazione statica da temere.
        inline static LogLevel s_Level = LogLevel::Trace;
    };
}

// ══════════════════════════════════════════════════════════════════════════════
//  Macro
//
//  Perché macro e non chiamate dirette: solo una macro può SPARIRE del tutto in
//  Release (argomenti compresi, che quindi non vengono nemmeno valutati) e può
//  catturare __FILE__ e __LINE__ senza che tu li scriva ogni volta.
//
//  IGNIS_DEBUG è definita da CMake (target_compile_definitions), non dedotta da
//  NDEBUG: vedi Ignis/CMakeLists.txt.
// ══════════════════════════════════════════════════════════════════════════════

#define IGNIS_CORE_INFO(...)  ::Ignis::Logger::Log(::Ignis::LogSource::Core,   ::Ignis::LogLevel::Info,  __FILE__, __LINE__, __VA_ARGS__)
#define IGNIS_CORE_WARN(...)  ::Ignis::Logger::Log(::Ignis::LogSource::Core,   ::Ignis::LogLevel::Warn,  __FILE__, __LINE__, __VA_ARGS__)
#define IGNIS_CORE_ERROR(...) ::Ignis::Logger::Log(::Ignis::LogSource::Core,   ::Ignis::LogLevel::Error, __FILE__, __LINE__, __VA_ARGS__)

#define IGNIS_INFO(...)       ::Ignis::Logger::Log(::Ignis::LogSource::Client, ::Ignis::LogLevel::Info,  __FILE__, __LINE__, __VA_ARGS__)
#define IGNIS_WARN(...)       ::Ignis::Logger::Log(::Ignis::LogSource::Client, ::Ignis::LogLevel::Warn,  __FILE__, __LINE__, __VA_ARGS__)
#define IGNIS_ERROR(...)      ::Ignis::Logger::Log(::Ignis::LogSource::Client, ::Ignis::LogLevel::Error, __FILE__, __LINE__, __VA_ARGS__)

#if defined(IGNIS_DEBUG)
    #define IGNIS_CORE_TRACE(...) ::Ignis::Logger::Log(::Ignis::LogSource::Core,   ::Ignis::LogLevel::Trace, __FILE__, __LINE__, __VA_ARGS__)
    #define IGNIS_TRACE(...)      ::Ignis::Logger::Log(::Ignis::LogSource::Client, ::Ignis::LogLevel::Trace, __FILE__, __LINE__, __VA_ARGS__)
#else
    // In Release il Trace non esiste: nessuna chiamata, nessuna valutazione degli
    // argomenti. Un log di traccia a ogni frame non deve costare niente in Release.
    #define IGNIS_CORE_TRACE(...) ((void)0)
    #define IGNIS_TRACE(...)      ((void)0)
#endif
