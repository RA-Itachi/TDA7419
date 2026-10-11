/*
 * Ejemplo del Spectrum Analyzer del TDA7419
 * Lee las 7 bandas y las muestra por Serial.
 * Pines: SAclk=26, SAout=34
 */

#include <Wire.h>
#include <TDA7419_Rafa.h>

TDA7419_Rafa audio;

#define PIN_SA_CLK  26
#define PIN_SA_OUT  34

// Frecuencias centrales de las 7 bandas (datasheet tabla 6)
const uint16_t bandHz[7] = { 62, 157, 396, 1000, 2510, 6340, 15000 };

uint16_t values[7];

// Suavizado para visualizacion agradable (peak hold con caida suave)
uint16_t smoothed[7] = {0};

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println(F("TDA7419_Rafa - Spectrum Analyzer"));

    if (!audio.begin(21, 22)) {
        Serial.println(F("ERROR: TDA7419 no responde"));
        while (1) { delay(1000); }
    }

    // Configurar pines del SA
    audio.attachSpectrumPins(PIN_SA_CLK, PIN_SA_OUT);

    // Configurar entrada (SE1) y volumen audible
    audio.setInput(1, 0, true);
    audio.setVolumeDb(-10, true);

    // Encender spectrum analyzer
    audio.setSpectrumEnabled(true);
    audio.setSpectrumQ(true);          // Q alto
    audio.setSpectrumSource(false);    // desde InGain
    Serial.println(F("Spectrum analyzer encendido"));
}

void loop() {
    // Leer con reset previo: mide el peak desde el ultimo ciclo
    if (audio.readSpectrumWithReset(values, 5)) {

        // Suavizado: subida rapida, bajada lenta (estilo VU meter)
        for (uint8_t i = 0; i < 7; i++) {
            if (values[i] > smoothed[i]) {
                smoothed[i] = values[i];                        // subida instantanea
            } else {
                smoothed[i] = (smoothed[i] * 7 + values[i]) / 8; // bajada suave
            }
        }

        Serial.println(F("--- Bandas ---"));
        for (uint8_t i = 0; i < 7; i++) {
            Serial.print(bandHz[i]);
            Serial.print(F(" Hz: "));
            Serial.print(smoothed[i]);
            Serial.print(F("  "));

            // Barra visual: 4095/50 = 82 por asterisco
            uint8_t bar = smoothed[i] / 82;
            for (uint8_t j = 0; j < bar; j++) Serial.print('*');
            Serial.println();
        }
    }
    delay(100);
}