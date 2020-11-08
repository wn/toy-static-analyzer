#define OPTIMIZER 1

#include "Optimisation.h"

#include "QPTypes.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace qpbackend {
namespace optimisation {
namespace {

struct CLAUSES_EDGE {
    CLAUSES_EDGE() {
        this->size = 0;
    }
    CLAUSES_EDGE(bool isEntity,
                 int totalWeightOfClauses,
                 const CLAUSE_LIST& clauses,
                 const std::string& arg1,
                 const std::string& arg2 = "")
    : isEntity(isEntity), size(clauses.size()), totalWeightOfClauses(totalWeightOfClauses),
      clauses(clauses), arg1(arg1), arg2(arg2) {
        if (arg2.empty()) {
            hash = "hash_" + prettyPrintCLAUSE(clauses[0]); // dash sign to represent an entity hash
        } else {
            hash = arg1 < arg2 ? arg1 + arg2 : arg2 + arg1;
        }
    }
    bool isEntity;
    int size; // size of clauses
    int totalWeightOfClauses;
    std::string hash;
    CLAUSE_LIST clauses;
    std::string arg1;
    std::string arg2;

    // Return the other argument in the CLAUSES_EDGE.
    // If there is no other arg, return empty string.
    std::string getOtherArg() {
        if (isEntity) {
            return "";
        }
        return arg2;
    }

    // more than 0 means "this" has a higher priority.
    int compare(const CLAUSES_EDGE& e) const {
        int hashCompare = e.hash.compare(hash);
        if (hashCompare == 0) {
            return 0;
        }
        if (e.size == 0) {
            return size == 0 ? 0 : 1;
        }
        if (e.isEntity != isEntity) {
            return e.isEntity ? -1 : 1;
        }
        if (e.size != size) {
            return e.size > size ? -1 : 1;
        }
        if (e.totalWeightOfClauses != totalWeightOfClauses) {
            return totalWeightOfClauses > e.totalWeightOfClauses ? -1 : 1;
        }
        // tiebreaker just to create a total order.
        return hashCompare > 0 ? -1 : 1;
    }
};

typedef std::unordered_map<std::string, std::unordered_map<std::string, std::vector<CLAUSE>>> Graph;
struct Node {
  public:
    Node(const std::string& name) : name(name){};

    CLAUSES_EDGE addEdge(const CLAUSE_LIST& edgesToSynonym, const std::string& synonym) {
        CLAUSES_EDGE edge = Node::getSynonymSynonymClausesWeight(edgesToSynonym, name, synonym);
        edges.push_back(edge);
        return edge;
    }

    CLAUSES_EDGE addEntitySynonymEdge(const CLAUSE& clause, const std::string& entity) {
        CLAUSES_EDGE edge = Node::getEntitySynonymClausesWeight(clause, name, entity);
        edges.push_back(edge);
        return edge;
    }

    std::string name;
    std::vector<CLAUSES_EDGE> edges;

  private:
    // Base on the weights of all clauses between two synonyms, generate a weight.
    static CLAUSES_EDGE getSynonymSynonymClausesWeight(const CLAUSE_LIST& clauses,
                                                       const std::string& arg1,
                                                       const std::string& arg2) {
        int totalWeight = 0;
        for (const auto& clause : clauses) {
            totalWeight += getClauseWeight(clause);
        }
        return { false, totalWeight, clauses, arg1, arg2 };
    }

    static int getClauseWeight(const CLAUSE& clause) {
        ClauseType type = std::get<0>(clause);
        switch (type) {
        case FOLLOWS:
            return 4;
        case FOLLOWST:
            return 11;
        case PARENT:
            return 3;
        case PARENTT:
            return 10;
        case USES:
            return 6;
        case MODIFIES:
            return 7;
        case CALLS:
            return 5;
        case CALLST:
            return 12;
        case NEXT:
            return 8;
        case NEXTT:
            return 13;
        case AFFECTS:
            return 9;
        case AFFECTST:
            return 14;
        case NEXTBIP:
            return 15;
        case NEXTBIPT:
            return 17;
        case AFFECTSBIP:
            return 16;
        case AFFECTSBIPT:
            return 18;
        case ASSIGN_PATTERN_EXACT:
            return 2;
        case ASSIGN_PATTERN_SUB_EXPR:
            return 2;
        case ASSIGN_PATTERN_WILDCARD:
            return 2;
        case IF_PATTERN:
            return 2;
        case WHILE_PATTERN:
            return 2;
        case WITH:
            return 1;
        case INVALID_CLAUSE_TYPE:
            // THIS SHOULD NOT HAPPEN.
            return 100;
        }
    }

    static CLAUSES_EDGE
    getEntitySynonymClausesWeight(const CLAUSE& clause, const std::string& synonym, const std::string& entity) {
        return { true, getClauseWeight(clause), { clause }, synonym, "" };
    }
};

class Group {
  public:
    Group() = default;
    // Constructor for entity-synonym clauses that do not connect to anything.
    Group(const std::string& synonym) {
        nodes.insert({ synonym, Node(synonym) });
    }

    // Given a graph and a node to visit, create a group containing all synonyms reachable from synonymToVisit.
    Group(const Graph& g, const std::string& synonymToVisit) {
        std::vector<std::string> toVisit = { synonymToVisit };

        while (!toVisit.empty()) {
            std::string visiting = toVisit.back();
            toVisit.pop_back();

            if (nodes.find(visiting) != nodes.end()) {
                continue;
            }

            Node node(visiting);
            for (const auto& synonymClausesPair : g.at(visiting)) {
                const CLAUSES_EDGE& edge =
                node.addEdge(synonymClausesPair.second, synonymClausesPair.first);
                toVisit.push_back(synonymClausesPair.first);
                if (edge.compare(start) > 0) {
                    start = edge;
                    startSynonym = visiting;
                }
            }
            nodes.insert({ visiting, node });
        }
    }

    void addConstant(const std::string& entity, const std::string& synonym, const CLAUSE& clause) {
        const CLAUSES_EDGE& edge = nodes.at(synonym).addEntitySynonymEdge(clause, entity);
        if (edge.compare(start) > 0) {
            start = edge;
            startSynonym = synonym;
        }
    }

    std::pair<bool, std::vector<CLAUSE_LIST>> sortGroup() const {
        std::vector<CLAUSE_LIST> sortedResult;

        auto cmp = [](const CLAUSES_EDGE& a, const CLAUSES_EDGE& b) { return a.compare(b) < 0; };
        std::priority_queue<CLAUSES_EDGE, std::vector<CLAUSES_EDGE>, decltype(cmp)> toVisit(cmp);
        for (const CLAUSES_EDGE& edge : nodes.at(startSynonym).edges) {
            toVisit.push(edge);
        }

        std::unordered_set<std::string> visited_CLAUSES;
        while (!toVisit.empty()) {
            CLAUSES_EDGE visiting = toVisit.top();
            toVisit.pop();

            if (visited_CLAUSES.find(visiting.hash) != visited_CLAUSES.end()) {
                continue;
            }
            visited_CLAUSES.insert(visiting.hash);
            sortedResult.push_back(visiting.clauses);

            // Get next visiting node:
            std::string nextNode = visiting.getOtherArg();
            if (nextNode.empty()) {
                // this is a entity-synonym clause
                continue;
            }
            for (const CLAUSES_EDGE& edge : nodes.at(nextNode).edges) {
                toVisit.push(edge);
            }
        }

        return { containSelect, sortedResult };
    }

    std::vector<std::string> getAllSynonyms() {
        std::vector<std::string> result;
        for (const auto& node : nodes) {
            result.push_back(node.first);
        }
        return result;
    }

    bool containSelect{ false };
    CLAUSES_EDGE start; // highest priority edge in the graph.
    std::string startSynonym;

    //  private:
    std::unordered_map<std::string, Node> nodes;
};

class Optimisation {
  public:
    explicit Optimisation(const CLAUSE_LIST& list, const std::unordered_set<std::string>& selectSynonyms) {
        Graph graph;
        std::vector<std::tuple<std::string, std::string, CLAUSE>> entitySynonymClauses; // <entity, synonym, clause>
        for (const CLAUSE& clause : list) {
            ArgType arg_type_1 = std::get<1>(clause).first;
            ArgType arg_type_2 = std::get<2>(clause).first;

            const std::string& arg1 = std::get<1>(clause).second;
            const std::string& arg2 = std::get<2>(clause).second;
            ClauseArgsType clauseTypes = getClauseArgsType(arg_type_1, arg_type_2);
            switch (clauseTypes) {
            case ENTITY_ENTITY:
            case ENTITY_WILDCARD:
            case WILDCARD_ENTITY:
            case WILDCARD_WILDCARD:
                entityEntityClauses.push_back({ { clause } });
                break;
            case ENTITY_SYNONYM:
            case WILDCARD_SYNONYM:
                entitySynonymClauses.emplace_back(arg1, arg2, clause);
                break;
            case SYNONYM_ENTITY:
            case SYNONYM_WILDCARD:
                entitySynonymClauses.emplace_back(arg2, arg1, clause);
                break;
            case SYNONYM_SYNONYM:
                if (graph.find(arg1) == graph.end()) {
                    graph[arg1] = {};
                }
                if (graph.find(arg2) == graph.end()) {
                    graph[arg2] = {};
                }
                if (graph[arg1].find(arg2) == graph[arg1].end()) {
                    graph[arg1][arg2] = {};
                }
                if (graph[arg2].find(arg1) == graph[arg2].end()) {
                    graph[arg2][arg1] = {};
                }
                if (arg1 == arg2) {
                    graph[arg1][arg1].push_back(clause);
                    break;
                }
                graph[arg1][arg2].push_back(clause);
                graph[arg2][arg1].push_back(clause);
                break;
            case INVALID_1:
            case INVALID_2:
                isInvalid = true;
                return;
            }
        }

        // Split the graph into groups of connected components.
        Optimisation::getGroupsFromGraph(graph, entitySynonymClauses, selectSynonyms);
    }

    std::vector<std::vector<CLAUSE_LIST>> generateSortedClauses() const {
        if (isInvalid) {
            return {};
        }
        std::vector<std::vector<CLAUSE_LIST>> result;
        if (!entityEntityClauses.empty()) {
            result = std::move(entityEntityClauses);
        }
        std::vector<std::pair<bool, std::vector<CLAUSE_LIST>>> groupsResult;
        for (const Group& group : groups) {
            groupsResult.push_back(group.sortGroup());
        }

        // sort by whether there is a select synonym in the group, then by size.
        sort(groupsResult.begin(), groupsResult.end(),
             [](const std::pair<bool, std::vector<CLAUSE_LIST>>& a,
                const std::pair<bool, std::vector<CLAUSE_LIST>>& b) {
                 if (a.first != b.first) {
                     return !a.first;
                 }
                 return a.second.size() < b.second.size();
             });

        result.reserve(result.size() + groupsResult.size());
        for (const std::pair<bool, std::vector<CLAUSE_LIST>>& i : groupsResult) {
            result.push_back(i.second);
        }
        return result;
    }

  private:
    bool isInvalid{ false };
    std::vector<std::vector<CLAUSE_LIST>> entityEntityClauses;
    std::vector<Group> groups;

    // Split the graph into groups of connected components.
    void getGroupsFromGraph(const Graph& graph,
                            const std::vector<std::tuple<std::string, std::string, CLAUSE>>& entitySynonymClauses,
                            const std::unordered_set<std::string>& selectSynonyms) {
        std::unordered_map<std::string, int> visitedSynonyms;
        // From the graph, we split them into groups, where elem in each group are connected.
        for (const auto& synonym : graph) {
            if (visitedSynonyms.find(synonym.first) != visitedSynonyms.end()) {
                continue;
            }
            groups.emplace_back(graph, synonym.first);
            for (const std::string& synonymInGroup : groups.back().getAllSynonyms()) {
                visitedSynonyms.insert({ synonymInGroup, groups.size() - 1 });
                bool isSynonymInSelect = selectSynonyms.find(synonymInGroup) != selectSynonyms.end();
                groups.back().containSelect = groups.back().containSelect || isSynonymInSelect;
            }
        }

        // attach EntitySynonyms into the groups.
        for (const auto& entitySynonym : entitySynonymClauses) {
            const std::string& entity = std::get<0>(entitySynonym);
            const std::string& synonym = std::get<1>(entitySynonym);
            const CLAUSE& clause = std::get<2>(entitySynonym);

            // Some entity synonyms might not have any groups to join to. Create a new group in that case.
            if (visitedSynonyms.find(synonym) == visitedSynonyms.end()) {
                groups.emplace_back(synonym);
                visitedSynonyms.insert({ synonym, groups.size() - 1 });
            }
            groups.at(visitedSynonyms.at(synonym)).addConstant(entity, synonym, clause);
        }
    }
};
} // namespace

// groups of clauselist of (list of edges with same synonym pair)
std::vector<std::vector<CLAUSE_LIST>>
optimizeQueries(const CLAUSE_LIST& clauses, const RETURN_CANDIDATE_LIST& returnCandidate) {
    if (OPTIMIZER) {
        std::unordered_set<std::string> selectSynonyms;
        for (const RETURN_CANDIDATE& c : returnCandidate) {
            selectSynonyms.insert(c.second);
        }
        return Optimisation(clauses, selectSynonyms).generateSortedClauses();
    }
    return { { clauses } }; // this is naive.
}
} // namespace optimisation
} // namespace qpbackend
