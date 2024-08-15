#pragma once

#include "Object.h"
#include "matrix/Matrix.h"

#include <functional>
#include <memory>
#include <vector>

namespace od {

class Graph{
public:
  Graph() = default;
  Graph(const Graph &) = default;
  Graph &operator=(const Graph &) = default;
  Graph(Graph &&) = default;
  Graph &operator=(Graph &&) = default;

  Graph(size_t N)
  : _adj(N, N)
  {}

  void add_edge(size_t u, size_t v)
  {
    _adj.get(u, v) = 1;
    _adj.get(v, u) = 1;
  }

  std::vector<std::vector<int>> find_subgraphs() const
  {

  }

private:
  matrix::Matrix<int> _adj;
};


class ObjectMerger {
public:
  ObjectMerger() = default;
  ObjectMerger(const ObjectMerger &) = default;
  ObjectMerger &operator=(const ObjectMerger &) = default;
  ObjectMerger(ObjectMerger &&) = default;
  ObjectMerger &operator=(ObjectMerger &&) = default;

  ObjectMerger(
      std::vector<std::shared_ptr<Object>> primary_objects,
      std::vector<std::shared_ptr<Object>> secondary_objects,
      std::function<void(std::shared_ptr<Object>, std::shared_ptr<Object>)>
          connect std::function<bool(std::shared_ptr<Object>,
                                     std::shared_ptr<Object>)>
              is_connected)
      : _primary_objects(primary_objects),
        _secondary_objects(secondary_objects), _connect(connect),
        is_connected(is_connected) {}

  void build_graph()
  {
    _graph = Graph(_primary_objects.size() + _secondary_objects.size());
    for (size_t i = 0; i < _primary_objects.size(); ++i)
    {
      for (size_t j = 0; j < _secondary_objects.size(); ++j)
      {
        if (_is_connected(_primary_objects[i], _secondary_objects[j]))
        {
          _graph.add_edge(i, j + _primary_objects.size());
        }
      }
    }
  }

private:
  std::vector<std::shared_ptr<Object>> _primary_objects;
  std::vector<std::shared_ptr<Object>> _secondary_objects;
  std::function<void(std::shared_ptr<Object>, std::shared_ptr<Object>)>
      _connect;
  std::function<bool(std::shared_ptr<Object>, std::shared_ptr<Object>)>
      _is_connected;
  Graph _graph;
};

} // namespace od