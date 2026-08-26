#pragma once

// ══════════════════════════════════════════════════════════════════════════════
//  Debug output di OpenGL — la GPU che dice cosa ha fatto, invece di tacere.
//
//  Senza questo, un errore GL si manifesta come schermo nero o come niente: la
//  chiamata sbagliata ritorna, il programma prosegue, e il sintomo arriva fasi
//  dopo. È il default silenzioso peggiore del progetto, perché non è nostro — sta
//  dentro il driver.
//
//  Vive in Private/Ignis/Renderer/ e non fra le classi wrapper: non possiede una
//  risorsa, quindi non è né uno Shader né un Buffer. È la lettura corretta di D11
//  scritta in D17: nessuna chiamata gl* fuori da Ignis/Private/Ignis/Renderer/.
//
//  L'header è PRIVATO di proposito. Un client non deve poter accendere o spegnere
//  la diagnostica del motore: è una scelta della build, non dell'applicazione.
// ══════════════════════════════════════════════════════════════════════════════

namespace Ignis
{
    // Da chiamare UNA VOLTA, subito dopo che GLAD ha caricato i puntatori e con il
    // contesto corrente. La chiama Window::Window; nessun altro deve chiamarla.
    //
    // In Debug: verifica di aver ottenuto davvero un contesto di debug, registra la
    // callback e accende l'output sincrono.
    // In Release: non fa nulla, e lo dice.
    //
    // NON lancia: un driver che non concede il contesto di debug è una condizione
    // del mondo reale che non impedisce di lavorare. Si degrada rumorosamente
    // (WARN), non in silenzio.
    void InitGLDebugOutput();
}
