#pragma once

#include "Object.h"
#include "ObjectMerger.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace od {

struct ObjectsPerRectangle {
  const std::vector<std::shared_ptr<Object>> &get_objects() const {
    return objects;
  }
  const std::vector<std::shared_ptr<Object>> &
  get_objects_touching_right() const {
    return objects_touching_right;
  }
  const std::vector<std::shared_ptr<Object>> &
  get_objects_touching_left() const {
    return objects_touching_left;
  }
  const std::vector<std::shared_ptr<Object>> &
  get_objects_touching_down() const {
    return objects_touching_down;
  }
  const std::vector<std::shared_ptr<Object>> &get_objects_touching_up() const {
    return objects_touching_up;
  }

  void set_rectangle(const Rectangle &rectangle) {
    this->rectangle = rectangle;
  }

  const Rectangle &get_rectangle() const { return rectangle; }

  void append_right(const ObjectsPerRectangle &other) {
    std::vector<std::shared_ptr<Object>> new_objects;
    // first add all objects that are not touching the right side
    for (const auto &object : objects) {
      if (std::find(objects_touching_right.begin(),
                    objects_touching_right.end(),
                    object) == objects_touching_right.end()) {
        new_objects.push_back(object);
      }
    }
    // next add all objects from other that are not touching the left side
    for (const auto &object : other.objects) {
      if (std::find(other.objects_touching_left.begin(),
                    other.objects_touching_left.end(),
                    object) == other.objects_touching_left.end()) {
        new_objects.push_back(object);
      }
    }
    // merge all objects

    auto object_merger = od::ObjectMerger{
        objects_touching_right, other.objects_touching_left,
        [](std::shared_ptr<Object> object1, std::shared_ptr<Object> object2) {
          object1->try_merge_right(*object2);
          return object1;
        },
        [](const std::shared_ptr<Object> &object1,
           const std::shared_ptr<Object> &object2) {
          return object1->touching_right(*object2);
        }};
    auto merged_objects = object_merger.connect_all_objects();

    for(const auto &object : merged_objects) {
      new_objects.push_back(object);
    }

    objects.clear();
    objects_touching_down.clear();
    objects_touching_up.clear();
    objects_touching_left.clear();
    objects_touching_right.clear();
    rectangle.merge_right(other.rectangle);
    for (const auto &object : new_objects) {
      insert_object(object);
    }
  }

  void append_down(const ObjectsPerRectangle &other) {
    std::vector<std::shared_ptr<Object>> new_objects;
    // first add all objects that are not touching the down side
    for (const auto &object : objects) {
      if (std::find(objects_touching_down.begin(), objects_touching_down.end(),
                    object) == objects_touching_down.end()) {
        new_objects.push_back(object);
      }
    }
    // next add all objects from other that are not touching the up side
    for (const auto &object : other.objects) {
      if (std::find(other.objects_touching_up.begin(),
                    other.objects_touching_up.end(),
                    object) == other.objects_touching_up.end()) {
        new_objects.push_back(object);
      }
    }
    // merge all objects
    
    auto object_merger = od::ObjectMerger{
        objects_touching_down, other.objects_touching_up,
        [](std::shared_ptr<Object> object1, std::shared_ptr<Object> object2) {
          object1->try_merge_down(*object2);
          return object1;
        },
        [](const std::shared_ptr<Object> &object1,
           const std::shared_ptr<Object> &object2) {
          return object1->touching_down(*object2);
        }};
    auto merged_objects = object_merger.connect_all_objects();

    for(const auto &object : merged_objects) {
      new_objects.push_back(object);
    }

    objects.clear();
    objects_touching_down.clear();
    objects_touching_up.clear();
    objects_touching_left.clear();
    objects_touching_right.clear();
    rectangle.merge_down(other.rectangle);
    for (const auto &object : new_objects) {
      insert_object(object);
    }
  }

  void insert_object(std::shared_ptr<Object> object) {
    objects.push_back(object);
    if (object->slices.touching_right(rectangle)) {
      objects_touching_right.push_back(object);
    }
    if (object->slices.touching_left(rectangle)) {
      objects_touching_left.push_back(object);
    }
    if (object->slices.touching_down(rectangle)) {
      objects_touching_down.push_back(object);
    }
    if (object->slices.touching_up(rectangle)) {
      objects_touching_up.push_back(object);
    }
  }

private:
  void
  append(std::function<const std::vector<std::shared_ptr<Object>> &(
             const ObjectsPerRectangle &)>
             get_objects,
         std::function<const std::vector<std::shared_ptr<Object>> &(
             const ObjectsPerRectangle &)>
             get_touching_objects_this,
         std::function<const std::vector<std::shared_ptr<Object>> &(
             const ObjectsPerRectangle &)>
             get_touching_objects_other,
         std::function<void(Rectangle &, const Rectangle &)> merge_rectangles,
         const ObjectsPerRectangle &other) {
    std::vector<std::shared_ptr<Object>> new_objects;
    // first add all objects that are not touching the right side
    const auto touching_objects_this = get_touching_objects_this(*this);
    for (auto object : get_objects(*this)) {
      if (std::find(touching_objects_this.begin(), touching_objects_this.end(),
                    object) == touching_objects_this.end()) {
        new_objects.push_back(object);
      }
    }
    // next add all objects from other that are not touching the left side
    auto touching_objects_other = get_touching_objects_other(other);
    for (auto object : get_objects(other)) {
      if (std::find(touching_objects_other.begin(),
                    touching_objects_other.end(),
                    object) == touching_objects_other.end()) {
        new_objects.push_back(object);
      }
    }
    // merge all objects
    for (auto &object : touching_objects_this) {
      std::vector<size_t> indexes_to_remove;
      size_t i = 0;
      for (auto other_object : touching_objects_other) {
        const auto merged = object->try_merge_right(*other_object);
        if (merged) {
          indexes_to_remove.push_back(i);
        }
        i++;
      }
      for (auto index : indexes_to_remove) {
        touching_objects_other.erase(touching_objects_other.begin() + index);
      }
      new_objects.push_back(object);
    }
    for (auto object : touching_objects_other) {
      new_objects.push_back(object);
    }
    objects.clear();
    objects_touching_down.clear();
    objects_touching_up.clear();
    objects_touching_left.clear();
    objects_touching_right.clear();
    merge_rectangles(rectangle, other.rectangle);
    for (const auto &object : new_objects) {
      insert_object(object);
    }
  }

  std::vector<std::shared_ptr<Object>> objects;
  Rectangle rectangle;
  std::vector<std::shared_ptr<Object>> objects_touching_right;
  std::vector<std::shared_ptr<Object>> objects_touching_left;
  std::vector<std::shared_ptr<Object>> objects_touching_down;
  std::vector<std::shared_ptr<Object>> objects_touching_up;
};

} // namespace od