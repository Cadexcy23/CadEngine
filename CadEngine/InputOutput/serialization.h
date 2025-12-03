#pragma once
#include <string>
#include <vector>

static class Serialization {
public:

    static void append_uint8(std::vector<uint8_t>& b, uint8_t v) { b.push_back(v); }
    static void append_u32(std::vector<uint8_t>& b, uint32_t v) {
        uint8_t* p = (uint8_t*)&v;
        b.insert(b.end(), p, p + sizeof(v));
    }
    static void append_i32(std::vector<uint8_t>& b, int32_t v) {
        uint8_t* p = (uint8_t*)&v;
        b.insert(b.end(), p, p + sizeof(v));
    }
    static void append_float(std::vector<uint8_t>& b, float v) {
        uint8_t* p = (uint8_t*)&v;
        b.insert(b.end(), p, p + sizeof(v));
    }
    static void append_double(std::vector<uint8_t>& b, double v) {
        uint8_t* p = (uint8_t*)&v;
        b.insert(b.end(), p, p + sizeof(v));
    }
    static void append_string(std::vector<uint8_t>& b, const std::string& s) {
        append_u32(b, (uint32_t)s.size());
        b.insert(b.end(), s.begin(), s.end());
    }
    static uint8_t read_u8(const std::vector<uint8_t>& b, size_t& idx) { return b[idx++]; }
    static uint32_t read_u32(const std::vector<uint8_t>& b, size_t& idx) {
        uint32_t v; memcpy(&v, b.data() + idx, sizeof(v)); idx += sizeof(v); return v;
    }
    static int32_t read_i32(const std::vector<uint8_t>& b, size_t& idx) {
        int32_t v; memcpy(&v, b.data() + idx, sizeof(v)); idx += sizeof(v); return v;
    }
    static float read_float(const std::vector<uint8_t>& b, size_t& idx) {
        float v; memcpy(&v, b.data() + idx, sizeof(v)); idx += sizeof(v); return v;
    }
    static double read_double(const std::vector<uint8_t>& b, size_t& idx) {
        double v; memcpy(&v, b.data() + idx, sizeof(v)); idx += sizeof(v); return v;
    }
    static std::string read_string(const std::vector<uint8_t>& b, size_t& idx) {
        uint32_t n = read_u32(b, idx);
        std::string s((char*)b.data() + idx, (char*)b.data() + idx + n);
        idx += n;
        return s;
    }

	static std::string getSetting(const std::string& setting);
	static void setSetting(const std::string& setting, const std::string& newValue);
};