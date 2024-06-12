#ifndef DBHandler_h
#define DBHandler_h

#include "Handler.hpp"
#include "misc/Config.hpp"
#include "misc/DBQuery.hpp"
#include "database/Schema.hpp"
#include "misc/Helpers.hpp"
#include "misc/ParseHelper.hpp"

static std::map<ECE141::Keywords, std::string> dbMessages{
    {ECE141::Keywords::create_kw,
     "Query OK, 1 row affected "},
    {ECE141::Keywords::use_kw,
     "Database changed "},
    {ECE141::Keywords::drop_kw,
     "Query OK, 0 rows affected "},
    {ECE141::Keywords::unknown_kw,
     "Invalid message, "}
};

    struct DBStatement : public Statement, public ECE141::ClauseHandler {
        DBStatement(ECE141::Model &theModel) : theModel(theModel) {}
        ECE141::KWList getKWList() override {
            static ECE141::KWList selectList={ECE141::Keywords::select_kw,ECE141::Keywords::from_kw,
                                              ECE141::Keywords::order_kw,ECE141::Keywords::where_kw,
                                              ECE141::Keywords::limit_kw,ECE141::Keywords::update_kw,
                                              ECE141::Keywords::set_kw,ECE141::Keywords::delete_kw};
            return selectList;
        }
        ECE141::StatusResult parseClause(ECE141::Tokenizer& aTokenizer) override {
            return SelectClauseRouting[aTokenizer.current().keyword](aTokenizer);
        }
        ECE141::StatusResult parseSelect(ECE141::Tokenizer& aTokenizer){
            ECE141::StatusResult theResult;
            myKeyWord=ECE141::Keywords::select_kw;
            aTokenizer.next();
            theQuery.all = (aTokenizer.current().data == "*");
            if(!theQuery.all){
                while(aTokenizer.current().keyword!=ECE141::Keywords::from_kw){
                    theQuery.columns.push_back(aTokenizer.current().data);
                    aTokenizer.next();
                    aTokenizer.skipIf(',');
                }
                aTokenizer.next(-1);
            }
            aTokenizer.next();
            return theResult;
        }
        ECE141::StatusResult parseFrom(ECE141::Tokenizer& aTokenizer){
            ECE141::StatusResult theResult;
            aTokenizer.next();
            ECE141::TokenSequencer theTS(aTokenizer,nullptr);
            std::string tableName;
            theTS.captureIf(tableName);
            if(!theTS.isValid()) {theResult.error=ECE141::Errors::illegalIdentifier;return theResult;}
            theQuery.addSchema(theModel.currentDatabase->myStorage.readSchema(tableName));
            if(ECE141::Helpers::in_array<ECE141::Keywords>(ECE141::gJoinTypes, aTokenizer.current().keyword))
                return parseJoin(aTokenizer);
            return theResult;
        }
        ECE141::StatusResult parseOrder(ECE141::Tokenizer& aTokenizer){
            ECE141::StatusResult theResult;
            aTokenizer.next();
            aTokenizer.skipIf(ECE141::Keywords::by_kw);
            ECE141::TokenSequencer theTS(aTokenizer, nullptr);
            std::string theAttributeName;
            theTS.captureIf(theAttributeName);
            if(!theTS.isValid()) {theResult.error=ECE141::Errors::illegalIdentifier;return theResult;}
            theQuery.orderBy=theAttributeName;
            if(aTokenizer.current().keyword==ECE141::Keywords::desc_kw)
            {theQuery.descending=true;aTokenizer.next();}
            return theResult;
        }
        ECE141::StatusResult parseWhere(ECE141::Tokenizer& aTokenizer){
            ECE141::StatusResult theResult;
            ECE141::Filters theFilter;
            auto theSchema = theQuery.fromTables[0];
            aTokenizer.skipIf(ECE141::Keywords::where_kw);
            theFilter.parse(aTokenizer,theSchema);
            theQuery.filters=theFilter;
            return theResult;
        }
        ECE141::StatusResult parseLimit(ECE141::Tokenizer& aTokenizer){
            ECE141::StatusResult theResult;
            ECE141::TokenSequencer theTS(aTokenizer, nullptr);
            theTS.skipIf(ECE141::Keywords::limit_kw);
            theTS.captureNumber(theQuery.limit);
            theResult.error = theTS.isValid() ? ECE141::Errors::noError : ECE141::Errors::unknownError;
            return theResult;
        }
        ECE141::StatusResult parseUpdate(ECE141::Tokenizer& aTokenizer){
            myKeyWord=ECE141::Keywords::update_kw;
            return parseFrom(aTokenizer);
        }
        ECE141::StatusResult parseDelete(ECE141::Tokenizer& aTokenizer){
            myKeyWord=ECE141::Keywords::delete_kw;
            aTokenizer.next();
            ECE141::StatusResult theResult;
            return theResult;
        }
        ECE141::StatusResult parseSet(ECE141::Tokenizer& aTokenizer){
            ECE141::StatusResult theResult;
            aTokenizer.next();
            theQuery.columns.push_back(aTokenizer.current().data);
            aTokenizer.next();
            ECE141::TokenSequencer theTS(aTokenizer, nullptr);
            aTokenizer.skipIf('=');
            if(aTokenizer.current().type==ECE141::TokenType::number)
                theQuery.updatedValue.setNumber(aTokenizer.current());
            else if(aTokenizer.current().type==ECE141::TokenType::identifier)
                theQuery.updatedValue.setVarChar(aTokenizer.current());
            else
                theResult.error=ECE141::Errors::illegalIdentifier;
            theResult.error = theTS.isValid() ? ECE141::Errors::noError : ECE141::Errors::illegalIdentifier;
            return theResult;
        }
        ECE141::StatusResult parseJoin(ECE141::Tokenizer& aTokenizer){
            ECE141::StatusResult theResult;
            ECE141::Join theJoin;
            theJoin.joinType=aTokenizer.current().keyword;
            aTokenizer.next();
            ECE141::TokenSequencer theTS(aTokenizer, nullptr);
            theTS.skipIf("join");
            std::string otherTable;
            theTS.captureIf(otherTable);
            theQuery.fromTables.push_back(theModel.currentDatabase->myStorage.readSchema(otherTable));
            theTS.skipIf(ECE141::Keywords::on_kw);
            theResult = parseJoinConditions(aTokenizer, theJoin);
            theQuery.joins.push_back(theJoin);
            theResult.error = theTS.isValid() ? ECE141::Errors::noError : ECE141::Errors::illegalIdentifier;
            if(ECE141::Helpers::in_array<ECE141::Keywords>(ECE141::gJoinTypes, aTokenizer.current().keyword))
                return parseJoin(aTokenizer);
            return theResult;
        }
        ECE141::StatusResult parseJoinConditions(ECE141::Tokenizer& aTokenizer, ECE141::Join& aJoin){
            ECE141::StatusResult theResult;
            theResult.error=ECE141::Errors::illegalIdentifier;
            std::vector<std::string> conditions;
            ECE141::TableField field1;
            ECE141::TableField field2;
            ECE141::ParseHelper theHelper(aTokenizer);
            if(theHelper.parseTableField(aTokenizer,field1)){
                if(aTokenizer.skipIf('=')){
                    if(theHelper.parseTableField(aTokenizer,field2)){
                        aJoin.conditions.first=std::make_shared<ECE141::TableField>(field1);
                        aJoin.conditions.second=std::make_shared<ECE141::TableField>(field2);
                        theResult.error=ECE141::Errors::noError;
                    }
                }
            }
            return theResult;
        }
        std::map<ECE141::Keywords, std::function<ECE141::StatusResult(ECE141::Tokenizer&)>>
        SelectClauseRouting {
                {ECE141::Keywords::select_kw,[this](ECE141::Tokenizer&aTokenizer)
                {return this->parseSelect(aTokenizer);}},
                {ECE141::Keywords::from_kw,[this](ECE141::Tokenizer&aTokenizer)
                {return this->parseFrom(aTokenizer);}},
                {ECE141::Keywords::order_kw,[this](ECE141::Tokenizer&aTokenizer)
                {return this->parseOrder(aTokenizer);}},
                {ECE141::Keywords::where_kw,[this](ECE141::Tokenizer&aTokenizer)
                {return this->parseWhere(aTokenizer);}},
                {ECE141::Keywords::limit_kw,[this](ECE141::Tokenizer&aTokenizer)
                {return this->parseLimit(aTokenizer);}},
                {ECE141::Keywords::update_kw,[this](ECE141::Tokenizer&aTokenizer)
                {return this->parseUpdate(aTokenizer);}},
                {ECE141::Keywords::set_kw,[this](ECE141::Tokenizer&aTokenizer)
                {return this->parseSet(aTokenizer);}},
                {ECE141::Keywords::delete_kw,[this](ECE141::Tokenizer&aTokenizer)
                {return this->parseDelete(aTokenizer);}},
        };
        ECE141::DBQuery theQuery;
        ECE141::Model& theModel;
    };

struct DBLevelProcessor : public Processor {
    DBLevelProcessor() {}

    ECE141::StatusResult process(std::shared_ptr<Statement >aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel) override {
        logger->log(LogLevel::Info,"Processing");
        return DBLevelProcessing[aStatement->myKeyWord](aStatement, aViewer, theModel);
    }

    static ECE141::StatusResult handleCreateCommand(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel){
        ECE141::StatusResult theResult;
        auto timer = ECE141::Config::getTimer();
        auto t1 = timer.now();
        ECE141::Database theNewDB(aStatement->dbName,ECE141::CreateFile{});
        auto time = timer.elapsed(t1);
        ECE141::staticView theView;
        theView.setMessage(dbMessages[ECE141::Keywords::create_kw]+"("+
        std::to_string(time)+" secs)");
        aViewer(theView);
        return theResult;
    }

    static ECE141::StatusResult handleDropCommand(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel){
        ECE141::StatusResult theResult;
        auto timer = ECE141::Config::getTimer();
        auto t1 = timer.now();
        theModel.dropDB(aStatement->dbName);
        auto time = timer.elapsed(t1);
        ECE141::staticView theView;
        theView.setMessage(dbMessages[ECE141::Keywords::drop_kw]+"("+std::to_string(time)+" secs)");
        aViewer(theView);
        return theResult;
    }

    static ECE141::StatusResult handleShowCommand(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel){
        ECE141::StatusResult theResult;
        auto timer = ECE141::Config::getTimer();
        auto t1 = timer.now();
        std::vector<std::string> dbList = theModel.getDBList();
        auto time = timer.elapsed(t1);
        ECE141::FolderView theView;
        theView.setFolderTitle("Database");
        for(const auto &db : dbList){
            theView.addRow(db);
        }
        theView.addSeparator();
        theView.addMessage("Query OK, "+std::to_string(dbList.size())+" rows affected ");
        theView.addMessage("("+std::to_string(time)+" secs)");
        aViewer(theView);
        return theResult;
    }

    static ECE141::StatusResult handleUseCommand(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel){
        ECE141::StatusResult theResult;
        auto timer = ECE141::Config::getTimer();
        auto t1 = timer.now();
        theModel.setDB(aStatement->dbName);
        auto time = timer.elapsed(t1);
        ECE141::staticView theView;
        theView.setMessage(dbMessages[ECE141::Keywords::use_kw]+"("+
        std::to_string(time)+" secs)");
        aViewer(theView);
        return theResult;
    }

    static ECE141::StatusResult handleDumpCommand(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel){
        ECE141::StatusResult theResult;
        auto timer = ECE141::Config::getTimer();
        auto t1 = timer.now();
        bool originallyEmpty{false};
        if(theModel.currentDatabase== nullptr){theModel.setDB(aStatement->dbName);originallyEmpty= true;}
        ECE141::FolderView theView;
        std::vector<std::string> titles = {"#","Type"};
        theView.setFolderTitle(titles);
        std::tuple<std::string, uint32_t, bool> theTuple = {"",0,false};
        uint32_t count{0};
        uint32_t rowCount{0};
        while(!std::get<2>(theTuple)){
            std::vector<std::string> rowData;
            theTuple=theModel.currentDatabase->myStorage.handleDump();
            rowData.push_back(std::to_string(count++));
            rowData.push_back(std::get<0>(theTuple));
            if(std::get<0>(theTuple)=="data")
                rowCount++;
            theView.addRow(rowData);
        }
        std::string empty="";
        if(originallyEmpty)
            theModel.setDB(empty);
        auto time = timer.elapsed(t1);
        theView.addLongSeparator();
        theView.addMessage("Query OK, " + std::to_string(rowCount) + " rows affected (" + std::to_string(time) + " secs)");
        aViewer(theView);
        return theResult;
    }

    static ECE141::StatusResult handleSelectCommand(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel){
        ECE141::StatusResult theResult;
        std::shared_ptr<DBStatement> theStatement = std::static_pointer_cast<DBStatement>(aStatement);
        ECE141::FolderView theView;
        auto timer = ECE141::Config::getTimer();
        auto t1 = timer.now();
        theStatement->theQuery.fromTables[0]=theModel.currentDatabase->myStorage.readSchema(theStatement->theQuery.fromTables[0]->name);
        if(theStatement->theQuery.all){
            theView.addCol(theStatement->theQuery.fromTables[0]->attributes.size());
            std::vector<std::string> names;
            for(auto& attr:theStatement->theQuery.fromTables[0]->attributes)
                names.push_back(attr->name);
            theView.setFolderTitle(names);
        }
        else{
            theView.addCol(theStatement->theQuery.columns.size());
            theView.setFolderTitle(theStatement->theQuery.columns);
        }
        ECE141::RowCollection theRows;
        theResult = theModel.currentDatabase->myStorage.getRows(theStatement->theQuery,theRows);
        std::vector<std::string> viewRow;
        for(auto& row: theRows){
            viewRow.clear();
            for(const auto& table : theStatement->theQuery.fromTables){
                for(auto& attr : table->attributes){
                    if(theStatement->theQuery.all ||
                       std::find(theStatement->theQuery.columns.begin(),theStatement->theQuery.columns.end(),
                                 attr->name)!=theStatement->theQuery.columns.end()){
                        if(attr->primarykey==true)
                            viewRow.push_back(std::to_string(row->getID()));
                        else
                            viewRow.push_back(ECE141::Storage::stringify(row->getData()[attr->name])); //before stringify, fix join columnns if necessary
                    }
                }
            }
            theView.addRow(viewRow);
        }
        auto time = timer.elapsed(t1);
        if(!theResult){
            ECE141::staticView errorView;
            aViewer(errorView);
        }
        theView.addMessage("Query OK, " + std::to_string(theRows.size()) + " rows affected (" + std::to_string(time) + " secs)");
        aViewer(theView);
        return theResult;
    }

    static ECE141::StatusResult handleUpdateCommand(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel){
        std::shared_ptr<DBStatement> theStatement = std::static_pointer_cast<DBStatement>(aStatement);
        auto timer = ECE141::Config::getTimer();
        auto t1 = timer.now();

        theStatement->theQuery.fromTables[0]=theModel.currentDatabase->myStorage.readSchema(theStatement->theQuery.fromTables[0]->name);
        auto theResult = theModel.currentDatabase->myStorage.updateRows(theStatement->theQuery);

        auto time = timer.elapsed(t1);
        ECE141::staticView theView;
        theView.setMessage("Query OK, " + std::to_string(theResult) + " rows affected (" +
        std::to_string(time) + " secs)");
        aViewer(theView);
        ECE141::StatusResult theRes;
        theRes.error = theResult ? ECE141::Errors::noError : ECE141::Errors::writeError;
        return theRes;
    }

    static ECE141::StatusResult handleDeleteCommand(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel){
        std::shared_ptr<DBStatement> theStatement = std::static_pointer_cast<DBStatement>(aStatement);
        auto timer = ECE141::Config::getTimer();
        auto t1 = timer.now();

        theStatement->theQuery.fromTables[0]=theModel.currentDatabase->myStorage.readSchema(theStatement->theQuery.fromTables[0]->name);
        auto theResult = theModel.currentDatabase->myStorage.deleteRows(theStatement->theQuery);

        auto time = timer.elapsed(t1);
        ECE141::staticView theView;
        theView.setMessage("Query OK, " + std::to_string(theResult) + " rows affected (" +
                           std::to_string(time) + " secs)");
        aViewer(theView);
        ECE141::StatusResult theRes;
        theRes.error = theResult ? ECE141::Errors::noError : ECE141::Errors::writeError;
        return theRes;
    }

    std::map<ECE141::Keywords, ECE141::StatusResult(*)(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel)> DBLevelProcessing = {
            {ECE141::Keywords::create_kw, &DBLevelProcessor::handleCreateCommand},
            {ECE141::Keywords::drop_kw, &DBLevelProcessor::handleDropCommand},
            {ECE141::Keywords::show_kw, &DBLevelProcessor::handleShowCommand},
            {ECE141::Keywords::use_kw, &DBLevelProcessor::handleUseCommand},
            {ECE141::Keywords::dump_kw, &DBLevelProcessor::handleDumpCommand},
            {ECE141::Keywords::select_kw, &DBLevelProcessor::handleSelectCommand},
            {ECE141::Keywords::update_kw, &DBLevelProcessor::handleUpdateCommand},
            {ECE141::Keywords::delete_kw, &DBLevelProcessor::handleDeleteCommand}
    };
};

struct DBLevelParser : public CORParser {
    DBLevelParser(std::shared_ptr<CORParser> nextHandler = nullptr) : CORParser(nextHandler) {}
    std::shared_ptr<Statement> parseStatement(ECE141::Tokenizer &aTokenizer, ECE141::Model& aModel) override {
        std::shared_ptr<Statement> aStatement = std::make_shared<DBStatement>(aModel);
        ECE141::TokenSequencer theSequencer(aTokenizer, aStatement);
        auto theStatement = std::static_pointer_cast<DBStatement>(aStatement);

        if(theSequencer.skipIf(ECE141::Keywords::create_kw).skipIf(ECE141::Keywords::database_kw)
                .captureIf(theStatement->dbName).isValid())
            aStatement->myKeyWord=ECE141::Keywords::create_kw;
        else if(theSequencer.skipIf(ECE141::Keywords::drop_kw).skipIf(ECE141::Keywords::database_kw)
                .captureIf(theStatement->dbName).isValid())
            aStatement->myKeyWord=ECE141::Keywords::drop_kw;
        else if(theSequencer.skipIf(ECE141::Keywords::show_kw).skipIf(ECE141::Keywords::databases_kw)
                .isValid())
            theStatement->myKeyWord=ECE141::Keywords::show_kw;
        else if(theSequencer.skipIf(ECE141::Keywords::use_kw).
                captureIf(theStatement->dbName).isValid())
            theStatement->myKeyWord=ECE141::Keywords::use_kw;
        else if(theSequencer.skipIf(ECE141::Keywords::dump_kw).skipIf(ECE141::Keywords::database_kw)
                .captureIf(theStatement->dbName).isValid())
            theStatement->myKeyWord=ECE141::Keywords::dump_kw;
        //try parsing select using clauses, which is much more complicated
        else if(auto theResult = theStatement->parse(aTokenizer)) {}
        else
            theStatement->error=ECE141::Errors::identifierExpected;
        return theStatement;
    }
    ECE141::StatusResult canHandle(ECE141::Tokenizer &aTokenizer) override {
        ECE141::StatusResult theResult;
        if(myProcessor.DBLevelProcessing.find(aTokenizer.current().keyword)!=myProcessor.DBLevelProcessing.end()){
            if(aTokenizer.current().keyword!=ECE141::Keywords::create_kw
               &&aTokenizer.current().keyword!=ECE141::Keywords::drop_kw
               &&aTokenizer.current().keyword!=ECE141::Keywords::show_kw)
            {return theResult;}
            if(aTokenizer.peek().keyword==ECE141::Keywords::database_kw&&aTokenizer.current().keyword!=ECE141::Keywords::show_kw
            ||aTokenizer.peek().keyword==ECE141::Keywords::databases_kw&&aTokenizer.current().keyword==ECE141::Keywords::show_kw)
            {return theResult;}
        }
        theResult.error=ECE141::Errors::unexpectedKeyword;
        return theResult;
    }
    ECE141::StatusResult handle(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel) override {
        if(aStatement->error==ECE141::Errors::noError)
        {logger->log(LogLevel::Info,"handler found");return myProcessor.process(aStatement,aViewer, theModel);}
        ECE141::StatusResult theBadResult;
        Processor::printErrorMessage(aViewer,errorMessages[aStatement->error]+"1");
        theBadResult.error=aStatement->error;
        return theBadResult;
    }
private:
    DBLevelProcessor myProcessor;
};

#endif //DBHandler_hpp