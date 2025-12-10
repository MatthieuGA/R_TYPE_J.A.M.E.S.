#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>

namespace server::network {

namespace detail {
/**
 * @brief Convert integral value to little-endian byte order
 *
 * @tparam T Integral type
 * @param value Value to convert
 * @return T Value in little-endian byte order
 */
template <typename T>
inline T ToLittleEndian(T value) {
    if constexpr (std::endian::native == std::endian::little) {
        return value;
    } else if constexpr (std::endian::native == std::endian::big) {
        return ByteSwap(value);
    } else {
        static_assert(std::endian::native == std::endian::little ||
                          std::endian::native == std::endian::big,
            "Mixed endianness not supported");
    }
}

/**
 * @brief Convert integral value from little-endian byte order
 *
 * @tparam T Integral type
 * @param value Value in little-endian byte order
 * @return T Value in native byte order
 */
template <typename T>
inline T FromLittleEndian(T value) {
    return ToLittleEndian(value);
}

/**
 * @brief Swaps the byte order of an integral value.
 *
 * This is a C++20 compatible alternative to std::byteswap (C++23).
 * Uses compiler built-in functions for efficient byte swapping.
 *
 * Note: constexpr only on GCC/Clang due to MSVC intrinsic limitations.
 *
 * @tparam T The integral type to swap (must be 2, 4, or 8 bytes).
 * @param value The value to byte-swap.
 * @return The value with bytes in reversed order.
 */
template <typename T>
#if defined(__GNUC__) || defined(__clang__)
static constexpr T ByteSwap(T value) {
#else
static inline T ByteSwap(T value) {
#endif
    static_assert(
        sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
        "ByteSwap only supports 1, 2, 4, or 8 byte integral types");
    if constexpr (sizeof(T) == 1)
        return value;
    if constexpr (sizeof(T) == 2) {
#if defined(_MSC_VER)
        return _byteswap_ushort(value);
#elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap16(value);
#else
    return (value >> 8) | (value << 8);
#endif
    }
    if constexpr (sizeof(T) == 4) {
#if defined(_MSC_VER)
        return _byteswap_ulong(value);
#elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap32(value);
#else
    return (value >> 24) | ((value << 8) & 0x00FF0000) |
           ((value >> 8) & 0x0000FF00) | (value << 24);
#endif
    }
    if constexpr (sizeof(T) == 8) {
#if defined(_MSC_VER)
        return _byteswap_uint64(value);
#elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap64(value);
#else
    return (value >> 56) | ((value << 40) & 0x00FF000000000000) |
           ((value << 24) & 0x0000FF0000000000) |
           ((value << 8) & 0x000000FF00000000) |
           ((value >> 8) & 0x00000000FF000000) |
           ((value >> 24) & 0x0000000000FF0000) |
           ((value >> 40) & 0x000000000000FF00) | (value << 56);
#endif
    }
}
}  // namespace detail

/**
 * @brief RFC-compliant 12-byte packet header (Section 4.1)
 *
 * Layout (Little Endian):
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     OpCode    |          PayloadSize          |  PacketIndex  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                             TickId                            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  PacketCount  |                   Reserved                    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * Total size: 12 bytes (packed, no padding)
 */
struct __attribute__((packed)) CommonHeader {
    uint8_t op_code;        // Command identifier (TCP: 0x01-0x07, UDP: 0x10+)
    uint16_t payload_size;  // Size of payload following this header
    uint8_t packet_index;   // Fragment index (0 to packet_count-1)
    uint32_t tick_id;       // Frame counter (0 for TCP packets)
    uint8_t packet_count;   // Total fragments for this tick
    std::array<uint8_t, 3> reserved;  // Padding, must be 0

    CommonHeader()
        : op_code(0),
          payload_size(0),
          packet_index(0),
          tick_id(0),
          packet_count(1),
          reserved{0, 0, 0} {}

    CommonHeader(uint8_t op, uint16_t size, uint32_t tick = 0, uint8_t idx = 0,
        uint8_t count = 1)
        : op_code(op),
          payload_size(size),
          packet_index(idx),
          tick_id(tick),
          packet_count(count),
          reserved{0, 0, 0} {}
};

/**
 * @brief Low-level buffer for packet serialization/deserialization
 *
 * Provides type-safe reading and writing of primitive types with
 * little-endian encoding. No virtual functions, no dynamic polymorphism.
 */
class PacketBuffer {
 public:
    PacketBuffer() : read_offset_(0) {
        buffer_.reserve(256);  // Reserve typical packet size
    }

    explicit PacketBuffer(std::vector<uint8_t> data)
        : buffer_(std::move(data)), read_offset_(0) {}

    explicit PacketBuffer(const uint8_t *data, size_t size)
        : buffer_(data, data + size), read_offset_(0) {}

    // Header serialization (RFC Section 4.1)
    void WriteHeader(const CommonHeader &header) {
        WriteUint8(header.op_code);
        WriteUint16(header.payload_size);
        WriteUint8(header.packet_index);
        WriteUint32(header.tick_id);
        WriteUint8(header.packet_count);
        WriteUint8(header.reserved[0]);
        WriteUint8(header.reserved[1]);
        WriteUint8(header.reserved[2]);
    }

    CommonHeader ReadHeader() {
        CommonHeader header;
        header.op_code = ReadUint8();
        header.payload_size = ReadUint16();
        header.packet_index = ReadUint8();
        header.tick_id = ReadUint32();
        header.packet_count = ReadUint8();
        header.reserved[0] = ReadUint8();
        header.reserved[1] = ReadUint8();
        header.reserved[2] = ReadUint8();
        return header;
    }

    // Write primitive types (guaranteed little-endian)
    void WriteUint8(uint8_t value) {
        buffer_.push_back(value);
    }

    void WriteUint16(uint16_t value) {
        value = detail::ToLittleEndian(value);
        uint8_t bytes[2];
        std::memcpy(bytes, &value, 2);
        buffer_.push_back(bytes[0]);
        buffer_.push_back(bytes[1]);
    }

    void WriteUint32(uint32_t value) {
        value = detail::ToLittleEndian(value);
        uint8_t bytes[4];
        std::memcpy(bytes, &value, 4);
        for (int i = 0; i < 4; ++i) {
            buffer_.push_back(bytes[i]);
        }
    }

    void WriteUint64(uint64_t value) {
        value = detail::ToLittleEndian(value);
        uint8_t bytes[8];
        std::memcpy(bytes, &value, 8);
        for (int i = 0; i < 8; ++i) {
            buffer_.push_back(bytes[i]);
        }
    }

    void WriteFloat(float value) {
        static_assert(sizeof(float) == 4);
        uint32_t bits;
        std::memcpy(&bits, &value, 4);
        WriteUint32(bits);
    }

    void WriteDouble(double value) {
        static_assert(sizeof(double) == 8);
        uint64_t bits;
        std::memcpy(&bits, &value, 8);
        WriteUint64(bits);
    }

    // Read primitive types (guaranteed little-endian)
    uint8_t ReadUint8() {
        CheckBounds(1);
        return buffer_[read_offset_++];
    }

    uint16_t ReadUint16() {
        CheckBounds(2);
        uint16_t value;
        std::memcpy(&value, &buffer_[read_offset_], 2);
        read_offset_ += 2;
        return detail::FromLittleEndian(value);
    }

    uint32_t ReadUint32() {
        CheckBounds(4);
        uint32_t value;
        std::memcpy(&value, &buffer_[read_offset_], 4);
        read_offset_ += 4;
        return detail::FromLittleEndian(value);
    }

    uint64_t ReadUint64() {
        CheckBounds(8);
        uint64_t value;
        std::memcpy(&value, &buffer_[read_offset_], 8);
        read_offset_ += 8;
        return detail::FromLittleEndian(value);
    }

    float ReadFloat() {
        uint32_t bits = ReadUint32();
        float value;
        std::memcpy(&value, &bits, 4);
        return value;
    }

    double ReadDouble() {
        uint64_t bits = ReadUint64();
        double value;
        std::memcpy(&value, &bits, 8);
        return value;
    }

    // Buffer access
    const std::vector<uint8_t> &Data() const {
        return buffer_;
    }

    std::vector<uint8_t> &Data() {
        return buffer_;
    }

    size_t Size() const {
        return buffer_.size();
    }

    size_t ReadOffset() const {
        return read_offset_;
    }

    size_t Remaining() const {
        return buffer_.size() - read_offset_;
    }

    void ResetReadOffset() {
        read_offset_ = 0;
    }

    void Clear() {
        buffer_.clear();
        read_offset_ = 0;
    }

 private:
    void CheckBounds(size_t bytes_needed) const {
        if (read_offset_ + bytes_needed > buffer_.size()) {
            throw std::out_of_range("PacketBuffer: read beyond buffer size");
        }
    }

    std::vector<uint8_t> buffer_;
    size_t read_offset_;
};

}  // namespace server::network
