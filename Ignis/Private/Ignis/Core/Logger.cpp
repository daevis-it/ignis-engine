#include "Ignis/Core/Logger.h"

#include <iostream>
#include <ctime>
#include <cstring>

// ── Tutto il codice specifico di piattaforma è confinato in questo file. ──
// Windows.h porta con sé un corredo di macro moleste (min, max, near, far):
// le due define qui sotto ne tagliano la maggior parte, e comunque il veleno
// non esce da questo .cpp perché nessun header pubblico lo include.
#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
    #include <io.h>
    #define IGNIS_ISATTY(fd) _isatty(fd)
    #define IGNIS_FILENO(f)  _fileno(f)
#else
    #include <unistd.h>
    #define IGNIS_ISATTY(fd) isatty(fd)
    #define IGNIS_FILENO(f)  fileno(f)
#endif

namespace Ignis
{
    namespace
    {
        // I colori si usano solo se l'output è davvero un terminale. Se qualcuno
        // redirige su file, i codici ANSI diventerebbero spazzatura dentro il file.
        bool s_UseColors = false;

        constexpr const char* COLOR_RESET  = "\033[0m";
        constexpr const char* COLOR_TRACE  = "\033[90m";   // grigio
        constexpr const char* COLOR_INFO   = "\033[32m";   // verde
        constexpr const char* COLOR_WARN   = "\033[33m";   // giallo
        constexpr const char* COLOR_ERROR  = "\033[1;31m"; // rosso acceso
        constexpr const char* COLOR_DIM    = "\033[90m";   // per orario e file:riga

        const char* LevelColor(LogLevel level)
        {
            switch (level)
            {
                case LogLevel::Trace: return COLOR_TRACE;
                case LogLevel::Info:  return COLOR_INFO;
                case LogLevel::Warn:  return COLOR_WARN;
                case LogLevel::Error: return COLOR_ERROR;
            }
            return COLOR_RESET;
        }

        const char* LevelName(LogLevel level)
        {
            switch (level)
            {
                case LogLevel::Trace: return "TRACE";
                case LogLevel::Info:  return "INFO ";
                case LogLevel::Warn:  return "WARN ";
                case LogLevel::Error: return "ERROR";
            }
            return "?????";
        }

        const char* SourceName(LogSource source)
        {
            return source == LogSource::Core ? "IGNIS" : " APP ";
        }

        // __FILE__ contiene il percorso intero: teniamo solo il nome del file.
        const char* BaseName(const char* path)
        {
            if (!path) return "?";
            const char* slash     = std::strrchr(path, '/');
            const char* backslash = std::strrchr(path, '\\');
            const char* last = slash > backslash ? slash : backslash;
            return last ? last + 1 : path;
        }

        // std::localtime NON è thread-safe, e le due varianti sicure hanno nomi e
        // firme DIVERSE fra MSVC e POSIX: è una delle divergenze classiche fra i
        // nostri due compilatori bersaglio. La isoliamo qui, una volta sola.
        void CurrentTime(char* buffer, std::size_t size)
        {
            const std::time_t now = std::time(nullptr);
            std::tm tm{};

        #if defined(_WIN32)
            if (localtime_s(&tm, &now) != 0) { buffer[0] = '\0'; return; }
        #else
            if (localtime_r(&now, &tm) == nullptr) { buffer[0] = '\0'; return; }
        #endif

            if (std::strftime(buffer, size, "%H:%M:%S", &tm) == 0)
                buffer[0] = '\0';
        }
    }

    void Logger::Init()
    {
        // Scriviamo su stderr (vedi Write), quindi è lì che va chiesto il terminale.
        const bool isTerminal = IGNIS_ISATTY(IGNIS_FILENO(stderr)) != 0;

    #if defined(_WIN32)
        s_UseColors = false;
        if (isTerminal)
        {
            // Il terminale Windows capisce le sequenze ANSI, ma solo dopo che gliel'hai
            // chiesto esplicitamente. Senza questo, invece dei colori si vedono i codici
            // grezzi (←[32m) sporcare ogni riga.
            HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
            if (handle != INVALID_HANDLE_VALUE && handle != nullptr)
            {
                DWORD mode = 0;
                if (GetConsoleMode(handle, &mode) &&
                    SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
                {
                    s_UseColors = true;
                }
            }
        }
    #else
        s_UseColors = isTerminal;
    #endif
    }

    void Logger::Write(LogSource source, LogLevel level,
                       const char* file, int line, std::string_view message)
    {
        char timeBuffer[16];
        CurrentTime(timeBuffer, sizeof(timeBuffer));

        const char* color = s_UseColors ? LevelColor(level) : "";
        const char* dim   = s_UseColors ? COLOR_DIM        : "";
        const char* reset = s_UseColors ? COLOR_RESET      : "";

        // std::cerr e non std::cout, e non è una svista: cerr NON è bufferizzato.
        // Quando il motore segfaulta, le ultime righe di cout resterebbero nel
        // buffer e non arriverebbero mai a schermo — cioè si perderebbe esattamente
        // il log che dice dove eravamo.
        std::cerr << dim << '[' << timeBuffer << "] " << reset
                  << color << '[' << SourceName(source) << "][" << LevelName(level) << "] "
                  << message << reset;

        // File e riga solo dove servono davvero. Su Trace e Info sarebbero rumore
        // che allunga ogni riga senza aggiungere niente.
        if (level == LogLevel::Warn || level == LogLevel::Error)
            std::cerr << dim << "  (" << BaseName(file) << ':' << line << ')' << reset;

        std::cerr << '\n';
    }
}
