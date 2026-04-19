#ifndef NETWORKPROTOCOL_H
#define NETWORKPROTOCOL_H

#include <cstdint>

namespace NetProto {

enum class MsgType : uint8_t {
    TICK_REGULATOR = 0x01, // Przesłanie wartości sterowania (u)
    TICK_OBJECT = 0x02,    // Przesłanie wartości regulowanej (y)
    CONFIG_PID = 0x03,     // Zserializowana konfiguracja PID
    CONFIG_ARX = 0x04,     // Zserializowana konfiguracja ARX
    SIM_CTRL = 0x05,        // Komendy sterujące (Start, Stop, Interwał)
    CONFIG_GEN = 0x06   // Zserializowana konfiguracja Generatora
};

// Nagłówek każdej ramki
#pragma pack(push, 1) // wymuszony brak wyrównywania bajtów w strukturze
struct PacketHeader {
    uint32_t totalSize; // Rozmiar całego pakietu (nagłówek + payload)
    MsgType type;       // Typ wiadomości
    uint32_t seqNum;    // Numer sekwencyjny próbki (do synchronizacji i opóźnień)
};
#pragma pack(pop)
}

#endif // NETWORKPROTOCOL_H
