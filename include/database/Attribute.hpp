//
//  Attribute.hpp
//  PA3
//
//  Created by rick gessner on 4/18/22.
//  Copyright © 2023 rick gessner. All rights reserved.
//

#ifndef Attribute_hpp
#define Attribute_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "tokenizer/keywords.hpp"
#include "misc/BasicTypes.hpp"

namespace ECE141 {

    static const std::map<std::string, DataTypes> ValidAttributeKeywords = {
            {"INT", DataTypes::int_type},
            {"VARCHAR", DataTypes::varchar_type},
            {"FLOAT", DataTypes::float_type},
            {"BOOLEAN", DataTypes::bool_type},
            {"TIMESTAMP", DataTypes::datetime_type}
    };

    struct AttributeData{
        std::string   name;
        DataTypes     type;
        uint16_t length=0;
        bool autoinc=false;
        bool primarykey=false;
        bool nullable=true;
    };
      
class Attribute : public std::enable_shared_from_this<Attribute> {
  protected:

    //what other properties do we need?

  public:
          
    Attribute(DataTypes aType=DataTypes::no_type);
    Attribute(std::string aName, DataTypes aType, uint32_t aSize=0);
    Attribute(const Attribute &aCopy);
    Attribute(const AttributeData &aData) {
        name=aData.name;
        type=aData.type;
        length=aData.length;
        autoinc=aData.autoinc;
        primarykey=aData.primarykey;
        nullable=aData.nullable;
    }
    ~Attribute();
        
    const std::string&  getName() const {return name;}
    DataTypes           getType() const {return type;}
    
    //need more getters and setters right?
    
    bool                isValid(); //is this  valid?

      std::string   name;
      DataTypes     type;
      uint16_t length=0;
      bool autoinc=false;
      bool primarykey=false;
      bool nullable=true;

  };
  
  using AttributeOpt = std::optional<Attribute>;
  using AttributeList = std::vector<std::shared_ptr<Attribute>>;

}


#endif /* Attribute_hpp */
