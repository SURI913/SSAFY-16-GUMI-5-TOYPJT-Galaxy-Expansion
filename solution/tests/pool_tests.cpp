#include "object_pool.hpp"
#include <iostream>
#include <stdexcept>
#include <string>

static void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
static const CosmicData initial{"First", "star", "#123456", 1.0};
static void reuse() {
    ObjectPool pool;
    auto* first = pool.Acquire(initial);
    const auto slot = first->poolSlotId;
    require(first->generation == 1, "new slot must start at generation 1");
    pool.Release(first);
    auto* second = pool.Acquire(initial);
    require(first == second, "must reuse the same object address");
    require(second->poolSlotId == slot && second->generation == 2,
            "slot must stay the same and generation must increase");
}
static void reinitialize() {
    ObjectPool pool;
    auto* first = pool.Acquire(initial);
    pool.Release(first);
    auto* second = pool.Acquire({"Second", "planet", "#abcdef", 3.0});
    require(second->data.name == "Second" && second->data.type == "planet" &&
            second->data.color == "#abcdef" && second->data.size == 3.0 &&
            second->active, "all fields must be reinitialized");
}
static void duplicate_return() {
    ObjectPool pool;
    auto* first = pool.Acquire(initial);
    pool.Release(first);
    bool rejected = false;
    try { pool.Release(first); }
    catch (const std::invalid_argument&) { rejected = true; }
    require(rejected, "duplicate return must throw invalid_argument");
    auto* a = pool.Acquire(initial);
    auto* b = pool.Acquire(initial);
    require(a != b && a->active && b->active,
            "two active acquisitions must never alias");
}
int main() {
    int failures = 0;
    const auto run = [&](const char* name, void (*test)()) {
        try { test(); std::cout << "PASS: " << name << '\n'; }
        catch (const std::exception& error) {
            ++failures;
            std::cerr << "FAIL: " << name << ": " << error.what() << '\n';
        }
    };
    run("same instance reuse", reuse);
    run("reinitialization", reinitialize);
    run("duplicate return", duplicate_return);
    return failures ? 1 : 0;
}
