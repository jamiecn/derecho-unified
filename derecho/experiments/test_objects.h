/**
 * @file test_objects.h
 *
 * Some ReplicatedObject definitions used by typed_subgroup_test and variants.
 */

#pragma once
#include <string>
#include <map>
#include <memory>

#include "derecho/derecho.h"
#include <mutils-serialization/SerializationSupport.hpp>

/**
 * Example replicated object, containing some serializable state and providing
 * two RPC methods.
 */
struct Foo{

    int state;

    int read_state() {
        return state;
    }
    bool change_state(int new_state) {
        if(new_state == state) {
            return false;
        }
        state = new_state;
        return true;
    }

    REGISTER_RPC_FUNCTIONS(Foo, read_state, change_state);

    /**
     * Constructs a Foo with an initial value.
     * @param initial_state
     */
    Foo(int initial_state = 0) : state(initial_state) {}
    Foo() = default;
    Foo(const Foo&) = default;
};

static_assert(std::is_standard_layout<Foo>::value, "Erorr: Foo not standard layout");
static_assert(std::is_pod<Foo>::value, "Erorr: Foo not POD");
static_assert(sizeof(Foo) == sizeof(int), "Error: RTTI?");

struct Faz{
    static constexpr std::size_t test_array_size = 131072;

    std::array<std::size_t,test_array_size> state;

    std::array<std::size_t,test_array_size> read_state() {
        whendebug(std::cout << std::endl << "executing read_state"  << std::endl <<std::endl);
        return state;
    }
    void change_state(std::array<std::size_t,test_array_size> new_state) {
        whendebug(std::cout << std::endl << "executing change_state "  << new_state[0] << std::endl <<std::endl);
        if(new_state == state) {
            return/* false*/;
        }
        state = new_state;
        return/* true*/;
    }

    REGISTER_RPC_FUNCTIONS(Faz, read_state, change_state);

    /**
     * Constructs a Faz with an initial value.
     * @param initial_state
     */
    Faz() = default;
    Faz(const Faz&) = default;
};

static_assert(std::is_standard_layout<Faz>::value, "Erorr: Faz not standard layout");
static_assert(std::is_pod<Faz>::value, "Erorr: Faz not POD");
static_assert(sizeof(Faz) == sizeof(std::size_t)*Faz::test_array_size, "Error: RTTI?");


class Bar : public mutils::ByteRepresentable {
    std::string log;

public:
    void append(const std::string& words) {
        log += words;
    }
    void clear() {
        log.clear();
    }
    std::string print() {
        return log;
    }

    REGISTER_RPC_FUNCTIONS(Bar, append, clear, print);

    DEFAULT_SERIALIZATION_SUPPORT(Bar, log);
    Bar(const std::string& s = "") : log(s) {}
};

class Cache : public mutils::ByteRepresentable {
    std::map<std::string, std::string> cache_map;

public:
    void put(const std::string& key, const std::string& value) {
        cache_map[key] = value;
    }
    std::string get(const std::string& key) {
        return cache_map[key];
    }
    bool contains(const std::string& key) {
        return cache_map.find(key) != cache_map.end();
    }
    bool invalidate(const std::string& key) {
        auto key_pos = cache_map.find(key);
        if(key_pos == cache_map.end()) {
            return false;
        }
        cache_map.erase(key_pos);
        return true;
    }

    REGISTER_RPC_FUNCTIONS(Cache, put, get, contains, invalidate);

    Cache() : cache_map() {}
    /**
     * This constructor is required by default serialization support, in order
     * to reconstruct an object after deserialization.
     * @param cache_map The state of the cache.
     */
    Cache(const std::map<std::string, std::string>& cache_map) : cache_map(cache_map) {}

    DEFAULT_SERIALIZATION_SUPPORT(Cache, cache_map);
};
