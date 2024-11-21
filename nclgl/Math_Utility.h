#pragma once

class Math_Utility {
public:
    static float Lerp(float from, float to, float by) {
        return (1.0f - by) * from + by * to;
    }

    static float SmoothStep(float t) {
        t = Clamp(t, 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    template<typename T>
    static constexpr const T& Clamp(const T& value, const T& lower, const T& upper) {
        return (value < lower) ? lower : (value > upper) ? upper : value;
    }
};