#pragma once
/*
Creation Date: 2022/10/11
Latest Update: 2023/04/02
Developer(s): 22-Qzh 21-THY
(C)Copyright: NJUST.Alliance - All rights reserved
Header Functions:
- 对大疆官方提供的CRC8校验的C++封装
- 此算法使用了自定义的参数模型，需要与互联网上常用的CRC校验区分开
*/

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <numeric>

namespace serial {
namespace verify {

// 内部中间类
namespace internal {

// CRC8基类，在类里定义table，继承它就能用
template <typename T>
class BaseCRC8 {
public:
    using ResultType = uint8_t;

protected:
    static uint8_t Calculate(uint8_t init, const uint8_t* data, size_t size) {
        uint8_t ucCRC8             = init;
        const uint8_t* pch_message = data;
        unsigned char uc_index;
        while (size--) {
            uc_index = ucCRC8 ^ (*pch_message++);
            ucCRC8   = T::table[uc_index];
        }
        return (ucCRC8);
    }
};

// 大疆和开源陀螺仪GY-H1用了同一个CRC映射表
class CommonBaseDjiGyH1CRC8 {
public:
    static constexpr uint8_t table[256] = {
        0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83, 0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f,
        0x41, 0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e, 0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60,
        0x82, 0xdc, 0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0, 0xe1, 0xbf, 0x5d, 0x03, 0x80,
        0xde, 0x3c, 0x62, 0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d, 0x7c, 0x22, 0xc0, 0x9e,
        0x1d, 0x43, 0xa1, 0xff, 0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5, 0x84, 0xda, 0x38,
        0x66, 0xe5, 0xbb, 0x59, 0x07, 0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58, 0x19, 0x47,
        0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a, 0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6, 0xa7,
        0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24, 0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
        0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9, 0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51,
        0x0f, 0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd, 0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e,
        0xcc, 0x92, 0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50, 0xaf, 0xf1, 0x13, 0x4d, 0xce,
        0x90, 0x72, 0x2c, 0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee, 0x32, 0x6c, 0x8e, 0xd0,
        0x53, 0x0d, 0xef, 0xb1, 0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73, 0xca, 0x94, 0x76,
        0x28, 0xab, 0xf5, 0x17, 0x49, 0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b, 0x57, 0x09,
        0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4, 0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16, 0xe9,
        0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a, 0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
        0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7, 0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b,
        0x35,
    };
};
}; // namespace internal

// 大疆的CRC8算法
// 这个类不可作为SerialUtil的模板参数传递
class DjiCRC8
    : public internal::CommonBaseDjiGyH1CRC8
    , public internal::BaseCRC8<DjiCRC8> {
public:
    static uint8_t Calculate(const uint8_t* data, size_t size) {
        return BaseCRC8::Calculate(0xff, data, size);
    }
};

// 开源陀螺仪GY-H1的CRC8算法
// 这个类不可作为SerialUtil的模板参数传递
class GyH1CRC8
    : public internal::CommonBaseDjiGyH1CRC8
    , public internal::BaseCRC8<GyH1CRC8> {
public:
    static uint8_t Calculate(const uint8_t* data, size_t size) {
        return BaseCRC8::Calculate(0x00, data, size);
    }
};

class CheckSum {
public:
    static uint8_t Calculate(const uint8_t* data, size_t size) {
        return std::accumulate(data, data + size, static_cast<uint8_t>(0));
    }
};

// 对数据包进行校验和的计算类，适用于CRC校验时包含包头，且校验位位于数据包尾部的简单情况。
// 一般作为SerialUtil的模板参数传递，也可以直接对数据包进行计算
// 若遇到CRC校验时不需包含包头，或校验位不位于数据包尾部的特殊情况，可以自己用实现了Append()和Verify()方法的自定义类作为模板参数传给SerialUtil
template <typename AlgorithmType>
class SimpleChecksumCalculator {
public:
    using ResultType = typename AlgorithmType::ResultType;

    template <typename T>
    static void Append(T& package) {
        static_assert(
            sizeof(T) >= sizeof(ResultType),
            "Size of package must be greater than size of result type.");

        ResultType checksum = AlgorithmType::Calculate(
            reinterpret_cast<const uint8_t*>(&package), sizeof(T) - sizeof(ResultType));
        *reinterpret_cast<ResultType*>(
            reinterpret_cast<uint8_t*>(&package) + sizeof(T) - sizeof(ResultType)) = checksum;
    }

    template <typename T>
    static bool Verify(const T& package) {
        static_assert(
            sizeof(T) >= sizeof(ResultType),
            "Size of package must be greater than size of result type.");

        ResultType checksum = AlgorithmType::Calculate(
            reinterpret_cast<const uint8_t*>(&package), sizeof(T) - sizeof(ResultType));
        return checksum
            == *reinterpret_cast<const ResultType*>(
                   reinterpret_cast<const uint8_t*>(&package) + sizeof(T) - sizeof(ResultType));
    }
};

// 对大疆的CRC8算法的进一步封装，适用于CRC校验时包含包头，且校验位位于数据包尾部的简单情况。
// 可对整个数据包进行计算，也可作为SerialUtil的模板参数传递
using DjiCRC8Calculator = SimpleChecksumCalculator<DjiCRC8>;

// 对开源陀螺仪GY-H1的CRC8算法的进一步封装，适用于CRC校验时包含包头，且校验位位于数据包尾部的简单情况。
// 可对整个数据包进行计算，也可作为SerialUtil的模板参数传递
using GyH1CRC8Calculator = SimpleChecksumCalculator<GyH1CRC8>;

using CheckSumCalculator = SimpleChecksumCalculator<CheckSum>;
}; // namespace verify
}; // namespace serial