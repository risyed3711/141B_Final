//
//  Row.hpp
//  PA3
//
//  Created by rick gessner on 4/2/23.
//

#ifndef Row_hpp
#define Row_hpp

#include <stdio.h>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <memory>
#include "Attribute.hpp"
#include "misc/BasicTypes.hpp"
#include "storage/Storable.hpp"
#include "database/Attribute.hpp"
#include "Schema.hpp"

//feel free to use this, or create your own version...

namespace ECE141 {

  //These really shouldn't be here...
  using Value = std::variant<int,std::string, float, bool>;
  using RowKeyValues = std::map<const std::string, Value>;

  class Row : public Storable {
  public:

    Row(uint32_t entityId=0, std::shared_ptr<Schema> aSchema=nullptr);
    Row(Row &aRow);
    
   // Row(const Attribute &anAttribute); //maybe?
    
    ~Row();
    
    Row& operator=(Row &aRow);
    bool operator==(Row &aCopy) const;
    
      //STUDENT: What other methods do you require?
                          
    Row&                set(const std::string &aKey,
                            const Value &aValue);
        
    RowKeyValues&       getData() {return data;}
    void setID(uint32_t id) {this->id=id;}
    void addLength(size_t s) {varcharLengths.push_back(s);}
    void setSchema(std::shared_ptr<Schema> aSchema) {mySchema=aSchema;}
    void fillEmptyAttributes();
    std::shared_ptr<Schema> getSchema() {return mySchema;}
    uint32_t getID(){return id;}

    virtual StatusResult encode(std::ostream &anOutput) const override;
    virtual StatusResult decode(std::istream &anInput) override;
    virtual bool initHeader(ECE141::Block &aBlock) const override;
    
    //uint32_t            entityId; //hash value of entity?
    //uint32_t            blockNumber;

  protected:
    RowKeyValues        data;
    uint32_t id;
    std::vector<size_t> varcharLengths;
    std::shared_ptr<Schema> mySchema;
  };

  //-------------------------------------------

  using RowCollection = std::vector<std::unique_ptr<Row> >;


}
#endif /* Row_hpp */

