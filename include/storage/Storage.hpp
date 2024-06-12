//
//  Storage.hpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#ifndef Storage_hpp
#define Storage_hpp

#include <string>
#include <fstream>
#include <iostream>
#include <deque>
#include <stack>
#include <optional>
#include <functional>
#include "misc/Errors.hpp"
#include "database/Schema.hpp"
#include "database/Row.hpp"
#include "misc/DBQuery.hpp"

namespace ECE141 {

  const int32_t kNewBlock=-1;

  using ChunkCall = std::function<StatusResult(Block &aBlock)>;
  struct Chunker{
      Chunker(std::istream& inputStream);
      StatusResult each(ChunkCall aCall);
  protected:
      size_t length;
      std::istream& stream;
  };


  using BlockVisitor = std::function<StatusResult(Block&, uint32_t)>;
  using BlockList = std::deque<uint32_t>;

 static std::map<char,std::string> BlockCharToString{
          {'D',"data"},
          {'S',"schema"},
          {'F',"free"},
          {'E',"empty"},
          {'M',"meta"}
  };

 enum class KeyMode{
     autokey,mankey,nokey
 };

  // USE: Our storage manager class...
  class Storage : public BlockIO {
  public:
        
    Storage(const std::string &aName, AccessMode aMode);
    ~Storage();

    bool  each(const BlockVisitor &aVisitor);

    //What if we also offered a "storable" interface? 
    //StatusResult save(const Storable &aStorable, int32_t aStartPos=kNewBlock);
    //StatusResult load(std::iostream &aStream, uint32_t aStartBlockNum);

     //StatusResult markBlockAsFree(uint32_t aPos); maybe?
     //uint32_t     getFreeBlock(); //pos of next free (or new)...

     Block makeBlock(ECE141::BlockType theType);
     void populateSchemaBlock(Block& aBlock, Schema &theSchema);//pos of next free (or new)...
      void writeToMeta(uint32_t blockum, Schema &theSchema);
      std::vector<std::string> getTables();
      std::shared_ptr<Schema> readSchema(std::string theName);
      int deleteTable(std::string name);
      StatusResult storeSchema(Schema &theSchema);
      StatusResult storeRow(Row* theRow, std::string theTableName);
      StatusResult schemaExists(std::string theName);
      StatusResult rowsMatchSchema(RowCollection &theRows, Schema &theSchema);
      StatusResult setID(Row* theRow, Schema& theSchema, RowCollection& rows);
      std::tuple<std::string, uint32_t, bool> handleDump();
      StatusResult getRows(DBQuery &theQuery, RowCollection& aCollection);
      int updateRows(DBQuery &theQuery);
      int deleteRows(DBQuery &theQuery);
      static std::string stringify(std::variant<int,std::string, float, bool> theKey);

  protected:

      void wipeBlock(uint32_t blockNum);
      uint32_t findBlock(std::string name);
      StatusResult updateChain(std::string theName, uint32_t pos);
      StatusResult replaceChain(std::string theName, uint32_t pos);
      std::pair<KeyMode,std::string> getKeyMode(Schema& theSchema);
      StatusResult setAutoKey(Row* theRow, Schema& theSchema, RowCollection& rows);
      StatusResult setNonAutoKey(Row* theRow, Schema& theSchema, std::pair<KeyMode,std::string> thePair);
      uint32_t hash(Row* theRow, std::pair<KeyMode,std::string> thePair);
      StatusResult join(DBQuery &aQuery, RowCollection& aCollection);
      StatusResult order(DBQuery &aQuery, RowCollection& aCollection);
      StatusResult orderVec(std::string aColumn, RowCollection& aCollection);
      StatusResult applyLimit(DBQuery& aQuery, RowCollection& aCollection);
      void changeRows(DBQuery& theQuery, RowCollection& aRows);
      StatusResult replaceRow(Row* aRow);
      std::pair<size_t,std::vector<size_t>> findRowPos(Row* aRow);
      size_t getNext(size_t myPos, std::string aName);
      size_t dumpCounter;
    friend class Database;
  };

}


#endif /* Storage_hpp */
