#pragma once

#include "Object.h"
#include "matrix/Matrix.h"

#include <functional>
#include <memory>
#include <vector>

namespace od {

// Function to perform a Depth-First Search (DFS) to explore all vertices connected to vertex `v`
inline void DFS(int v, const matrix::Matrix<int>& adjMatrix, std::vector<bool>& visited, std::vector<int>& subgraph) {
    visited[v] = true;
    subgraph.push_back(v);  // Add vertex `v` to the current subgraph
    
    for (int i = 0; i < adjMatrix.width(); i++) {
        if (adjMatrix.get(v, i) == 1 && !visited[i]) {  // If there's an edge and vertex `i` is not visited
            DFS(i, adjMatrix, visited, subgraph);
        }
    }
}

class Graph {
public:
  Graph() = default;
  Graph(const Graph &) = default;
  Graph &operator=(const Graph &) = default;
  Graph(Graph &&) = default;
  Graph &operator=(Graph &&) = default;

  Graph(size_t N) : _adj(N, N) {}

  void add_edge(size_t u, size_t v) {
    _adj.get(u, v) = 1;
    _adj.get(v, u) = 1;
  }

  std::vector<std::vector<int>> find_subgraphs() const {
    int n = _adj.width();
    std::vector<bool> visited(n, false);
    std::vector<std::vector<int>> subgraphs;

    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            std::vector<int> subgraph;
            DFS(i, _adj, visited, subgraph);
            subgraphs.push_back(subgraph);
        }
    }

    return subgraphs;
  }

  std::vector<int> get_connections(size_t index) const {
    std::vector<int> connections;
    for (size_t i = 0; i < _adj.width(); ++i) {
      if (_adj.get(index, i) == 1) {
        connections.push_back(i);
      }
    }
    return connections;
  }

  size_t width() const { return _adj.width(); }

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
      std::function<std::shared_ptr<Object>(std::shared_ptr<Object>, std::shared_ptr<Object>)>
          connect,
      std::function<bool(std::shared_ptr<Object>, std::shared_ptr<Object>)>
          is_connected)
      : _primary_objects(primary_objects),
        _secondary_objects(secondary_objects), _connect(connect),
        _is_connected(is_connected) {}

  std::vector<std::shared_ptr<Object>> connect_all_objects() {
    std::vector<std::shared_ptr<Object>> result;
    build_graph();
    const auto subgraphs = _graph.find_subgraphs();
    for (const auto subgraph : subgraphs) {
      result.push_back(connect_objects(subgraph));
    }
    return result;
  }

private:
  std::vector<std::shared_ptr<Object>> _primary_objects;
  std::vector<std::shared_ptr<Object>> _secondary_objects;
  std::function<std::shared_ptr<Object>(std::shared_ptr<Object>, std::shared_ptr<Object>)>
      _connect;
  std::function<bool(std::shared_ptr<Object>, std::shared_ptr<Object>)>
      _is_connected;
  Graph _graph;

  void build_graph() {
    _graph = Graph(_primary_objects.size() + _secondary_objects.size());
    for (size_t i = 0; i < _primary_objects.size(); ++i) {
      for (size_t j = 0; j < _secondary_objects.size(); ++j) {
        if (_is_connected(_primary_objects[i], _secondary_objects[j])) {
          _graph.add_edge(i, j + _primary_objects.size());
        }
      }
    }
  }

  std::shared_ptr<Object> connect_objects(std::vector<int> subgraph) {
    if (subgraph.size() == 1) {
      return get_object_by_index(subgraph[0]);
    }
    std::sort(subgraph.begin(), subgraph.end());
    std::shared_ptr<Object> merged_object = get_object_by_index(subgraph[0]);
    std::vector<int> visited(_graph.width(), 0);
    merge_connections(merged_object, subgraph, subgraph[0], visited);
    return merged_object;
  }

  void merge_connections(std::shared_ptr<Object> object_to_connect_to,
                         std::vector<int> subgraph, int index,
                         std::vector<int> &visited) {
    if (visited[index] == 1) {
      return;
    }
    visited[index] = 1;
    const auto connections = _graph.get_connections(index);
    for (const auto connection : connections) {
      const auto object_to_connect = get_object_by_index(connection);
      if (is_primary_by_index(connection)) {
        object_to_connect_to = _connect(object_to_connect, object_to_connect_to);
      } else {
        object_to_connect_to = _connect(object_to_connect_to, object_to_connect);
      }
    }
    for (const auto connection : connections) {
      merge_connections(object_to_connect_to, subgraph, connection, visited);
    }
  }

  std::shared_ptr<Object> get_object_by_index(int index) {
    if (index < _primary_objects.size()) {
      return _primary_objects[index];
    }
    return _secondary_objects[index - _primary_objects.size()];
  }

  bool is_primary_by_index(size_t index) {
    return index < _primary_objects.size();
  }
};

} // namespace od