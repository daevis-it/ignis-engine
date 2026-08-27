#pragma once

#include <filesystem>
#include <string>

namespace Ignis
{
    // ══════════════════════════════════════════════════════════════════════════
    //  Dove stanno le cose (decisione D15).
    //
    //  I percorsi si risolvono rispetto alla CARTELLA DELL'ESEGUIBILE, non alla
    //  directory di lavoro. La cwd non è un ancoraggio: ApplicationSpecification::
    //  WorkingDirectory la sposta già oggi, e il Launcher della Fase 5 la sposterà
    //  sempre. Un asset trovato o non trovato a seconda di dove hai lanciato il
    //  programma è un bug che si manifesta solo sulla macchina di qualcun altro.
    //
    //  È il seme del VFS della Fase 5: quando arriverà, cambierà cosa c'è sotto a
    //  Resolve, non chi la chiama. Si chiama Paths e non FileSystem apposta —
    //  "FileSystem" prometterebbe di aprire e leggere, che è il VFS, non questo.
    // ══════════════════════════════════════════════════════════════════════════
    class Paths
    {
    public:
        // Da chiamare UNA VOLTA all'avvio, prima di qualsiasi Resolve. La chiama
        // main() in EntryPoint.h, subito dopo Logger::Init().
        //
        // LANCIA std::runtime_error se il sistema non sa dire dov'è l'eseguibile.
        // Non c'è ripiego sulla cwd, ed è deliberato: un ripiego silenzioso
        // riporterebbe esattamente il bug che questa classe esiste per evitare,
        // ma con in più la convinzione di averlo risolto.
        //
        // Perché all'avvio e non alla prima chiamata: il fallimento va dove
        // sappiamo già gestirlo — il try/catch di main() — invece che a metà di
        // un frame, dove non c'è nessuno a raccoglierlo.
        static void Init();

        // La cartella che contiene l'eseguibile in esecuzione. Assoluta.
        static const std::filesystem::path& ExecutableDirectory();

        // Un percorso relativo diventa assoluto rispetto all'eseguibile.
        // Un percorso GIÀ assoluto viene restituito com'è: chi passa un percorso
        // assoluto sa quello che fa, e riscriverlo sarebbe una sorpresa.
        static std::filesystem::path Resolve(const std::filesystem::path& relativo);

        // UTF-8, e non è pignoleria: su Windows std::filesystem::path::string()
        // converte nella codepage ANSI, quindi un nome utente accentato esce
        // storto o fa lanciare. Le librerie C che riceveranno questi percorsi
        // (ImGui, stb_image) vogliono UTF-8. Un solo punto di conversione, qui.
        static std::string ToUtf8(const std::filesystem::path& percorso);
    };
}
