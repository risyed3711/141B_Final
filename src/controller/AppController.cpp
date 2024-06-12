//
//  CommandProcessor.cpp
//  ECEDatabase
//
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.
//

#include <iostream>
#include <memory>
#include <vector>
#include "controller/AppController.hpp"
#include "tokenizer/Tokenizer.hpp"

namespace ECE141 {
  
  AppController::AppController(std::ostream &anOutput) : running{true} {}
  
  AppController::~AppController() {}
  
  // USE: ----------------------------------------------------- CHAIN OF RESPONSIBILITY PATTERN
  
  //build a tokenizer, tokenize input, ask processors to handle...
  StatusResult AppController::handleInput(std::istream &anInput,ViewListener aViewer){
    Tokenizer theTokenizer(anInput);
    StatusResult theResult=theTokenizer.tokenize();

    //initialize COR here
    std::shared_ptr<AppLevelParser> theAppParser = std::make_shared<AppLevelParser>();
    std::shared_ptr<DBLevelParser> theDBParser = std::make_shared<DBLevelParser>(theAppParser);
    std::shared_ptr<SQLLevelParser> theSQLParser = std::make_shared<SQLLevelParser>(theDBParser);

    std::shared_ptr<Statement> theStatement;
    while (theResult && theTokenizer.more()){
        if(auto theHandler=theSQLParser->findHandler(theTokenizer)){
            theStatement = theHandler->parseStatement(theTokenizer, appModel);
            theResult=theHandler->handle(theStatement, aViewer, appModel);
            theTokenizer.next();
            if(theStatement->isQuitting) this->running = false;
            if(theResult) theTokenizer.skipIf(';');
        }
        else{
            Processor::printErrorMessage(aViewer,errorMessages[theStatement->error]);
            theResult=Errors::unknownCommand;
        }
    }
    return theResult;
  }

  OptString AppController::getError(StatusResult &aResult) const {

    static std::map<ECE141::Errors, std::string_view> theMessages = {
      {Errors::illegalIdentifier, "Illegal identifier"},
      {Errors::unknownIdentifier, "Unknown identifier"},
      {Errors::databaseExists, "Database exists"},
      {Errors::tableExists, "Table Exists"},
      {Errors::syntaxError, "Syntax Error"},
      {Errors::unknownCommand, "Unknown command"},
      {Errors::unknownDatabase,"Unknown database"},
      {Errors::unknownTable,   "Unknown table"},
      {Errors::unknownError,   "Unknown error"}
    };

    std::string_view theMessage="Unknown Error";
    if(theMessages.count(aResult.error)) {
      theMessage=theMessages[aResult.error];
    }
    return theMessage;
  }




}
