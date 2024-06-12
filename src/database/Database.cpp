//
//  Database.cpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#include <string>
#include <iostream>
#include <iomanip>
#include <map>
#include "database/Database.hpp"
#include "misc/Config.hpp"

namespace ECE141 {
  
  Database::Database(const std::string aName, AccessMode aMode) : myStorage(aName, aMode),
  name(aName), changed(true) {
    std::string thePath = Config::getDBPath(name);

    //write first meta block into db
    if(std::holds_alternative<CreateFile>(aMode)){
        Block firstBlock = myStorage.makeBlock(BlockType::meta_block);
        myStorage.writeBlock(0, firstBlock);
    }
  }


  Database::~Database() {
    if(changed) {
      //stuff to save?

      //suggests that this is called when, in your model class, your current DB object is being
      //destroyed to make way for a new one.
    }
  }

  std::string Database::getName(){
      return name;
  }

  // USE: Dump command for debug purposes...
  StatusResult Database::dump(std::ostream &anOutput) {    
    return StatusResult{Errors::noError};
  }

}
