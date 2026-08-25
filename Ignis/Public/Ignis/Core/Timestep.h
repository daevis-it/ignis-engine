#pragma once

namespace Ignis
{
    // Il tempo trascorso dal frame precedente.
    //
    // Esiste come TIPO e non come float nudo per una ragione precisa: rende esplicita
    // l'unità di misura. "Questo float è in secondi o millisecondi?" è la domanda
    // dietro metà dei bug di movimento che vanno mille volte troppo veloce — con
    // GetSeconds() e GetMilliseconds() la domanda non si pone.
    class Timestep
    {
    public:
        constexpr explicit Timestep(float seconds = 0.0f) : m_Seconds(seconds) {}

        // Conversione implicita verso float, così si può scrivere la cosa naturale:
        //     posizione += velocita * ts;
        // È implicita solo in QUESTA direzione: il costruttore è explicit, quindi un
        // float non diventa un Timestep per sbaglio.
        constexpr operator float() const { return m_Seconds; }

        constexpr float GetSeconds()      const { return m_Seconds; }
        constexpr float GetMilliseconds() const { return m_Seconds * 1000.0f; }

    private:
        float m_Seconds;
    };
}
