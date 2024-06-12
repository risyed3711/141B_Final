#ifndef Statement_h
#define Statement_h

#include <iostream>
#include "misc/Errors.hpp"
#include "tokenizer/keywords.hpp"
#include "tokenizer/Tokenizer.hpp"
#include "database/Attribute.hpp"
#include "misc/ParseHelper.hpp"

    struct Statement : public std::enable_shared_from_this<Statement> {
        Statement() : error(ECE141::Errors::noError), isQuitting(false) {}
        ECE141::Keywords myKeyWord;
        bool isQuitting;
        std::string dbName;
        ECE141::Errors error;
    };
namespace ECE141{
struct ClauseHandler : public std::enable_shared_from_this<ClauseHandler>{
        ~ClauseHandler()=default;
        StatusResult parse(ECE141::Tokenizer &aTokenizer){
            StatusResult theResult;

            while(theResult && aTokenizer.more()){
                auto KWs=getKWList();
                if(std::find(KWs.begin(),KWs.end(),aTokenizer.current().keyword)!=KWs.end()){
                    theResult = parseClause(aTokenizer);
                }
                else
                    aTokenizer.next();
                if(theResult) {aTokenizer.skipIf(';');}
            }
            return theResult;
        }

        virtual KWList getKWList()=0;
        virtual StatusResult parseClause(Tokenizer& aTokenizer)=0;
    };
}

#endif //Statement_h