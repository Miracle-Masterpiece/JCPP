#include <cpp/lang/string_pool.hpp>

namespace jstd
{
    using scp = string_const_pool;
    
    scp::string_const_pool(tca::base_allocator* allocator) : m_allocator(allocator), m_map(allocator) {

    }

    scp::~string_const_pool() {

    }
    
    shared_ptr<const string> scp::get(const char* str) {
        JSTD_DEBUG_CODE(
            if (m_allocator == nullptr)
                throw_except<illegal_state_exception>("allocator is null");
        );
        shared_ptr<const string>* in_map = m_map.get(cstr(str));
        if (in_map == nullptr) {
            tca::scope_allocator scope = m_allocator;
            shared_ptr<const string> shared = 
                                                make_shared<const string>(string(str));
            m_map.put(str, shared);
            return shared;
        }

        else {
            return *in_map;
        }
    }

    bool scp::deintern(const char* str) {
        JSTD_DEBUG_CODE(
            if (str == nullptr)
                throw_except<null_pointer_exception>("str is null");
        );
        return m_map.remove(str);
    }
    
    bool scp::deintern(shared_ptr<const string> str) {
        return deintern(str->c_string());
    }

    void scp::print_log() const {
        for (const map_node<cstr, shared_ptr<const string>>& entry : m_map)
            system::tsprintf("[%s]\n", entry.get_value()->c_string());
    }

    bool scp::intern(const char* str) {
        JSTD_DEBUG_CODE(
            if (m_allocator == nullptr)
                throw_except<illegal_state_exception>("allocator is null");
        );
        tca::scope_allocator scope = m_allocator;
        return m_map.put(str, make_shared<const string>(str));
    }

    int32_t scp::to_string(char buf[], int32_t bufsize) const {
        return std::snprintf(buf, bufsize, "string_poool [size: %lli]", (long long) m_map.size());
    }

    void scp::clear() {
        m_map.clear();
    }

    /**
     * ###################################################################
     *      C O N C U R R E N C Y _ S T R I N G _ C O N S T _ P O O l
     * ###################################################################
     * 
     */

    using cscp = concurrency_string_const_pool;
    cscp::concurrency_string_const_pool(tca::base_allocator* allocator) : m_strpool(allocator), m_mutex() {

    }

    cscp::~concurrency_string_const_pool() {

    }
    
    shared_ptr<const string> cscp::get(const char* str) {
        unique_lock lock(m_mutex);
        return m_strpool.get(str);
    }
    
    bool cscp::intern(const char* str) {
        unique_lock lock(m_mutex);
        return m_strpool.intern(str);
    }
    
    bool cscp::deintern(const char* str) {
        unique_lock lock(m_mutex);
        return m_strpool.deintern(str);
    }
    
    bool cscp::deintern(shared_ptr<const string> str) {
        unique_lock lock(m_mutex);
        return m_strpool.deintern(str);
    }
    
    void cscp::print_log() const {
        m_strpool.print_log();
    }
    
    int32_t cscp::to_string(char buf[], int32_t bufsize) const {
        return m_strpool.to_string(buf, bufsize);
    }

    void cscp::clear() {
        m_strpool.clear();
    }
}