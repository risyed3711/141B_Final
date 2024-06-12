#ifndef AppHandler_h
#define AppHandler_h

#include "Handler.hpp"


    static std::map<ECE141::Keywords,  std::string> appMessages{
            {ECE141::Keywords::help_kw,
                    "create database {name}: make a new database in storage folder\n"
                    "drop database {name}: delete a known database from storage folder\n"
                    "show databases: list databases in storage folder\n"
                    "use database {name}: load a known database for use\n"
                    "dump database {name}: dump information about every block in current database\n"
                    "create table {name} {identifiers}: create a new empty table in your current database\n"
                    "drop table {name}: drop existing table from current database\n"
                    "describe table {name}: describe schema representing information about specified table\n"
                    "select {identifier} from {name} {filters}: select data from table\n"
                    "insert {rows} into {name}: insert rows into table\n"
                    "update {name} set {filters}: update data within a table\n"
                    "delete from {name} {filters}: delete data from a table\n"
                    "show tables: show the tables within current database\n"
                    "about: show members\n"
                    "help: show list of commands\n"
                    "quit: stop app\n"
                    "version: show app version\n"
                    },
            {ECE141::Keywords::quit_kw,
                    "DB::141 is shutting down"
                    },
            {ECE141::Keywords::version_kw,
                    "Version: 10"},
            {ECE141::Keywords::about_kw,
                    "Authors: Rizwan and Ethan"}
};

    struct AppLevelProcessor : Processor {
        AppLevelProcessor() {}

        ECE141::StatusResult process(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel) override{
            logger->log(LogLevel::Info,"Processing");
            return AppLevelProcessing[aStatement->myKeyWord](aViewer);
        }

        static ECE141::StatusResult handleHelpCommand(ECE141::ViewListener aViewer){
            ECE141::StatusResult theResult;
            ECE141::staticView theView;
            theView.setMessage(appMessages[ECE141::Keywords::help_kw]);
            aViewer(theView);
            return theResult;
        }
        static ECE141::StatusResult handleQuitCommand(ECE141::ViewListener aViewer){
            ECE141::StatusResult theResult;
            ECE141::staticView theView;
            theView.setMessage(appMessages[ECE141::Keywords::quit_kw]);
            aViewer(theView);
            return theResult;
        }
        static ECE141::StatusResult handleVersionCommand(ECE141::ViewListener aViewer){
            ECE141::StatusResult theResult;
            ECE141::staticView theView;
            theView.setMessage(appMessages[ECE141::Keywords::version_kw]);
            aViewer(theView);
            return theResult;
        }
        static ECE141::StatusResult handleAboutCommand(ECE141::ViewListener aViewer){
            ECE141::StatusResult theResult;
            ECE141::staticView theView;
            theView.setMessage(appMessages[ECE141::Keywords::about_kw]);
            aViewer(theView);
            return theResult;
        }

        std::map<ECE141::Keywords, ECE141::StatusResult(*)(ECE141::ViewListener aViewer)> AppLevelProcessing = {
                {ECE141::Keywords::help_kw, &AppLevelProcessor::handleHelpCommand},
                {ECE141::Keywords::quit_kw, &AppLevelProcessor::handleQuitCommand},
                {ECE141::Keywords::version_kw, &AppLevelProcessor::handleVersionCommand},
                {ECE141::Keywords::about_kw, &AppLevelProcessor::handleAboutCommand},
        };
    };

    struct AppLevelParser : public CORParser {
        AppLevelParser(std::shared_ptr<CORParser> nextHandler = nullptr) : CORParser(nextHandler) {}
        std::shared_ptr<Statement> parseStatement(ECE141::Tokenizer &aTokenizer, ECE141::Model& aModel) override {
            std::shared_ptr<Statement> aStatement = std::make_shared<Statement>();
            ECE141::TokenSequencer theSequencer(aTokenizer, aStatement);
            aStatement->myTokenizer=&aTokenizer;

            if(theSequencer.skipIf(ECE141::Keywords::about_kw).isValid())
                aStatement->myKeyWord=ECE141::Keywords::about_kw;
            else if(theSequencer.skipIf(ECE141::Keywords::version_kw).isValid())
                aStatement->myKeyWord=ECE141::Keywords::version_kw;
            else if(theSequencer.skipIf(ECE141::Keywords::quit_kw).isValid()){
                aStatement->myKeyWord=ECE141::Keywords::quit_kw;
                aStatement->isQuitting=true;
            }
            else if(theSequencer.skipIf(ECE141::Keywords::help_kw).isValid())
                aStatement->myKeyWord=ECE141::Keywords::help_kw;
            return aStatement;
        }
        ECE141::StatusResult canHandle(ECE141::Tokenizer &aTokenizer) override {
            ECE141::StatusResult theResult;
            if(myProcessor.AppLevelProcessing.find(aTokenizer.current().keyword)!=myProcessor.AppLevelProcessing.end())
                return theResult;
            theResult.error=ECE141::Errors::unexpectedKeyword;
            return theResult;
        }
        ECE141::StatusResult handle(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel) override {
            if(aStatement->error==ECE141::Errors::noError)
            {logger->log(LogLevel::Info,"handler found");theModel.addTokens(*aStatement->myTokenizer);return myProcessor.process(aStatement,aViewer, theModel);}
            ECE141::StatusResult theBadResult;
            theBadResult.error=aStatement->error;
            return theBadResult;
        }
    private:
        AppLevelProcessor myProcessor;
    };


#endif // AppHandler_h