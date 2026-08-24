# Ignis Engine

**Ignis Engine** è un game engine 2D/3D personalizzato scritto in C++.
Questo progetto nasce con un obiettivo didattico e architetturale: comprendere a fondo le logiche dei motori grafici moderni (come Unity e Unreal Engine) costruendone uno da zero, partendo da un framework 2D per poi espandersi al 3D, includendo un Editor visivo integrato.

---

## 🛠️ Stack Tecnologico e Ambiente

*   **Linguaggio:** C++20 (Sfruttando funzionalità moderne come `std::format`).
*   **Sistema Operativo:** Linux (Sviluppato su Linux Mint 22.3).
*   **IDE:** JetBrains CLion.
*   **Build System:** CMake (Minimo 3.10).
*   **Sistema a Finestre e Input:** GLFW 3.
*   **Grafica (API):** OpenGL 3.3 Core Profile.
*   **OpenGL Loader:** GLAD (Generato per C/C++, OpenGL 3.3, Core profile).
*   **Interfaccia Utente (GUI):** Dear ImGui (Ramo `docking` per supportare i Viewport).

---

## 🏗️ Architettura del Progetto

Il progetto segue il pattern classico dei motori grafici, basato su una forte separazione delle responsabilità.

### Struttura delle Directory

*   `src/`: Contiene tutti i file sorgente (`.cpp`, `.c`).
*   `include/`: Contiene tutti gli header file (`.h`) e le librerie esterne (`glad`, `KHR`, `imgui`).

### Core Components (Attuali)

1.  **Application (`Application.h` / `.cpp`)**
    *   Il cuore pulsante del motore. Mantiene in vita il processo tramite il **Game Loop**.
    *   Usa il pattern **Singleton** per rendere accessibile l'istanza globale da qualsiasi punto.
2.  **Window (`Window.h` / `.cpp`)**
    *   Incapsula GLFW, crea la finestra nativa e imposta il contesto OpenGL. Inizializza GLAD.
3.  **Input (`Input.h` / `.cpp`)**
    *   Sistema di input statico e disaccoppiato. Permette di leggere tastiera e mouse ovunque, interrogando direttamente GLFW attraverso l'istanza di `Application`.
4.  **Logger (`Logger.h`)**
    *   Sistema di logging custom basato su `std::format` (C++20) per stampare messaggi formattati nel terminale, sostituendo il classico `std::cout`.
5.  **ImGuiLayer (`ImGuiLayer.h` / `.cpp`)**
    *   Gestisce l'inizializzazione e il rendering dell'interfaccia utente (Dear ImGui). Abilita funzionalità avanzate come Docking e Multi-Viewport, permettendo di trascinare le finestre dell'Editor fuori dalla finestra principale.

---

## 📜 Log delle Scelte Implementative

1.  **Standard C++20:** Adozione dello standard C++20 per garantire un codice più pulito ed espressivo, in linea con gli standard dell'industria AAA.
2.  **OOP vs Procedurale:** Abbandonato il singolo file `main.cpp` procedurale a favore di una struttura a oggetti (`Application`, `Window`, `Layer`).
3.  **Input Disaccoppiato:** Stile di input globale (es. `Input::IsKeyPressed()`) simile all'approccio di Unity.

---

## ⚠️ Errori Comuni e Troubleshooting

*   **Libreria GLFW su Linux:** In caso di errori CMake su `glfw3`, installare i pacchetti dev (`sudo apt install libglfw3-dev`).
*   **Ordine di Inclusione GLAD:** L'header `<glad/glad.h>` deve sempre precedere `<GLFW/glfw3.h>`.
*   **Gestione File di ImGui:** Per il ramo *docking* di ImGui con OpenGL3, è fondamentale includere anche il file `imgui_impl_opengl3_loader.h` nella cartella di inclusione.
*   **Cache di CMake (File Oggetto Sporchi):** Se si sostituiscono file fisici nel progetto (es. aggiornamento di una libreria come ImGui) mantenendo gli stessi nomi, CMake potrebbe usare i vecchi file compilati in memoria (`.o`). **Soluzione:** Eseguire sempre un "Clean Project" o eliminare manualmente la cartella `cmake-build-debug` prima di ricompilare.

---

## 🚀 Prossimi Passi

1.  **Event System:** Implementazione di un sistema di eventi (Window Close, Key Press, Mouse Move) per gestire la comunicazione tra i moduli in modo reattivo (Observer pattern/Dispatching).
2.  **Rendering 2D:** Astrazione delle API di OpenGL (Shader, Vertex Array, Buffer) per il rendering grafico.