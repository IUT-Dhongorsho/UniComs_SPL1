#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>

namespace ADPCM {

static const int STEP_TABLE[89] = {
    7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,
    50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,
    253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,
    1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,
    3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,
    10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,
    27086,29794,32767
};

static const int INDEX_TABLE[16] = {
    -1,-1,-1,-1,2,4,6,8,
    -1,-1,-1,-1,2,4,6,8
};

struct State { int16_t predictor = 0; int stepIndex = 0; };

inline std::vector<uint8_t> encode(const std::vector<int16_t> &pcm, State &s)
{
    std::vector<uint8_t> out;
    out.reserve((pcm.size() + 1) / 2);
    for (size_t i = 0; i < pcm.size(); i += 2) {
        uint8_t byte = 0;
        for (int n = 0; n < 2; ++n) {
            int sample = (i + n < pcm.size()) ? pcm[i + n] : 0;
            int step   = STEP_TABLE[s.stepIndex];
            int diff   = sample - s.predictor;
            int code   = 0;
            if (diff < 0) { code = 8; diff = -diff; }
            if (diff >= step)     { code |= 4; diff -= step; }
            if (diff >= step / 2) { code |= 2; diff -= step / 2; }
            if (diff >= step / 4) { code |= 1; }
            int delta = step >> 3;
            if (code & 4) delta += step;
            if (code & 2) delta += step >> 1;
            if (code & 1) delta += step >> 2;
            if (code & 8) delta = -delta;
            s.predictor = static_cast<int16_t>(std::clamp(s.predictor + delta, -32768, 32767));
            s.stepIndex = std::clamp(s.stepIndex + INDEX_TABLE[code & 0xF], 0, 88);
            byte |= (code & 0xF) << (n * 4);
        }
        out.push_back(byte);
    }
    return out;
}

inline std::vector<int16_t> decode(const std::vector<uint8_t> &data, State &s)
{
    std::vector<int16_t> out;
    out.reserve(data.size() * 2);
    for (uint8_t byte : data) {
        for (int n = 0; n < 2; ++n) {
            int code  = (byte >> (n * 4)) & 0xF;
            int step  = STEP_TABLE[s.stepIndex];
            int delta = step >> 3;
            if (code & 4) delta += step;
            if (code & 2) delta += step >> 1;
            if (code & 1) delta += step >> 2;
            if (code & 8) delta = -delta;
            s.predictor = static_cast<int16_t>(std::clamp(s.predictor + delta, -32768, 32767));
            s.stepIndex = std::clamp(s.stepIndex + INDEX_TABLE[code], 0, 88);
            out.push_back(s.predictor);
        }
    }
    return out;
}

} // namespace ADPCM