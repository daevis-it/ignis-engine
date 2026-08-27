#include "Ignis/Core/Paths.h"

#include "Ignis/Core/Base.h"
#include "Ignis/Core/Logger.h"

#include <format>
#include <stdexcept>

// ── Tutto il codice specifico di piattaforma è confinato qui, come in Logger.cpp.
// Windows.h porta con sé macro moleste: le due define ne tagliano la maggior parte,
// e il veleno non esce da questo .cpp perché nessun header pubblico lo include.
#if defined(IGNIS_PLATFORM_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
#endif

namespace Ignis
{
    namespace
    {
        std::filesystem::path s_ExecutableDirectory;
        bool                  s_Initialized = false;

        // Il percorso dell'eseguibile secondo il sistema operativo.
        //
        // argv[0] NON è un'alternativa: è solo quello che il chiamante ha deciso di
        // scrivere nella exec, può essere un nome nudo, un percorso relativo, o una
        // bugia. Le due API qui sotto chiedono al kernel, che lo sa davvero.
        std::filesystem::path PercorsoEseguibile()
        {
#if defined(IGNIS_PLATFORM_WINDOWS)
            // La W non è decorazione: GetModuleFileNameA convertirebbe nella codepage
            // locale, e un nome utente accentato uscirebbe storto. Costruire il path
            // da wstring evita del tutto la conversione.
            std::wstring buffer(MAX_PATH, L'\0');

            for (;;)
            {
                const DWORD scritti = GetModuleFileNameW(nullptr, buffer.data(),
                                                         static_cast<DWORD>(buffer.size()));
                if (scritti == 0)
                    throw std::runtime_error(std::format(
                        "GetModuleFileNameW non riuscita (GetLastError = {}).", GetLastError()));

                // Se il buffer è troppo piccolo, GetModuleFileNameW TRONCA e restituisce
                // la dimensione del buffer invece di fallire. È il motivo di questo
                // ciclo: un controllo su != 0 accetterebbe un percorso monco.
                if (scritti < buffer.size())
                {
                    buffer.resize(scritti);
                    break;
                }

                if (buffer.size() >= 32768)   // limite dei percorsi estesi di Windows
                    throw std::runtime_error(
                        "Il percorso dell'eseguibile supera i 32768 caratteri.");

                buffer.resize(buffer.size() * 2);
            }

            return std::filesystem::path(buffer);
#else
            std::error_code ec;
            std::filesystem::path percorso = std::filesystem::read_symlink("/proc/self/exe", ec);
            if (ec)
                throw std::runtime_error(std::format(
                    "Impossibile leggere /proc/self/exe: {}. Senza, non si può sapere "
                    "dove stanno gli asset.", ec.message()));

            return percorso;
#endif
        }
    }

    void Paths::Init()
    {
        const std::filesystem::path eseguibile = PercorsoEseguibile();
        s_ExecutableDirectory = eseguibile.parent_path();
        s_Initialized         = true;

        IGNIS_CORE_INFO("Eseguibile: {}", ToUtf8(eseguibile));
        IGNIS_CORE_INFO("I percorsi relativi si risolvono da qui: {}",
                        ToUtf8(s_ExecutableDirectory));
    }

    const std::filesystem::path& Paths::ExecutableDirectory()
    {
        // Assert e non eccezione: chiamare Resolve prima di Init non è una condizione
        // del mondo reale, è un bug nostro — l'engine possiede il proprio main() e
        // chiama Init lì, quindi l'invariante è garantita per costruzione. L'unico
        // modo di violarla è chiamare da un inizializzatore statico, prima di main.
        IGNIS_CORE_ASSERT(s_Initialized,
                          "Paths::Init() non è ancora stata chiamata: nessun percorso "
                          "può essere risolto. Chiamata da un inizializzatore statico?");
        return s_ExecutableDirectory;
    }

    std::filesystem::path Paths::Resolve(const std::filesystem::path& relativo)
    {
        if (relativo.is_absolute())
            return relativo;

        return ExecutableDirectory() / relativo;
    }

    std::string Paths::ToUtf8(const std::filesystem::path& percorso)
    {
        // u8string() e non string(): la prima è UTF-8 per contratto su ogni sistema,
        // la seconda usa la codifica narrow nativa — che su Windows è la codepage
        // ANSI e può perdere caratteri o lanciare.
        //
        // In C++20 u8string() restituisce std::u8string (char8_t). Il cast è la
        // conversione standard verso i char: stessi byte, tipo diverso.
        const std::u8string utf8 = percorso.u8string();
        return std::string(reinterpret_cast<const char*>(utf8.data()), utf8.size());
    }
}
