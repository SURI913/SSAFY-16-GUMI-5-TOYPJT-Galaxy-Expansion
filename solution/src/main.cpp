#include "object_pool.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
using nlohmann::json;

static void wait_ack(const std::string& action) {
    std::string line;
    if (!std::getline(std::cin, line)) throw std::runtime_error("Bridge closed before ACK");
    const auto reply = json::parse(line);
    if (reply.value("type", "") == "error")
        throw std::runtime_error(reply.value("message", "Relay rejected event"));
    if (reply.value("type", "") != "ack" || reply.value("action", "") != action)
        throw std::runtime_error("Unexpected ACK for " + action);
    std::cerr << "[ack] " << action << '\n';
}
static void emit_acquire(const CosmicObject& object) {
    const auto& d = object.data;
    std::cout << json{{"type", "acquire"}, {"poolSlotId", object.poolSlotId},
        {"generation", object.generation},
        {"data", {{"name", d.name}, {"type", d.type},
                  {"color", d.color}, {"size", d.size}}}}.dump() << std::endl;
}

int main(int argc, char** argv) {
    try {
        bool smoke = argc == 2 && std::string(argv[1]) == "--smoke";
        std::string line;
        if (!std::getline(std::cin, line)) throw std::runtime_error("Missing init");
        auto init = json::parse(line);
        if (init.at("type") != "init" || !init.at("participantId").is_string())
            throw std::runtime_error("Expected init with participantId");
        if (smoke) {
            std::cerr << "PASS: JSONL entry point received init (pool not tested)\n";
            return 0;
        }
        std::cout << json{{"type", "reset"}}.dump() << std::endl;
        wait_ack("reset");
        ObjectPool pool;
        auto* first = pool.Acquire({"Aurora", "star", "#5dcaff", 1.2});
        emit_acquire(*first);
        wait_ack("acquire");
        const auto slot = first->poolSlotId;
        const auto generation = first->generation;
        pool.Release(first);
        std::cout << json{{"type", "release"}, {"poolSlotId", slot},
            {"generation", generation}}.dump() << std::endl;
        wait_ack("release");
        auto* second = pool.Acquire({"Reborn", "planet", "#a78bfa", 1.6});
        emit_acquire(*second);
        wait_ack("acquire");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
