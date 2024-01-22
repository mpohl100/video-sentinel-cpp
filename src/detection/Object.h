#pragma once

#include "Slices.h"

#include "matrix/Matrix.h"

#include "Rectangle.h"

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
  Object(const Slices& slices) : slices{slices} {}

  bool try_merge_right(const Object& other){
    if(slices.touching_right(other.slices)){
      slices.merge_right(other.slices);
      return true;
    }
    return false;
  }

  Slices slices;
};

struct ObjectsPerRectangle {
  void append_right(const ObjectsPerRectangle& other){
    std::vector<Object> new_objects;
    // first add all objects that are not touching the right side
    for(const auto& object : objects){
      if(!std::find(objects_touching_right.begin(), objects_touching_right.end(), &object)){
        new_objects.push_back(object);
      }
    }
    // next add all objects from other that are not touching the left side
    for(const auto& object : other.objects){
      if(!std::find(other.objects_touching_left.begin(), other.objects_touching_left.end(), &object)){
        new_objects.push_back(object);
      }
    }
    // merge all objects
    std::vector<Object*> other_objects_touching_left = other.objects_touching_left;
    for(auto& object : objects_touching_right){
      std::vector<size_t> indexes_to_remove;
      size_t i = 0;
      for(auto* other_object : other_objects_touching_left){
        const auto merged = object->try_merge_right(other_object);
        if(merged){
          indexes_to_remove.push_back(i);
        }
        i++;
      }
      for(auto index : indexes_to_remove){
        other_objects_touching_left.erase(other_objects_touching_left.begin() + index);
      }
      new_objects.push_back(*object);
    }
    for(auto* object : other_objects_touching_left){
      new_objects.push_back(*object);
    }
    objects.clear();
    for(const auto& object : new_objects){
      insert_object(object);
    }
    rectangle = rectangle.merge_right(other.rectangle);
  }

  void append_down(const ObjectsPerRectangle& other){
    throw std::runtime_error("append_down not implemented");
  }

  void insert_object(const Object &object){
    objects.push_back(object);
    if(object.slices.touching_right(rectangle)){
      objects_touching_right.push_back(&objects.back());
    }
    if(object.slices.touching_left(rectangle)){
      objects_touching_left.push_back(&objects.back());
    }
    if(object.slices.touching_down(rectangle)){
      objects_touching_down.push_back(&objects.back());
    }
    if(object.slices.touching_up(rectangle)){
      objects_touching_up.push_back(&objects.back());
    }
  }

private:
  std::vector<Object> objects;
  Rectangle rectangle;
  std::vector<Object*> objects_touching_right;
  std::vector<Object*> objects_touching_left;
  std::vector<Object*> objects_touching_down;
  std::vector<Object*> objects_touching_up;
};

struct AllObjects {
  AllObjects() = default;
  AllObjects(const AllObjects &) = default;
  AllObjects(AllObjects &&) = default;
  AllObjects &operator=(const AllObjects &) = default;
  AllObjects &operator=(AllObjects &&) = default;
  AllObjects(size_t rows, size_t cols) : objects_per_rectangle{rows, cols} {}

  ObjectsPerRectangle &get(size_t x, size_t y) {
    return objects_per_rectangle.get(x, y);
  }

  size_t get_rows() const { return objects_per_rectangle.height(); }
private:
  matrix::Matrix<ObjectsPerRectangle> objects_per_rectangle;
};

} // namespace od