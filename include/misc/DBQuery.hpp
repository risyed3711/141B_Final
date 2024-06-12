//
//  DBQuery.hpp
//  PA5
//
//  Created by rick gessner on 4/7/23.
//

#ifndef DBQuery_h
#define DBQuery_h

#include "database/Schema.hpp"
#include "misc/Filters.hpp"
#include "misc/Join.hpp"

namespace ECE141 {

  // State held from a fancy select statement

  struct Property {
    Property(std::string aName, uint32_t aTableId=0) : name(aName), tableId(aTableId), desc(true) {}
    std::string     name;
    uint32_t        tableId;
    bool            desc;
  };

  using PropertyList = std::vector<Property>;

  //--------------------------
  
  class DBQuery {
  public:

    DBQuery(std::shared_ptr<Schema> aSchema=nullptr, bool allFields=true)
      : all(allFields) , limit(-1), count(false) {fromTables.push_back(aSchema);descending=false;}
    
    DBQuery(const DBQuery &aQuery) {fromTables=aQuery.fromTables;}
    void addSchema(std::shared_ptr<Schema> aSchema) {
        if(fromTables[0]== nullptr)
            fromTables[0]=aSchema;
        else
            fromTables.push_back(aSchema);
    }
                 
    //from (table) -- *, or a comma separated list of fields
    //where specific options...
    //offset
    //limit
    //group, etc.
    //maybe joins?  

    /*
    bool Matches(const ) const {
      return true;
    }
     */

    //might have to make fromTable a vector later?
    std::vector<std::shared_ptr<Schema>>  fromTables;
    Filters  filters;
    bool     all; //if user used SELECT * FROM...
    std::vector<std::string> columns;
    std::string orderBy;
    bool descending;
    size_t limit;
    Operand updatedValue;
    std::vector<Join> joins;
    bool count;
    std::string countParam;
    std::string countDispName;
    //e.g. what if the user says, "SELECT name, age FROM students WHERE..."

  };

}

#endif /* DBQuery_h */


