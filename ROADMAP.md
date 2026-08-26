# Ignis Engine — Roadmap

> Documento di lavoro. Aggiornato: 2026-08-26 (apertura iterazione #2).
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
| D3 | GLAD | Resta committato in `vendor/glad/` | Non è una libreria che evolve: è codice *generato una volta*. Va versionato come se fosse proprio, annotando i parametri di generazione. **Rigenerato per GL 4.5 Core il 2026-08-25 con D8** — questa riga diceva ancora 3.3 fino al 2026-08-26. |
| D4 | Struttura repo | Tre target: `Ignis` (lib statica), `IgnisEditor` (exe), `Sandbox` (exe) | Confini fisici invece che convenzionali. `Sandbox` prova che l'engine è linkabile dall'esterno: in un target unico gli errori di accoppiamento restano invisibili perché tanto compila lo stesso. |
| D5 | Standard | C++20 (non 23) | `std::format` è disponibile sia su GCC 13 sia su MSVC 2022+. C++23 è supportato a macchie diverse dai due compilatori: rischio di codice che compila su una macchina e non sull'altra. |
| D6 | Precompiled header | Sì, da subito (`ignispch.h`) | ImGui e GLFW sono decine di migliaia di righe di header ricompilate per ogni `.cpp`. Introdurlo ora costa dieci minuti; a Fase 3 significa toccare cinquanta file. |
| D7 | Astrazione RendererAPI | Rimandata | OpenGL diretto in Fase 2, dietro classi RAII. L'astrazione multi-API si introduce quando esiste una seconda API vera, non prima. Vulkan non è "un'altra implementazione dietro la stessa interfaccia": command buffer, render pass e sincronizzazione esplicita cambierebbero l'interfaccia stessa. Le astrazioni multi-API scritte prima di conoscere la seconda API si buttano quasi sempre. |
| D8 | **OpenGL 4.5 Core con DSA** (era 3.3) | Deciso il 2026-08-25 | Direct State Access elimina il modello *bind-to-edit*: `glNamedBufferData(id, …)` invece di `glBindBuffer` + `glBufferData`. Meno stato globale, meno bug da stato sporco, e un modello concettualmente più vicino a Vulkan/DX12. 4.5 e non 4.6 perché DSA è già completo in 4.5 e copre GPU dal 2014 invece che dal 2017 — utile se altri indie useranno l'engine. **Costo accettato: macOS resta fuori** (Apple è ferma a 4.1 e ha deprecato OpenGL nel 2018). |
| D9 | Niente Vulkan, per ora | Deciso il 2026-08-25 | I concetti che l'engine deve insegnare — batching, scene graph, ECS, framebuffer, materiali — sono **identici** nelle due API. Vulkan aggiunge concetti di *driver* (swapchain, descriptor set, sincronizzazione) e circa mille righe prima del primo triangolo. In più richiede l'SDK LunarG installato a mano, contro la decisione D2. Ciò che sopravvivrebbe a un'eventuale migrazione — scena, ECS, asset, editor — è esattamente ciò che stiamo per costruire. |
| D10 | **Campo d'azione: Linux e Windows desktop** | Deciso il 2026-08-25 | Fuori scope: macOS, Web/WebGL, console, mobile. Il web in particolare è **incompatibile con D8**: WebGL 2 è OpenGL ES 3.0 e DSA lì non esiste. Meglio un territorio piccolo, attuabile e testabile su due macchine vere che una compatibilità dichiarata e mai verificata. Steam Deck è Linux, quindi il grosso del gaming indie è coperto. Espansioni future (es. export Android) come **moduli**, se e quando serviranno. |
| D11 | **Nessuna chiamata OpenGL fuori dalle classi wrapper** | Deciso il 2026-08-25 | Tutto GL vive dentro `Shader`, `Buffer`, `Texture`, `VertexArray`, `Framebuffer`. Non è l'astrazione multi-API di D7 — è più modesto e più utile: se un giorno servisse un backend diverso, si riscrivono cinque file invece di cercare `gl*` in tutto il progetto. Costa nulla oggi perché è comunque il modo pulito di scrivere il renderer. |
| D12 | **Ogni libreria di terze parti sta dietro un'interfaccia dell'engine** | Deciso il 2026-08-25 | Generalizzazione di ciò che il task `05c` ha fatto con GLFW: i client non devono sapere quale libreria di finestre, audio, fisica o rete c'è sotto. Due effetti: le librerie diventano sostituibili, e l'API di Ignis resta piccola. |

| D13 | **Data-driven come forma mentis, non come slogan** | Deciso il 2026-08-25 | Il modello è quello sperimentato su Unreal: **il codice definisce i verbi, i dati definiscono tutto il resto**. C++ (e domani Vesta) dice *cosa un sistema sa fare*; i file dicono *quali entità esistono, con quali componenti, con quali valori*. Aggiungere contenuto non deve richiedere una ricompilazione, e una scena è un file, non codice. |

| D14 | **Viewport ImGui disattivati; docking attivo** | Deciso il 2026-08-25 | I viewport (pannelli trascinabili fuori come finestre del sistema) si rompono su Linux in modo dipendente dal window manager: le finestre senza decorazioni che ImGui crea non ricevono il focus della **tastiera** — il mouse sì. Problema noto e aperto a monte (ocornut/imgui #2117, thread dedicato ai WM Linux; il maintainer: *"I am not a Linux user"*). Linux è il sistema di sviluppo primario: una feature che si rompe lì non ha posto. Il **docking**, che è la parte utile, resta attivo. Da rivalutare alla Fase 4. |

| D15 | **I percorsi degli asset si risolvono rispetto all'ESEGUIBILE, non alla cwd** | Deciso il 2026-08-26 | D13 vieta i percorsi hardcodati, quindi gli shader sono file `.glsl`, non stringhe dentro un `.cpp` che si buttano al primo task. Ma la cwd non è un ancoraggio affidabile: `ApplicationSpecification::WorkingDirectory` la sposta già oggi, e il Launcher della Fase 5 la sposterà sempre. La forma minima è **una funzione sola** che risolve un percorso relativo rispetto alla cartella dell'eseguibile. Costa dieci righe adesso ed è esattamente la giuntura su cui monterà il VFS in Fase 5: chi chiama non cambia, cambia solo cosa c'è sotto. Portabilità: la cartella dell'eseguibile si ottiene da `argv[0]` — che non è affidabile su tutti i sistemi — oppure dalle API native (`/proc/self/exe` su Linux, `GetModuleFileNameW` su Windows). **Va isolata in un punto solo**, come `localtime_r`/`localtime_s` in `Logger.cpp`. |

| D16 | **Il primo client del Renderer 2D è il `Sandbox`, non l'editor** | Deciso il 2026-08-26 | Il quad si disegna nel `Sandbox`. Se il renderer fosse usabile solo dall'editor avremmo sbagliato l'API pubblica senza accorgercene, ed è esattamente il motivo per cui `Sandbox` esiste (D4). L'editor prenderà la scena dentro un framebuffer alla Fase 4: è un caso d'uso in più, non il primo. |

| D17 | **Il perimetro di D11 è il modulo Renderer, non solo le cinque classi** | Deciso il 2026-08-26 | D11 elenca `Shader`, `Buffer`, `Texture`, `VertexArray`, `Framebuffer` — cioè le classi che possiedono una *risorsa*. Ma due punti del renderer chiamano GL senza possedere niente: il **debug output** (`glDebugMessageCallback`, task `12`) e il **RenderCommand** (`glViewport`, `glClear`, `glDrawElements`, task `13`). Non sono un'eccezione a D11, sono la sua lettura corretta: la regola vera è *nessuna chiamata GL fuori da `Ignis/Private/Ignis/Renderer/`*. Fuori da lì — `Application`, `Window`, i layer, i client — `gl*` resta vietato, e oggi le tre chiamate in `Application.cpp` sono l'unica violazione aperta. |

### Decisioni rimandate

- **`imgui.ini` è relativo alla directory di lavoro** — ImGui salva lì il layout dei
  pannelli (posizione, dimensione, struttura del docking), riscrivendolo ogni 5 secondi
  quando cambia. Il default `IniFilename = "imgui.ini"` è **relativo alla cwd**, e il
  sorgente di ImGui stesso avverte: *"most apps will want to lock this to an absolute path"*.
  Oggi è innocuo — il file nasce accanto all'eseguibile ed è nel `.gitignore`. **Diventa un
  problema con il Launcher (Fase 5)**: cambiando cwd per aprire un progetto, il layout
  dell'editor finirebbe *dentro la cartella del progetto dell'utente*, uno diverso per
  progetto, con i pannelli che si spostano a seconda di cosa apri. Cura: puntare
  `io.IniFilename` a un percorso stabile (accanto all'eseguibile o nella configurazione
  dell'editor) **quando ci sarà il Launcher**, non prima. Per un gioco che usasse ImGui per
  il debug, `io.IniFilename = nullptr` disattiva del tutto il salvataggio.

- **`Input` legato alla finestra principale** — `Input::IsKeyPressed` interroga sempre
  `Application::Get().GetWindow()`. Con una finestra sola è corretto; con più finestre
  (viewport ImGui, o un futuro multi-window) il polling non vede i tasti di quella che ha
  davvero il focus. È il motivo per cui ESC non chiudeva l'app quando il focus era su un
  pannello staccato. **Va risolto PRIMA di riattivare i viewport (D14), non dopo**:
  altrimenti si riattiva una feature sopra un limite noto.

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

### Fase 0 — Revisione delle fondamenta — **CHIUSA il 2026-08-25**
Non aggiunge una singola feature. Salda i debiti finché il progetto è embrionale e
costano poco: ristrutturazione dell'albero, build a tre target portabile, correzione dei
bug di ciclo di vita, pulizia di Logger ed Event system, README riscritto.

### Fase 1 — Il guscio dell'applicazione — **CHIUSA il 2026-08-25**
LayerStack, propagazione degli eventi a ritroso con `Handled`, ImGuiLayer come layer vero
con cattura dell'input, Timestep nel game loop, `ApplicationSpecification` con argomenti
da riga di comando. Da qui in poi ogni sistema nuovo ha un posto dove nascere.

### Fase 2 — Renderer 2D *(prossima: iterazione #2)*
Astrazione di OpenGL **4.5 con DSA** a strati sottili: `Shader`, `VertexBuffer`/`IndexBuffer`,
`BufferLayout`, `VertexArray`, `Texture2D`. Tutte RAII e **non copiabili** — copy
`= delete`, move implementato: il doppio `glDelete*` su un ID condiviso è il bug classico
di questa fase, e nasce proprio da una classe che libera una risorsa GPU nel distruttore
ma resta copiabile per default. Poi `Renderer2D` con batching e `OrthographicCamera`.

*Verifiche: un quad colorato a schermo; poi mille quad in una sola draw call.*

### Fase 3 — Scena ed ECS
Entità e componenti (`Transform`, `SpriteRenderer`, `Camera`, `Tag`), `Scene` che li
aggiorna e li disegna, serializzazione su file di testo. Qui l'engine smette di essere
una demo e comincia a contenere qualcosa.

> **Cosa impone D13 all'ECS, e perché va saputo prima di scriverlo.**
> Perché il data-driven funzioni davvero, un componente deve essere tre cose insieme:
> 1. **dato puro** — nessuna logica dentro; la logica sta nei sistemi;
> 2. **serializzabile** — scrivibile e rileggibile da file;
> 3. **ispezionabile senza codice UI scritto a mano** — è la reflection di Vesta a generare
>    il pannello dell'Inspector.
>
> Il terzo punto è il discrimine vero. **Se aggiungere un componente costasse mezz'ora di
> codice per disegnarne il pannello, il data-driven morirebbe per attrito** e si finirebbe a
> scrivere tutto in C++ — cioè l'opposto dell'obiettivo.
>
> **Nota la convergenza:** serializzabilità e reflection servono *contemporaneamente*
> all'editor (Inspector), al salvataggio, al networking (D-networking) e al replay. Un solo
> meccanismo, quattro benefici — ed è la ragione per cui vale la pena farlo bene.
>
> **LA decisione della Fase 3, da prendere allora ma da avere in testa ora:** componenti
> come dato puro con la logica nei *sistemi* (ECS in senso stretto), oppure componenti che
> contengono anche comportamento (il modello `MonoBehaviour` di Unity, o i nodi di Godot).
> Il primo è più adatto al networking e alla serializzazione; il secondo è più immediato per
> chi arriva da Unity. **Non è una scelta di implementazione: è la forma dell'engine.**

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

## Task — stato

### Iterazione #1 — CHIUSA (2026-08-25)

Tutti verificati su **entrambi** i bersagli, Linux/GCC e Windows/MSVC, da CLion.

| # | Task | Esito |
|---|---|---|
| `01` | Ristrutturazione albero e build a tre target | albero Public/Private, FetchContent, preset |
| `02` | Logger | livelli, canali engine/client, colori, **formato verificato dal compilatore** |
| `03` | PCH e Core defines | `IGNIS_ASSERT`/`IGNIS_VERIFY`, PCH (**−39%** sulla ricompilazione) |
| `04` | Ciclo di vita GLFW | RAII, error callback, **ordine corretto per costruzione** |
| `05a` | Migrazione a OpenGL 4.5 Core | GLAD rigenerato, DSA disponibile |
| `05b` | Window | `WindowProps`, viewport che segue il resize, VSync, eventi completi |
| `05c` | GLFW fuori dall'API pubblica | keycode dell'engine generati da GLFW, ~920 KB via dagli header |
| `06` | Pulizia Event system | `enum class`, macro, tre `EventType` fantasma rimossi |
| `07` | README come memoria di lavoro | aggiornato a ogni task, non solo alla fine |
| `08` | LayerStack | propagazione a ritroso, `Handled` che finalmente serve |
| `09` | ImGuiLayer come overlay | cattura input; **viewport disattivati** (D14) |
| `10` | Timestep | `steady_clock`, tetto a 0,1 s verificato con `SIGSTOP` |
| `11` | ApplicationSpecification | `EnableImGui`, `WorkingDirectory`, argomenti; ESC e dockspace al client |

**Due regressioni introdotte e corrette durante l'iterazione**, entrambe dallo stesso
meccanismo — ricostruire un file da una copia vecchia:

1. `Application.h` da una copia pre-`05b` → **il compilatore l'ha preso subito**, danno zero.
2. `Ignis/CMakeLists.txt` da una copia pre-`03` → persi `IGNIS_DEBUG` e il PCH. **Nessun
   sintomo per tre task**: assert disattivati e log di traccia spariti, con la build verde.
   Ora `Base.h` contiene un `#error` che rende impossibile ripetere l'omissione in silenzio,
   e il configure dichiara il PCH.

### Revisione strutturale di fine iterazione #1

Controllo dell'albero Public/Private fatto alla chiusura, su segnalazione. Esito:

- **Nessuna violazione del confine**: nessun header pubblico include un header privato, e
  tutti gli include del progetto usano il prefisso `Ignis/` — il vocabolario unico regge.
- **`Private/Ignis/Core/GLFWContext.h` è al posto giusto**: è privato di proposito (D-04) e
  lo includono solo file dell'engine.
- **`Private/ignispch.h` sta alla radice di Private**, fuori dalla struttura `Ignis/...`:
  non è codice dell'engine ma un artefatto di build, nessuno lo include a mano (lo forza
  CMake). Ora è documentato nel file stesso.
- **Corretti due include superflui in `Application.h`** (header pubblico):
  `Timestep.h` non serviva affatto, e `ImGuiLayer.h` serviva solo per un puntatore — ora è
  una dichiarazione in avanti, con l'include vero spostato in `Application.cpp`. Verificato
  che un client che include `Ignis.h` **non veda più ImGui**.
- **Aggiunta una guardia in CMake**: `Public/` e `Private/` mappano entrambe su `Ignis/...`,
  quindi un file con lo stesso percorso relativo nelle due zone sarebbe ambiguo e vincerebbe
  in silenzio quello pubblico. Ora la configure fallisce con un messaggio esplicito.
  *Verificata creando apposta un omonimo.*

### Iterazione #2 — APERTA (2026-08-26)

Fase 2, il Renderer 2D. Task da `12` a `21`. **Il contenuto dell'iterazione arriva fino al
`20` incluso** (batching con più texture); il `21` è la chiusura. Verifica finale
dell'iterazione: **mille quad in una sola draw call, dimostrato da un contatore, non a
occhio.**

| # | Task | Verifica |
|---|---|---|
| `12` | Contesto di debug OpenGL, `glDebugMessageCallback` nel Logger | una chiamata GL illegale produce una riga ERROR col messaggio del driver; **controtest**: senza callback, silenzio |
| `13` | `RenderCommand` + `Renderer`; le tre `gl*` escono da `Application.cpp` | zero `gl*` in `Application.cpp`; il colore di sfondo si cambia da una riga del client |
| `14` | `Shader` (RAII, copy `= delete`, move; uniform via `glProgramUniform*`) | shader con errore di sintassi → messaggio del driver e fallimento rumoroso; **controtest**: shader valido passa |
| `15` | `VertexBuffer`, `IndexBuffer`, `BufferLayout` (`glCreateBuffers`, `glNamedBufferStorage`) | stride e offset calcolati dal layout uguali a valori attesi **distinti** (tipi diversi, niente 4/4/4) |
| `16` | `VertexArray` (`glVertexArrayAttribFormat` / `AttribBinding`) | **un quad colorato a schermo** |
| `17` | `OrthographicCamera` — primo uso vero di GLM | il quad resta quadrato al resize; muovendo la camera si sposta in senso **opposto** |
| `18` | `Texture2D` + `stb_image` | quad texturato; **controtest**: file mancante → errore esplicito, non una texture bianca |
| `19` | `Renderer2D` con batching + `Renderer2D::Stats` | 1000 quad → `DrawCalls == 1`; **controtest**: con limite 20000, disegnarne 20001 → `DrawCalls == 2` |
| `20` | Slot di texture nel batch (32 sampler, indice per vertice) | due texture diverse e due tinte diverse in **una** draw call |
| `21` | Chiusura: pulizia contestuale, README, ROADMAP, commit | — |

Decisioni prese all'apertura: **D15** (percorsi relativi all'eseguibile), **D16** (il quad
lo disegna il `Sandbox`), **D17** (perimetro di D11).

**Debito saldato in apertura:** il README diceva il falso in sei punti (OpenGL 3.3 in tre
posti fra cui D3, un blocco `### Window` e uno `### Input` rimasti da prima dei task `05b`
e `05c`, il flag `Handled` descritto come "non lo legge nessuno" quando il LayerStack c'è
dal task `08`, i viewport ImGui dati per attivi quando D14 li ha spenti, "due chiamate GL"
che erano tre, e uno snippet di `GetCategoryFlags` con la firma pre-task-`06`). Corretti
tutti prima di scrivere una riga di codice. **D3 in questo file diceva ancora 3.3**: era
stata contraddetta da D8 e nessuno l'aveva aggiornata.

## Sistemi da integrare

Valutazioni fatte il 2026-08-25, **da riverificare al momento dell'uso**: le librerie si
muovono, e una raccomandazione di un anno fa è un'ipotesi, non un fatto.
Vale per tutte la decisione **D12**: stanno dietro un'interfaccia nostra.

### Audio — Fase 4/5

**miniaudio** è la raccomandazione. Single-header, dominio pubblico, zero dipendenze, e si
porta dietro i backend nativi di ogni sistema (WASAPI su Windows, ALSA/PulseAudio su Linux).
Ha due livelli: uno basso per il device, e `ma_engine` di alto livello con **audio spaziale
3D già dentro** — che copre "basilare ma funzionale" per entrambi i target 2D e 3D.

Alternative: **SoLoud** (più alto livello, voci e fade pronti, ma una dipendenza da
compilare in più); **OpenAL Soft** (standard per l'audio posizionale, API vecchia e più
cerimonia); FMOD/Wwise (commerciali, fuori scala).

> Il pezzo difficile dell'audio non è farlo suonare: è il **threading**. Il callback audio
> gira su un thread ad alta priorità dove non si può allocare, non si può prendere un lock e
> non si può loggare — violarlo produce crackle che sembrano bug di altro. Il nostro strato
> deve rendere difficile sbagliarlo.

### Importazione asset — Fase 5

**2D:** `stb_image` per PNG/JPG. Single-header, standard di fatto.

**3D:** qui c'è una decisione architetturale prima che una di libreria.

> **L'importazione appartiene all'EDITOR, non al runtime.** Un gioco spedito non deve saper
> leggere FBX: carica un formato nostro, già ottimizzato. L'editor importa (FBX, OBJ, glTF)
> e converte una volta sola. È la struttura di tutti gli engine seri, ed è coerente con la
> separazione engine/editor che abbiamo già: **l'importer è codice dell'editor**.

Per l'importer: **Assimp** copre ~40 formati incluso FBX ma è pesante; **ufbx** è
single-file e fa solo FBX, molto più leggero. **glTF 2.0** è lo standard aperto moderno
(PBR e animazioni scheletriche nella spec) e si legge con `cgltf`. FBX resta necessario
perché è ciò che esce dai DCC, ma è proprietario e ogni esportatore lo scrive a modo suo.

### Fisica — Fase 3 (2D) e Fase 7 (3D) — **DECISA: Box2D v3 + Box3D**

**2D: Box2D v3.** È la libreria "nuova, moderna ed efficiente": Erin Catto l'ha riscritta da
zero in C con solver SIMD e sub-stepping. Matura e usata.

**3D: Box3D.** Jolt Physics resta l'alternativa se Box3D si rivelasse impraticabile —
multi-thread, deterministica, licenza ZLIB, provata in produzione su Horizon Forbidden West —
ma la scelta è Box3D per coerenza con Box2D.

> **DECISO il 2026-08-25: si usa Box3D, alpha compreso.** Erin Catto l'ha rilasciato il
> 30 giugno 2026 con la stessa filosofia di Box2D — C17, solver SIMD, sub-stepping,
> determinismo cross platform, mondi grandi con posizioni in double. La coerenza vale il
> rischio: **stessa mano, stesso stile di API per il 2D e per il 3D**, invece di due
> librerie che ragionano in modi diversi dentro lo stesso engine.
>
> **Cosa comporta usare software alpha, scritto qui perché non venga scoperto debuggando:**
> 1. **Il commit va pinnato**, come ImGui — e qui conta di più: un'API instabile su un branch
>    mobile fa smettere di compilare il progetto senza che tu abbia toccato niente.
> 2. **"È un bug della libreria" diventa un'ipotesi legittima** quando qualcosa non torna, e
>    va tenuta nella lista delle cause possibili invece che esclusa a priori. Con una libreria
>    matura sarebbe l'ultima ipotesi; qui non lo è.
> 3. **La documentazione è incompleta**: si leggeranno i sorgenti e i test. Per un progetto
>    didattico è un vantaggio, non un costo.
> 4. Un autore attivo su un progetto giovane **corregge i bug segnalati bene**.
>
> L'autore stesso avverte: *"I still consider Box3D to be alpha software"*, *"the engine
> needs more testing and more complete documentation"*. Decisione presa consapevolmente.

### Gamepad — dopo la Fase 1

**Nessuna libreria nuova: GLFW ce l'ha già.** Dalla 3.3 espone un'API gamepad
(`glfwGetGamepadState`, `glfwJoystickPresent`, `glfwSetJoystickCallback`) che normalizza i
controller in un layout stile Xbox usando il database di mappature di SDL, incluso in GLFW.

> **I gamepad si pollano, non generano eventi.** `glfwSetJoystickCallback` avvisa solo di
> connessione e disconnessione; bottoni e assi vanno chiesti ogni frame. Non è un limite di
> GLFW, è come funzionano i gamepad a livello di sistema.

Forma naturale, coerente con quella già in uso per tastiera e mouse:

- **Connessione/disconnessione → eventi** (`GamepadConnectedEvent`,
  `GamepadDisconnectedEvent`): serve per "Controller 2 connesso" e per mettere in pausa
  quando si scollega.
- **Bottoni e assi → polling in `Input`.** Per gli assi analogici è l'unica forma sensata:
  interessa *quanto* è inclinata la levetta adesso, non che è cambiata.
- **Eventi di pressione/rilascio dei bottoni → sintetizzati** confrontando lo stato del
  frame con quello precedente. Servono ai menu, non al gioco: si aggiungono quando ci sarà
  un menu.

**Trappola:** un controller assente dal database non viene riconosciuto come gamepad e
`glfwGetGamepadState` fallisce. Cura: caricare un `gamecontrollerdb.txt` aggiornato con
`glfwUpdateGamepadMappings`. **È un file di dati, non codice** — quindi allineato a D13:
supportare un controller nuovo non deve richiedere una ricompilazione.

### Localizzazione — Fase 4/5

Soluzione **minimale**: file chiave→valore, uno per lingua, caricati a runtime.
Scartate: **gettext** (ottimo tooling con Poedit, ma la libreria C è legata al locale di
sistema e su Windows è scomoda); **ICU** (completo, decine di MB); **Fluent** di Mozilla
(moderno e gestisce plurali e genere, ma le implementazioni C++ sono immature).

Tre regole che separano una localizzazione che regge da una da rifare:

1. **Chiavi simboliche, mai il testo inglese.** `ui.menu.play`, non `"Play"`. Usare
   l'inglese come chiave sembra comodo finché non correggi un refuso nell'originale e
   invalidi ogni traduzione esistente.
2. **Interpolazione con indici posizionali:** `"{0} ha sconfitto {1}"`, non `"{} ha
   sconfitto {}"` — in altre lingue l'ordine si inverte e il traduttore deve poterli
   spostare. **`std::format` supporta già gli indici posizionali**, non serve altro.
3. **Chiave mancante = rumore.** Mostrare `##ui.menu.play##`, non una stringa vuota. È la
   regola dei default silenziosi applicata al testo: una stringa vuota in una UI sembra un
   problema di layout e fa cercare nel posto sbagliato.

**Formato consigliato: CSV.** Sembra povero rispetto a JSON o YAML, ed è esattamente il
punto: **un CSV si apre in Excel, Google Sheets o LibreOffice**, con una colonna per lingua.
Il giorno che chiedi a qualcuno di tradurre il gioco, gli mandi un foglio invece di
insegnargli una sintassi. Si parsa in cinquanta righe.

**Plurali — il pezzo che una tabella chiave→valore non copre.** "1 nemico" / "2 nemici" ha
regole diverse per lingua: l'inglese e l'italiano hanno due forme, il russo tre, l'arabo sei.
La via minimale è una chiave per forma (`enemy.count.one`, `enemy.count.other`) con una
funzione per lingua che sceglie la forma dal numero — le regole stanno nei dati CPLDR, non
nel codice. Da fare quando servirà una lingua slava, non prima: ma **la firma della funzione
di traduzione deve prevedere il conteggio fin dall'inizio**, altrimenti va cambiata ovunque.

> **Il costo vero della localizzazione non è il testo: sono i font.** Cirillico, greco e CJK
> richiedono glyph range e atlas diversi in ImGui, e il cinese da solo sono migliaia di
> glifi. E il tedesco è mediamente il 30% più lungo dell'inglese: ogni pannello che oggi va
> giusto giusto domani trabocca. **Va progettato quando si costruisce il rendering del
> testo, non aggiustato dopo.**

### Networking — Fase 6/7, con un avvertimento anticipato

**ENet** per il trasporto: UDP affidabile, piccolo, collaudato. Alternativa più moderna:
**GameNetworkingSockets** di Valve (crittografia inclusa, più pesante).

> **Il trasporto è la parte facile, e va detto prima e non dopo.** La replica di stato non è
> una libreria: è un design fatto di autorità, interpolazione, riconciliazione e predizione
> lato client. E soprattutto **impone vincoli sull'architettura della scena** — separazione
> netta fra simulazione e presentazione, aggiornamenti deterministici, stato serializzabile.
>
> **CONFERMATO il 2026-08-25: l'ECS della Fase 3 va progettato senza chiudersi le porte
> al networking.** Non si implementa la rete, ma le tre proprietà che la rendono possibile
> vanno rispettate fin dall'inizio, perché aggiungerle dopo significa riscrivere la scena:
> - **separazione netta fra simulazione e presentazione** — lo stato che conta deve poter
>   avanzare senza che nulla venga disegnato;
> - **aggiornamenti deterministici** — stesso stato più stesso input dà stesso risultato,
>   quindi niente dipendenze da frame time variabile nella simulazione;
> - **stato serializzabile** — ogni componente deve poter essere scritto e riletto.
>
> Sono tre proprietà che rendono l'ECS migliore comunque, anche se la rete non arrivasse
> mai: sono le stesse che servono al salvataggio, al replay e ai test deterministici.
>
> Onestamente: è la voce con il rapporto sforzo/risultato peggiore dell'elenco. Vale la pena
> solo se è un obiettivo vero, non un "sarebbe bello".

### Plugin dell'editor: C++ o Vesta? — decisione rimandata

Oggi `IgnisEditor` ha solo `Private/`, ed è corretto: non esporta niente. Ma il giorno che
esistessero plugin, **la soluzione non è aggiungere `Public/` all'eseguibile**.

> **Un eseguibile non si linka.** Un plugin non può includere gli header di `IgnisEditor` e
> chiamarne le funzioni come farebbe con una libreria: servirebbe esportare i simboli
> dall'eseguibile (`-rdynamic` su Linux, una import library su Windows), che funziona ma è
> la strada fragile.

**Se i plugin saranno C++**, la struttura è lo split in due target:

```
IgnisEditorCore    libreria — Public/ (API dei plugin) + Private/
IgnisEditor        eseguibile — un guscio che la avvia
```

I plugin linkano la libreria, non l'eseguibile. È come è fatto Unreal: l'editor è una
collezione di moduli, l'eseguibile è quasi vuoto.

**Preparazione a costo zero, valida da subito:** tenere `EditorApp.cpp` minimale. Oggi sono
quaranta righe; il giorno dello split il codice si sposta nella libreria e il guscio resta
com'è. **Spaccare adesso** significherebbe inventare un'API per plugin che non esistono,
senza sapere cosa vorranno fare.

**Ma la strada più probabile è un'altra: plugin come script Vesta.** Un plugin C++ caricato
dinamicamente impone tutto il campo minato dell'ABI (allocatori diversi ai due lati della
DLL, RTTI che non si riconosce, eccezioni che non attraversano) e obbliga a **congelare
l'API** proprio mentre l'engine cambia forma. Un plugin in AngelScript non ha nessuno di
quei problemi: niente ABI, niente ricompilazione, e chi lo scrive usa **lo stesso linguaggio
che già impara per fare i giochi** — molto più accessibile, per la fascia "super indie", di
"compilati una DLL con le stesse flag dell'editor".

**Se i plugin sono script, all'editor non serve mai una `Public/` in C++**: gli serve una
superficie di scripting, che vive dentro Vesta (Fase 6).

**Decisione da prendere quando arriverà il primo plugin vero**, non prima.

### Moduli e plugin dell'engine — molto più avanti

Vanno distinte due cose che si chiamano quasi allo stesso modo:

- **Modularità interna** (adesso, gratis): confini netti, target separati, D11 e D12. È ciò
  che permette di aggiungere un sottosistema senza rompere gli altri.
- **Plugin caricati dinamicamente** (DLL/`.so`, molto dopo): impongono un'**ABI stabile**, e
  in C++ passare oggetti attraverso il confine di una DLL è un campo minato — allocatori
  diversi ai due lati, RTTI che non si riconosce, eccezioni che non attraversano. Su un
  engine che cambia forma ogni settimana, un'ABI stabile blocca proprio dove serve libertà.

**Il modo migliore di prepararsi ai moduli è tenere puliti i confini interni**, non
predisporre un sistema di plugin che poi vincola.

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
