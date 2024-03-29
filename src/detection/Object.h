#pragma once

#include "Slices.h"

#include "matrix/Matrix.h"

#include "Rectangle.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace od {

struct Object {
  Object() = default;
  Object(const Object &) = default;
  Object(Object &&) = default;
  Object &operator=(const Object &) = default;
  Object &operator=(Object &&) = default;
  Object(const Slices &slices) : slices{slices} {}

  bool try_merge_right(Object &other) {
    bool do_log = false;
    if (get_bounding_box().to_math2d_rectangle().area() > 100 &&
        other.get_bounding_box().to_math2d_rectangle().area() > 100) {
      do_log = true;
      std::cout << "Encountered two bigger rectangles";
    }
    if (slices.touching_right(other.slices)) {
      slices.merge_right(other.slices);
      if (do_log) {
        std::cout << "Merged right: " << slices.to_rectangle().to_string()
                  << " with " << other.slices.to_rectangle().to_string()
                  << std::endl;
      }
      return true;
    }
    if (do_log) {
      std::cout << "Did not merge right: " << slices.to_rectangle().to_string()
                << " with " << other.slices.to_rectangle().to_string()
                << std::endl;
    }
    return false;
  }

  bool try_merge_down(Object &other) {
    if (slices.touching_down(other.slices)) {
      slices.merge_down(other.slices);
      return true;
    }
    return false;
  }

  od::Rectangle get_bounding_box() const { return slices.to_rectangle(); }

  Slices slices;
};

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
    for (auto &object : objects_touching_right) {
      std::vector<size_t> indexes_to_remove;
      size_t i = 0;
      for (auto other_object : other_objects_touching_left) {
        const auto merged = object->try_merge_right(*other_object);
        if (merged) {
          indexes_to_remove.push_back(i);
        }
        i++;
      }
      for (auto index : indexes_to_remove) {
        other_objects_touching_left.erase(other_objects_touching_left.begin() +
                                          index);
      }
      new_objects.push_back(object);
    }
    for (auto object : other_objects_touching_left) {
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
    std::vector<std::shared_ptr<Object>> other_objects_touching_up =
        other.objects_touching_up;
    for (auto &object : objects_touching_down) {
      std::vector<size_t> indexes_to_remove;
      size_t i = 0;
      for (auto other_object : other_objects_touching_up) {
        const auto merged = object->try_merge_down(*other_object);
        if (merged) {
          indexes_to_remove.push_back(i);
        }
        i++;
      }
      for (auto index : indexes_to_remove) {
        other_objects_touching_up.erase(other_objects_touching_up.begin() +
                                        index);
      }
      new_objects.push_back(object);
    }
    for (auto object : other_objects_touching_up) {
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

struct AllObjects {
  AllObjects() = default;
  AllObjects(const AllObjects &) = default;
  AllObjects(AllObjects &&) = default;
  AllObjects &operator=(const AllObjects &) = default;
  AllObjects &operator=(AllObjects &&) = default;
  AllObjects(size_t rows, size_t cols) : objects_per_rectangle{cols, rows} {}

  ObjectsPerRectangle &get(size_t row, size_t col) {
    return objects_per_rectangle.get(col, row);
  }

  size_t get_rows() const { return objects_per_rectangle.height(); }
  size_t get_cols() const { return objects_per_rectangle.width(); }

private:
  matrix::Matrix<ObjectsPerRectangle> objects_per_rectangle;
};

} // namespace od