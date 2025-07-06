#include <cpp/lang/utils/random.hpp>
#include <cmath>

#define abs(x) (x < 0 ? -x : x)

namespace jstd
{

    uint64_t random::next0() {
        // const uint64_t a = 25214903917L;                            // Multiplier constant.
        // const uint64_t c = abs(next1() + next1()) & 0xfff;          // Increment constant.
        // m_seed = (m_seed * a + c);
        // return m_seed;
        m_seed = next1();
        return m_seed;
    }
    
    uint64_t random::next1() {
        const uint64_t a = 25214903917L;
        const uint64_t c = 17;
        return (m_seed * a + c);
    }
    
    random::random(int64_t seed) : m_seed(seed) {

    }
    
    random::random(const random& rnd) : m_seed(rnd.m_seed)  {

    }
    
    random::random(random&& rnd) : m_seed(rnd.m_seed) {

    }
    
    random& random::operator=(const random& rnd) {
        m_seed = rnd.m_seed;
        return *this;
    }
    
    random& random::operator=(random&& rnd) {
        m_seed = rnd.m_seed;
        return *this;
    }
    
    random::~random() {

    }

    template<>
    float random::next() {
        int64_t v = next0();
        return abs(v) / (float) num_limits<int64_t>::max();
    }

    template<>
    double random::next() {
        int64_t v = next0();
        return abs(v) / (double) num_limits<int64_t>::max();
    }

    template<>
    long double random::next() {
        int64_t v = next0();
        return abs(v) / (long double) num_limits<int64_t>::max();
    }

}