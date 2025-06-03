#include <iostream>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <bitset>
#include <cassert>
#include <iomanip>

class
Hamming_Code final {
private:
    //default constructor
    Hamming_Code() = delete;
    //copy constructor
    Hamming_Code( const Hamming_Code &) = delete;
    //move constructor
    Hamming_Code( Hamming_Code&& ) = delete;
    //copy assignment
    Hamming_Code& operator=(const Hamming_Code&) = delete;
    //move assignment
    Hamming_Code& operator=(Hamming_Code&&) = delete;
    //destructor
    ~Hamming_Code() = delete;

public:
    static inline
    int get_bit(const void *in, size_t n) {
        return (((const uint8_t*)in)[n / CHAR_BIT] & (1 << (n % CHAR_BIT))) != 0;
    }

    static inline
    void set_bit(void *out, size_t n, int bit) {
        if (bit) {
            ((uint8_t*)out)[n / CHAR_BIT] |= (1 << (n % CHAR_BIT));
        } else {
            ((uint8_t*)out)[n / CHAR_BIT] &= ~(1 << (n % CHAR_BIT));
        }
    }

    static inline
    void flip_bit(void *out, size_t n) {
        ((uint8_t*)out)[n / CHAR_BIT] ^= (1 << (n % CHAR_BIT));
    }

    static inline
    size_t encode(void *out, const void *in, size_t in_bits) {
        size_t i, j, k;
        size_t s = 0;

        for (i = j = 0; j < in_bits; ++i) {
            if ((i + 1) & i) {
                if (get_bit(in, j)) {
                    s ^= i + 1;
                }
                set_bit(out, i, get_bit(in, j));
                ++j;
            }
        }
        for (k = 1; k < i; k <<= 1) {
            set_bit(out, k - 1, s & k);
        }

        return i;
    }

    static inline
    size_t decode(void *out, const void *in, size_t in_bits, size_t *out_syndrome) {
        size_t i, j;
        size_t s = 0;

        for (i = j = 0; i < in_bits; ++i) {
            if (get_bit(in, i)) {
                s ^= i + 1;
            }
            if ((i + 1) & i) {
                set_bit(out, j, get_bit(in, i));
                ++j;
            }
        }

        *out_syndrome = s;
        return j;
    }

    static inline
    size_t decode_and_fix(void *out, const void *in, size_t in_bits) {
        size_t s;

        size_t out_bits = decode(out, in, in_bits, &s);
        if (s && s <= in_bits && (s & (s - 1))) {
            size_t k;
            for (k = 0; 1u << k < s; ++k);
            flip_bit(out, s - k - 1);
        }

        return out_bits;
    }

    static inline
    void print_value(const std::string &name, const uint8_t val[], size_t val_size_bytes) {
        std::cout << name << "(" << val_size_bytes << ")" << ": ";
        for (size_t i = 0; i < val_size_bytes; ++i) {
            std::cout << std::hex << std::uppercase << (unsigned)val[i] << " ";
        }
        std::cout << std::endl;
    }
};

int main()
{
    uint16_t a = 0x1234;
    std::cout << "a = " << std::bitset<sizeof(a) * CHAR_BIT>(a) << std::endl;

    uint8_t msg[4] = {1, 2, 3, 4};
    uint8_t encoded[5] = {};
    uint8_t decoded[4] = {};
    size_t s = -1;
    size_t i = 0;

    std::cout << std::endl << "Before" << std::endl;
    Hamming_Code::print_value("msg", msg, sizeof(msg));
    Hamming_Code::print_value("encoded", encoded, sizeof(encoded));
    Hamming_Code::print_value("decoded", decoded, sizeof(decoded));

    size_t encoded_bits = Hamming_Code::encode(encoded, msg, sizeof(msg) * CHAR_BIT);
    size_t decoded_bits = Hamming_Code::decode(decoded, encoded, encoded_bits, &s);
    assert(s == 0);
    assert(encoded_bits == sizeof(msg) * CHAR_BIT + 6);
    assert(decoded_bits == sizeof(msg) * CHAR_BIT);
    assert(memcmp(msg, decoded, sizeof(msg)) == 0);

    std::cout << std::endl << "After" << std::endl;
    Hamming_Code::print_value("msg", msg, sizeof(msg));
    Hamming_Code::print_value("encoded", encoded, sizeof(encoded));
    Hamming_Code::print_value("decoded", decoded, sizeof(decoded));
    std::cout << std::endl;

    for (i = 0; i < encoded_bits; ++i) {
        Hamming_Code::flip_bit(encoded, i);
        decoded_bits = Hamming_Code::decode(decoded, encoded, encoded_bits, &s);
        decoded_bits = Hamming_Code::decode_and_fix(decoded, encoded, encoded_bits);
        assert(decoded_bits == sizeof(msg) * CHAR_BIT);
        assert(memcmp(msg, decoded, sizeof(msg)) == 0);
        Hamming_Code::flip_bit(encoded, i);
    }

    return 0;
}
