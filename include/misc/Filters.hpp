//
//  Filters.hpp
//  RGAssignment5
//
//  Created by rick gessner on 4/4/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef Filters_h
#define Filters_h

#include <stdio.h>
#include <vector>
#include <memory>
#include <string>
#include "tokenizer/Tokenizer.hpp"
#include "database/Row.hpp"

namespace ECE141 {
  
  class Schema;
  using KeyValues = std::map<std::string, Value>;

  struct Operand {
    Operand() {}
    Operand(std::string &aName, TokenType aType, Value &aValue, std::string aSchema="")
      : ttype(aType), dtype(DataTypes::varchar_type), name(aName),
        value(aValue), schemaName(aSchema) {}

    void setAttribute(Token& aToken, std::shared_ptr<Schema> aSchema, std::string attrName);
    void setVarChar(Token& aToken);
    void setNumber(Token& aToken);
    
    TokenType   ttype; //is it a field, or const (#, string)...
    DataTypes   dtype;
    std::string name;  //attr name
    Value       value;
    std::string      schemaName;
  };
  
  //---------------------------------------------------
               
  struct Expression {
    Operand     lhs;  //id
    Operand     rhs;  //usually a constant; maybe a field...
    Operators   op;   //=     //users.id=books.author_id
    Logical     logic; //and, or, not...
    
    Expression(const Operand &aLHSOperand={},
               const Operators anOp=Operators::unknown_op,
               const Operand &aRHSOperand={})
      : lhs(aLHSOperand), rhs(aRHSOperand),
        op(anOp), logic(Logical::no_op) {}
    
    Expression(const Expression &anExpr) :
      lhs(anExpr.lhs), rhs(anExpr.rhs),
      op(anExpr.op), logic(anExpr.logic) {}
    
    bool operator()(RowKeyValues &aList);

  };
  
  using Expressions = std::vector<std::shared_ptr<Expression>>;

  //---------------------------------------------------

  class Filters {
  public:
    
    Filters();
    Filters(const Filters &aFilters);
    ~Filters();
    
    size_t        getCount() const {return expressions.size();}
    bool          matches(RowKeyValues &aList) const;
    Filters&      add(std::shared_ptr<Expression> anExpression);
    Expressions&  getExpressions() {return expressions;}    
    StatusResult  parse(Tokenizer &aTokenizer,std::shared_ptr<Schema> aSchema);
    
  protected:
    Expressions   expressions;
  };
 
}

#endif /* Filters_h */
