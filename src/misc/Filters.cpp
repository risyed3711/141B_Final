//
//  Filters.cpp
//  Datatabase5
//
//  Created by rick gessner on 3/5/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <string>
#include <limits>
#include "misc/Filters.hpp"
#include "tokenizer/keywords.hpp"
#include "misc/Helpers.hpp"
#include "database/Schema.hpp"
#include "database/Attribute.hpp"
#include "misc/ParseHelper.hpp"
//#include "Compare.hpp"

namespace ECE141 {
  
  using Comparitor = bool (*)(Value &aLHS, Value &aRHS);

  bool equals(Value &aLHS, Value &aRHS) {
    bool theResult=false;
    if(aLHS==aRHS)
        theResult=true;
    return theResult;
  }
  bool notEquals(Value &aLHS, Value &aRHS){
      return !equals(aLHS,aRHS);
  }
  bool greaterThan(Value &aLHS, Value &aRHS){
      bool theResult=false;
      if(aLHS>aRHS)
          theResult=true;
      return theResult;
  }
  bool greaterThanOrEqual(Value &aLHS, Value &aRHS){
      return equals(aLHS,aRHS) || greaterThan(aLHS,aRHS);
  }
  bool lessThan(Value &aLHS, Value &aRHS){
      return !greaterThanOrEqual(aLHS,aRHS);
  }
  bool lessThanOrEqual(Value &aLHS, Value &aRHS){
      return !greaterThan(aLHS,aRHS);
  }

  static std::map<Operators, Comparitor> comparitors {
    {Operators::equal_op, equals},
    {Operators::notequal_op, notEquals},
    {Operators::gt_op, greaterThan},
    {Operators::gte_op, greaterThanOrEqual},
    {Operators::lt_op, lessThan},
    {Operators::lte_op, lessThanOrEqual},
  };

  bool Expression::operator()(RowKeyValues &aList) {
      Operand* theAttr;
      Operand* theValue;
      theAttr = (lhs.ttype==TokenType::identifier) ? &lhs : &rhs;
      theValue = (theAttr=&lhs) ? &rhs : &lhs;
      for(auto& attr : aList){
          if(attr.first==theAttr->name){
              return comparitors[op](attr.second,theValue->value);
          }
      }
      return false;
  }
  
  //--------------------------------------------------------------
  
  Filters::Filters()  {}
  
  Filters::Filters(const Filters &aCopy)  {
  }
  
  Filters::~Filters() {
    //no need to delete expressions, they're unique_ptrs!
  }

  Filters& Filters::add(std::shared_ptr<Expression> anExpression) {
    expressions.push_back(anExpression);
    return *this;
  }
    
  //compare expressions to row; return true if matches
  bool Filters::matches(RowKeyValues &aList) const {
    
    //STUDENT: You'll need to add code here to deal with
    //         logical combinations (AND, OR, NOT):
    //         like:  WHERE zipcode=92127 AND age>20
    
    for(auto &theExpr : expressions) {
      if(!(*theExpr)(aList)) {
        return false;
      }
    }
    return true;
  }
 

  //where operand is field, number, string...
    
  //STUDENT: Add validation here...
  bool validateOperands(Operand &aLHS, Operand &aRHS, Schema &aSchema) {
    if(TokenType::identifier==aLHS.ttype) { //most common case...
      //STUDENT: Add code for validation as necessary
      return true;
    }
    else if(TokenType::identifier==aRHS.ttype) {
      //STUDENT: Add code for validation as necessary
      return true;
    }
    return false;
  }

  bool isValidOperand(Token &aToken) {
    //identifier, number, string...
    if(aToken.type==TokenType::identifier) return true;
    return false;
  }

  //STUDENT: This starting point code may need adaptation...
  StatusResult Filters::parse(Tokenizer &aTokenizer,std::shared_ptr<Schema> aSchema) {
    StatusResult  theResult;
    ParseHelper theHelper(aTokenizer);
    while(theResult && (2<aTokenizer.remaining())) {
      if(isValidOperand(aTokenizer.current())) {
        std::shared_ptr<Expression> theExpr = std::make_shared<Expression>();
        if((theResult=theHelper.parseExpression(aSchema,theExpr))) {
          expressions.push_back(theExpr);
          //add logic to deal with bool combo logic...
        }
      }
      else break;
    }
    return theResult;
  }

  void Operand::setAttribute(ECE141::Token &aToken, std::shared_ptr<Schema> aSchema, std::string attrName) {
      ttype=aToken.type;
      name=attrName;
      for(const auto& attr : aSchema->attributes){
          if(attr->name==attrName)
              dtype=attr->type;
      }
      this->schemaName=aSchema->name;
  }

  void Operand::setNumber(ECE141::Token &aToken) {
      ttype=TokenType::number;
      dtype=DataTypes::int_type;
      if (aToken.data.find('.')!=std::string::npos) {
        dtype=DataTypes::float_type;
        value=std::stof(aToken.data);
      }
      else value=std::stoi(aToken.data);
  }

  void Operand::setVarChar(ECE141::Token &aToken) {
      ttype=aToken.type;
      value=aToken.data;
      dtype=DataTypes::varchar_type;
  }

}

