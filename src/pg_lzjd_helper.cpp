#include <cstdlib>
#include <cstdint>
#include <string>
#include <x86intrin.h>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>

#include "LZJD.h"

using namespace std;
namespace bi = boost::archive::iterators;

extern "C" {

vector<int32_t> cstring_to_lzjd(char* hash) {
    string line = hash;
    auto first_colon = line.find(":", 0);
    auto second_colon = line.find(":", first_colon + 1);
    string path = line.substr(first_colon + 1, second_colon - first_colon - 1);
    string base64ints = line.substr(second_colon + 1, line.size() - second_colon);
    auto size = base64ints.size();
    while (size > 0 && base64ints[size - 1] == '=')
        size--;
    base64ints = base64ints.substr(0, size);


    //TODO this is not 100% kosher, but C++ is a pain. 

    typedef
    bi::transform_width<
            bi::binary_from_base64<bi::remove_whitespace < string::const_iterator>>,
            8, 6
            >
            base64_dec;

    vector<uint8_t> int_parts;

    copy(
            base64_dec(base64ints.cbegin()),
            base64_dec(base64ints.cend()),
            std::back_inserter(int_parts)
            );

    vector<int32_t> decoded_ints(int_parts.size() / 4);
    for (int i = 0; i < int_parts.size(); i += 4) {
        //big endian extraction of the right value
        int32_t dec_i = (int_parts[i + 0] << 24) | (int_parts[i + 1] << 16) | (int_parts[i + 2] << 8) | (int_parts[i + 3] << 0);
        decoded_ints[i / 4] = dec_i;
        //                cout << dec_i << ", ";
    }
    return decoded_ints;
}

int32_t lzjd_similarity(char *hash1, char *hash2) {
    vector<int32_t> l1 = cstring_to_lzjd(hash1);
    vector<int32_t> l2 = cstring_to_lzjd(hash2);
    return similarity(l1, l2);
}

} // End Extern C