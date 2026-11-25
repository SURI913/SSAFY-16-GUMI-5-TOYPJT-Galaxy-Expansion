#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

struct CosmicData {
    std::string name;
    std::string type;
    std::string color;
    double size;
};

struct CosmicObject {
    CosmicData data;
    std::string poolSlotId;
    std::uint64_t generation = 0;
    bool active = false;
};

// The pool owns its objects. Callers borrow pointers only until Release.
// Do not dereference a returned pointer or use it after the pool is destroyed.
// Decide storage and capacity policy before implementing the TODOs.
class ObjectPool {
public:
    ObjectPool() = default;
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    CosmicObject* Acquire(const CosmicData& data);
    void Release(CosmicObject* object);
private:
    std::vector<CosmicObject*> available_; //재사용 가능한 객체 주소
    std::unordered_map< CosmicObject* ,std::unique_ptr<CosmicObject>> object_; //실제 객체
    
};
