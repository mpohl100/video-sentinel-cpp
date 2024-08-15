#pragma once

#include "Object.h"

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
    std::vector<std::shared_ptr<Object>> other_objects_touching_left =
        other.objects_touching_left;
    struct Merger {
      std::shared_ptr<Object> object_touching_right;
      std::shared_ptr<Object> object_touching_left;
    };
    // accumulate all mergers
    std::vector<Merger> mergers;
    std::vector<std::shared_ptr<Object>> not_to_be_merged;
    std::set<size_t> indexes_to_merge_left;
    std::set<size_t> indexes_to_merge_right;
    size_t i = 0;
    for (auto &object : objects_touching_right) {
      size_t j = 0;
      for (auto other_object : other_objects_touching_left) {
        const auto merge_candidate = object->touching_right(*other_object);
        if (merge_candidate) {
          indexes_to_merge_left.insert(i);
          indexes_to_merge_right.insert(j);
          mergers.push_back({object, other_object});
        }
        j++;
      }
      i++;
    }

    // do one merger at a time and update remaining mergers
    i = 0;
    for (const auto &merger : mergers) {
      // do the merger and update all remaining mergers
      merger.object_touching_left->try_merge_right(
          *merger.object_touching_right);
      i++;
      for (size_t j = i; j < mergers.size(); ++j) {
        if (mergers[j].object_touching_right == merger.object_touching_right) {
          mergers[j].object_touching_right = merger.object_touching_left;
        }
      }
    }
    // collect all merged objects
    std::vector<std::shared_ptr<Object>> new_merged_objects;
    for (const auto &merger : mergers) {
      new_merged_objects.push_back(merger.object_touching_left);
    }
    std::unique(new_merged_objects.begin(), new_merged_objects.end());
    for (auto object : new_merged_objects) {
      new_objects.push_back(object);
    }

    // collect all not merged objects
    i = 0;
    for (auto object : objects_touching_right) {
      if (indexes_to_merge_left.find(i) == indexes_to_merge_left.end()) {
        new_objects.push_back(object);
      }
      i++;
    }

    size_t j = 0;
    for (auto object : other_objects_touching_left) {
      if (indexes_to_merge_right.find(j) == indexes_to_merge_right.end()) {
        new_objects.push_back(object);
      }
      j++;
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
    std::vector<std::shared_ptr<Object>> other_objects_touching_up =
        other.objects_touching_up;

    struct Merger {
      std::shared_ptr<Object> object_touching_down;
      std::shared_ptr<Object> object_touching_up;
    };

    // accumulate all mergers
    std::vector<Merger> mergers;
    std::vector<std::shared_ptr<Object>> not_to_be_merged;
    std::set<size_t> indexes_to_merge_down;
    std::set<size_t> indexes_to_merge_up;
    size_t i = 0;
    for (auto &object : objects_touching_down) {
      size_t j = 0;
      for (auto other_object : other_objects_touching_up) {
        const auto merge_candidate = object->touching_down(*other_object);
        if (merge_candidate) {
          indexes_to_merge_down.insert(i);
          indexes_to_merge_up.insert(j);
          mergers.push_back({object, other_object});
        }
        j++;
      }
      i++;
    }

    // do one merger at a time and update remaining mergers
    i = 0;
    for (const auto &merger : mergers) {
      // do the merger and update all remaining mergers
      merger.object_touching_down->try_merge_down(
          *merger.object_touching_up);
      i++;
      for (size_t j = i; j < mergers.size(); ++j) {
        if (mergers[j].object_touching_up == merger.object_touching_up) {
          mergers[j].object_touching_up = merger.object_touching_down;
        }
      }
    }
    // collect all merged objects
    std::vector<std::shared_ptr<Object>> new_merged_objects;
    for (const auto &merger : mergers) {
      new_merged_objects.push_back(merger.object_touching_down);
    }
    std::unique(new_merged_objects.begin(), new_merged_objects.end());
    for (auto object : new_merged_objects) {
      new_objects.push_back(object);
    }

    // collect all not merged objects
    i = 0;
    for (auto object : objects_touching_down) {
      if (indexes_to_merge_down.find(i) == indexes_to_merge_down.end()) {
        new_objects.push_back(object);
      }
      i++;
    }

    size_t j = 0;
    for (auto object : other_objects_touching_up) {
      if (indexes_to_merge_up.find(j) == indexes_to_merge_up.end()) {
        new_objects.push_back(object);
      }
      j++;
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

}