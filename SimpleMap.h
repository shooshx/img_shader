#pragma once

template<typename K, typename V, int MaxSZ>
class SimpleMap
{
private:
    struct Pair {
        K key;
        V value;
        bool occupied;
    };
    Pair m_kv[MaxSZ];
public:
    SimpleMap() {
        memset(m_kv, 0, sizeof(m_kv));
    }
    bool set(K k, V v) {
        for (int i = 0; i < MaxSZ; ++i)
            if (m_kv[i].key == k) {
                m_kv[i].value = v;
                return true;
            }
        for(int i = 0; i < MaxSZ; ++i)
            if (!m_kv[i].occupied) {
                m_kv[i] = Pair{k,v,true};
                return true;
            }
        return false;
    }
    V get(K k) {
        for (int i = 0; i < MaxSZ; ++i)
            if (m_kv[i].key == k)
                return m_kv[i].value;
        return 0;
    }
    void remove(K k) {
        for (int i = 0; i < MaxSZ; ++i)
            if (m_kv[i].key == k)
                m_kv[i].occupied = false;
    }

    Pair* begin() {
        for (int i = 0; i < MaxSZ; ++i)
            if (m_kv[i].occupied) 
                &m_kv[i];
        return end(); // one after the last one
    }
    Pair* end() {
        return &m_kv[MaxSZ];
    }

};