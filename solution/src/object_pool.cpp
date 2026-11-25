#include "object_pool.hpp"
#include <stdexcept>
#include <utility>

CosmicObject* ObjectPool::Acquire(const CosmicData& data) {
    if (!available_.empty()) {

        auto obj = available_.back();

        obj->data = data;
        obj->active = true;
        obj->generation++;

        available_.pop_back();

        return obj;
    }
    
    //없으면 새로 생성해서 반환
    auto newObj = std::make_unique<CosmicObject>(
        CosmicObject{ data, std::to_string(object_.size()), 1, true }
    );

    CosmicObject* obj = newObj.get();
    object_.insert({obj, std::move(newObj)});// value는 소유권이여야만(복사 X/주소값만 전달 X)

    return obj;
}

void ObjectPool::Release(CosmicObject* object) {

    if(object == nullptr) throw std::invalid_argument("빈 객체");

    if (!object_.contains(object)) throw std::invalid_argument("풀에 없는 객체");

    if (!object->active) throw std::invalid_argument("이미 반환된 객체");

    available_.push_back(object);
    object->active = false;
    //순서 빼먹지마라
}
