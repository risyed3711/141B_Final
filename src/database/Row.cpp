//
//  Row.cpp
//  PA3
//
//  Created by rick gessner on 4/2/22.
//


#include <stdio.h>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <memory>
#include "database/Row.hpp"

namespace ECE141 {
  
  Row::Row(uint32_t entityId, std::shared_ptr<Schema> aSchema) : mySchema(aSchema) {}
  Row::Row(Row &aRow) {*this=aRow;}

  Row::~Row() {}

  Row& Row::operator=(Row &aRow) {
      for(const auto & pair : aRow.data){
          data[pair.first]=pair.second;
      }
      id=aRow.id;
      varcharLengths=aRow.varcharLengths;
      mySchema=aRow.mySchema;
      return *this;
  }
  bool Row::operator==(Row &aCopy) const {return false;}

  //STUDENT: What other methods do you require?
                      
  Row& Row::set(const std::string &aKey,const Value &aValue) {
      data[aKey]=aValue;
      return *this;
  }

  StatusResult Row::encode(std::ostream &anOutput) const {
      char buffer='\0';
      size_t x{0};
      StatusResult theResult;
      anOutput.write(reinterpret_cast<const char*>(&id),sizeof(id));anOutput.flush();
      anOutput.write(mySchema->name.c_str(),mySchema->name.length());
      if(mySchema->name.length()<kNameLength){
          auto count = kNameLength-mySchema->name.length();
          while(count){
              anOutput.write(&buffer,1);
              count--;
          }
      }
      for(auto &attribute:mySchema->attributes){
          if(!attribute->primarykey){
              if(auto val = std::get_if<int>(&data.find(attribute->name)->second)){
                  anOutput.write(reinterpret_cast<const char*>(val),sizeof(int)); anOutput.flush();
              }
              else if(auto val = std::get_if<std::string>(&data.find(attribute->name)->second)){
                  anOutput.write(reinterpret_cast<const char*>(val->c_str()),val->size()); anOutput.flush();
                  if(val->size()<varcharLengths[x]){
                      auto count = varcharLengths[x]-val->size();
                      while(count){
                          anOutput.write(&buffer,1);
                          count--;
                      }
                  }
                  x++;
                  anOutput.flush();
              }
              else if(auto val = std::get_if<float>(&data.find(attribute->name)->second)){
                  anOutput.write(reinterpret_cast<const char*>(val),sizeof(float)); anOutput.flush();
              }
              else{
                  auto bVal = std::get_if<bool>(&data.find(attribute->name)->second);
                  anOutput.write(reinterpret_cast<const char*>(bVal),sizeof(bool)); anOutput.flush();
              }
          }
      }
      if(!anOutput.good())
          theResult.error=Errors::unknownError;
      return theResult;
  }

  StatusResult Row::decode(std::istream &anInput) {
      StatusResult theResult;
      char buffer[kNameLength];
      anInput.read(reinterpret_cast<char*>(&id),sizeof(id));
      anInput.read(reinterpret_cast<char*>(&buffer),kNameLength);
      for(const auto& attribute : mySchema->attributes){
          if(!attribute->primarykey){
              std::pair<std::string,Value> thePair;
              thePair.first=attribute->name;
              switch(attribute->type){
                  case(DataTypes::bool_type):
                      bool theBool;
                      anInput.read(reinterpret_cast<char*>(&theBool),sizeof(bool));
                      thePair.second=theBool;
                      break;
                  case(DataTypes::float_type):
                      float theFloat;
                      anInput.read(reinterpret_cast<char*>(&theFloat),sizeof(float));;
                      thePair.second=theFloat;
                      break;
                  case(DataTypes::int_type):
                      int theInt;
                      anInput.read(reinterpret_cast<char*>(&theInt),sizeof(int));
                      thePair.second=theInt;
                      break;
                  case(DataTypes::varchar_type):
                      char buffer[attribute->length];
                      anInput.read(reinterpret_cast<char*>(&buffer),attribute->length);
                      std::string theString=buffer;
                      thePair.second=theString;
                      varcharLengths.push_back(attribute->length);
              }
              data[thePair.first]=thePair.second;
          }
      }
      return theResult;
  }

  bool Row::initHeader(ECE141::Block &aBlock) const {
      return true;
  }

  void Row::fillEmptyAttributes() {
      std::map<DataTypes,Value> theDefaultValues {
              {DataTypes::varchar_type,""},
              {DataTypes::bool_type,false},
              {DataTypes::int_type,0},
              {DataTypes::float_type,0}
      };
      for(const auto & attr : mySchema->attributes){
          if(data.find(attr->name)==data.end()&&!attr->autoinc){
              data[attr->name]=theDefaultValues[attr->type];
          }
      }
  }
    
}
