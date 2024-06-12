//
//  Schema.cpp
//  PA3
//
//  Created by rick gessner on 3/2/23.
//

#include "database/Schema.hpp"

namespace ECE141 {

 //STUDENT: Implement this class...

  Schema::Schema(const std::string aName, AttributeList) : name(aName) {}

  Schema::Schema(const Schema &aCopy) {
    name=aCopy.name;
  }

  StatusResult Schema::encode(std::ostream &anOutput) const {
      char separator='*';
      char buffer='\0';
      StatusResult theResult;
      for(auto attr: attributes){
          anOutput.write(attr->name.c_str(),attr->name.length());
          if(attr->name.length()<kNameLength)
              anOutput.write(&separator,sizeof(separator));
          if(attr->name.length()<kNameLength-1)
              anOutput.write(&buffer,kNameLength-1-attr->name.length());
          anOutput.write(reinterpret_cast<char*>(&attr->type),sizeof(attr->type));
          anOutput.write(reinterpret_cast<char*>(&attr->length),sizeof(attr->length));
          anOutput.write(reinterpret_cast<char*>(&attr->autoinc),sizeof(attr->autoinc));
          anOutput.write(reinterpret_cast<char*>(&attr->primarykey),sizeof(attr->primarykey));
          anOutput.write(reinterpret_cast<char*>(&attr->nullable),sizeof(attr->nullable));
          anOutput.flush();
      }
      if(!anOutput.good())
          theResult.error=Errors::unknownError;
      return theResult;
  }

  StatusResult Schema::decode(std::istream &anInput) {
      Attribute theAttribute;
      StatusResult theResult;
      char buffer[kPayloadSize];
      char*ptr=buffer;
      anInput.read(ptr,kPayloadSize);
      bool done;
      do{
          done=true;
          if(BlockIO::notZeroes(ptr,kNameLength)){
              std::string temp(ptr, BlockIO::getBufferLength(ptr, '*'));
              theAttribute.name=temp;
              ptr+=kNameLength;
              std::memmove(&(theAttribute.type),ptr,sizeof(theAttribute.type));
              ptr+=sizeof(theAttribute.type);
              std::memmove(&(theAttribute.length),ptr,sizeof(theAttribute.length));
              ptr+=sizeof(theAttribute.length);
              std::memmove(&(theAttribute.autoinc),ptr,sizeof(theAttribute.autoinc));
              ptr+=sizeof(theAttribute.autoinc);
              std::memmove(&(theAttribute.primarykey),ptr,sizeof(theAttribute.primarykey));
              ptr+=sizeof(theAttribute.primarykey);
              std::memmove(&(theAttribute.nullable),ptr,sizeof(theAttribute.nullable));
              ptr+=sizeof(theAttribute.nullable);
              addAttribute(theAttribute);
              done=false;
          }
      }while(!done);
      if(anInput.good())
          theResult.error=Errors::unknownError;
      return theResult;
  }

  bool Schema::initHeader(ECE141::Block &aBlock) const {
      aBlock.header.type=static_cast<char>(BlockType::schema_block);
      return true;
  }

  bool Schema::hasAttribute(std::string attributeName) {
      for(const auto& attr : attributes){
          if(attr->name==attributeName)
              return true;
      }
      return false;
  }

}
