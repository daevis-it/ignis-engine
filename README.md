# Ignis Engine

Game engine 2D/3D scritto da zero in C++20, con Editor visivo integrato.
Progetto personale a scopo di apprendimento: capire il **perché** dei motori moderni
costruendone uno, non replicarne le feature.

> **Stato attuale — FASE 0 CHIUSA (task `01`–`05c`). Prossima: Fase 1.**
> Il progetto è stato ristrutturato da eseguibile monolitico a **tre target**
> (`Ignis` libreria + `IgnisEditor` + `Sandbox`), con separazione **Public/Private** degli
> header e dipendenze via **FetchContent**.
> Il **Logger** è stato riscritto: livelli, canali separati engine/client, colori,
> e formato **verificato dal compilatore**. Aggiunti `Base.h` (macro di piattaforma,
> `IGNIS_ASSERT`, `IGNIS_VERIFY`) e un **precompiled header** che taglia del ~39% il
> tempo di ricompilazione dell'engine.
> **Tutto verificato su entrambi i bersagli:** Linux Mint / GCC 13 e Windows / MSVC, in
> entrambi i casi da CLion. Il resto del codice applicativo è ancora quello
> dell'iterazione #0, con i suoi bug noti — vedi *Cosa non funziona ancora*.
>
> **Visual Studio 2026 non è supportato**, vedi *Trappole già pagate*.
>
> Quello che l'engine sa fare oggi: aprire una finestra OpenGL 3.3, ricevere eventi di
> tastiera/mouse/finestra e smistarli, esporre un Input statico, loggare, e disegnare
> l'interfaccia ImGui con docking e multi-viewport. **Non disegna ancora nulla di suo:**
> non esiste un renderer.

---

## Cos'è, e cosa non è

**È** un percorso di apprendimento deliberato. Ogni sottosistema viene costruito capendo
il problema che risolve, non copiando una soluzione. Un giorno servirà a fare piccoli
giochi indie 2D o 3D, e a insegnare game dev in famiglia.

**Non è** un concorrente di Unity, Unreal o Godot, e non prova a esserlo. Per i progetti
seri si usa Unreal — questo motore esiste perché usare uno strumento e capire come è
fatto sono due cose diverse.

---

## Stack e ambiente

| | |
|---|---|
| **Linguaggio** | C++20 (`std::format`, `std::unique_ptr`, lambda, template variadici) |
| **Build** | CMake ≥ 3.20 + Ninja, con `CMakePresets.json` |
| **Piattaforme** | Linux (sviluppo primario) e Windows (secondo bersaglio, non opzionale) |
| **Compilatori** | GCC 13 su Linux · MSVC (Visual Studio 2026) su Windows |
| **IDE** | **CLion su entrambi i sistemi.** Visual Studio 2026 non funziona con questo setup |
| **Finestre e input** | GLFW 3.4 (via FetchContent) |
| **Grafica** | OpenGL **4.5 Core Profile** con Direct State Access |
| **GL loader** | GLAD (generato per GL **4.5** Core, versionato in `Ignis/vendor/glad/`) |
| **Matematica** | GLM 1.0.1 (via FetchContent — dichiarata, non ancora usata) |
| **GUI** | Dear ImGui, branch `docking`, pinnato a `fd13a1e8` |

### Compilare

```bash
# Linux — una volta sola, gli header di sviluppo X11 e OpenGL
sudo apt install build-essential cmake ninja-build \
     libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev

cmake --preset linux-gcc-debug          # la prima volta scarica GLFW, GLM, ImGui
cmake --build build/linux-gcc-debug
./build/linux-gcc-debug/bin/IgnisEditor
```

**`libglfw3-dev` non serve più**, e non va usata: GLFW viene compilata dai sorgenti insieme
al progetto. Se l'hai installata da un'iterazione precedente puoi lasciarla, ma non è quella
che finisce nel binario.

Su Windows serve Visual Studio 2026 con il workload *Desktop development with C++*, più CMake
e Ninja (entrambi inclusi nel workload). I preset usano il generator **Ninja** anche su
Windows: va lanciato da un *Developer Command Prompt* (o da CLion/VS, che impostano
l'ambiente MSVC da soli), altrimenti `cl.exe` non è nel PATH.

```
cmake --preset windows-msvc-debug
cmake --build build/windows-msvc-debug
```

---

## Architettura

### I tre target

```
Ignis          libreria statica — il motore. Non conosce né l'editor né i giochi.
IgnisEditor    eseguibile — un client. Linka Ignis.
Sandbox        eseguibile — un secondo client. Linka Ignis.
```

**`Sandbox` non è un doppione dell'editor: è il test dei confini.** Con un solo target,
un accoppiamento sbagliato resta invisibile perché tanto compila lo stesso. Con due client
distinti sulla stessa libreria, il codice dell'editor che si infila nell'engine smette di
compilare da qualche parte, e lo scopri subito invece che a tre fasi di distanza.

### Public / Private

```
Ignis/
├── Public/Ignis/…      ← header esposti ai client   (include dir PUBLIC)
├── Private/Ignis/…     ← sorgenti e header interni  (include dir PRIVATE)
└── vendor/glad/        ← target separato, codice generato
```

La separazione **non è una convenzione, è imposta dal compilatore**:
`target_include_directories` espone solo `Public/`, quindi un client che prova a includere
un header interno non trova il file. Errore di compilazione, non cattiva abitudine.

> **Perché `Private/` ripete il livello `Ignis/`.** Sembra ridondanza da copia-incolla, ma
> serve a mantenere **un vocabolario solo**. Se `Private/` non lo ripetesse, un header
> interno si includerebbe come `"Core/Roba.h"` mentre tutto il resto del progetto usa
> `"Ignis/Core/Roba.h"`: due modi di scrivere la stessa cosa a seconda di dove ti trovi.
> Col livello ripetuto, **ogni include del progetto comincia per `Ignis/`**, pubblico o
> privato che sia.

Gli eseguibili hanno solo `Private/`: non esportano niente, quindi una `Public/` dichiarerebbe
un'intenzione falsa. Nascerà se e quando qualcosa dovrà esporre.

### Come parte il motore

Il `main()` **non appartiene al client**. Vive in `EntryPoint.h`, che il client include una
volta sola nel file dove definisce `CreateApplication()`:

```
main()                    ← in EntryPoint.h, dentro l'engine
  └─ Ignis::CreateApplication()   ← factory implementata dal CLIENT
       └─ new EditorApplication   ← deriva da Ignis::Application
            └─ Application::Run() ← il game loop, dentro l'engine
```

> **Perché l'inversione.** Il ciclo di vita del processo è responsabilità del motore: se
> domani l'avvio deve leggere un file di configurazione, aprire una console su Windows o
> montare un VFS, quel codice va scritto una volta nell'engine, non copiato in ogni gioco.
> Al client resta una sola domanda: *quale Application vuoi?*

`EntryPoint.h` **non è incluso da `Ignis.h`** ed è deliberato: includerlo due volte
produrrebbe due `main()` nello stesso binario.

### Application

Singleton di fatto (`Application::Get()`), possiede la finestra e fa girare il loop.
Riceve gli eventi dalla `Window` attraverso una callback installata alla costruzione e li
smista con l'`EventDispatcher`.

### Window

Incapsula GLFW: crea la finestra, imposta il contesto OpenGL 3.3 Core, inizializza GLAD e
installa le callback native. Ogni callback recupera la propria `WindowData` con
`glfwGetWindowUserPointer` e **traduce l'evento GLFW in un evento Ignis**, che spedisce
all'`Application` tramite `EventCallback`.

> **Perché il puntatore utente invece di una variabile globale.** Le callback GLFW sono
> funzioni C: non possono catturare stato. Il puntatore utente è il canale che GLFW offre
> per riattaccare l'oggetto C++ alla finestra che ha generato l'evento — ed è ciò che
> permetterà di avere più finestre senza riscrivere niente.

### Ciclo di vita di GLFW

`GLFWContext` (in `Private/`) è RAII sul contesto globale: `glfwInit()` nel costruttore,
`glfwTerminate()` nel distruttore. `Application` lo dichiara **prima** di `m_Window`.

```cpp
std::unique_ptr<GLFWContext> m_GLFWContext;   // costruito 1°, distrutto per ULTIMO
std::unique_ptr<Window>      m_Window;        // costruito 2°, distrutto per PRIMO
```

> **Quelle due righe sono codice funzionale, non stile.** I membri si distruggono in ordine
> inverso di dichiarazione, quindi la finestra muore prima che GLFW venga terminata — **per
> costruzione, non per attenzione**. Prima `glfwTerminate()` stava nel corpo di
> `~Application` e girava *prima* di `~Window`: si distruggeva una finestra già liberata, e
> l'errore era invisibile perché non c'era nessuna error callback. Se qualcuno riordina quei
> due membri il compilatore tace e il bug torna.

Verificato con un test che conta gli errori riportati dalla callback GLFW durante la
distruzione, compilato in due varianti dallo stesso sorgente: ordine attuale **0 errori**,
ordine precedente **1 errore** (`The GLFW library is not initialized`). Il controtest serve
a dimostrare che il test *sa fallire*.

**`glfwSetErrorCallback` è installata prima di `glfwInit`**, non dopo: GLFW lo consente
apposta, ed è l'unico modo di vedere anche gli errori dell'inizializzazione stessa. La
differenza pratica, eseguendo senza display:

```
prima:  [ERRORE] Errore durante l'inizializzazione di GLFW!      → poi abort dentro ImGui
ora:    [ERROR]  GLFW [65550]: X11: The DISPLAY environment variable is missing
        [ERROR]  Avvio fallito: glfwInit() non riuscita. …       → exit ordinato
```

Quel messaggio GLFW **esisteva già**: semplicemente non lo ascoltava nessuno.

### Errori fatali di avvio: eccezioni

`GLFWContext` e `Window` **lanciano** `std::runtime_error` se non riescono a inizializzarsi;
`main()` in `EntryPoint.h` cattura, logga e ritorna `EXIT_FAILURE`.

> **Le eccezioni servono solo qui.** Un errore di avvio succede una volta o mai, quindi non
> costa niente, e c'è un punto unico dove si decide cosa farne — utile a Fase 5, quando il
> Launcher dovrà poter dire "questo progetto non si apre" invece di sparire. **Non sono
> flusso di controllo e non entrano nel game loop.**

> **Attenzione a una sottigliezza in `Window`:** se `gladLoadGLLoader` fallisce lanciamo *dal
> costruttore*, e quando un costruttore lancia **il distruttore non viene chiamato**. La
> finestra appena creata resterebbe appesa, quindi lì c'è una `glfwDestroyWindow` esplicita.
> È l'unico punto del progetto in cui una pulizia manuale è corretta.

### Window

```cpp
Window window({ .Title = "Ignis Engine", .Width = 1280, .Height = 720, .VSync = true });
```

`WindowProps` è un aggregato e non tre parametri sciolti: al task `11`
l'`ApplicationSpecification` dovrà passarli, e **allargare una struct non tocca i punti di
costruzione, mentre cambiare una firma li tocca tutti**.

> **Le dimensioni sono PIXEL del framebuffer, non screen coordinates.**
> `glfwCreateWindow` prende screen coords, ma dopo la creazione interroghiamo
> `glfwGetFramebufferSize` e usiamo quelli. Su schermi HiDPI i due numeri divergono, e
> `glViewport` vuole i pixel. Per lo stesso motivo il resize passa da
> `glfwSetFramebufferSizeCallback` e **non** da `glfwSetWindowSizeCallback`.

> **`glViewport` lo chiama `Application`, non `Window`.** La Window è il sistema di
> finestre, non il renderer: emette l'evento e basta. Quando il Renderer esisterà si
> prenderà quel compito senza che la Window cambi di una riga. `OnWindowResize` ritorna
> `false`: l'evento **non** è consumato, perché il resize interessa anche a chi viene dopo.

Verificato con un test che legge `GL_VIEWPORT` dopo un ridimensionamento programmatico:
crea a 800×600, ridimensiona a 640×480. Con il fix il viewport segue; senza la chiamata a
`glViewport` resta a 800×600.

`Window` dichiara copy e move come `= delete`: possiede una `GLFWwindow*`, e copiarla
darebbe due oggetti con lo stesso handle e una doppia `glfwDestroyWindow`. Il move sarebbe
implementabile ma richiederebbe di ri-puntare `glfwSetWindowUserPointer` al nuovo indirizzo
di `m_Data`: finché nessuno ne ha bisogno, **`= delete` è più onesto di un move
sottilmente rotto**.

### Input e keycode

```cpp
if (Input::IsKeyPressed(KeyCode::Escape)) { … }
if (Input::IsMouseButtonPressed(MouseCode::ButtonLeft)) { … }
```

`KeyCodes.h` contiene 121 tasti ed è **generato** dai `#define` di `glfw3.h`, non trascritto
a mano. I valori numerici coincidono con quelli GLFW di proposito, così la conversione è un
cast invece di una tabella da mantenere allineata.

> **Il prezzo di quella scorciatoia sono quattordici `static_assert` in `Input.cpp`**, sui
> confini di ogni blocco della tabella:
> ```cpp
> static_assert(static_cast<int>(KeyCode::Escape) == GLFW_KEY_ESCAPE);
> ```
> Se un aggiornamento di GLFW cambiasse anche un solo valore, **il progetto smette di
> compilare**. Senza, un tasto smetterebbe di funzionare e nessuno saprebbe perché.

**Input ed eventi non si duplicano, si completano:** gli eventi dicono *cos'è successo* (il
salto si fa lì), l'Input dice *com'è adesso* (il movimento continuo si fa qui). Chiedere lo
stato dentro un evento è il modo classico di ottenere controlli che "saltano".

### GLFW non è visibile ai client

Nessun header pubblico include più glad, GLFW o ImGui. È bastato che `GLFWwindow` sia un
**tipo opaco**: `struct GLFWwindow;` dichiarato in avanti regge un puntatore, e i ~920 KB
di `glad.h` escono dall'API pubblica.

Le include directory che il compilatore passa a `Sandbox` sono ora tre —
`Sandbox/Private`, `Ignis/Public`, `glm-src` — e in `Ignis/CMakeLists.txt`
`glad`, `glfw` e `imgui` sono passati da `PUBLIC` a `PRIVATE`.

Verificato nei due sensi: un client che scrive `glfwInit()` **non compila**
(*'glfwInit' was not declared in this scope*), lo stesso client con `KeyCode::Escape`
compila.

### Event system

Gerarchia di eventi (`WindowClose`, `WindowResize`, `KeyPressed`, `MouseMoved`, …) con due
assi indipendenti: il **tipo** (`EventType`, uno solo per evento) e le **categorie**
(`EventCategory`, in bitmask, quindi più di una per evento).

```cpp
// Un evento di tastiera appartiene a DUE categorie contemporaneamente:
int GetCategoryFlags() const override { return EventCategoryKeyboard | EventCategoryInput; }
```

> **Perché il bitmasking e non un secondo enum.** Un `KeyPressedEvent` è
> *contemporaneamente* un evento di tastiera e un evento di input. Con un campo singolo
> dovresti scegliere quale delle due verità scrivere; con i bit le tieni entrambe, e un
> filtro "tutto ciò che è input, qualunque cosa sia" diventa un `AND` fra interi.

L'`EventDispatcher` usa i template per confrontare il tipo runtime dell'evento con il tipo
che una funzione sa gestire, e in caso di corrispondenza fa il cast e la chiama. Il flag
`Handled` serve a fermare la propagazione — **oggi lo scrive solo l'`Application` e non lo
legge nessuno**, perché manca il LayerStack (task `08`).

### Input

Interfaccia statica che interroga GLFW direttamente, disaccoppiata dagli eventi: gli eventi
dicono *cos'è successo*, l'Input risponde a *com'è adesso*. Servono entrambi — il salto
si fa sull'evento, il movimento continuo sullo stato.

### Logger

Quattro livelli (`Trace`, `Info`, `Warn`, `Error`) e due canali indipendenti: `IGNIS_CORE_*`
per il motore, `IGNIS_*` per il client. Output su `std::cerr`, con orario, colori, e
`file:riga` su Warn ed Error.

```cpp
IGNIS_CORE_INFO("Finestra creata con successo: {}x{}", width, height);
IGNIS_INFO("Sandbox avviato");            // dal gioco: prefisso [ APP ]
Logger::SetLevel(LogLevel::Warn);         // zittisce Trace e Info
```

> **Il formato è verificato dal compilatore, e questo cancella un'intera categoria di bug.**
> La firma prende `std::format_string<Args...>`, non `std::string_view`: una stringa
> costruita a runtime **non compila più**, e un segnaposto in più degli argomenti è un
> errore di compilazione invece di un'eccezione a runtime. Il vecchio
> `Logger::Info(e.ToString())` oggi viene rifiutato con *"the value of … is not usable in
> a constant expression"*.

> **Perché `std::cerr` anche per l'informativo.** `cerr` non è bufferizzato. Quando il
> motore segfaulta, le ultime righe scritte su `cout` resterebbero nel buffer e non
> arriverebbero mai a schermo: si perderebbe esattamente il log che dice dov'eri.

> **Perché macro e non chiamate dirette.** Solo una macro può sparire del tutto in Release
> — argomenti compresi, che non vengono nemmeno valutati — e può catturare `__FILE__` e
> `__LINE__` senza che tu li scriva ogni volta. `IGNIS_CORE_TRACE` in Release è
> letteralmente `((void)0)`.

I colori si attivano **solo se l'output è un terminale**: redirigendo su file i codici ANSI
diventerebbero spazzatura dentro il file. Su Windows vanno anche abilitati esplicitamente
(vedi *Trappole già pagate*).

### Base.h — piattaforma, assert, debug break

`IGNIS_PLATFORM_WINDOWS` / `IGNIS_PLATFORM_LINUX` dedotte dal compilatore, con un `#error`
esplicito se non è nessuna delle due: meglio non compilare che compilare su un sistema mai
testato. `IGNIS_DEBUG` arriva invece da **CMake** (`$<$<CONFIG:Debug>:IGNIS_DEBUG>`) e non
da `NDEBUG`, così resta possibile un giorno fare una Debug ottimizzata o una Release con
assert senza combattere contro lo standard.

```cpp
IGNIS_CORE_ASSERT(larghezza > 0, "Larghezza non valida: {} (attesa > 0)", larghezza);
IGNIS_CORE_VERIFY(Inizializza(), "L'inizializzazione deve riuscire");
```

Output quando fallisce — condizione stringificata **e** messaggio con i valori veri:

```
[IGNIS][ERROR] ASSERT FALLITO:  larghezza > 0            (Application.cpp:46)
[IGNIS][ERROR] Larghezza non valida: 0 (attesa > 0)      (Application.cpp:46)
```

> **Un assert non è la gestione di un errore, e confonderli è il modo più comune di
> farsi male.** Un assert verifica un'**invariante interna**: se è falsa, il bug è nostro —
> per questo può sparire in Release. Un file mancante, un `glfwInit` fallito, un puntatore
> GL nullo **non sono invarianti**: sono condizioni del mondo reale, e vanno gestite sempre,
> anche in Release. Questo non contraddice *niente default silenziosi*, lo precisa.

> **Niente side effect dentro un assert.** `IGNIS_ASSERT(Inizializza(), …)` funziona in
> Debug e in Release **non chiama più `Inizializza()`**. Per quei casi c'è `IGNIS_VERIFY`,
> che valuta sempre la condizione e controlla solo in Debug. Verificato con due contatori
> distinti: in Release `ASSERT` lascia il suo a 0, `VERIFY` porta il suo a 1.

**Il messaggio dell'assert è obbligatorio**, e non per pedanteria: renderlo opzionale
richiederebbe `__VA_OPT__`, che su MSVC funziona solo con `/Zc:preprocessor` — un flag con
una storia di attriti proprio con `<Windows.h>`, che includiamo in `Logger.cpp`. Avrebbe
aperto una divergenza fra i due compilatori dentro il meccanismo che serve quando qualcosa
va storto.

### Precompiled header

`Ignis/Private/ignispch.h`, attivo solo sul target `Ignis` (editor e Sandbox sono un file
ciascuno). Contiene **solo libreria standard**.

| ricompilazione completa dell'engine | media su 3 misure |
|---|---|
| con PCH | **1,93 s** |
| senza PCH | 3,15 s |

> **Il guadagno non viene dal numero di file ma dal peso degli header.** Con cinque soli
> `.cpp` sembrerebbe inutile, e invece taglia il 39%: `<format>` in C++20 è enorme e
> `Logger.h` lo tira dentro in ogni unità di traduzione.

Cosa **non** c'è dentro, deliberatamente: ImGui, GLFW e GLAD (li usano due file su cinque —
metterli lì significherebbe che ogni file dell'engine si porta dentro OpenGL senza motivo),
e neppure `Logger.h` / `Base.h`. Quest'ultima è una scelta di stile: renderebbe le macro
disponibili ovunque senza include, ma nasconderebbe le dipendenze. **Il PCH è
un'ottimizzazione, non un modo per smettere di scrivere gli include.**

### ImGuiLayer

Inizializza Dear ImGui con i backend GLFW + OpenGL3, abilita **docking** e
**multi-viewport** (finestre trascinabili fuori dall'applicazione). `Begin()` e `End()`
delimitano il frame UI dentro il game loop.

> **L'ordine di inizializzazione conta e non è documentato in nessun errore.**
> `Window` installa le proprie callback GLFW nel costruttore; **poi**
> `ImGuiLayer::Init()` chiama `ImGui_ImplGlfw_InitForOpenGL(window, true)`, e quel `true`
> fa sì che ImGui installi le sue callback *salvando le precedenti e richiamandole a
> catena*. Funziona solo perché la Window viene prima. Invertire i due passi farebbe
> sparire silenziosamente tutti gli eventi dell'engine.

---

## Decisioni consolidate

| # | Decisione | Perché |
|---|---|---|
| **D1** | Toolchain Windows: **MSVC** | Compilatore nativo, miglior supporto driver NVIDIA e tooling grafico. Più severo di GCC: fa emergere gli errori di portabilità invece di nasconderli. |
| **D2** | Dipendenze via **FetchContent** | Versioni dichiarate nel CMake con commit precisi, scaricate e compilate al configure. Nessuna installazione a sistema, build riproducibile ovunque. |
| **D3** | **GLAD resta versionato** | Non è una libreria che evolve: è codice *generato una volta* per GL 3.3 Core. Sta in `Ignis/vendor/glad/` come target separato, così i warning severi dell'engine non lo toccano. |
| **D4** | **Tre target**, non uno | Vedi *Architettura*. `Sandbox` è il test che i confini esistano davvero. |
| **D5** | **C++20**, non 23 | `std::format` esiste su GCC 13 e MSVC 2022+. Del C++23 i due compilatori supportano sottoinsiemi diversi: si finirebbe con codice che compila sul portatile e non sulla workstation. |
| **D6** | **ImGui pinnato a un commit** | `docking` è un branch, non un tag: si muove. Un pin significa che fra sei mesi il repo compila ancora con l'ImGui su cui il codice è stato scritto. |
| **D7** | **Astrazione RendererAPI rimandata** | OpenGL diretto dietro classi RAII. L'astrazione multi-API si introduce quando esiste una seconda API vera, non prima. |

**Licenza: non ancora scelta.** Il repo è pubblico ma senza licenza è legalmente
"tutti i diritti riservati": nessuno può riusare il codice né contribuire. Da decidere.

---

## Cosa non funziona ancora

Elenco onesto di ciò che è rotto o assente. Ogni voce rimanda al task che la chiude
(dettaglio in [`ROADMAP.md`](ROADMAP.md)).

### Bug aperti

**Il resize non aggiorna il viewport OpenGL** → task `05`
`glViewport` non compare in nessun punto del progetto. Non si nota oggi perché si disegna
solo un `glClear` e ImGui gestisce il proprio viewport, ma al primo triangolo esce fuori.
In più si usa `glfwSetWindowSizeCallback`, che dà *screen coordinates*: per `glViewport`
serve `glfwSetFramebufferSizeCallback`, e su schermi HiDPI i due numeri differiscono.

### Default silenziosi

### Buchi architetturali

- **Nessun LayerStack** → task `08`. `ImGuiLayer` è un membro concreto di `Application`;
  `OnEvent` intercetta `WindowClose` e finisce lì. Il flag `Handled` non lo legge nessuno.
- **`io.WantCaptureKeyboard` / `WantCaptureMouse` mai consultati** → task `09`.
  Conseguenza pratica: scrivendo in un campo di testo di ImGui, il gioco riceve gli stessi
  tasti. E `Input::IsKeyPressed(GLFW_KEY_ESCAPE)` nel loop polla GLFW ignorando ImGui.
- **Nessun timestep, nessun VSync** → task `10`. Il loop gira libero bruciando CPU e GPU, e
  non esiste un delta time da passare a nulla.
- **Finestra hardcodata** a `800, 600, "Ignis Engine"` dentro il costruttore → task `11`.

### Debiti minori

- **`Window.h` espone `<glad/glad.h>` e `<GLFW/glfw3.h>`** → task `05`. Essendo un header
  pubblico, ogni client che include Ignis si porta dentro tutto GLFW: l'engine non sta
  nascondendo la sua libreria di finestre, la sta ridistribuendo. È il motivo per cui
  oggi `target_link_libraries(Ignis PUBLIC …)` invece di `PRIVATE`.
- **`EventType` dichiara sette valori che nessuno emette** → task `06`:
  `WindowFocus`, `WindowLostFocus`, `WindowMoved`, `KeyTyped`, `AppTick`, `AppUpdate`,
  `AppRender`. Un enum che promette cose inesistenti è documentazione falsa.
- **`EventCategory` è un enum non-scoped** → task `06`: `Ignis::None` è un simbolo nudo nel
  namespace, che prima o poi collide.
- **`repeatCount` è hardcodato a 0 o 1** → task `05`, non conta le ripetizioni reali.
- **`Event.h` usa `std::ostream` senza includere `<ostream>`** → task `06`. Compila per
  inclusione transitiva, cioè per fortuna.
- **Warning attivi in build.** `-Wall -Wextra` segnala parametri non usati in
  `Window.cpp` (`scancode`, `mods`) e in `EntryPoint.h` (`argc`, `argv`). **Sono voluti e
  restano visibili**: si azzerano ai task `05` e `11`, quando quei parametri cominceranno
  a servire davvero. Silenziarli adesso vorrebbe dire nascondere un promemoria.

---

## Trappole già pagate

**Ordine di inclusione GLAD.** `<glad/glad.h>` deve sempre precedere `<GLFW/glfw3.h>`:
GLAD definisce i simboli OpenGL che GLFW altrimenti dichiara per conto suo, e il conflitto
si manifesta come una valanga di errori di ridefinizione.

**GLFW 3.4 su Linux vuole Wayland anche se usi X11.** Di default compila entrambi i
backend, e quello Wayland pretende `wayland-scanner` in fase di *configure*: senza, la
configure muore con `Failed to find wayland-scanner`. Mint gira su X11, quindi
`cmake/Dependencies.cmake` forza `GLFW_BUILD_WAYLAND OFF`. Da riaccendere il giorno che
serva davvero.

**Gli header X11 di sviluppo servono comunque.** Compilando GLFW dai sorgenti servono
`libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`, `libxi-dev`: con `libglfw3-dev`
precompilata non servivano, perché qualcun altro aveva già compilato. L'errore è esplicito
(`RandR headers not found`), ma arriva a metà configure.

**I window hints chiedono un MINIMO, non una versione esatta — e la versione target la
decide GLAD.** Chiediamo 4.5 Core, ma le due macchine rispondono diversamente:

```
Windows / RTX 5070 :  GLAD ha caricato OpenGL 4.5  ·  4.5.0 NVIDIA 610.88
Linux   / Iris Xe  :  GLAD ha caricato OpenGL 4.6  ·  4.6 (Core Profile) Mesa 25.2.8
```

Entrambi sono **conformi alla spec**: i Core profile dalla 3.2 in poi sono retrocompatibili,
quindi un'implementazione può restituire qualsiasi contesto ≥ a quello richiesto. NVIDIA dà
esattamente il minimo, Mesa dà il massimo che ha. In GLFW non esiste un modo di dire "4.5 e
non un dito di più".

> **Quindi ciò che tiene allineate le due macchine non sono gli hint: è GLAD.** Il binding è
> generato per 4.5, perciò i simboli 4.6 non esistono nel progetto e una chiamata a una
> funzione 4.6 non compila su nessuna delle due. **La versione target di Ignis si cambia
> rigenerando GLAD, non modificando i window hints.**

**I colori ANSI su Windows vanno chiesti.** Il terminale Windows moderno capisce le
sequenze ANSI, ma solo dopo una `SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)`
sull'handle giusto — `STD_ERROR_HANDLE`, dato che scriviamo su `cerr`. Senza, al posto dei
colori si vedono i codici grezzi (`←[32m`) sporcare ogni riga. Sta in `Logger::Init()`.

**`localtime` sicura ha nomi diversi sui due compilatori.** `std::localtime` non è
thread-safe, e le varianti che lo sono divergono: MSVC ha `localtime_s(&tm, &time)`, POSIX
ha `localtime_r(&time, &tm)` — **argomenti invertiti**, non solo nome diverso. È isolata in
un solo punto di `Logger.cpp`. È il tipo di divergenza che compila su una macchina e non
sull'altra, quindi vale la regola: ogni volta che si tocca un'API di sistema, si controlla.

**Visual Studio 2026 non digerisce questo progetto; CLion sì.** Su Windows il progetto
configura e compila senza attriti da CLion con la toolchain MSVC, mentre aprendolo
direttamente in VS 2026 dà problemi gravi. La causa non è ancora stata diagnosticata —
i sospetti sono l'integrazione CMake di VS che ignora o reinterpreta `CMakePresets.json`,
oppure il suo generator che entra in conflitto con Ninja. **CLion è l'IDE di riferimento
su entrambi i sistemi**, il che ha anche il pregio di togliere una variabile: stesso
editor, stessi preset, stesso comportamento sulle due macchine. Nota utile: RenderDoc e
NSight si attaccano all'eseguibile, quindi non serve aver compilato da VS per usarli.

**Cache CMake e GLOB.** Aggiungendo file a una directory raccolta con `file(GLOB)`, CMake
usa i vecchi riferimenti finché non lo si ricarica a mano. **Risolto**: tutti i glob del
progetto usano `CONFIGURE_DEPENDS`, che li rivaluta a ogni build. Se qualcosa sembra
ancora non essere compilato, il sospetto giusto è una cache sporca: cancellare
`build/<preset>` è sempre sicuro.

**MSVC e i commenti accentati.** I sorgenti sono in italiano, quindi contengono UTF-8.
Senza `/utf-8` MSVC li interpreta nella codepage locale (warning C4819 e stringhe
corrotte). La flag è in `Ignis/CMakeLists.txt` insieme a `/Zc:__cplusplus`, che serve
perché altrimenti MSVC riporta `__cplusplus` fermo al 2003.

---

## Convenzioni

- **Namespace `Ignis`** per tutto il codice del motore.
- **`m_` per i membri**, `s_` per gli statici. Metodi in `PascalCase`.
- **L'engine è una libreria, editor e giochi sono client.** Codice che serve solo
  all'editor non entra in `Ignis`.
- **Il vendor non si modifica.** Se servisse una patch, va documentata qui.
- **Nessun percorso hardcodato nei sorgenti**: scene, asset e configurazioni stanno in file.
- **Chi possiede una risorsa dichiara come si copia e come si sposta.** Ogni classe che
  incapsula una risorsa GPU o di sistema deve dichiarare copy e move esplicitamente —
  copy `= delete` di default. Una classe RAII copiabile per sbaglio produce doppie
  `glDelete*` su ID condivisi: un bug che si manifesta fasi dopo, quando gli oggetti
  finiscono nei contenitori. **Regola valida dal primo giorno del Renderer.**
- **Una feature che compila su un solo sistema non è finita.**

---

## Dove sta la verità

- **Questo README** descrive ciò che **funziona adesso**. Se qui c'è scritto che una cosa
  c'è, deve esserci nel codice.
- **[`ROADMAP.md`](ROADMAP.md)** descrive ciò che **verrà**: fasi, task, decisioni prese e
  decisioni rimandate.
- **Il codice batte entrambi.** Quando una scelta si appoggia a un'affermazione di questi
  file, va verificata sui sorgenti. Sono la mappa, non il territorio.
