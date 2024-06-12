//
//  ParseHelper.hpp
//  RGAssignment4
//
//  Created by rick gessner on 4/18/21.
//

#ifndef ParseHelper_hpp
#define ParseHelper_hpp

#include <stdio.h>
#include "tokenizer/keywords.hpp"
#include "BasicTypes.hpp"
#include "tokenizer/Tokenizer.hpp"
#include "database/Attribute.hpp"
#include "Filters.hpp"

namespace ECE141 {

    struct TableName {
        TableName(const std::string &aTableName, const std::string &anAlias="")
                : table(aTableName), alias(anAlias) {}
        TableName(const TableName &aCopy) : table(aCopy.table), alias(aCopy.alias) {}
        TableName& operator=(const std::string &aName) {
            table=aName;
            return *this;
        }
        operator const std::string() {return table;}
        std::string table;
        std::string alias;
    };
    struct TableField {
        TableField(const std::string &aName="", const std::string &aTable="")
                : fieldName(aName), table(aTable) {}
        TableField(const TableField &aCopy) : fieldName(aCopy.fieldName), table(aCopy.table) {}
        std::string fieldName;
        std::string table;
        StatusResult CaptureTableField(Tokenizer& aTokenizer);
    };
  //-------------------------------------------------
  
  class Entity;
  struct Expression;
  using StringList = std::vector<std::string>;

  enum class OperandMode{
      lhs,rhs
  };
  
  struct ParseHelper {
            
    ParseHelper(Tokenizer &aTokenizer) : tokenizer(aTokenizer) {}
        
    StatusResult parseTableName(TableName &aTableName);
    StatusResult parseTableField(Tokenizer &aTokenizer, TableField &aField);
    StatusResult parseAttributeOptions(Attribute &anAttribute);

    StatusResult parseAttribute(Attribute &anAttribute);

    StatusResult parseIdentifierList(StringList &aList);

    StatusResult parseAssignments(Expressions &aList, std::shared_ptr<Schema> aSchema);
    
    StatusResult parseValueList(StringList &aList);
            
    StatusResult parseOperator(Operators &anOp);
    StatusResult parseOperand(std::shared_ptr<Schema> aSchema,
                              std::shared_ptr<Expression> anExpr, OperandMode theMode);
    StatusResult parseExpression(std::shared_ptr<Schema> aSchema,
                                 std::shared_ptr<Expression> anExpr);

    Tokenizer &tokenizer;
  };

}

#endif /* ParseHelper_hpp */

