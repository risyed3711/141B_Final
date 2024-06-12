//
//  BlockIO.hpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#ifndef BlockIO_hpp
#define BlockIO_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <variant>
#include <functional>
#include "misc/Errors.hpp"
#include "misc/Config.hpp"
#include "storage/Cache.hpp"
#include <cstring>
#include <memory>

namespace ECE141 {

  enum class BlockType {
      empty_block=0,
    data_block='D',
    free_block='F',
    //other types?
    meta_block='M',
    schema_block='S'
  };


  //a small header that describes the block...
  struct BlockHeader {
   
    BlockHeader(BlockType aType=BlockType::empty_block, uint32_t next=0)
      : type(static_cast<char>(aType)), next(next) {}

    BlockHeader(const BlockHeader &aCopy) {
      *this=aCopy;
    }
        
    void empty() {
      type=static_cast<char>(BlockType::free_block);
    }
    
    BlockHeader& operator=(const BlockHeader &aCopy) {
      type=aCopy.type;
      next=aCopy.next;
      return *this;
    }
   
    char      type;     //char version of block type
    uint32_t next;
    //other properties?
  };

  const size_t kBlockSize = 1024;
  const size_t kPayloadSize = kBlockSize - sizeof(BlockHeader);
  const size_t kNameLength = 64;
  const size_t rowIdOffsetIntoBlockPayload = sizeof(char);
  const size_t rowOwnerOffsetIntoBlockPayload = sizeof(char)+sizeof(uint32_t);
  const size_t rowPayloadOffsetIntoBlockPayload = sizeof(char)+sizeof(uint32_t)+kNameLength;

  //enables ability for a row or schema to take up more than one block. If done == 1,  the row/schema takes more than one block.
    struct RowSchemaPayload{
        char more;
        char payload[kPayloadSize-sizeof(more)];
    };
  
  //block .................
  class Block {
  public:
    Block(BlockType aType=BlockType::data_block, uint32_t next=0) {
        header.type=static_cast<char>(aType);
        header.next=next;
        char*ptr = payload;
        std::memset(ptr, 0, kPayloadSize);
    }
    Block(const Block &aCopy);
    
    Block& operator=(const Block &aCopy);
   
    StatusResult write(std::ostream &aStream);
    void wipe();
        
    BlockHeader   header;
    char          payload[kPayloadSize];
  };

  //------------------------------

   struct CreateFile {
    operator std::ios_base::openmode() {
      return std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc;
    }
  }; //tags for db-open modes...
  struct OpenFile {
    operator std::ios_base::openmode() {
      return std::ios::binary | std::ios::in | std::ios::out;
    }
  };

  using AccessMode=std::variant<CreateFile, OpenFile>;

  class BlockIO {
  public:
    
    BlockIO(const std::string &aName, AccessMode aMode);

    uint32_t              getBlockCount();
    uint32_t getFreeBlock();
    static size_t getBufferLength(char* arr, char delim);
    static bool notZeroes(char* arr, size_t size);
    
    virtual StatusResult  readBlock(uint32_t aBlockNumber, Block &aBlock);
    virtual StatusResult  writeBlock(uint32_t aBlockNumber, Block &aBlock);
    
  protected:
    StatusResult storeWriteThrough(uint32_t aBlockNum, std::shared_ptr<Block> aBlock);
    std::fstream stream;
    std::unique_ptr<LRUCache<uint32_t,Block>> theCache;
  };

}


#endif /* BlockIO_hpp */
