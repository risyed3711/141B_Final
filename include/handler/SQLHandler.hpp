#ifndef SQLHandler_h
#define SQLHandler_h

#include "Handler.hpp"
#include "database/Attribute.hpp"
#include "database/Schema.hpp"
#include "database/Row.hpp"
#include "DBHandler.hpp"

static std::map<ECE141::Keywords, std::string> sqlMessages{
        {ECE141::Keywords::create_kw,
         "Query OK, 1 row affected "}
};

static std::map<ECE141::DataTypes, std::string> typeStrings{
        {ECE141::DataTypes::no_type, "none"},
        {ECE141::DataTypes::bool_type, "bool"},
        {ECE141::DataTypes::datetime_type, "date"},
        {ECE141::DataTypes::float_type, "float"},
        {ECE141::DataTypes::int_type, "integer"},
        {ECE141::DataTypes::varchar_type, "varchar"}
};

static std::map<ECE141::DataTypes, std::string> typeDefaults{
        {ECE141::DataTypes::no_type, "Null"},
        {ECE141::DataTypes::bool_type, "False"},
        {ECE141::DataTypes::datetime_type, "Null"},
        {ECE141::DataTypes::float_type, "0.0"},
        {ECE141::DataTypes::int_type, "Null"},
        {ECE141::DataTypes::varchar_type, "Null"}
};

struct SQLStatement : public Statement {
    SQLStatement() {}
    std::string sqlName;
    std::vector<ECE141::AttributeData> attributes;
    std::vector<std::string> dest;
    std::vector<std::variant<int, std::string, float, bool>> values;
};

struct SQLLevelProcessor : public Processor{
    SQLLevelProcessor() {}

    ECE141::StatusResult process(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel) override {
        logger->log(LogLevel::Info,"Processing");
        return SQLLevelProcessing[aStatement->myKeyWord](aStatement, aViewer, theModel);
    }

    static ECE141::StatusResult handleCreateCommand(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel){
        ECE141::StatusResult theResult;
        auto timer = ECE141::Config::getTimer();
        auto t1 = timer.now();
        std::shared_ptr<SQLStatement> theStatement = std::static_pointer_cast<SQLStatement>(aStatement);
        ECE141::Schema theSchema(theStatement->sqlName);
        for(const auto &attribute_data : theStatement->attributes){
            ECE141::Attribute theAttribute(attribute_data);
            theSchema.addAttribute(theAttribute);
        }
        theModel.currentDatabase->myStorage.storeSchema(theSchema);
        auto time = timer.elapsed(t1);
        ECE141::staticView theView;
        theView.setMessage(dbMessages[ECE141::Keywords::create_kw]+"("+std::to_string(time)+" secs)");
        aViewer(theView);
        return theResult;
    }


    static ECE141::StatusResult handleDropCommand(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel){
        ECE141::StatusResult theResult;
        std::shared_ptr<SQLStatement> theStatement = std::static_pointer_cast<SQLStatement>(aStatement);
        auto timer = ECE141::Config::getTimer();
        auto t1 = timer.now();
        auto rows = theModel.currentDatabase->myStorage.deleteTable(theStatement->sqlName);
        auto time = timer.elapsed(t1);
        ECE141::staticView theView;
        theView.setMessage("Query OK, "+ std::to_string(rows) + " rows affected " +"("+std::to_string(time)+" secs)");
        aViewer(theView);
        return theResult;
    }

    static ECE141::StatusResult handleShowCommand(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel){
        ECE141::StatusResult theResult;
        auto timer = ECE141::Config::getTimer();
        auto t1 = timer.now();
        auto list = theModel.currentDatabase->myStorage.getTables();
        auto time = timer.elapsed(t1);
        ECE141::FolderView theView;
        theView.setFolderTitle("Tables in " + theModel.currentDatabase->getName());
        for(const auto &table : list){
            theView.addRow(table);
        }
        theView.addSeparator();
        theView.addMessage(std::to_string(list.size()) + " rows in set " +
            dbMessages[ECE141::Keywords::show_kw]+"("+std::to_string(time)+" secs)");
        aViewer(theView);
        return theResult;
    }

    static ECE141::StatusResult handleDescribeCommand(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel){
        ECE141::StatusResult theResult;
        auto timer = ECE141::Config::getTimer();
        auto t1 = timer.now();

        std::shared_ptr<SQLStatement> theStatement = std::static_pointer_cast<SQLStatement>(aStatement);
        auto attributes = theModel.currentDatabase->myStorage.readSchema(theStatement->sqlName)->attributes;
        ECE141::Schema theSchema(theStatement->sqlName);
        theSchema.attributes=attributes;

        auto time = timer.elapsed(t1);
        ECE141::FolderView theView;

        std::vector<std::string> titles{"Field", "Type", "Null", "Key", "Default", "Extra"};
        theView.addCol(5);
        theView.setFolderTitle(titles);
        int rows = 0;
        for(const auto anAttribute : attributes){
            std::vector<std::string> columns;
            std::string type = typeStrings[anAttribute->getType()];
            std::string nullable; anAttribute->nullable ? nullable = "YES" : nullable = "NO";
            std::string isKey; anAttribute->primarykey ? isKey = "YES" : isKey = "";
            std::string theDefault = typeDefaults[anAttribute->getType()];
            std::string extra; anAttribute->primarykey&&anAttribute->autoinc ? extra = "auto_increment primary key" : extra = "";

            columns.push_back(anAttribute->getName());
            columns.push_back(type);
            columns.push_back(nullable);
            columns.push_back(isKey);
            columns.push_back(theDefault);
            columns.push_back(extra);

            theView.addRow(columns);
            rows += 1;
        }
        theView.addSeparator();
        theView.addMessage(std::to_string(attributes.size())+" rows in set "+dbMessages[ECE141::Keywords::describe_kw]+"("+std::to_string(time)+" secs)");
        aViewer(theView);
        return theResult;
    }

    static ECE141::StatusResult handleInsertCommand(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel) {
        ECE141::StatusResult theResult;
        std::shared_ptr<SQLStatement> theStatement = std::static_pointer_cast<SQLStatement>(aStatement);
        auto timer = ECE141::Config::getTimer();
        auto t1 = timer.now();

        ECE141::RowCollection rows;
        //by this point, type of argument and # of arguments should match
        if(theModel.currentDatabase->myStorage.schemaExists(theStatement->sqlName)){
            auto theSchema = theModel.currentDatabase->myStorage.readSchema(theStatement->sqlName);
            std::vector<ECE141::Value>::iterator iter=theStatement->values.begin();
            for(size_t i=0; i<theStatement->values.size()/theStatement->dest.size();i++){
                std::unique_ptr<ECE141::Row> theRow = std::make_unique<ECE141::Row>(0,theSchema);
                for(const auto &key:theStatement->dest){
                    theRow->getData()[key]=*iter;
                    iter++;
                }
                theRow->fillEmptyAttributes();
                theModel.currentDatabase->myStorage.setID(theRow.get(),*theSchema.get(),rows);
                rows.push_back(std::move(theRow));
            }
            if(theModel.currentDatabase->myStorage.rowsMatchSchema(rows,*theSchema.get())){
                for(auto &row : rows)
                    theModel.currentDatabase->myStorage.storeRow(row.get(), theStatement->sqlName);
            }
        }

        auto time = timer.elapsed(t1);
        ECE141::staticView theView;
        std::string theMsg = rows.size()==1 ? " row affected " : " rows affected ";
        theView.setMessage("Query OK, "+ std::to_string(rows.size()) + theMsg + "("
        +std::to_string(time)+" secs)");
        aViewer(theView);
        return theResult;
    }

    std::map<ECE141::Keywords, ECE141::StatusResult(*)(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel)> SQLLevelProcessing = {
            {ECE141::Keywords::create_kw, &SQLLevelProcessor::handleCreateCommand},
            {ECE141::Keywords::drop_kw, &SQLLevelProcessor::handleDropCommand},
            {ECE141::Keywords::show_kw, &SQLLevelProcessor::handleShowCommand},
            {ECE141::Keywords::describe_kw, &SQLLevelProcessor::handleDescribeCommand},
            {ECE141::Keywords::insert_kw, &SQLLevelProcessor::handleInsertCommand}
    };
};

struct SQLLevelParser : public CORParser {
    SQLLevelParser(std::shared_ptr<CORParser> nextHandler = nullptr) : CORParser(nextHandler) {}
    std::shared_ptr<Statement> parseStatement(ECE141::Tokenizer &aTokenizer, ECE141::Model& aModel) override {
        std::shared_ptr<Statement> aStatement = std::make_shared<SQLStatement>();
        ECE141::TokenSequencer theSequencer(aTokenizer, aStatement);
        auto theStatement = std::static_pointer_cast<SQLStatement>(aStatement);
        theStatement->myTokenizer=&aTokenizer;

        if(theSequencer.skipIf(ECE141::Keywords::create_kw).skipIf(ECE141::Keywords::table_kw)
        .captureIf(theStatement->sqlName).skipIf("(")
        .captureAttributes(theStatement->attributes, RawDataMode::createMode).isValid())
            theStatement->myKeyWord=ECE141::Keywords::create_kw;
        else if(theSequencer.skipIf(ECE141::Keywords::show_kw).skipIf(ECE141::Keywords::tables_kw)
        .isValid())
            theStatement->myKeyWord=ECE141::Keywords::show_kw;
        else if(theSequencer.skipIf(ECE141::Keywords::drop_kw).skipIf(ECE141::Keywords::table_kw)
        .captureIf(theStatement->sqlName).isValid())
            theStatement->myKeyWord=ECE141::Keywords::drop_kw;
        else if(theSequencer.skipIf(ECE141::Keywords::describe_kw)
        .captureIf(theStatement->sqlName).isValid())
            theStatement->myKeyWord=ECE141::Keywords::describe_kw;
        else if(theSequencer.skipIf(ECE141::Keywords::insert_kw).skipIf(ECE141::Keywords::into_kw)
                .captureIf(theStatement->sqlName).skipIf("(")
                .captureList(theStatement->dest, RawDataMode::insertMode).skipIf(ECE141::Keywords::values_kw)
                .skipIf("(").captureListValues(theStatement->values, RawDataMode::insertMode).isValid())
            theStatement->myKeyWord=ECE141::Keywords::insert_kw;
        else
            theStatement->error=ECE141::Errors::identifierExpected;
        return theStatement;
    }
    ECE141::StatusResult canHandle(ECE141::Tokenizer &aTokenizer) override {
        ECE141::StatusResult theResult;
        if(myProcessor.SQLLevelProcessing.find(aTokenizer.current().keyword)!=myProcessor.SQLLevelProcessing.end()){
            if(aTokenizer.current().keyword!=ECE141::Keywords::create_kw
            &&aTokenizer.current().keyword!=ECE141::Keywords::drop_kw
              &&aTokenizer.current().keyword!=ECE141::Keywords::show_kw)
            {return theResult;}
            if(aTokenizer.peek().keyword==ECE141::Keywords::table_kw&&aTokenizer.current().keyword!=ECE141::Keywords::show_kw
            ||aTokenizer.peek().keyword==ECE141::Keywords::tables_kw&&aTokenizer.current().keyword==ECE141::Keywords::show_kw)
            {return theResult;}
        }
        theResult.error=ECE141::Errors::unexpectedKeyword;
        return theResult;
    }
    ECE141::StatusResult handle(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel) override {
        if(aStatement->error==ECE141::Errors::noError)
        {logger->log(LogLevel::Info,"handler found");theModel.addTokens(*aStatement->myTokenizer);return myProcessor.process(aStatement,aViewer, theModel);}
        ECE141::StatusResult theBadResult;
        Processor::printErrorMessage(aViewer, errorMessages[aStatement->error]+"1");
        theBadResult.error=ECE141::Errors::unknownCommand;
        return theBadResult;
    }
private:
    SQLLevelProcessor myProcessor;
};

#endif //SQLHandler_h