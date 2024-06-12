#ifndef Handler_h
#define Handler_h

#include <iostream>
#include <memory>
#include "tokenizer/TokenSequencer.hpp"
#include "tokenizer/keywords.hpp"
#include "view/View.hpp"
#include "database/Database.hpp"
#include "model/Model.hpp"
#include "Statement.hpp"
#include "misc/Config.hpp"
#include "misc/Logger.hpp"

    static std::map<ECE141::Errors, std::string> errorMessages {
            {ECE141::Errors::identifierExpected,
             "Error 101: Identifier expected at line "},
            {ECE141::Errors::unknownCommand,
             "Error 3000: Unknown command at line "}
    };

    struct CORParser : public std::enable_shared_from_this<CORParser> {

        CORParser(std::shared_ptr<CORParser> nextHandler = nullptr) : next(nextHandler)
        {logger=&Logger::getInstance();}

        virtual ~CORParser() {}

        virtual std::shared_ptr<CORParser> findHandler(ECE141::Tokenizer &aTokenizer){
            if(canHandle(aTokenizer)){
                return shared_from_this();
            }
            else if(next && next->findHandler(aTokenizer)){
                return next->findHandler(aTokenizer);
            }
            return nullptr;
        }

        virtual std::shared_ptr<Statement> parseStatement(ECE141::Tokenizer &aTokenizer, ECE141::Model& theModel)=0;
        virtual ECE141::StatusResult canHandle(ECE141::Tokenizer &aTokenizer)=0;
        virtual ECE141::StatusResult handle(std::shared_ptr<Statement> aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel)=0;

        std::shared_ptr<CORParser> next;
        Logger* logger;
    };

    struct Processor{
        Processor() {logger=&Logger::getInstance();}
        virtual ~Processor() {}
        virtual ECE141::StatusResult process(std::shared_ptr<Statement >aStatement, ECE141::ViewListener& aViewer, ECE141::Model &theModel)=0;
        static void printErrorMessage(ECE141::ViewListener &aViewer, std::string theMsg){
            ECE141::staticView theView;
            theView.setMessage(theMsg);
            aViewer(theView);
        }
        Logger* logger;
    };

#endif // Handler_h