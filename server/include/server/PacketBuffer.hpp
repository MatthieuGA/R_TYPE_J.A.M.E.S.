#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>

// C++20 alternative for std::byteswap
template <typename T>
constexpr T byteswap(T value) {
    if constexpr (sizeof(T) == 2) {
        return __builtin_bswap16(value);
    } else if constexpr (sizeof(T) == 4) {
        return __builtin_bswap32(value);
    } else if constexpr (sizeof(T) == 8) {
        return __builtin_bswap64(value);
    }
}

namespace server::network {

namespace detail {
template <typename T>
inline T to_little_endian(T value) {
    if constexpr (std::endian::native == std::endian::little) {
        return value;
    } else if constexpr (std::endian::native == std::endian::big) {
        return byteswap(value);
    } else {
        static_assert(std::endian::native == std::endian::little ||
                          std::endian::native == std::endian::big,
            "Mixed endianness not supported");
    }
}

template <typename T>
inline T from_little_endian(T value) {
    return to_little_endian(value);
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
    void write_header(const CommonHeader &header) {
        write_uint8(header.op_code);
        write_uint16(header.payload_size);
        write_uint8(header.packet_index);
        write_uint32(header.tick_id);
        write_uint8(header.packet_count);
        write_uint8(header.reserved[0]);
        write_uint8(header.reserved[1]);
        write_uint8(header.reserved[2]);
    }

    CommonHeader read_header() {
        CommonHeader header;
        header.op_code = read_uint8();
        header.payload_size = read_uint16();
        header.packet_index = read_uint8();
        header.tick_id = read_uint32();
        header.packet_count = read_uint8();
        header.reserved[0] = read_uint8();
        header.reserved[1] = read_uint8();
        header.reserved[2] = read_uint8();
        return header;
    }

    // Write primitive types (guaranteed little-endian)
    void write_uint8(uint8_t value) {
        buffer_.push_back(value);
    }

    void write_uint16(uint16_t value) {
        value = detail::to_little_endian(value);
        uint8_t bytes[2];
        std::memcpy(bytes, &value, 2);
        buffer_.push_back(bytes[0]);
        buffer_.push_back(bytes[1]);
    }

    void write_uint32(uint32_t value) {
        value = detail::to_little_endian(value);
        uint8_t bytes[4];
        std::memcpy(bytes, &value, 4);
        for (int i = 0; i < 4; ++i) {
            buffer_.push_back(bytes[i]);
        }
    }

    void write_uint64(uint64_t value) {
        value = detail::to_little_endian(value);
        uint8_t bytes[8];
        std::memcpy(bytes, &value, 8);
        for (int i = 0; i < 8; ++i) {
            buffer_.push_back(bytes[i]);
        }
    }

    void write_float(float value) {
        static_assert(sizeof(float) == 4);
        uint32_t bits;
        std::memcpy(&bits, &value, 4);
        write_uint32(bits);
    }

    void write_double(double value) {
        static_assert(sizeof(double) == 8);
        uint64_t bits;
        std::memcpy(&bits, &value, 8);
        write_uint64(bits);
    }

    // Read primitive types (guaranteed little-endian)
    uint8_t read_uint8() {
        check_bounds(1);
        return buffer_[read_offset_++];
    }

    uint16_t read_uint16() {
        check_bounds(2);
        uint16_t value;
        std::memcpy(&value, &buffer_[read_offset_], 2);
        read_offset_ += 2;
        return detail::from_little_endian(value);
    }

    uint32_t read_uint32() {
        check_bounds(4);
        uint32_t value;
        std::memcpy(&value, &buffer_[read_offset_], 4);
        read_offset_ += 4;
        return detail::from_little_endian(value);
    }

    uint64_t read_uint64() {
        check_bounds(8);
        uint64_t value;
        std::memcpy(&value, &buffer_[read_offset_], 8);
        read_offset_ += 8;
        return detail::from_little_endian(value);
    }

    float read_float() {
        uint32_t bits = read_uint32();
        float value;
        std::memcpy(&value, &bits, 4);
        return value;
    }

    double read_double() {
        uint64_t bits = read_uint64();
        double value;
        std::memcpy(&value, &bits, 8);
        return value;
    }

    // Buffer access
    const std::vector<uint8_t> &data() const {
        return buffer_;
    }

    std::vector<uint8_t> &data() {
        return buffer_;
    }

    size_t size() const {
        return buffer_.size();
    }

    size_t read_offset() const {
        return read_offset_;
    }

    size_t remaining() const {
        return buffer_.size() - read_offset_;
    }

    void reset_read_offset() {
        read_offset_ = 0;
    }

    void clear() {
        buffer_.clear();
        read_offset_ = 0;
    }

 private:
    void check_bounds(size_t bytes_needed) const {
        if (read_offset_ + bytes_needed > buffer_.size()) {
            throw std::out_of_range("PacketBuffer: read beyond buffer size");
        }
    }

    std::vector<uint8_t> buffer_;
    size_t read_offset_;
};

}  // namespace server::network
