//
//  Storage.cpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//


#include <sstream>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <cstring>
#include <iostream>
#include "storage/Storage.hpp"
#include "misc/Config.hpp"
#include <cstring>

namespace ECE141 {


  // USE: ctor ---------------------------------------

  Storage::Storage(const std::string &aName, AccessMode aMode)
    : BlockIO(aName, aMode) {dumpCounter=0;}

  // USE: dtor ---------------------------------------
  Storage::~Storage() {
  }

  Chunker::Chunker(std::istream &inputStream) : stream(inputStream) {
      stream.seekg(0,std::ios::end);
      length=inputStream.tellg();
  }

  StatusResult Chunker::each(ECE141::ChunkCall aCall) {
      StatusResult theResult;
      stream.seekg(0,std::ios::beg);
      size_t temp=length;
      while(temp){
          size_t size = std::min(kPayloadSize-sizeof(char),temp);
          Block theBlock;
          temp-=size;
          std::memset(theBlock.payload,static_cast<char>(temp ? 1 : 0),sizeof(char));
          stream.read(reinterpret_cast<char*>(theBlock.payload + sizeof(char)),size);
          aCall(theBlock);
      }
      return theResult;
  }

  bool Storage::each(const BlockVisitor &aVisitor) {
      auto max = getBlockCount();
      uint32_t count{0};
      Block theBlock;
      while(count<max){
          readBlock(count,theBlock);
          aVisitor(theBlock,count);
          count++;
      }
      return true;
  }

    Block Storage::makeBlock(ECE141::BlockType theType){
      return Block(theType);
  }

  StatusResult Storage::storeSchema(ECE141::Schema &theSchema) {
      std::stringstream theStr;
      theSchema.encode(theStr);
      Chunker theChunker(theStr);
      return theChunker.each([&](Block &aBlock){
          StatusResult theResult;
          aBlock.header.type=static_cast<char>(BlockType::schema_block);
          auto pos=getFreeBlock();
          writeBlock(pos,aBlock);
          if(findBlock(theSchema.name))
              updateChain(theSchema.name,pos);
          else
              writeToMeta(pos,theSchema);
          return theResult;
      });
  }

  StatusResult Storage::storeRow(Row* theRow, std::string theTableName) {
      std::stringstream theStr;
      theRow->encode(theStr);
      Chunker theChunker(theStr);
      return theChunker.each([&](Block &aBlock){
          StatusResult theResult;
          aBlock.header.type=static_cast<char>(BlockType::data_block);
          auto pos = getFreeBlock();
          writeBlock(pos,aBlock);
          updateChain(theTableName, pos);
          return theResult;
      });
  }

  StatusResult Storage::schemaExists(std::string theName) {
      StatusResult theResult;
      if(!findBlock(theName))
          theResult.error=Errors::invalidTableName;
      return theResult;
  }

  StatusResult Storage::rowsMatchSchema(RowCollection &theRows, Schema &theSchema) {
      StatusResult theResult;
      auto attributes=theSchema.attributes;
      for(auto &row:theRows){
          size_t size=row->getData().size();
          //if(attributes.size()-1>size){theResult.error=Errors::invalidArguments;break;}
          for(const auto&attribute:attributes){
              if(row->getData().count(attribute->name)){
                  if(attribute->type==DataTypes::varchar_type){
                      if(std::get_if<std::string>(&row->getData()[attribute->name])->size()>attribute->length){
                          theResult.error=Errors::invalidArguments;
                          break;
                      }
                      else
                          row->addLength(attribute->length);
                  }
                  size--;
              }
              else if(!attribute->primarykey||!attribute->autoinc){
                      theResult.error=Errors::invalidArguments;break;
              }
          }
          if(size){theResult.error=Errors::invalidArguments;break;}
      }
      return theResult;
  }

  void Storage::writeToMeta(uint32_t blockNum, ECE141::Schema &theSchema) {
      uint32_t offSet{0};
      size_t count;
      Block theBlock;
      uint32_t temp;
      char* ptr;
      char separator = '*';
      do{
          readBlock(0,theBlock);
          ptr=theBlock.payload;
          auto dataSize=kNameLength+sizeof(uint32_t);
          count=0;
          while(count*dataSize<sizeof(theBlock.payload)-dataSize){
              std::memmove(&temp, ptr+count*dataSize+kNameLength, sizeof(uint32_t));
              if(temp==0){
                  /*
                  stream.seekp(offSet+sizeof(BlockHeader)+count*dataSize,std::ios::beg);
                  stream.write(theSchema.name.c_str(),theSchema.name.length());stream.flush();
                   */
                  std::memmove(theBlock.payload+count*dataSize,theSchema.name.c_str(),theSchema.name.length());
                  /*
                  if(theSchema.name.length()<kNameLength)
                      stream.write(reinterpret_cast<char*>(&separator),kNameLength-theSchema.name.length());stream.flush();
                      */
                  if(theSchema.name.length()<kNameLength)
                      std::memmove(theBlock.payload+count*dataSize+theSchema.name.length(),&separator,sizeof(separator));
                  /*
                  stream.seekp(offSet+sizeof(BlockHeader)+count*dataSize+kNameLength,std::ios::beg);
                  stream.write(reinterpret_cast<char*>(&blockNum),sizeof(uint32_t));stream.flush();
                   */
                  std::memmove(theBlock.payload+count*dataSize+kNameLength,&blockNum,sizeof(uint32_t));
                  writeBlock(offSet,theBlock);
                  //replace this entire block of code ^^^ with some writeBlock logic
                  return;
              }
              count++;
          }
          offSet=theBlock.header.next;
      }while(offSet!=0);
      Block tempBlock=makeBlock(BlockType::meta_block);
      auto loc = ECE141::BlockIO::getFreeBlock();
      writeBlock(loc,tempBlock);
      temp=0;
      do{
          readBlock(temp,theBlock);
          if(!theBlock.header.next){
              theBlock.header.next=loc;
              writeBlock(temp,theBlock);
              break;
          }
          temp=theBlock.header.next;
      }while(temp!=0);
      writeToMeta(blockNum, theSchema);
  }

  std::vector<std::string> Storage::getTables(){
      size_t offSet{0};
      Block metaBlock;
      std::vector<std::string> tables;
      size_t count;
      char* ptr;
      char buffer[64];
      auto dataSize=kNameLength+sizeof(uint32_t);
      do{
          stream.seekg(offSet,std::ios::beg);
          stream.read(reinterpret_cast<char*>(&metaBlock),sizeof(metaBlock));stream.flush();
          count=0;
          ptr=metaBlock.payload;
          while(count*dataSize<sizeof(metaBlock.payload)-dataSize){
              std::memmove(buffer, ptr+count*dataSize, kNameLength);
              if(notZeroes(buffer, sizeof(buffer))){
                  std::string result(buffer, getBufferLength(buffer, '*'));
                  tables.push_back(result);
              }
              count++;
          }
          offSet+=kBlockSize;
      }while(metaBlock.header.next!=0);
      return tables;
  }

  std::shared_ptr<Schema> Storage::readSchema(std::string theName){
      auto offSet = findBlock(theName);
      if(!offSet)
          return nullptr;
      std::vector<ECE141::Attribute> theAttributes;
      Block theBlock;
      std::shared_ptr<Schema> theSchema = std::make_shared<Schema>(theName);
      char* ptr;
      RowSchemaPayload thePayload;
      std::stringstream theStream;
      do {
          readBlock(offSet,theBlock);
          ptr=theBlock.payload;
          std::memmove(&thePayload,ptr,kPayloadSize);
          theStream.write(thePayload.payload,kPayloadSize-sizeof(char));theStream.flush();
      }while(thePayload.more!=0);
      theSchema->decode(theStream);
      return theSchema;
  }

  std::tuple<std::string, uint32_t, bool> Storage::handleDump() {
      Block theBlock;
      readBlock(dumpCounter++,theBlock);
      bool theBool=dumpCounter==getBlockCount();
      uint32_t done = theBool ? 1 : 0;
      if(done)
          dumpCounter=0;
      return {BlockCharToString[theBlock.header.type],done,theBool};
  }

  uint32_t Storage::findBlock(std::string name){
      size_t offSet{0};
      Block theBlock;
      do{
          readBlock(offSet,theBlock);
          auto present = std::search(theBlock.payload, theBlock.payload+sizeof(theBlock.payload), name.begin(), name.end());
          if(present != theBlock.payload+sizeof(theBlock.payload)){
              present+=kNameLength;
              uint32_t ans;
              std::memcpy(&ans, present, sizeof(ans));
              return ans;
          }
          offSet=theBlock.header.next;
      }while(offSet!=0);
      return 0;
  }

  int Storage::deleteTable(std::string name){
      //wipe out every block , schema and row, associated with this name. just leave meta
      auto schemaLocation = findBlock(name);
      int ans{0};
      Block tempBlock;
      readBlock(schemaLocation,tempBlock);
      auto temp = tempBlock.header.next;
      while(temp!=0){
          replaceChain(name,temp);
          wipeBlock(temp);
          readBlock(schemaLocation,tempBlock);
          temp=tempBlock.header.next;
          ans++;
      }
      wipeBlock(schemaLocation);

      size_t offSet{0};
      Block theBlock;
      auto dataLength = kNameLength+sizeof(uint32_t);
      do{
          stream.seekg(offSet,std::ios::beg);
          stream.read(reinterpret_cast<char*>(&theBlock),sizeof(theBlock));stream.flush();
          auto present = std::search(theBlock.payload, theBlock.payload+sizeof(theBlock.payload), name.begin(), name.end());
          if(present != theBlock.payload+sizeof(theBlock.payload)){
              std::memset(present,0,dataLength);
              stream.seekp(offSet,std::ios::beg);
              stream.write(reinterpret_cast<char*>(&theBlock),sizeof(theBlock));stream.flush();
          }
          offSet=theBlock.header.next;
      }while(offSet!=0);
      return ans;
  }

  void Storage::wipeBlock(uint32_t blockNum){
      Block theBlock;
      readBlock(blockNum,theBlock);
      theBlock.header.type=static_cast<char>(ECE141::BlockType::free_block);
      theBlock.header.next=0;
      char* ptr = theBlock.payload;
      std::memset(ptr, 0, sizeof(theBlock.payload));
      writeBlock(blockNum,theBlock);
      ///change later to wipe chain of blocks, following the next linking
  }

  StatusResult Storage::updateChain(std::string theName, uint32_t pos) {
      StatusResult theResult;
      auto firstPos = findBlock(theName);
      Block theBlock;
      readBlock(firstPos, theBlock);
      while(theBlock.header.next!=0){
          firstPos=theBlock.header.next;
          readBlock(theBlock.header.next,theBlock);
      }
      theBlock.header.next=pos;
      writeBlock(firstPos,theBlock);
      return theResult;
  }

  StatusResult Storage::setID(Row* theRow, ECE141::Schema &theSchema, RowCollection& rows) {
      auto pair = getKeyMode(theSchema);
      if(pair.first==KeyMode::autokey)
          return setAutoKey(theRow,theSchema, rows);
      return setNonAutoKey(theRow, theSchema, pair);
  }

  std::pair<KeyMode,std::string> Storage::getKeyMode(ECE141::Schema &theSchema) {
      for(auto& attribute : theSchema.attributes ){
          if(attribute->primarykey){
              if(attribute->autoinc)
                  return std::make_pair(KeyMode::autokey,"");
              else
                  return std::make_pair(KeyMode::mankey,attribute->name);
          }
      }
      return std::make_pair(KeyMode::nokey,"");
  }

  StatusResult Storage::setAutoKey(Row* theRow, Schema& theSchema, RowCollection& rows){
      StatusResult theResult;
      Block theBlock;
      readBlock(findBlock(theSchema.name),theBlock);
      while(theBlock.header.next!=0)
          readBlock(theBlock.header.next,theBlock);
      if(theBlock.header.type=='S'&&rows.empty()) { theRow->setID(1); return theResult;}
      if(rows.empty()){
          uint32_t oldId;
          std::memmove(&oldId,theBlock.payload+rowIdOffsetIntoBlockPayload,sizeof(uint32_t));
          theRow->setID(oldId+1);
      }
      else{
          auto it = rows.rbegin();
          theRow->setID((*it)->getID()+1);
      }
      return theResult;
  }

  StatusResult Storage::setNonAutoKey(Row* theRow, Schema& theSchema, std::pair<KeyMode,std::string> thePair){
      StatusResult theResult;
      Block theBlock;
      readBlock(findBlock(theSchema.name),theBlock);
      while(theBlock.header.type=='S' && theBlock.header.next!=0)
          readBlock(theBlock.header.next,theBlock);
      uint32_t theKey = hash(theRow,thePair);
      if(theBlock.header.type=='S') {theRow->setID(theKey); return theResult;}
      uint32_t temp;
      while(theBlock.header.next!=0){
          std::memmove(&temp,theBlock.payload+rowIdOffsetIntoBlockPayload,sizeof(uint32_t));
          if(theKey==temp) {theResult.error=Errors::unknownIndex;return theResult;}
          readBlock(theBlock.header.next,theBlock);
      }
      theRow->setID(theKey);
      return theResult;
  }

  StatusResult Storage::getRows(DBQuery &theQuery, RowCollection& aCollection){
      std::vector<uint32_t> repeats;
      char more{0};
      std::stringstream theStream;
      char tempName[kNameLength];
      each([&](Block& aBlock, uint32_t theInt) -> StatusResult{
          StatusResult theResult;
          if(aBlock.header.type=='D'){
              if(std::find(repeats.begin(),repeats.end(),theInt)==repeats.end()){
                  std::memmove(&tempName,aBlock.payload+rowOwnerOffsetIntoBlockPayload,kNameLength);
                  std::string tempStr = tempName;
                  auto tempSchema = readSchema(tempStr);
                  do{
                      theStream.write(aBlock.payload+rowIdOffsetIntoBlockPayload,kPayloadSize-rowIdOffsetIntoBlockPayload);
                      std::memmove(&more,aBlock.payload,sizeof(char));
                      if(more)
                          repeats.push_back(aBlock.header.next);
                      readBlock(aBlock.header.next,aBlock);
                  }while(more!=0);
                  std::unique_ptr<Row> theRow = std::make_unique<Row>();
                  theRow->setSchema(tempSchema);
                  for(auto& schema : theQuery.fromTables){
                      if(schema->name==theRow->getSchema()->name){
                          theRow->decode(theStream);
                          auto temp = theRow->getData();
                          if(getKeyMode(*(theRow->getSchema())).first==KeyMode::autokey){
                              for(const auto& attr : theRow->getSchema()->attributes){
                                  if(attr->primarykey&&attr->autoinc)
                                      temp[attr->name]=static_cast<int>(theRow->getID());
                              }
                          }
                          if(theQuery.filters.getExpressions().empty() ||
                             theQuery.filters.matches(temp))
                              aCollection.push_back(std::move(theRow));
                          break;
                      }
                  }
                  theStream.str("");
              }
              else{
                  repeats.erase(std::find(repeats.begin(),repeats.end(),theInt));
              }
          }
          return theResult;
      });
      StatusResult theResult;
      theResult.error=Errors::unknownError;
      if(join(theQuery,aCollection)){
          if(order(theQuery,aCollection)){
              return applyLimit(theQuery, aCollection);
          }
      }
      return theResult;
  }

  StatusResult Storage::join(DBQuery &aQuery, RowCollection& aCollection){
      StatusResult theResult;
      if(!aQuery.joins.empty()){
          for(auto& join : aQuery.joins){
              if(!theResult) {return theResult;}
              theResult=join.apply(aCollection,aQuery.columns);
          }
      }
      return theResult;
  }

  int Storage::updateRows(ECE141::DBQuery &theQuery) {
      RowCollection theRows;
      auto theResult=getRows(theQuery,theRows);
      changeRows(theQuery, theRows);
      for(const auto& row : theRows){
          if(theResult)
              theResult=replaceRow(row.get());
          else
              break;
      }
      if(theResult)
          return theRows.size();
      else
          return -1;
  }

  StatusResult Storage::replaceRow(Row* aRow){
      auto thePos = findRowPos(aRow); // std::pair<size_t, std::vector<size_t>>
      std::stringstream theStr;
      aRow->encode(theStr);
      Chunker theChunker(theStr);
      size_t index{0};
      return theChunker.each([&](Block &aBlock){
          StatusResult theResult;
          aBlock.header.type=static_cast<char>(BlockType::data_block);
          auto pos = !index ? thePos.first : thePos.second[index-1];
          auto next = getNext(pos, aRow->getSchema()->name); //find who Im supposed to point to
          aBlock.header.next = next;
          writeBlock(pos,aBlock);
          index++;
          return theResult;
      });
  }

  size_t Storage::getNext(size_t myPos, std::string aName){
      auto firstPos = findBlock(aName);
      Block theBlock;
      readBlock(firstPos, theBlock);
      while(theBlock.header.next!=myPos){
          readBlock(theBlock.header.next,theBlock);
      }
      readBlock(theBlock.header.next,theBlock);
      return theBlock.header.next;
  }

  std::pair<size_t,std::vector<size_t>> Storage::findRowPos(Row* aRow){
      std::pair<size_t,std::vector<size_t>> ans;
      auto thePos = findBlock(aRow->getSchema()->name);
      Block theBlock;
      readBlock(thePos,theBlock);
      std::stringstream theStream;
      char more{0};
      while(true){
          theStream.str("");
          ans.second.clear();
          thePos = theBlock.header.next;
          readBlock(thePos,theBlock);
          do{
              theStream.write(theBlock.payload+rowIdOffsetIntoBlockPayload,kPayloadSize-rowIdOffsetIntoBlockPayload);
              std::memmove(&more,theBlock.payload,sizeof(char));
              if(more){
                  ans.second.push_back(theBlock.header.next);
                  readBlock(theBlock.header.next,theBlock);
              }
          }while(more!=0);
          Row tempRow;
          tempRow.setSchema(aRow->getSchema());
          tempRow.decode(theStream);
          if(tempRow.getID()==aRow->getID()){
              ans.first=thePos;
              return ans;
          }
      }
  }

  void Storage::changeRows(DBQuery& theQuery, RowCollection& aRows){
      for(auto &row : aRows){
          for(auto & attr: row->getData()){
              if(attr.first==theQuery.columns[0])
                  attr.second=theQuery.updatedValue.value;
          }
      }
  }

  int Storage::deleteRows(ECE141::DBQuery &theQuery) {
      Block theBlock;
      std::stringstream theStream;
      readBlock(findBlock(theQuery.fromTables[0]->name),theBlock);
      auto curr = theBlock.header.next;
      int count{0};
      while(curr!=0){
          theStream.str("");
          readBlock(curr,theBlock);
          auto old = curr;
          curr=theBlock.header.next;
          theStream.write(theBlock.payload+rowIdOffsetIntoBlockPayload, kPayloadSize-rowIdOffsetIntoBlockPayload);
          //^^note that there is NOT support for rows taking up more than a block. just like in getRows.
          Row tempRow;
          tempRow.setSchema(readSchema(theQuery.fromTables[0]->name));
          tempRow.decode(theStream);
          auto temp = tempRow.getData();
          if(getKeyMode(*(tempRow.getSchema())).first==KeyMode::autokey){
              for(const auto & attr : tempRow.getSchema()->attributes){
                  if(attr->primarykey&&attr->autoinc)
                      temp[attr->name]=static_cast<int>(tempRow.getID());
              }
          }
          if(theQuery.filters.matches(temp)){
              replaceChain(tempRow.getSchema()->name,old);
              wipeBlock(old);
              count++;
          }
      }
      return count;
  }

  StatusResult Storage::replaceChain(std::string theName, uint32_t pos) {
      StatusResult theResult;
      auto curr = findBlock(theName);
      Block theBlock;
      readBlock(curr, theBlock);
      while(theBlock.header.next!=0){
          if(theBlock.header.next==pos){
              Block tempBlock;
              readBlock(theBlock.header.next,tempBlock);
              theBlock.header.next=tempBlock.header.next;
              theResult=writeBlock(curr,theBlock);
          }
          if(!theBlock.header.next) {break;}
          curr=theBlock.header.next;
          readBlock(curr,theBlock);
      }
      return theResult;
  }

  StatusResult Storage::applyLimit(DBQuery& aQuery, RowCollection& aCollection){
      StatusResult theResult;
      if(aQuery.limit>=0&&aCollection.size()>aQuery.limit){
          auto it = aCollection.end() - (aCollection.size()-aQuery.limit);
          aCollection.erase(it, aCollection.end());
      }
      return theResult;
  }

  StatusResult Storage::order(DBQuery& aQuery, RowCollection& aCollection){
      StatusResult theResult;
      for(auto & attr : aQuery.fromTables[0]->attributes){
          if(attr->name==aQuery.orderBy){
              if(attr->type==DataTypes::bool_type){
                  theResult.error=Errors::invalidCommand;
              }
              else
                  theResult=orderVec(aQuery.orderBy, aCollection);
          }
      }
      if(aQuery.descending)
          std::reverse(aCollection.begin(),aCollection.end());
      return theResult;
  }

  StatusResult Storage::orderVec(std::string aColumn, RowCollection& aCollection){
      StatusResult theResult;
      auto max = aCollection.size();
      bool swapped;
      do{
          swapped=false;
          for(size_t i=1;i<max;i++){
              if(aCollection[i-1]->getData()[aColumn]>aCollection[i]->getData()[aColumn]){
                  std::swap(aCollection[i-1],aCollection[i]);
                  swapped=true;
              }
          }
          max--;
      }while(swapped);
      return theResult;
  }

  uint32_t Storage::hash(Row* theRow, std::pair<KeyMode,std::string> thePair){
      std::vector<std::variant<int,std::string, float, bool>> theVars;
      std::string theKey;
      if(thePair.first==KeyMode::mankey)
          theVars.push_back(theRow->getData()[thePair.second]);
      else{
          for(auto &pair : theRow->getData())
              theVars.push_back(pair.second);
      }
      for(auto& var: theVars)
          theKey+=stringify(var);
      for(const char& c : theKey){
          uint32_t hash = 0;
          for (char c : theKey)
              hash = (hash << 7) + hash + c;
          return hash;
      }
      return 0;
  }

  std::string Storage::stringify(std::variant<int,std::string, float, bool> theKey){
      std::string tempStr;
      if(auto val = std::get_if<int>(&theKey))
          tempStr=std::to_string(*val);
      else if(auto val = std::get_if<std::string>(&theKey))
          tempStr=*val;
      else if(auto val = std::get_if<float>(&theKey))
          tempStr=std::to_string(*val);
      else if(auto val = std::get_if<bool>(&theKey))
          tempStr=std::to_string(*val);
      return tempStr;
  }

}

