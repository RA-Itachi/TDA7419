/*
 * Ejemplo basico de la libreria TDA7419_Rafa
 * Configura el TDA7419 con valores tipicos y prueba el volumen.
 */

#include <Wire.h>
#include <TDA7419_Rafa.h>

TDA7419_Rafa audio;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println(F("TDA7419_Rafa - Ejemplo basico"));

    // Inicializar (SDA=21, SCL=22 en ESP32)
    if (!audio.begin(21, 22)) {
        Serial.println(F("ERROR: TDA7419 no responde en 0x44"));
        while (1);
    }
    Serial.println(F("TDA7419 detectado OK"));

    // Configurar entrada SE1 (para BT401) sin ganancia extra
    audio.setInput(1, 0, true);

    // Loudness plano (sin atenuacion)
    audio.setLoudness(0, 0, false, true);

    // Soft mute OFF, pin influence IIC, mute time 0.96ms, step 2.26ms
    audio.setSoftMute(false, false, 1, 4, false);

    // Volumen a -20 dB
    audio.setVolumeDb(-20, true);

    // Filtros planos
    audio.setFilterBass(0, 0, false);
    audio.setFilterMiddle(0, 2, false);
    audio.setFilterTreble(0, 0, false);

    // Subwoofer cutoff 80Hz, middle 1kHz, bass 80Hz
    audio.setSubMB(1, 1, 1, false, false);

    // Mixing deshabilitado
    audio.setMixGainEff(false, false, false, false, 1);

    Serial.println(F("Configuracion completa"));
}

void loop() {
    // Barrido de volumen de -79 a +15 dB
    for (int8_t dB = -79; dB <= 15; dB += 5) {
        Serial.print(F("Volumen: "));
        Serial.print(dB);
        Serial.println(F(" dB"));
        audio.setVolumeDb(dB, true);
        delay(500);
    }
    delay(2000);
}