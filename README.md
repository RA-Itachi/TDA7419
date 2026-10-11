# TDA7419_Rafa

Librería Arduino optimizada y corregida para el procesador de audio **TDA7419**
de STMicroelectronics.

## Características

- Control completo del TDA7419 vía I2C
- Volumen con escala en dB (-79 a +15 dB)
- Filtros de 3 bandas (bass, middle, treble) con Q programable
- Loudness con frecuencia central programable
- Soft mute y soft step configurables
- Subwoofer con cutoff programable
- Mixing y gain effect
- **Spectrum analyzer de 7 bandas** integrado
- API modular y documentada

## Instalación

1. Copiar la carpeta `TDA7419_Rafa` a `~/Arduino/libraries/`
2. Reiniciar el IDE de Arduino
3. `#include <TDA7419_Rafa.h>`

## Uso básico

\`\`\`cpp
#include <TDA7419_Rafa.h>

TDA7419_Rafa audio;

void setup() {
    audio.begin();
    audio.setInput(1, 0, true);      // SE1, 0dB, AutoZero
    audio.setVolumeDb(-20, true);    // -20 dB, soft step
}

void loop() {}
\`\`\`

## Spectrum Analyzer

\`\`\`cpp
audio.attachSpectrumPins(26, 34);    // SAclk=26, SAout=34
audio.setSpectrumEnabled(true);
audio.setSpectrumQ(true);

uint16_t values[7];
if (audio.readSpectrum(values)) {
    // values[0..6] = nivel de las 7 bandas
}
\`\`\`

## Correcciones respecto a la librería original de Alexander Liman

- Bug en `setSub_M_B()`: el smoothing filter escribía sobre bass DC
- Bug en `setSpektor()`: bits mal mapeados según datasheet
- Volumen ahora acepta dB directamente (antes solo paso)
- Añadidas funciones de conversión dB <-> paso
- Añadido spectrum analyzer con `readSpectrum()`
- Optimización: switch/case reemplazados por tablas
- Documentación completa en cada función

## Licencia

MIT