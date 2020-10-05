#include "ResultTable.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace qpbackend {
ResultTable::ResultTable(const std::string& synName, const std::unordered_set<std::string>& vals) {
    for (const auto& val : vals) {
        table.push_back({ val });
    }
    rowNum = vals.size();
    colNum = 1;
}

ResultTable::ResultTable(const std::vector<std::string>& synNames,
                         const std::unordered_set<std::vector<std::string>, StringVectorHash>& listOfTuples) {
    colNum = 0;
    for (const auto& syn : synNames) {
        if (colIndexTable.find(syn) != colIndexTable.end()) {
            handleError("The table header contains duplicates");
        }
        colIndexTable[syn] = colNum;
        colNum++;
    }

    for (const auto& row : listOfTuples) {
        // check if the table shape match
        if (row.size() != synNames.size()) {
            handleError("The table shape does not match the table header provided");
        }
        table.push_back(row);
    }
    rowNum = listOfTuples.size();
}

bool ResultTable::isEmpty() const {
    return rowNum == 0;
}

bool ResultTable::isSynonymContained(const std::string& synonymName) const {
    return colIndexTable.find(synonymName) != colIndexTable.end();
}

bool ResultTable::mergeTable(ResultTable&& other) {
    // get list of common synonyms
    std::vector<std::string> commonSynonyms;
    std::vector<std::string> theirUniqueSynonyms;
    std::vector<int> ourCommonIndices;
    std::vector<int> theirCommonIndices;
    std::vector<int> theirUniqueIndices;
    for (auto const& synPair : other.colIndexTable) {
        const std::string& syn = synPair.first;
        if (isSynonymContained(syn)) {
            commonSynonyms.push_back(syn);
            ourCommonIndices.push_back(colIndexTable[syn]);
            theirCommonIndices.push_back(synPair.second);
        } else {
            theirUniqueSynonyms.push_back(syn);
            theirUniqueIndices.push_back(synPair.second);
        }
    }

    std::vector<std::vector<std::string>> tmpTable;
    if (commonSynonyms.empty()) { // no common properties, just cartesian product
        for (const auto& ourRow : table) {
            for (const auto& theirRow : other.table) {
                tmpTable.push_back(ourRow);
                for (const auto& theirPropIdx : theirUniqueIndices) {
                    tmpTable.back().push_back(theirRow[theirPropIdx]);
                }
            }
        }
    } else { // has common properties, take intersection
        std::unordered_map<std::vector<std::string>, std::vector<int>, StringVectorHash> ourOrganized =
        groupTableByProperties(ourCommonIndices);
        std::unordered_map<std::vector<std::string>, std::vector<int>, StringVectorHash> theirOrganized =
        other.groupTableByProperties(theirCommonIndices);
        for (const auto& p : theirOrganized) {
            const std::vector<std::string>& key = p.first;
            if (ourOrganized.find(key) != ourOrganized.end()) { // add to the new table if there are intersection
                for (const auto& ourRowIdx : ourOrganized[key]) {
                    std::vector<std::string> ourRow = table[ourRowIdx];
                    for (const auto& theirRowIdx : p.second) {
                        tmpTable.push_back(ourRow);
                        for (const auto& theirPropIdx : theirUniqueIndices) {
                            tmpTable.back().push_back(other.table[theirRowIdx][theirPropIdx]);
                        }
                    }
                }
            }
        }
    }

    // update properties
    for (const auto& newSyn : theirUniqueSynonyms) {
        colIndexTable[newSyn] = colNum;
        colNum++;
    }
    table = std::move(tmpTable);
    rowNum = table.size();

    return rowNum > 0;
}

bool ResultTable::updateSynonymValueSet(const std::string& synonymName,
                                        std::unordered_set<std::string>& result) const {
    if (!isSynonymContained(synonymName)) {
        return false;
    }

    result.clear();
    int idx = colIndexTable.at(synonymName);
    for (const auto& row : table) {
        result.insert(row[idx]);
    }
    return true;
}

bool ResultTable::updateSynonymValueTupleSet(const std::vector<std::string>& synonymNames,
                                             std::unordered_set<std::vector<std::string>, StringVectorHash>& result) const {
    // check if all synonyms are in the table
    std::vector<int> indices;
    for (const auto& synName : synonymNames) {
        if (!isSynonymContained(synName)) {
            return false;
        } else {
            indices.push_back(colIndexTable.at(synName));
        }
    }

    result.clear();
    for (const auto& row : table) {
        std::vector<std::string> newTuple;
        for (int idx : indices) {
            newTuple.push_back(row[idx]);
        }
        result.insert(newTuple);
    }
    return true;
}

std::unordered_map<std::string, std::vector<int>> ResultTable::groupTableByProperty(int index) const {
    std::unordered_map<std::string, std::vector<int>> resultMap;
    for (size_t idx = 0; idx < table.size(); idx++) {
        const std::string& key = table[idx][index];
        if (resultMap.find(key) == resultMap.end()) { // not in the map
            resultMap[key] = {};
        }
        resultMap[key].push_back((int)idx);
    }
    return resultMap;
}

std::unordered_map<std::vector<std::string>, std::vector<int>, StringVectorHash>
ResultTable::groupTableByProperties(std::vector<int> indices) const {
    std::unordered_map<std::vector<std::string>, std::vector<int>, StringVectorHash> resultMap;
    for (size_t idx = 0; idx < table.size(); idx++) {
        std::vector<std::string> vecKey;
        for (const auto& colIdx : indices) {
            vecKey.push_back(table[idx][colIdx]);
        }
        if (resultMap.find(vecKey) == resultMap.end()) { // not in the map
            resultMap[vecKey] = {};
        }
        resultMap[vecKey].push_back((int)idx);
    }
    return resultMap;
}

bool ResultTable::operator==(const ResultTable& other) const {
    return (colNum == other.colNum) && (rowNum == other.rowNum) &&
           (colIndexTable == other.colIndexTable) && (table == other.table);
}

std::ostream& operator<<(std::ostream& os, const ResultTable& rt) {
    // print cardinalities
    std::string rolInfo = "number of rows: " + std::to_string(rt.rowNum) + "\n";
    std::string colInfo = "number of columns: " + std::to_string(rt.colNum) + "\n";

    // print the header
    std::string headerInfo = "the table header: \n";
    for (const auto& p : rt.colIndexTable) {
        headerInfo += std::to_string(p.second) + ": " + p.first + " ";
    }
    headerInfo += "\n";

    // print the table
    std::string contentInfo = "the table content: \n";
    for (const auto& row : rt.table) {
        for (const auto& e : row) {
            contentInfo += e + " ";
        }
        contentInfo += "\n";
    }

    os << rolInfo << colInfo << headerInfo << contentInfo;
    return os;
}

void ResultTable::sortTable() {
    // sort the table
    std::vector<std::string> syns;
    for (const auto& p : colIndexTable) {
        syns.push_back(p.first);
    }
    std::sort(syns.begin(), syns.end());

    std::vector<std::vector<std::string>> newTable;
    for (const auto& row : table) {
        std::vector<std::string> newRow;
        for (size_t idx = 0; idx < row.size(); idx++) {
            newRow.push_back(row[colIndexTable[syns[idx]]]);
        }
        newTable.push_back(newRow);
    }
    std::sort(newTable.begin(), newTable.end(), CompareStrVec());
    table = std::move(newTable);

    // update the header
    colIndexTable.clear();
    for (size_t idx = 0; idx < syns.size(); idx++) {
        colIndexTable[syns[idx]] = idx;
    }
}

void handleError(const std::string& msg) {
    throw msg;
}
} // namespace qpbackend
