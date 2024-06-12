//
//  Schema.hpp
//  PA3
//
//  Created by rick gessner on 3/18/23.
//  Copyright © 2023 rick gessner. All rights reserved.
//

#ifndef Schema_hpp
#define Schema_hpp

#include <memory>
#include <string>
#include <vector>

#include "Attribute.hpp"
#include "misc/Errors.hpp"
#include "storage/BlockIO.hpp"
#include "storage/Storable.hpp"


namespace ECE141 {

class Schema : public Storable, public std::enable_shared_from_this<Schema> {
  public:
                          Schema(const std::string aName, AttributeList={});
                          Schema(const Schema &aCopy);
    
                          ~Schema() {};

    const std::string&    getName() const {return name;}
    void addAttribute(const Attribute& aAttribute) {attributes.push_back(std::make_shared<Attribute>(aAttribute));}
    bool hasAttribute(std::string attributeName);

    virtual StatusResult encode(std::ostream &anOutput) const override;
    virtual StatusResult decode(std::istream &anInput) override;
    virtual bool initHeader(ECE141::Block &aBlock) const override;
           
  //protected:
        
    AttributeList   attributes;
    std::string     name;
    
    //how will you manage creation of primary keys?

  };
  
}
#endif /* Schema_hpp */
