/*
 * ============================================================================
 *   TDA7419_Rafa - Libreria optimizada para el procesador de audio TDA7419
 * ============================================================================
 *
 *   Descripcion:
 *     Control completo del TDA7419 via I2C: selector de entrada, ganancia,
 *     loudness, soft mute, volumen con escala en dB, filtros de 3 bandas
 *     (bass/middle/treble), subwoofer, mixing, atenuadores de altavoces y
 *     spectrum analyzer de 7 bandas.
 *
 *   Caracteristicas:
 *     - Volumen con escala real en dB (de -79 dB a +15 dB)
 *     - Soft step y soft mute configurables
 *     - Filtros de 3 bandas con Q y frecuencia programables
 *     - Spectrum analyzer integrado (7 bandas)
 *     - API modular y documentada
 *
 *   Basada en:
 *     Libreria original de Alexander Liman (liman324@yandex.ru)
 *     https://rcl-radio.ru/?p=57658
 *
 *   Licencia: MIT
 *   Version:  1.0.0
 * ============================================================================
 */

#ifndef TDA7419_RAFA_H
#define TDA7419_RAFA_H

#include <Arduino.h>
#include <Wire.h>

// ============================================================================
//   DIRECCION I2C Y SUBADDRESSES (segun datasheet, tabla 7)
// ============================================================================
#define TDA7419_ADDR        0x44    // Direccion I2C 7-bit (0x88 >> 1)

// Subaddress (recibir datos del uP)
#define TDA7419_SUB_INPUT       0x40  // Main Source Selector
#define TDA7419_SUB_LOUDNESS    0x41  // Main Loudness
#define TDA7419_SUB_SOFT        0x42  // Soft Mute / Clock Generator
#define TDA7419_SUB_VOLUME      0x43  // Volume
#define TDA7419_SUB_TREBLE      0x44  // Treble
#define TDA7419_SUB_MIDDLE      0x45  // Middle
#define TDA7419_SUB_BASS        0x46  // Bass
#define TDA7419_SUB_INPUT2      0x47  // Second Source Selector
#define TDA7419_SUB_SUB_M_B     0x48  // Subwoofer / Middle / Bass
#define TDA7419_SUB_MIX_G_EFF   0x49  // Mixing / Gain Effect
#define TDA7419_SUB_ATT_LF      0x4A  // Speaker Attenuator Left Front
#define TDA7419_SUB_ATT_RF      0x4B  // Speaker Attenuator Right Front
#define TDA7419_SUB_ATT_LT      0x4C  // Speaker Attenuator Left Rear
#define TDA7419_SUB_ATT_RT      0x4D  // Speaker Attenuator Right Rear
#define TDA7419_SUB_MIX_LEV     0x4E  // Mixing Level Control
#define TDA7419_SUB_ATT_SUB     0x4F  // Subwoofer Attenuator
#define TDA7419_SUB_SPECTRUM    0x50  // Spectrum Analyzer / Clock / AC Mode
#define TDA7419_SUB_TEST        0x51  // Testing Audio Processor

// ============================================================================
//   CONSTANTES DEL SPECTRUM ANALYZER
// ============================================================================
#define TDA_SA_BANDS            7       // 7 bandas
#define TDA_SA_RESET_MS         5       // > 4.5 ms (tintres)
#define TDA_SA_SETTLE_US        3       // > 2 us   (tsadel)

// ============================================================================
//   CLASE PRINCIPAL
// ============================================================================
class TDA7419_Rafa
{
public:
    // ------------------------------------------------------------------------
    //   CONSTRUCTOR
    // ------------------------------------------------------------------------
    TDA7419_Rafa();

    // ------------------------------------------------------------------------
    //   INICIALIZACION
    // ------------------------------------------------------------------------
    bool begin();
    bool begin(int sdaPin, int sclPin);
    bool isPresent();

    // ------------------------------------------------------------------------
    //   MAIN SOURCE SELECTOR (subaddress 0x40)
    // ------------------------------------------------------------------------
    void setInput(uint8_t input, uint8_t gainDb, bool autoZero);

    // ------------------------------------------------------------------------
    //   MAIN LOUDNESS (subaddress 0x41)
    // ------------------------------------------------------------------------
    void setLoudness(uint8_t attDb, uint8_t centerFreq,
                     bool highBoost, bool softStep);

    // ------------------------------------------------------------------------
    //   SOFT MUTE / CLOCK GENERATOR (subaddress 0x42)
    // ------------------------------------------------------------------------
    void setSoftMute(bool softMute, bool pinInfluence, uint8_t muteTime,
                     uint8_t stepTime, bool clockFast);

    // ------------------------------------------------------------------------
    //   VOLUMEN (subaddress 0x43)
    // ------------------------------------------------------------------------
    void setVolumeDb(int8_t dB, bool soft);
    void setVolumeStep(uint8_t step, bool soft);
    int8_t getVolumeDb() const { return _volDb; }

    // ------------------------------------------------------------------------
    //   ATENUADORES DE ALTAVOCES (subaddresses 0x4A - 0x4F)
    // ------------------------------------------------------------------------
    void setAttLF(int8_t dB, bool soft);
    void setAttRF(int8_t dB, bool soft);
    void setAttLT(int8_t dB, bool soft);
    void setAttRT(int8_t dB, bool soft);
    void setAttSub(int8_t dB, bool soft);
    void setAttMix(int8_t dB, bool soft);

    // ------------------------------------------------------------------------
    //   FILTROS DE 3 BANDAS
    // ------------------------------------------------------------------------
    void setFilterTreble(int8_t gainDb, uint8_t centerF, bool vrefOut);
    void setFilterMiddle(int8_t gainDb, uint8_t q, bool soft);
    void setFilterBass(int8_t gainDb, uint8_t q, bool soft);

    // ------------------------------------------------------------------------
    //   SECOND SOURCE SELECTOR (subaddress 0x47)
    // ------------------------------------------------------------------------
    void setInput2(uint8_t input, uint8_t gainDb, bool autoZero);

    // ------------------------------------------------------------------------
    //   SUBWOOFER / MIDDLE / BASS (subaddress 0x48)
    // ------------------------------------------------------------------------
    void setSubMB(uint8_t subCutoff, uint8_t midCenter, uint8_t bassCenter,
                  bool bassDcMode, bool smoothFilt);

    // ------------------------------------------------------------------------
    //   MIXING / GAIN EFFECT (subaddress 0x49)
    // ------------------------------------------------------------------------
    void setMixGainEff(bool mixLeft, bool mixRight, bool mixEnable,
                       bool subEnable, uint8_t gainEff);

    // ------------------------------------------------------------------------
    //   SPECTRUM ANALYZER (subaddress 0x50)
    // ------------------------------------------------------------------------
    void setSpectrumEnabled(bool enable);
    void setSpectrumQ(bool qHigh);
    void setSpectrumSource(bool fromBass);
    void setCouplingMode(uint8_t mode);

    // ------------------------------------------------------------------------
    //   SPECTRUM ANALYZER - LECTURA
    // ------------------------------------------------------------------------
    /**
     * @brief Configura los pines GPIO para el spectrum analyzer.
     *        Debe llamarse antes de readSpectrum().
     * @param clkPin Pin conectado a SAclk (salida)
     * @param outPin Pin conectado a SAout (entrada ADC)
     */
    void attachSpectrumPins(uint8_t clkPin, uint8_t outPin);

    /**
     * @brief Realiza una lectura completa de las 7 bandas SIN reset previo.
     *        Util para mediciones continuas donde queremos el "peak acumulado".
     */
    bool readSpectrum(uint16_t *values);

    /**
     * @brief Realiza una lectura con reset previo de los condensadores.
     *        Util para medir el "peak desde el ultimo ciclo".
     *        Este es el metodo recomendado para visualizadores de espectro.
     * @param values  Buffer de 7 uint16_t donde se guardan los valores ADC
     * @param resetMs Tiempo de reset en ms (por defecto 5 ms)
     * @return true si la lectura fue exitosa
     */
    bool readSpectrumWithReset(uint16_t *values, uint16_t resetMs = TDA_SA_RESET_MS);

    /**
     * @brief Resetea los condensadores del spectrum analyzer.
     */
    void resetSpectrum();

    // ------------------------------------------------------------------------
    //   BAJO NIVEL
    // ------------------------------------------------------------------------
    void writeWire(uint8_t subAddr, uint8_t data);

private:
    int8_t   _volDb;
    uint8_t  _saClkPin;
    uint8_t  _saOutPin;
    bool     _saAttached;
    bool     _saEnabled;

    uint8_t  dBToStep(int8_t dB);
    uint8_t  gainToBits(int8_t gain);
};

#endif // TDA7419_RAFA_H