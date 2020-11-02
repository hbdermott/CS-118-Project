#include <stdint.h>
class CRC {
    public:
        CRC();
        uint64_t get_crc_code(uint8_t *bytes, int size);
    private:
        uint64_t table[256];
        static const uint64_t m_polynomial = 0x42F0E1EBA9EA3693;
};
