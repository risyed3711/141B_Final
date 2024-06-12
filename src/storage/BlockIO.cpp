//
//  BlockIO.cpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#include <cstring>
#include "storage/BlockIO.hpp"
#include "misc/Config.hpp"

namespace ECE141 {

  //Block::Block(BlockType aType, uint32_t next) {}

  Block::Block(const Block &aCopy) {
    *this=aCopy;
  }

  Block& Block::operator=(const Block &aCopy) {
    std::memcpy(payload, aCopy.payload, kPayloadSize);
    header=aCopy.header;
    return *this;
  }

  StatusResult Block::write(std::ostream &aStream) {
    return StatusResult{Errors::noError};
  }

  //---------------------------------------------------

  struct modeToInt {
    std::ios::openmode operator()(CreateFile &aVal) {return aVal;}
      std::ios::openmode operator()(OpenFile &aVal) {return aVal;}
  };

  BlockIO::BlockIO(const std::string &aName, AccessMode aMode) : theCache(nullptr) {
    std::string thePath = Config::getDBPath(aName);
    
    auto theMode=std::visit(modeToInt(), aMode);
    stream.clear(); // Clear flag just-in-case...
    stream.open(thePath.c_str(), theMode); //force truncate if...
    stream.close();
    stream.open(thePath.c_str(), theMode);

    theCache=std::make_unique<LRUCache<uint32_t,Block>>(Config::getCacheSize(CacheType::block));

  }

  // USE: write data a given block (after seek) ---------------------------------------
  StatusResult BlockIO::writeBlock(uint32_t aBlockNum, Block &aBlock) {
      if(Config::useCache(CacheType::block)){
          if(theCache->contains(aBlockNum))
              theCache->update(aBlockNum,aBlock);
          //else
              //theCache->put(aBlockNum,aBlock);
      }
      stream.seekp(aBlockNum*kBlockSize, std::ios::beg);
      stream.write(reinterpret_cast<char*>(&aBlock),sizeof(aBlock));
      stream.flush();
      return StatusResult{Errors::noError};
  }

  // USE: write data a given block (after seek) ---------------------------------------
  StatusResult BlockIO::readBlock(uint32_t aBlockNumber, Block &aBlock) {
      bool storeInCache{false};
      if(Config::useCache(CacheType::block)){
          if(theCache->contains(aBlockNumber)) {
              aBlock=theCache->get(aBlockNumber);
              return StatusResult{Errors::noError};
          }
          storeInCache=true;
      }
      stream.seekg(aBlockNumber*kBlockSize, std::ios::beg);
      stream.read(reinterpret_cast<char*>(&aBlock),sizeof(aBlock));
      stream.flush();
      if(storeInCache)
          theCache->put(aBlockNumber,aBlock);
      return StatusResult{Errors::noError};
  }


    // USE: count blocks in file ---------------------------------------
    uint32_t BlockIO::getBlockCount()  {
        stream.seekg(0,std::ios::end);
        uint32_t end = stream.tellg();
        return end/kBlockSize; //What should this be?
    }

    uint32_t BlockIO::getFreeBlock(){
        Block aBlock;
        uint32_t offSet{0};
        auto max=getBlockCount();
        do{
            readBlock(offSet/kBlockSize,aBlock);
            offSet+=kBlockSize;
        }while(aBlock.header.type!='F'&&aBlock.header.type!=0&&offSet<max*kBlockSize);
        return offSet<max ? offSet/kBlockSize - kBlockSize: offSet/kBlockSize;
    }

    size_t BlockIO::getBufferLength(char* arr, char delim){
        for(size_t i = 0; i < kNameLength; i++){
            if(arr[i]==delim)
                return i;
        }
        return kNameLength;
    }

    bool BlockIO::notZeroes(char* arr, size_t size){
        for(size_t i = 0; i < size; i++){
            if(arr[i]!='\0')
                return true;
        }
        return false;
    }

}
