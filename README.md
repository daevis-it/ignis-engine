# Ignis Engine

Game engine 2D/3D scritto da zero in C++20, con Editor visivo integrato.
Progetto personale a scopo di apprendimento: capire il **perché** dei motori moderni
costruendone uno, non replicarne le feature.

> **Stato attuale — ITERAZIONE #2 IN CORSO (Fase 2, Renderer 2D). Ultimo task chiuso: `13`.**
> Fasi 0 e 1 complete (task `01`–`11`), chiuse il 2026-08-25.
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
> Quello che l'engine sa fare oggi: aprire una finestra OpenGL 4.5 Core, ricevere eventi di
> tastiera/mouse/finestra e smistarli, esporre un Input statico, loggare, disegnare
> l'interfaccia ImGui con il docking attivo (i viewport no, vedi D14), e — dal task `12` —
> **far parlare il driver OpenGL** invece di ingoiare i suoi errori.
> **Non disegna ancora nulla di suo:** non esiste un renderer.

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

> **Una guardia in CMake impedisce gli omonimi.** Siccome `Public/` e `Private/` mappano
> entrambe su `Ignis/...`, un file con lo stesso percorso relativo nelle due zone sarebbe
> ambiguo: vincerebbe quello pubblico, primo negli include path, e la versione privata
> verrebbe ignorata **senza un solo messaggio** — il classico "ho modificato il file e non
> cambia niente". La configure ora fallisce con un errore esplicito.

**`Private/ignispch.h` sta alla radice**, fuori dalla struttura `Private/Ignis/...`, e non è
una svista: non è codice dell'engine ma un artefatto di build. Non ha namespace, non dichiara
niente, e nessuno lo include a mano — lo forza CMake in testa a ogni unità di traduzione.

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

### LayerStack

Un livello dell'applicazione (`Layer`) può agganciarsi, aggiornarsi, disegnare interfaccia
e ricevere eventi. L'editor è un layer, un gioco è un layer, l'interfaccia è un *overlay*.

| | direzione | perché |
|---|---|---|
| **Update** | dal basso verso l'alto | il gioco si aggiorna prima della UI che lo mostra |
| **Render** | dal basso verso l'alto | il fondo si disegna prima di ciò che ci sta sopra |
| **Eventi** | **dall'alto verso il basso** | chi sta sopra vede il clic per primo |

> **L'ordine degli eventi è l'inverso di quello visivo, e non è arbitrario: è l'unica regola
> che rende sensata un'interfaccia.** Se clicchi su un pannello che copre il gioco deve
> rispondere il pannello — cioè la cosa che *vedi*, che è quella disegnata per ultima.
> Propagando a ritroso, il primo a ricevere è l'ultimo ad aver disegnato.

Il ciclo si interrompe appena un layer mette `Handled = true`. Gli **overlay** stanno sempre
in cima anche se inseriti prima, così un layer di gioco aggiunto dopo non finisce mai sopra
l'interfaccia.

> **`PopLayer` e `PopOverlay` sono due funzioni distinte di proposito.** Ognuna cerca solo
> nella propria zona: se cercassero ovunque, rimuovere un overlay con `PopLayer`
> decrementerebbe il confine interno e **ogni inserimento successivo finirebbe nel posto
> sbagliato** — un layer di gioco sopra l'interfaccia, con la causa a ore di distanza dal
> sintomo.

`LayerStack` è dichiarato in `Application` **dopo** `m_Window`: i layer possiederanno
risorse GPU e devono morire mentre il contesto OpenGL è ancora vivo. Stessa regola d'ordine
del ciclo di vita GLFW, applicata a un caso nuovo.

### Cattura dell'input

`ImGuiLayer` è un overlay, quindi riceve gli eventi per primo, e li consuma quando ImGui li
vuole:

```cpp
if (event.IsInCategory(EventCategory::Mouse) && io.WantCaptureMouse)    event.Handled = true;
if (event.IsInCategory(EventCategory::Keyboard) && io.WantCaptureKeyboard) event.Handled = true;
```

Tastiera e mouse si consumano **indipendentemente**: il puntatore sopra un pannello non
deve rubare i tasti al gioco, e un campo di testo attivo non deve bloccare il clic sul mondo.

> **Qui non stiamo passando l'input a ImGui: glielo stiamo CHIEDENDO.** ImGui riceve gli
> eventi dalle proprie callback GLFW, installate a catena sopra le nostre. Noi gli chiediamo
> soltanto se quello che è appena successo riguardava lui.
>
> `WantCaptureMouse`/`WantCaptureKeyboard` sono calcolati durante `ImGui::NewFrame()`,
> quindi il valore letto è quello del **frame precedente**. È il comportamento normale e un
> frame di latenza è impercettibile — ma se un clic sembrasse "passare attraverso" nel primo
> frame in cui apri un pannello, la causa è questa.

`SetBlockEvents(false)` disattiva il filtro: servirà alla Fase 4, quando il viewport di
gioco dell'editor vorrà l'input anche col puntatore sopra l'area di ImGui.

### Timestep

```cpp
void OnUpdate(Timestep ts) override {
    m_Posizione += m_Velocita * ts;              // conversione implicita a float
    IGNIS_INFO("{:.2f} ms", ts.GetMilliseconds());
}
```

Esiste come **tipo** e non come `float` nudo perché rende esplicita l'unità: *"questo float è
in secondi o millisecondi?"* è la domanda dietro metà dei bug di movimento mille volte troppo
veloce. La conversione è implicita solo verso `float`; il costruttore è `explicit`, quindi un
float non diventa un Timestep per sbaglio.

Il tempo viene da **`std::chrono::steady_clock`**, non da `glfwGetTime` né da `system_clock`:

> `system_clock` può **saltare all'indietro** per una sincronizzazione NTP o un cambio d'ora,
> e un delta time negativo fa cose molto strane in una simulazione. `steady_clock` è monotono
> per contratto, e non lega il timing a GLFW — utile il giorno che servisse far girare
> l'engine headless per dei test.

**Il delta è limitato a 0,1 s.** Con un breakpoint di trenta secondi, il frame successivo
avrebbe `ts = 30.0` e qualunque cosa si muova attraverserebbe la mappa. Verificato
sospendendo il processo con `SIGSTOP`:

```
Delta time troncato: 1.509s -> 0.100s (stallo o breakpoint?)
```

Non è un default silenzioso: quando scatta, si vede.

### ApplicationSpecification

```cpp
Ignis::Application* Ignis::CreateApplication(ApplicationCommandLineArgs args) {
    ApplicationSpecification spec;
    spec.Name          = "Sandbox";
    spec.Window.Title  = "Sandbox — un gioco su Ignis";
    spec.EnableImGui   = false;      // un gioco non paga ImGui
    spec.CommandLineArgs = args;
    return new SandboxApplication(spec);
}
```

**`EnableImGui`** decide se l'`ImGuiLayer` viene creato. Con `false` non c'è contesto, non
c'è atlas dei font sulla GPU, non c'è `NewFrame`/`Render` a vuoto ogni frame. *Toglie il
costo a runtime, non i simboli dal binario* — per quello servirebbe separare ImGui a livello
di build, ed è annotato per la Fase 5.

**`WorkingDirectory`**, se valorizzata, sposta la directory di lavoro all'avvio — prima di
ogni altra cosa, perché altrimenti ogni percorso relativo verrebbe risolto rispetto alla
cartella sbagliata. Una cartella inesistente **ferma l'avvio anche in Release**: non è un
assert, è una condizione del mondo reale.

**`CommandLineArgs`** è la giuntura per il Launcher della Fase 5, che avvierà l'editor come
`IgnisEditor /percorso/progetto`. Oggi l'editor logga l'argomento e non ci fa niente:
impalcatura, non arredo.

> **Tre cose sono uscite dall'engine con questo task, e non è pulizia estetica.**
> Il **dockspace** era imposto da `Application` a ogni client: una superficie di docking a
> tutto schermo è una scelta dell'editor, non del motore — ora vive nell'`EditorLayer`, e con
> lui se n'è andato `<imgui.h>` da `Application.cpp`.
> **ESC** era cablato nel game loop: nessun gioco vero si chiude con ESC. Ora c'è
> `Application::Close()` e la decisione è del client — **ed essendo diventato un evento
> invece che polling, passa dall'ImGuiLayer: scrivendo in un campo di testo ESC non chiude
> più l'applicazione.** Prima lo faceva.

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

### RenderCommand

Lo strato più sottile possibile sopra OpenGL: una funzione per verbo, nessuno stato,
nessuna decisione.

```cpp
RenderCommand::SetViewport(0, 0, larghezza, altezza);
RenderCommand::SetClearColor({ 0.9f, 0.1f, 0.6f, 1.0f });
RenderCommand::Clear();
```

Non è l'astrazione multi-API di D7 — è D11/D17 applicati alle poche chiamate che non
appartengono a nessuna classe wrapper, perché non possiedono una risorsa.
**Nessun tipo OpenGL nelle firme:** chi include l'header non vede glad, non lo linka, e non
sa quale API grafica c'è sotto. `GLint` e `GLsizei` vivono solo nel `.cpp`.

`Clear()` pulisce **solo** il color buffer. Il depth entrerà quando ci sarà una profondità
da azzerare: aggiungerlo adesso, senza depth buffer nel framebuffer, sarebbe una riga che
non fa niente e sembra farlo.

> **Il colore di sfondo è una scelta del CLIENT, e il punto in cui l'engine mette il
> proprio default non è indifferente.** Sta nel costruttore di `Application`, non in
> `Run()`: il costruttore della classe base gira *prima* del corpo del costruttore del
> client, quindi un gioco che chiami `SetClearColor` nel proprio costruttore sovrascrive
> il default. Con la stessa riga in `Run()`, l'engine ricablerebbe il colore **dopo** la
> scelta del client, e quella scelta non avrebbe alcun effetto — senza un solo messaggio
> d'errore. Verificato con due eseguibili e due colori inconfondibili: editor
> verde-azzurro, Sandbox magenta.

Che sia l'engine a pulire lo schermo è **impalcatura dichiarata**: al task `19` sarà la
scena a decidere cosa c'è dietro. Non si toglie prima, perché un client che si dimenticasse
di pulire vedrebbe spazzatura — default silenzioso, di nuovo.

### Debug output di OpenGL

`InitGLDebugOutput()` (in `Private/Ignis/Renderer/GLDebug.cpp`) registra una callback che
il driver chiama a ogni errore, comportamento deprecato, UB o problema di prestazioni. La
chiama `Window::Window` subito dopo il caricamento di GLAD, prima di qualunque altra `gl*`.

```
[IGNIS][ERROR] GL ERRORE [API] #1: GL_INVALID_ENUM in glEnable(GL_TEXTURE_2D)  (GLDebug.cpp:63)
```

> **Senza, un errore GL non ha sintomo.** La chiamata sbagliata ritorna, il programma
> prosegue, e quello che vedi è uno schermo nero — o niente — molte righe dopo. È il
> default silenzioso peggiore del progetto, perché non è nostro: sta dentro il driver.

Tre cose non ovvie, tutte deliberate:

- **Il contesto di debug si CHIEDE, non si ottiene.** `glfwWindowHint(GLFW_CONTEXT_DEBUG)`
  è una richiesta come quella della versione: il driver può ignorarla. Per questo
  `InitGLDebugOutput` rilegge `GL_CONTEXT_FLAGS` e cerca `GL_CONTEXT_FLAG_DEBUG_BIT`
  invece di dare per buona l'intenzione — e se manca lo dice con un WARN. *Verificare
  l'effetto, non il testo*, applicato al driver invece che al CMake.
- **`GL_DEBUG_OUTPUT_SYNCHRONOUS` è metà del valore.** Senza, il driver può accodare i
  messaggi e la callback scatta quando la chiamata colpevole è già uscita dallo stack.
  Con, il breakpoint cade sulla riga giusta. Costa prestazioni: è solo in Debug.
- **Le NOTIFICATION sono filtrate a livello GL.** Su NVIDIA sono un fiume a ogni
  allocazione di buffer e annegherebbero i TRACE degli eventi. Il ramo nella callback
  resta: per riaccenderle si toglie una riga sola.

Non c'è `IGNIS_DEBUGBREAK()` sui messaggi ad alta severità, ed è una scelta: i messaggi
HIGH arrivano anche dai backend ImGui e dal driver, e un trap all'avvio su una sola delle
due macchine sarebbe un falso positivo costoso. La riga è commentata sul posto.

Verificato su **entrambi i bersagli** con `glEnable(GL_TEXTURE_2D)` (illegale in core
profile): riga ERROR presente. **Controtest:** commentando la sola
`glDebugMessageCallback(...)` la riga sparisce e l'INFO di avvio resta — una variabile
sola, quindi il test sa fallire.

### Input e keycode

```cpp
if (Input::IsKeyPressed(KeyCode::Escape)) { … }
if (Input::IsMouseButtonPressed(MouseCode::ButtonLeft)) { … }
```

`KeyCodes.h` contiene 121 tasti ed è **generato** dai `#define` di `glfw3.h`, non trascritto
a mano. I valori numerici coincidono con quelli GLFW di proposito, così la conversione è un
cast invece di una tabella da mantenere allineata.

> **Il prezzo di quella scorciatoia sono diciotto `static_assert` in `Input.cpp`**
> (quattordici sui tasti, quattro sui pulsanti del mouse), piazzati sui confini di ogni
> blocco della tabella:
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
IGNIS_EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)
```

> **Perché il bitmasking e non un secondo enum.** Un `KeyPressedEvent` è
> *contemporaneamente* un evento di tastiera e un evento di input. Con un campo singolo
> dovresti scegliere quale delle due verità scrivere; con i bit le tieni entrambe, e un
> filtro "tutto ciò che è input, qualunque cosa sia" diventa un `AND` fra interi.

L'`EventDispatcher` usa i template per confrontare il tipo runtime dell'evento con il tipo
che una funzione sa gestire, e in caso di corrispondenza fa il cast e la chiama. Il flag
`Handled` ferma la propagazione: lo legge `Application::OnEvent` a ogni giro del ciclo a
ritroso sul `LayerStack`, e lo scrive chiunque abbia consumato l'evento — oggi
l'`Application` per `WindowClose`, l'`ImGuiLayer` quando ImGui vuole l'input, e
l'`EditorLayer` per ESC.

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

Inizializza Dear ImGui con i backend GLFW + OpenGL3 e abilita il **docking**. I
**viewport** (pannelli trascinabili fuori come finestre del sistema) sono **disattivati**:
la riga `ViewportsEnable` è commentata sul posto con la motivazione — vedi D14 in ROADMAP e
*Trappole già pagate*. `Begin()` e `End()` delimitano il frame UI dentro il game loop.

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
| **D3** | **GLAD resta versionato** | Non è una libreria che evolve: è codice *generato una volta*, oggi per **GL 4.5 Core** (rigenerato con D8; i parametri di generazione sono in `Ignis/vendor/glad/CMakeLists.txt`). Sta in `Ignis/vendor/glad/` come target separato, così i warning severi dell'engine non lo toccano. |
| **D4** | **Tre target**, non uno | Vedi *Architettura*. `Sandbox` è il test che i confini esistano davvero. |
| **D5** | **C++20**, non 23 | `std::format` esiste su GCC 13 e MSVC 2022+. Del C++23 i due compilatori supportano sottoinsiemi diversi: si finirebbe con codice che compila sul portatile e non sulla workstation. |
| **D6** | **ImGui pinnato a un commit** | `docking` è un branch, non un tag: si muove. Un pin significa che fra sei mesi il repo compila ancora con l'ImGui su cui il codice è stato scritto. |
| **D7** | **Astrazione RendererAPI rimandata** | OpenGL diretto dietro classi RAII. L'astrazione multi-API si introduce quando esiste una seconda API vera, non prima. |

**Licenza: non ancora scelta.** Il repo è pubblico ma senza licenza è legalmente
"tutti i diritti riservati": nessuno può riusare il codice né contribuire. Da decidere.

---

## Cosa non funziona ancora

**I bug e i default silenziosi della revisione iniziale sono tutti chiusi.** Quel che resta
è assenza, non rottura — e va detto con precisione:

### Non c'è ancora un renderer

Dal task `13` esiste il `RenderCommand`, che sa impostare il viewport, il colore di sfondo
e pulire lo schermo. Ma **l'engine non sa ancora disegnare niente di suo**: nessuno
`Shader`, nessun `Buffer`, nessun `VertexArray`, nessuna `Texture`. È la Fase 2, ed è il
contenuto dell'iterazione #2.

### Debiti noti, con la loro condizione

**`Input` è legato alla finestra principale.** `Input::IsKeyPressed` interroga sempre
`Application::Get().GetWindow()`. Con una finestra sola è corretto; con più finestre il
polling non vedrebbe i tasti di quella che ha davvero il focus. **Va risolto prima di
riattivare i viewport ImGui (D14), non dopo.**

**Un gioco si porta ImGui nel binario anche spegnendolo.** `EnableImGui = false` toglie il
costo a runtime — contesto, font, `NewFrame`/`Render` — ma i simboli restano linkati
(~1000 in `Sandbox`). Per farli sparire davvero serve separare ImGui a livello di build,
lavoro che ha senso quando ci sarà una pipeline di packaging vera (Fase 5).

**Restano due `gl*` fuori dal perimetro di D17, in `Window.cpp`:** `gladLoadGLLoader` e le
tre `glGetString` che stampano vendor, renderer e versione all'avvio. Non sono disegno —
sono l'inizializzazione del contesto, e stanno dove il contesto nasce. Le lasciamo lì
consapevolmente: spostarle significherebbe inventare un `RendererContext` per tre righe
informative. **È l'unica eccezione nota al grep**, quindi va detta invece di far sembrare
il perimetro più pulito di com'è.

### Impalcatura dichiarata, che aspetta il suo tetto

- **`ApplicationSpecification::CommandLineArgs`** — l'editor logga l'argomento e non lo usa.
  Aspetta il Launcher (Fase 5).
- **`ImGuiLayer::SetBlockEvents`** — il meccanismo c'è, nessuno lo chiama. Aspetta il
  viewport di gioco dell'editor (Fase 4).
- **Il pannello "prova della cattura input"** nell'`EditorLayer` — è il banco di prova con
  cui verifichiamo le funzionalità nuove finché l'editor non avrà pannelli veri.
- **Il blocco `if (ViewportsEnable)` in `ImGuiLayer::End()`** — resta nel codice e non
  scatta. È il codice che servirà intatto se i viewport torneranno: più onesto lasciarlo lì
  che riscriverlo a memoria fra sei mesi.

## Trappole già pagate

**CLion non usa i preset finché non glielo dici, e non te lo dice.** All'apertura del
progetto crea un profilo CMake suo, che builda in `cmake-build-debug/` invece che in
`build/<preset>/`. Tutto funziona, quindi non te ne accorgi: semplicemente stai compilando
in un albero **configurato da CLion e non dal repo**, con la sua toolchain e i suoi flag al
posto di quelli dichiarati in `CMakePresets.json`. Due build tree, due download di
FetchContent, e i preset che documentiamo qui li usa solo chi compila da terminale.

> **È la stessa famiglia di errori dell'albero assemblato da epoche diverse, spostata sulla
> build:** ciò che provi non è ciò che il repo descrive, e la differenza non produce nessun
> messaggio. Se un giorno un flag dei preset (`/utf-8`, `-Wall`, `IGNIS_DEBUG`) facesse la
> differenza, la faresti solo tu e solo su una macchina.

I preset di `CMakePresets.json` vanno abilitati nei profili CMake di CLion, e vale su
**entrambi** i sistemi: la stessa trappola è identica su Windows. `cmake-build-*/` è nel
`.gitignore`, quindi il vecchio albero non è mai finito nel repo — si cancella e basta.

**`APIENTRY` sulla callback di debug non è decorazione.** `GLDEBUGPROC` (glad.h, riga 751)
la dichiara `APIENTRY`, che su Windows vale `__stdcall` e su Linux è vuoto (righe 654-658).
Su GCC la si può dimenticare senza accorgersene; MSVC rifiuta la conversione. È la classica
divergenza che compila su una macchina e non sull'altra — vale per **ogni** callback passata
a una libreria C, non solo per questa.

**Il testo dei messaggi di debug lo scrive il driver, non noi.** Mesa cita la chiamata
colpevole (`GL_INVALID_ENUM in glEnable(GL_TEXTURE_2D)`), NVIDIA usa una formulazione
propria, e anche l'`id` numerico differisce. Un test che cercasse una stringa precisa
passerebbe su una macchina e fallirebbe sull'altra: si verifica che **compaia una riga
ERROR di tipo `ERRORE` da sorgente `[API]`**, non il suo contenuto.

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

**`imgui.ini` nasce dove lanci l'eseguibile.** ImGui salva lì il layout dei pannelli —
posizione, dimensione, struttura del docking — riscrivendolo ogni 5 secondi quando cambia.
È il motivo per cui l'editor ritrova i pannelli dove li avevi lasciati, ed è nel
`.gitignore`. Il percorso di default è **relativo alla directory di lavoro**, e il sorgente
di ImGui avverte: *"most apps will want to lock this to an absolute path"*.

> **Con `ApplicationSpecification::WorkingDirectory` questo diventerà un problema.** Il
> giorno che il Launcher aprirà un progetto spostando la cwd, il layout dell'editor verrebbe
> salvato **dentro la cartella del progetto** — uno diverso per progetto, con i pannelli che
> cambiano posizione a seconda di cosa apri. Cura prevista alla Fase 5. Un gioco che usasse
> ImGui per il debug può disattivare del tutto il salvataggio con `io.IniFilename = nullptr`.

Il Sandbox non lo crea più: con `EnableImGui = false` non esiste nessun contesto ImGui.

**I viewport ImGui non funzionano su Linux, e il sintomo non punta alla causa.** Con
`ImGuiConfigFlags_ViewportsEnable` un pannello trascinato fuori dalla finestra diventa una
finestra del sistema operativo — e su Linux non riceve il focus della **tastiera**, mentre
il mouse continua a funzionare perché ImGui ne traccia la posizione globalmente. Il sintomo
osservato: **un campo di testo staccato si attiva col clic e poi non scrive**, come fosse in
sola lettura. Dipende dal window manager ed è un problema aperto a monte
([ocornut/imgui #2117](https://github.com/ocornut/imgui/issues/2117)).

Lo stesso scenario espone un limite nostro: `Input::IsKeyPressed` interroga sempre la
finestra principale, quindi con il focus su un pannello staccato **ESC non chiudeva
l'applicazione**. Due sintomi diversi, una sola causa scatenante.

I viewport sono quindi **disattivati** (vedi D14 in ROADMAP); il docking resta attivo.
Il modo in cui la causa è stata isolata vale come metodo: commentare **una sola riga**
(`ViewportsEnable`) e riprovare gli stessi test. Entrambi i sintomi sono spariti insieme —
il che li ha legati a una causa sola, invece di lasciarli due bug distinti da inseguire.

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
