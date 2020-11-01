#include <stdint.h>
class CRC64 {
    public:
        CRC64();
        ~CRC64() {delete [] table;}
        uint64_t getCRC(uint8_t *bytes, int size);
    private:
        uint64_t * table = nullptr;
        static const uint64_t m_poly = 0x42F0E1EBA9EA3693;
};
