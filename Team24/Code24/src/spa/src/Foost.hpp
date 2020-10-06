#pragma once

#include <unordered_set>

/*
 * Fake boost library
 */
namespace foost {
template <typename T>
std::unordered_set<T> SetIntersection(const std::unordered_set<T>& set1, const std::unordered_set<T>& set2) {
    std::unordered_set<T> result;
    if (set1.size() <= set2.size()) {
        for (const T& elem : set1) {
            if (set2.find(elem) != set2.end()) {
                result.insert(elem);
            }
        }
    } else {
        for (const T& elem : set2) {
            if (set1.find(elem) != set1.end()) {
                result.insert(elem);
            }
        }
    }
    return result;
}

template <typename T>
std::unordered_set<T>
getVisitedInDFS(const T& start, const std::unordered_map<T, std::unordered_set<T>>& graph, bool isTransitive) {
    auto it = graph.find(start);
    if (it == graph.end()) {
        return {};
    }
    const std::unordered_set<T>& nextHops = it->second;

    if (!isTransitive) {
        return nextHops;
    }

    std::unordered_set<T> visited;
    std::vector<T> toVisit;
    for (const T& elem : nextHops) {
        toVisit.push_back(elem);
    }

    while (!toVisit.empty()) {
        T visiting = toVisit.back();
        toVisit.pop_back();
        if (visited.find(visiting) != visited.end()) {
            continue;
        }
        visited.insert(visiting);
        auto it = graph.find(visiting);
        if (it == graph.end()) {
            continue;
        }
        toVisit.insert(toVisit.end(), it->second.begin(), it->second.end());
    }
    return visited;
}
} // namespace foost
