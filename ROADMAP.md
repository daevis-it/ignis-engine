# Ignis Engine — Roadmap

> Documento di lavoro. Aggiornato: 2026-08-24.
> **La verità sta nel codice.** Questo file è la mappa, non il territorio: quando una
> decisione ci si appoggia, va verificata sui sorgenti.
> Il `README.md` è la memoria operativa (stato per task, trappole pagate); questo file
> è la visione a lungo raggio e il registro delle decisioni.

---

## Contesto e vincoli

Game engine 2D/3D scritto da zero in C++, a scopo didattico e architetturale: capire il
**perché** dei motori moderni costruendone uno, non replicarne le feature. Obiettivo a
valle: una base propria per piccoli giochi indie e per insegnare game dev.

- **Sviluppo primario:** Linux Mint 22.3, GCC, CLion.
- **Secondo bersaglio, non opzionale:** Windows (Ryzen + RTX), Visual Studio 2026 / MSVC.
- **Regola derivata:** una feature che compila solo su un sistema non è finita.

---

## Decisioni prese

| # | Decisione | Scelta | Motivo |
|---|---|---|---|
| D1 | Toolchain Windows | MSVC / Visual Studio 2026 | Compilatore nativo, miglior supporto driver NVIDIA e tooling grafico (RenderDoc, NSight). Più severo di GCC: fa emergere gli errori di portabilità invece di nasconderli. |
| D2 | Dipendenze | CMake FetchContent | GLFW, GLM e ImGui dichiarati con tag git precisi, scaricati e compilati al configure. Repo pulito, build riproducibile su qualsiasi macchina senza installare nulla a mano. |
| D3 | GLAD | Resta committato in `vendor/glad/` | Non è una libreria che evolve: è codice *generato una volta* per GL 3.3 Core. Va versionato come se fosse proprio, annotando i parametri di generazione. |
| D4 | Struttura repo | Tre target: `Ignis` (lib statica), `IgnisEditor` (exe), `Sandbox` (exe) | Confini fisici invece che convenzionali. `Sandbox` prova che l'engine è linkabile dall'esterno: in un target unico gli errori di accoppiamento restano invisibili perché tanto compila lo stesso. |
| D5 | Standard | C++20 (non 23) | `std::format` è disponibile sia su GCC 13 sia su MSVC 2022+. C++23 è supportato a macchie diverse dai due compilatori: rischio di codice che compila su una macchina e non sull'altra. |
| D6 | Precompiled header | Sì, da subito (`ignispch.h`) | ImGui e GLFW sono decine di migliaia di righe di header ricompilate per ogni `.cpp`. Introdurlo ora costa dieci minuti; a Fase 3 significa toccare cinquanta file. |
| D7 | Astrazione RendererAPI | Rimandata | OpenGL diretto in Fase 2, dietro classi RAII. L'astrazione multi-API si introduce quando esiste una seconda API vera, non prima. |

### Decisioni rimandate

- **Licenza** — nessuna per ora. Conseguenza: il repo è pubblico ma legalmente "tutti i
  diritti riservati", quindi nessuno può riusare il codice né contribuire. Da decidere
  prima di dare visibilità al progetto. Candidate: MIT, Apache 2.0, GPL v3.
- **Il linguaggio di Vesta** — l'obiettivo dichiarato è un linguaggio "fra il C++ e la
  comodità del C#", orizzontale sulle logiche di Ignis, nello spirito di GDScript per
  Godot. Le strade: linguaggio proprio da zero (lexer, parser, VM — massimo apprendimento,
  costo paragonabile a quello dell'engine stesso), oppure adottarne uno esistente
  (**AngelScript** ha sintassi quasi identica a C#/C++ ed è nato per i game engine; **Lua**
  è lo standard di fatto; **Wren** è piccolo e leggibile). **La decisione non va presa
  ora**: il Vesta Header Tool serve identico in tutti i casi, quindi si può costruire il
  prerequisito senza scegliere. Riprendere quando la Fase 3 avrà prodotto componenti veri
  da esporre.

- **Generator CMake su Windows** — da verificare con `cmake --help` sulla macchina reale.
  Raccomandazione: **Ninja + `cl.exe`** invece del generator Visual Studio, perché è
  indipendente dalla versione dell'IDE e sensibilmente più veloce. Visual Studio si apre
  per il debugger, non per buildare.

---

## Le sette fasi

### Fase 0 — Revisione delle fondamenta *(in corso)*
Non aggiunge una singola feature. Salda i debiti finché il progetto è embrionale e
costano poco: ristrutturazione dell'albero, build a tre target portabile, correzione dei
bug di ciclo di vita, pulizia di Logger ed Event system, README riscritto.

### Fase 1 — Il guscio dell'applicazione
LayerStack, propagazione degli eventi a ritroso con `Handled`, ImGuiLayer come layer vero
con cattura dell'input, Timestep nel game loop, `ApplicationSpecification` con argomenti
da riga di comando. Da qui in poi ogni sistema nuovo ha un posto dove nascere.

### Fase 2 — Renderer 2D
Astrazione di OpenGL a strati sottili: `Shader`, `VertexBuffer`/`IndexBuffer`,
`BufferLayout`, `VertexArray`, `Texture2D`. Tutte RAII e **non copiabili** — copy
`= delete`, move implementato: il doppio `glDelete*` su un ID condiviso è il bug classico
di questa fase, e nasce proprio da una classe che libera una risorsa GPU nel distruttore
ma resta copiabile per default. Poi `Renderer2D` con batching e `OrthographicCamera`.

*Verifiche: un quad colorato a schermo; poi mille quad in una sola draw call.*

### Fase 3 — Scena ed ECS
Entità e componenti (`Transform`, `SpriteRenderer`, `Camera`, `Tag`), `Scene` che li
aggiorna e li disegna, serializzazione su file di testo. Qui l'engine smette di essere
una demo e comincia a contenere qualcosa.

### Fase 4 — Editor
Rendering della scena dentro un framebuffer mostrato in un pannello ImGui, Scene
Hierarchy, Inspector, gizmo di trasformazione, selezione col mouse. L'editor diventa
usabile.

### Fase 5 — Asset, progetti e Launcher
Virtual File System, asset handle, il concetto di "progetto utente" separato dall'engine.
**Il Launcher / Project Manager vive qui**, ma la sua giuntura è in Fase 1: l'editor deve
accettare un percorso di progetto dagli argomenti (`IgnisEditor /percorso/progetto`). Se
l'avvio resta hardcodato, il giorno del Launcher va riscritto il boot del motore.

### Fase 6 — Vesta
*(già "vii" — rinominata il 2026-08-25. Le "vesta" erano i fiammiferi di cera vittoriani,
chiamati così dalla dea romana custode del fuoco: il nome tiene insieme il fiammifero e
la fiamma di Ignis.)*

Il livello di scripting e reflection dell'engine. Tre mattoni, in quest'ordine:

1. **Vesta Header Tool** — il parser pre-compilazione che legge i decoratori nei nostri
   header e genera il codice di reflection. **È il prerequisito di tutto il resto, e
   soprattutto è indipendente da quale linguaggio di scripting sceglieremo:** serve
   identico che si adotti Lua, AngelScript, o un linguaggio nostro.
2. **Il binding runtime** — esporre le classi C++ riflesse a chi le vuole chiamare.
3. **Il linguaggio** — vedi *decisioni rimandate*.

Realistica **solo dopo la Fase 3**: la reflection senza componenti da riflettere non ha
oggetto, e un linguaggio senza un'API da esporre non ha di che parlare.

### Fase 7 — 3D
Mesh, materiali, luci, camera prospettica.

---

## Task correnti — Fase 0 e Fase 1

Ogni task chiude con una verifica che deve poter **fallire in un solo modo**.

### Fase 0

**`01` — Ristrutturazione albero e build a tre target**
`01a` sposta i file nel nuovo albero (`Ignis/`, `IgnisEditor/`, `Sandbox/`, `vendor/glad/`).
`01b` riscrive `CMakeLists.txt`: tre target, C++20, `cmake_minimum_required` a 3.20+,
GLOB con `CONFIGURE_DEPENDS`.
`01c` aggiunge FetchContent (GLFW, GLM, ImGui) e i `CMakePresets.json` per Linux-GCC e
Windows-MSVC, con le flag `/utf-8` e `/Zc:__cplusplus` su MSVC.
*`01a` da solo non compila: le tre sotto-milestone si committano insieme.*
**Verifica:** `IgnisEditor` e `Sandbox` partono entrambi e aprono una finestra. Su
Windows: clone pulito, configure, build, senza installare nulla a mano.

**`02` — Logger**
Format string sempre letterale, livelli Trace/Info/Warn/Error, macro `IGNIS_*`, colori
ANSI su terminale.
**Verifica:** un evento il cui `ToString()` contiene una graffa viene stampato
letteralmente invece di far lanciare `format_error` a `std::vformat`.

**`03` — PCH e Core defines**
`ignispch.h`, `IGNIS_ASSERT` che passa dal Logger e rompe nel debugger (`__debugbreak()`
su MSVC, `__builtin_trap()` su GCC), macro di piattaforma, tipi base.
**Verifica:** un assert deliberatamente falso ferma il programma nel punto giusto su
entrambi i sistemi. Annotare i tempi di build prima e dopo il PCH.

**`04` — Ciclo di vita GLFW**
Init e terminate spostati fuori da `Application`, `glfwSetErrorCallback` installata,
`gladLoadGLLoader` controllato, ordine di distruzione corretto.
**Verifica:** l'errore GLFW oggi silenzioso alla chiusura diventa visibile, e poi sparisce
quando l'ordine è corretto. Due osservazioni distinte, non una.

**`05` — Window**
`WindowProps`, `glfwSetFramebufferSizeCallback` con `glViewport`, VSync attivabile, eventi
mancanti (focus, moved, char), `repeatCount` che conta davvero.
**Verifica:** con un `glClearColor` non nero, ridimensionare la finestra e vedere il
colore riempire tutta l'area nuova.

**`06` — Event system**
`EventCategory` scoped, rimozione (o emissione) dei sette `EventType` mai usati, macro per
il boilerplate ripetuto, `#include <ostream>` mancante.
**Verifica:** non-regressione dichiarata in anticipo — non deve cambiare niente di visibile.

**`07` — README come memoria di lavoro**
Stato per task, decisioni datate, note aperte, trappole pagate. "Vesta" (ex "vii") spostata
da architettura a visione futura, perché nel codice non esiste ancora una riga.
**Verifica:** rileggerlo fingendo di non ricordare nulla del progetto e capire dove si è.

### Fase 1

**`08` — LayerStack**
`Layer` con `OnAttach`/`OnDetach`/`OnUpdate`/`OnEvent`, stack con overlay separati,
propagazione a ritroso che si ferma su `Handled`.
**Verifica:** due layer con nomi distinti nel log, un evento che il primo consuma e che al
secondo non arriva mai.

**`09` — ImGuiLayer diventa un Layer**
Smette di essere membro di `Application`, entra come overlay, consulta
`WantCaptureKeyboard` / `WantCaptureMouse` per bloccare l'input al gioco.
**Verifica:** scrivere in un `InputText` di ImGui mentre un layer di gioco logga i tasti —
il gioco non deve vedere niente.

**`10` — Timestep**
`Timestep` nel loop, `OnUpdate(ts)` su tutti i layer, VSync attivo.
**Verifica:** un layer che stampa il delta time. Con VSync circa 16.6 ms, senza molto
meno: due numeri distinguibili, non "sembra fluido".

**`11` — ApplicationSpecification**
Nome, dimensioni, working directory e `CommandLineArgs` — la giuntura per il Launcher.
**Verifica:** `IgnisEditor pippo.ignis` logga l'argomento ricevuto. Non fa nulla con quel
percorso: è impalcatura, non arredo.

---

## Stato all'inizio della Fase 0

Base più solida di quanto il README lasciasse intendere: eventi con bitmasking, dispatcher
template, entry point invertito con factory sono scelte corrette. I problemi sono altrove.

**Bug reali**

1. `glfwDestroyWindow` viene chiamato **dopo** `glfwTerminate`: `~Application` termina GLFW
   nel corpo, poi `~Window` (membro, distrutto successivamente) prova a distruggere una
   finestra già liberata. Nessuna `glfwSetErrorCallback` installata, quindi l'errore è
   invisibile.
2. `glViewport` non compare nel progetto: il resize non aggiorna il viewport OpenGL. Inoltre
   si usa `glfwSetWindowSizeCallback` (screen coordinates) invece di
   `glfwSetFramebufferSizeCallback` (pixel), e i due valori divergono su schermi HiDPI.
3. `Logger::Info(e.ToString())` passa una stringa costruita a runtime come *format string*:
   il primo evento che contiene `{` o `}` fa lanciare `format_error` a `std::vformat`.

**Default silenziosi**

- `glfwInit()` fallito viene loggato, poi si prosegue comunque a creare la finestra.
- `gladLoadGLLoader` non è controllato: se fallisce, ogni puntatore a funzione GL è nullo e
  la prima chiamata segfaulta senza spiegazione.
- `Window::Window` in caso di errore fa `return`, lasciando l'oggetto in stato invalido.

**Buchi architetturali**

- Nessun LayerStack: `ImGuiLayer` è un membro concreto di `Application`, `OnEvent` non
  instrada nulla, il flag `Handled` esiste e non lo legge nessuno.
- `io.WantCaptureKeyboard` / `WantCaptureMouse` non vengono mai consultati: scrivendo in un
  campo ImGui, il gioco riceve gli stessi tasti.
- Nessun timestep e nessuna `glfwSwapInterval`: il loop gira libero bruciando CPU e GPU.
- Build monolitica, con engine, editor e vendor in un solo eseguibile e `EditorApp.cpp`
  fisicamente dentro `src/Ignis/`. La separazione dichiarata nel README non esiste nella build.
- `find_package(glfw3 REQUIRED)` rende il progetto buildabile solo su Linux con
  `libglfw3-dev` installato.

**Minori**

- `EventType` dichiara sette valori che nessuno emette (`WindowFocus`, `WindowLostFocus`,
  `WindowMoved`, `KeyTyped`, `AppTick`, `AppUpdate`, `AppRender`).
- `EventCategory` è un enum non-scoped: `Ignis::None` è un simbolo nudo nel namespace.
- `repeatCount` è hardcodato a 0 o 1, non conta le ripetizioni reali.
- `Event.h` usa `std::ostream` senza includere `<ostream>`.
- `cmake_minimum_required(VERSION 3.10)` con CMake 4.x: compatibilità deprecata.
- `GLOB` senza `CONFIGURE_DEPENDS`, che è la trappola già annotata nel troubleshooting.
