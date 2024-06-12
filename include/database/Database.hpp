//
//  Database.hpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#ifndef Database_hpp
#define Database_hpp

#include <stdio.h>
#include <fstream> 
#include "storage/Storage.hpp"

namespace ECE141 {

  //db should be related to storage somehow...
  //TODO HAS-A???

  class Database {
  public:    
            Database(const std::string aPath, AccessMode);            
    virtual ~Database();

    StatusResult    dump(std::ostream &anOutput); //debug...
    std::string getName();

    Storage myStorage;

  protected:
    
    std::string     name;
    bool            changed;  //might be helpful, or ignore if you prefer.
  };

}
#endif /* Database_hpp */
