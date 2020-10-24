#include "crc.h"

CRC64::CRC64(){
    table = new uint64_t [256];
    for(int i = 0; i < 256; i++){
        uint64_t byte = ((uint64_t)(i) << 56);
        for(uint8_t bit = 0; bit < 8; bit++){
            if((byte & 0x8000000000000000) != 0){
                byte <<= 1;
                byte ^= m_poly;
            }
            else
                byte <<= 1;
        }
        table[i] = byte;
    }
}

uint64_t CRC64::getCRC(uint8_t bytes[], int size){
    uint64_t crc = 0;
    for(int i = 0; i < size; i++){
        uint8_t byte = (uint8_t)bytes[i];
        uint8_t pos = (uint8_t)((crc ^ ((uint64_t)(byte) << 56)) >> 56);
        crc = (uint64_t)((crc << 8) ^ (uint64_t)(table[pos]));
    }
    return crc;
}