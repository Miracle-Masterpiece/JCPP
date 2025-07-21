#include <cpp/lang/utils/smooth_noise.hpp>
#include <cpp/lang/math/math.hpp>

namespace jstd
{

    smooth_noise::smooth_noise(curve_func curve) : m_values(m_buffer, sizeof(m_buffer)), m_curve_func(curve) {

    }

    smooth_noise::smooth_noise(int64_t seed, curve_func curve) : smooth_noise(curve) {
        set_seed(seed);
        generate_values();
    }

    smooth_noise::smooth_noise(const smooth_noise& n) : smooth_noise(n.m_seed, n.m_curve_func) {
        std::memcpy(m_buffer, n.m_buffer, sizeof(smooth_noise));
    }

    smooth_noise::smooth_noise(smooth_noise&& n) : smooth_noise(n.m_seed, n.m_curve_func) {

    }
    
    smooth_noise& smooth_noise::operator=(const smooth_noise& n) {
        if (&n != this) {
            std::memcpy(m_buffer, n.m_buffer, sizeof(smooth_noise));
            m_seed          = n.m_seed;
            m_curve_func    = n.m_curve_func;
        }
        return *this;
    }
    
    smooth_noise& smooth_noise::operator=(smooth_noise&& n) {
        if (&n != this) {
            std::memcpy(m_buffer, n.m_buffer, sizeof(smooth_noise));
            m_seed          = n.m_seed;
            m_curve_func    = n.m_curve_func;
        }
        return *this;
    }
    
    smooth_noise::~smooth_noise() {

    }

    int64_t smooth_noise::get_seed() const {
        return m_seed;
    }
    
    void smooth_noise::set_seed(int64_t seed) {
        m_seed = seed;
    }

    void smooth_noise::generate_values() {
        random(m_seed).values<int8_t>(m_values.data(), m_values.length, num_limits<int8_t>::max());
    }

    float smooth_noise::value_at(int64_t x, int64_t y) const {
        int64_t hash = math::hash2d(x, y);
        return m_values[hash & (m_values.length - 1)] / (float) num_limits<int8_t>::max();
    }

    float smooth_noise::get(int64_t x, int64_t y, int32_t scale) const {
        if (scale == 0)
            return 0;
        int64_t xChunk = (x / scale);
        int64_t yChunk = (y / scale);

        int64_t xStart = xChunk * scale;
        int64_t yStart = yChunk * scale;

        float xc = (float) ((x - xStart) / (double) scale);
        float yc = (float) ((y - yStart) / (double) scale);

        float v0 = value_at(xChunk,     yChunk);
        float v1 = value_at(xChunk + 1, yChunk);

        float v2 = value_at(xChunk,     yChunk + 1);
        float v3 = value_at(xChunk + 1, yChunk + 1);

        xc = m_curve_func(xc);
        yc = m_curve_func(yc);
        
        float a = math::lerp(v0, v1, xc);
        float b = math::lerp(v2, v3, xc);
        float c = math::lerp(a, b, yc);

        return c;
    }

    float smooth_noise::get(int64_t x, int64_t y, int32_t scale, int32_t octaves, float scale_factor, float freeq_factor) const {
        JSTD_DEBUG_CODE(
            if (scale <= 0)
                throw_except<illegal_argument_exception>("Invalid scale: %lli", (long long) scale);
            if (octaves <= 0)
                throw_except<illegal_argument_exception>("Invalid count octaves: %lli", (long long) octaves);
            if (scale_factor < 0)
                throw_except<illegal_argument_exception>("Invalid scale_factor octaves: %f", scale_factor);
            if (freeq_factor < 0)
                throw_except<illegal_argument_exception>("Invalid freeq_factor octaves: %f", freeq_factor);
        );

        float out       = 0;
        float amplitude = 1;
        float freeq     = 1;
        float c         = 0;

        while (octaves-- > 0) {
            out += get(x, y, (int32_t) (scale * freeq)) * amplitude;
            c += amplitude;
            amplitude   *= scale_factor;
            freeq       *= freeq_factor;
        }

        return out / c;
    }

    void smooth_noise::set_curve_fuc(curve_func func) {
        JSTD_DEBUG_CODE(
            if (func == nullptr)
                throw_except<illegal_argument_exception>("func pointer == null");
        );
        m_curve_func = func;
    }

    /*static*/ float smooth_noise::no_smooth(float x) {
        return x;
    }
    
}