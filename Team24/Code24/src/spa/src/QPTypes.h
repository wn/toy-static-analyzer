#ifndef QPTYPES_H
#define QPTYPES_H

#include <string>

namespace qpbackend {

enum EntityType { STMT, READ, PRINT, CALL, WHILE, IF, ASSIGN, VARIABLE, CONSTANT, PROCEDURE };

enum RelationType { FOLLOWS, FOLLOWST, PARENT, PARENTT, USES, MODIFIES };

typedef std::tuple<RelationType, std::string, std::string> RELATIONTUPLE;
typedef std::tuple<std::string, std::string, std::string> PATTERNTUPLE;
} // namespace qpbackend

#endif // QPTYPES_H
