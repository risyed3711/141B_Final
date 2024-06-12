//
//  View.hpp
//  PA1
//
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.
//


#ifndef View_h
#define View_h


#include <iostream>
#include <functional>
#include "misc/Errors.hpp"

namespace ECE141 {

  //completely generic view, which you will subclass to show information
  class View {
  public:
    virtual         ~View() {}
    virtual bool    show(std::ostream &aStream)=0;
  protected:
      std::string message;
  };

  class staticView : public View {
  public:
      bool show(std::ostream &aStream) override {
          aStream << message << std::endl;
          return true;
      }
      StatusResult setMessage(const std::string &aMessage){
          ECE141::StatusResult theResult;
          this->message = aMessage;
          return theResult;
      }
  };

  class FolderView: public View {
  public:
      FolderView() {
          separator="+----------------------------+\n";
          longseparator="+----------------------------+----------------------------+\n";
          lineLength=separator.length();
      }
      StatusResult addCol(int numCol = 1){
          StatusResult success;
          for(auto i= 1; i<=numCol; i++) {
              separator = "+----------------------------" + separator;
          }
          return success;
      }
      StatusResult setFolderTitle(std::string title){
          message+=separator;
          addRow(title);
          return addSeparator();
      }
      StatusResult setFolderTitle(std::vector<std::string> titles){
          message+=separator;
          addRow(titles);
          return addSeparator();
      }
      StatusResult addRow(std::string rowName){
          StatusResult success;
          message+="| "+rowName;
          padWhiteSpace(rowName.length());
          message+="|\n";
          return success;
      }
      StatusResult addRow(std::vector<std::string> rowNames){
          StatusResult success;
          for(const auto& col : rowNames) {
              message += "| " + col;
              padWhiteSpace(col.length());
          }
          message+="|\n";
          return success;
      }
      StatusResult addSeparator(){
          StatusResult success;
          message+=separator;
          return success;
      }
      StatusResult addLongSeparator(){
          StatusResult success;
          message+=longseparator;
          return success;
      }
      bool show(std::ostream &aStream) override {
          aStream << message << std::endl;
          return true;
      }
      StatusResult addMessage(std::string msg){
          StatusResult theResult;
          message+=msg;
          return theResult;
      }
  protected:
      StatusResult padWhiteSpace(size_t length){
          StatusResult success;
          size_t temp=length+3;
          while(temp++<lineLength-1){
              message+=" ";
          }
          return success;
      }
      std::string separator;
      std::string longseparator;
      size_t lineLength;
  };

  using ViewListener = std::function<void(View &aView)>;

}

#endif /* View_h */
