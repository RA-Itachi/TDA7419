/*
 * ============================================================================
 *   TDA7419_Rafa - Implementacion
 * ============================================================================
 *   Ver TDA7419_Rafa.h para documentacion completa.
 *   Version 1.1.0 - correcciones de dBToStep y setAtt*
 * ============================================================================
 */

#include "TDA7419_Rafa.h"

// ============================================================================
//   TABLAS DE CONVERSION
// ============================================================================
static const uint8_t gainTable[31] = {
    0b00001111, // -15
    0b00001110, // -14
    0b00001101, // -13
    0b00001100, // -12
    0b00001011, // -11
    0b00001010, // -10
    0b00001001, // -9
    0b00001000, // -8
    0b00000111, // -7
    0b00000110, // -6
    0b00000101, // -5
    0b00000100, // -4
    0b00000011, // -3
    0b00000010, // -2
    0b00000001, // -1
    0b00000000, // 0
    0b00010001, // +1
    0b00010010, // +2
    0b00010011, // +3
    0b00010100, // +4
    0b00010101, // +5
    0b00010110, // +6
    0b00010111, // +7
    0b00011000, // +8
    0b00011001, // +9
    0b00011010, // +10
    0b00011011, // +11
    0b00011100, // +12
    0b00011101, // +13
    0b00011110, // +14
    0b00011111  // +15
};

// ============================================================================
//   CONSTRUCTOR
// ============================================================================
TDA7419_Rafa::TDA7419_Rafa()
{
    _volDb      = 0;
    _saClkPin   = 255;
    _saOutPin   = 255;
    _saAttached = false;
    _saEnabled  = false;
}

// ============================================================================
//   INICIALIZACION
// ============================================================================
bool TDA7419_Rafa::begin()
{
    Wire.begin();
    return isPresent();
}

bool TDA7419_Rafa::begin(int sdaPin, int sclPin)
{
    Wire.begin(sdaPin, sclPin);
    return isPresent();
}

bool TDA7419_Rafa::isPresent()
{
    Wire.beginTransmission(TDA7419_ADDR);
    return (Wire.endTransmission() == 0);
}

// ============================================================================
//   METODOS AUXILIARES
// ============================================================================

// ============================================================================
//   CONVERSION dB -> paso segun Tabla 11 del datasheet
// ============================================================================
//
//   Rango aceptado: -79 a +15 dB
//
//   Tramo positivo (0 a +15 dB):
//     paso = dB           (0..15)
//
//   Tramo negativo (-1 a -79 dB):
//     paso = 16 + |dB|    (17..95)
//
//   El paso se envia al chip en el campo D6..D0.
//   El bit D7 (soft step) se añade en las funciones setVolume/setAtt.
//
//   Ejemplos:
//     dB = +15 -> paso 15  (0b0001111)
//     dB =   0 -> paso  0  (0b0000000)
//     dB =  -1 -> paso 17  (0b0010001)
//     dB = -79 -> paso 95  (0b1011111)
//
uint8_t TDA7419_Rafa::dBToStep(int8_t dB)
{
    if (dB >= 0)   return (uint8_t)dB;              // 0..+15
    if (dB <= -79) return 95;                       // -79
    return (uint8_t)(16 + (-dB));                   // -1..-78
}

uint8_t TDA7419_Rafa::gainToBits(int8_t gain)
{
    if (gain < -15) gain = -15;
    if (gain > +15) gain = +15;
    return gainTable[gain + 15];
}

// ============================================================================
//   MAIN SOURCE SELECTOR
// ============================================================================
void TDA7419_Rafa::setInput(uint8_t input, uint8_t gainDb, bool autoZero)
{
    if (input > 5)   input  = 5;
    if (gainDb > 15) gainDb = 15;

    uint8_t data = (input & 0x07)
                 | ((gainDb & 0x0F) << 3)
                 | (autoZero ? 0x00 : 0x80);

    writeWire(TDA7419_SUB_INPUT, data);
}

// ============================================================================
//   MAIN LOUDNESS
// ============================================================================
void TDA7419_Rafa::setLoudness(uint8_t attDb, uint8_t centerFreq,
                                bool highBoost, bool softStep)
{
    if (attDb > 15)      attDb      = 15;
    if (centerFreq > 3)  centerFreq = 3;

    uint8_t data = (attDb      & 0x0F)
                 | ((centerFreq & 0x03) << 4)
                 | (highBoost ? 0x40 : 0x00)
                 | (softStep  ? 0x80 : 0x00);

    writeWire(TDA7419_SUB_LOUDNESS, data);
}

// ============================================================================
//   SOFT MUTE / CLOCK GENERATOR
// ============================================================================
/*
 * OJO con la nomenclatura de los parametros (bit D0 y D1 del datasheet):
 *   softMute:     true  -> D0 = 1 -> audio pasa (mute OFF)
 *                 false -> D0 = 0 -> mute ON
 *   pinInfluence: true  -> D1 = 1 -> IIC only (ignora pin 21)
 *                 false -> D1 = 0 -> Pin and IIC
 *
 * Para tener audio SIEMPRE hay que llamar con:
 *   audio.setSoftMute(true, true, ...)
 */
void TDA7419_Rafa::setSoftMute(bool softMute, bool pinInfluence,
                                uint8_t muteTime, uint8_t stepTime,
                                bool clockFast)
{
    if (muteTime > 2) muteTime = 2;
    if (stepTime > 7) stepTime = 7;

    uint8_t data = (softMute     ? 0x01 : 0x00)
                 | (pinInfluence ? 0x02 : 0x00)
                 | ((muteTime  & 0x03) << 2)
                 | ((stepTime  & 0x07) << 4)
                 | (clockFast   ? 0x80 : 0x00);

    writeWire(TDA7419_SUB_SOFT, data);
}

// ============================================================================
//   VOLUMEN
// ============================================================================
void TDA7419_Rafa::setVolumeDb(int8_t dB, bool soft)
{
    uint8_t step = dBToStep(dB);
    writeWire(TDA7419_SUB_VOLUME, (step & 0x7F) | (soft ? 0x80 : 0x00));
}

void TDA7419_Rafa::setVolumeStep(uint8_t step, bool soft)
{
    if (step > 95) step = 95;

    uint8_t data = (step & 0x7F)
                 | (soft ? 0x80 : 0x00);

    writeWire(TDA7419_SUB_VOLUME, data);
}

// ============================================================================
//   ATENUADORES DE ALTAVOCES
// ============================================================================
void TDA7419_Rafa::setAttLF(int8_t dB, bool soft)
{
    uint8_t step = dBToStep(dB);
    writeWire(TDA7419_SUB_ATT_LF, (step & 0x7F) | (soft ? 0x80 : 0x00));
}

void TDA7419_Rafa::setAttRF(int8_t dB, bool soft)
{
    uint8_t step = dBToStep(dB);
    writeWire(TDA7419_SUB_ATT_RF, (step & 0x7F) | (soft ? 0x80 : 0x00));
}

void TDA7419_Rafa::setAttLT(int8_t dB, bool soft)
{
    uint8_t step = dBToStep(dB);
    writeWire(TDA7419_SUB_ATT_LT, (step & 0x7F) | (soft ? 0x80 : 0x00));
}

void TDA7419_Rafa::setAttRT(int8_t dB, bool soft)
{
    uint8_t step = dBToStep(dB);
    writeWire(TDA7419_SUB_ATT_RT, (step & 0x7F) | (soft ? 0x80 : 0x00));
}

void TDA7419_Rafa::setAttSub(int8_t dB, bool soft)
{
    uint8_t step = dBToStep(dB);
    writeWire(TDA7419_SUB_ATT_SUB, (step & 0x7F) | (soft ? 0x80 : 0x00));
}

void TDA7419_Rafa::setAttMix(int8_t dB, bool soft)
{
    uint8_t step = dBToStep(dB);
    writeWire(TDA7419_SUB_MIX_LEV, (step & 0x7F) | (soft ? 0x80 : 0x00));
}

// ============================================================================
//   FILTROS DE 3 BANDAS
// ============================================================================
void TDA7419_Rafa::setFilterTreble(int8_t gainDb, uint8_t centerF, bool vrefOut)
{
    if (centerF > 3) centerF = 3;

    uint8_t data = gainToBits(gainDb)
                 | ((centerF & 0x03) << 5)
                 | (vrefOut ? 0x80 : 0x00);

    writeWire(TDA7419_SUB_TREBLE, data);
}

void TDA7419_Rafa::setFilterMiddle(int8_t gainDb, uint8_t q, bool soft)
{
    if (q > 3) q = 3;

    uint8_t data = gainToBits(gainDb)
                 | ((q & 0x03) << 5)
                 | (soft ? 0x80 : 0x00);

    writeWire(TDA7419_SUB_MIDDLE, data);
}

void TDA7419_Rafa::setFilterBass(int8_t gainDb, uint8_t q, bool soft)
{
    if (q > 3) q = 3;

    uint8_t data = gainToBits(gainDb)
                 | ((q & 0x03) << 5)
                 | (soft ? 0x80 : 0x00);

    writeWire(TDA7419_SUB_BASS, data);
}

// ============================================================================
//   SECOND SOURCE SELECTOR
// ============================================================================
void TDA7419_Rafa::setInput2(uint8_t input, uint8_t gainDb, bool autoZero)
{
    if (input > 5)   input  = 5;
    if (gainDb > 15) gainDb = 15;

    uint8_t data = (input & 0x07)
                 | ((gainDb & 0x0F) << 3)
                 | (autoZero ? 0x00 : 0x80);

    writeWire(TDA7419_SUB_INPUT2, data);
}

// ============================================================================
//   SUBWOOFER / MIDDLE / BASS
// ============================================================================
void TDA7419_Rafa::setSubMB(uint8_t subCutoff, uint8_t midCenter,
                             uint8_t bassCenter, bool bassDcMode,
                             bool smoothFilt)
{
    if (subCutoff  > 3) subCutoff  = 3;
    if (midCenter  > 3) midCenter  = 3;
    if (bassCenter > 3) bassCenter = 3;

    uint8_t bassBits = (bassCenter << 4);

    uint8_t data = (subCutoff & 0x03)
                 | ((midCenter & 0x03) << 2)
                 | (bassBits & 0x30)
                 | (bassDcMode ? 0x40 : 0x00)
                 | (smoothFilt ? 0x80 : 0x00);

    writeWire(TDA7419_SUB_SUB_M_B, data);
}

// ============================================================================
//   MIXING / GAIN EFFECT
// ============================================================================
void TDA7419_Rafa::setMixGainEff(bool mixLeft, bool mixRight, bool mixEnable,
                                  bool subEnable, uint8_t gainEff)
{
    if (gainEff > 10) gainEff = 10;

    static const uint8_t gainEffBits[11] = {
        0xC0, 0x00, 0x10, 0x20, 0x30, 0x40,
        0x50, 0x60, 0x70, 0x80, 0x90
    };

    uint8_t data = (mixLeft   ? 0x01 : 0x00)
                 | (mixRight  ? 0x02 : 0x00)
                 | (mixEnable ? 0x04 : 0x00)
                 | (subEnable ? 0x08 : 0x00)
                 | (gainEffBits[gainEff] & 0xF0);

    writeWire(TDA7419_SUB_MIX_G_EFF, data);
}

// ============================================================================
//   SPECTRUM ANALYZER - CONFIGURACION
// ============================================================================
void TDA7419_Rafa::setSpectrumEnabled(bool enable)
{
    _saEnabled = enable;

    uint8_t data = 0x00;
    if (enable) {
        data |= 0x10;  // D4: run ON
    }
    writeWire(TDA7419_SUB_SPECTRUM, data);
}

void TDA7419_Rafa::setSpectrumQ(bool qHigh)
{
    uint8_t data = (qHigh ? 0x40 : 0x00) | 0x10;
    writeWire(TDA7419_SUB_SPECTRUM, data);
}

void TDA7419_Rafa::setSpectrumSource(bool fromBass)
{
    uint8_t data = (fromBass ? 0x00 : 0x20) | 0x10;
    writeWire(TDA7419_SUB_SPECTRUM, data);
}

void TDA7419_Rafa::setCouplingMode(uint8_t mode)
{
    if (mode > 3) mode = 3;
    uint8_t data = (mode & 0x03) | 0x10;
    writeWire(TDA7419_SUB_SPECTRUM, data);
}

// ============================================================================
//   SPECTRUM ANALYZER - LECTURA
// ============================================================================
void TDA7419_Rafa::attachSpectrumPins(uint8_t clkPin, uint8_t outPin)
{
    _saClkPin   = clkPin;
    _saOutPin   = outPin;
    _saAttached = true;

    pinMode(_saClkPin, OUTPUT);
    digitalWrite(_saClkPin, LOW);
    pinMode(_saOutPin, INPUT);
}

void TDA7419_Rafa::resetSpectrum()
{
    if (!_saAttached) return;

    digitalWrite(_saClkPin, HIGH);
    delay(TDA_SA_RESET_MS);
    digitalWrite(_saClkPin, LOW);
}

bool TDA7419_Rafa::readSpectrum(uint16_t *values)
{
    if (!_saAttached || !values) return false;
    if (!_saEnabled) {
        for (uint8_t i = 0; i < TDA_SA_BANDS; i++) values[i] = 0;
        return false;
    }

    digitalWrite(_saClkPin, LOW);
    delayMicroseconds(10);

    for (uint8_t i = 0; i < TDA_SA_BANDS; i++) {
        digitalWrite(_saClkPin, HIGH);
        delayMicroseconds(TDA_SA_SETTLE_US);
        values[i] = analogRead(_saOutPin);
        digitalWrite(_saClkPin, LOW);
        delayMicroseconds(10);
    }

    digitalWrite(_saClkPin, LOW);
    return true;
}

bool TDA7419_Rafa::readSpectrumWithReset(uint16_t *values, uint16_t resetMs)
{
    if (!_saAttached || !values) return false;
    if (!_saEnabled) {
        for (uint8_t i = 0; i < TDA_SA_BANDS; i++) values[i] = 0;
        return false;
    }

    digitalWrite(_saClkPin, HIGH);
    delay(resetMs);
    digitalWrite(_saClkPin, LOW);
    delayMicroseconds(10);

    for (uint8_t i = 0; i < TDA_SA_BANDS; i++) {
        digitalWrite(_saClkPin, HIGH);
        delayMicroseconds(TDA_SA_SETTLE_US);
        values[i] = analogRead(_saOutPin);
        digitalWrite(_saClkPin, LOW);
        delayMicroseconds(10);
    }

    digitalWrite(_saClkPin, LOW);
    return true;
}

// ============================================================================
//   BAJO NIVEL
// ============================================================================
void TDA7419_Rafa::writeWire(uint8_t subAddr, uint8_t data)
{
    Wire.beginTransmission(TDA7419_ADDR);
    Wire.write(subAddr);
    Wire.write(data);
    Wire.endTransmission();
}