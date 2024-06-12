#ifndef TokenSequencer_hpp
#define TokenSequencer_hpp


#include <variant>
#include "Tokenizer.hpp"
#include "handler/Statement.hpp"
#include "database/Attribute.hpp"

enum class TokenLevel{
    dbLevel, sqlLevel
};

enum class RawDataMode{
    insertMode, createMode
};

namespace ECE141{
    class TokenSequencer{
    public:
        TokenSequencer(Tokenizer &aTokenizer, std::shared_ptr<Statement> aStatement) : myTokenizer(aTokenizer),
        valid{true}, myStatement(aStatement) {}

        TokenSequencer& skipIf(Keywords theKeyword){
            if(valid){
                if(myTokenizer.current().keyword!=theKeyword) valid=false;
                else myTokenizer.next();
            }
            return *this;
        }

        ///change str back to "(" if broken
        TokenSequencer& skipIf(std::string str){
            if(valid){
                if(toUpper(myTokenizer.current().data)!= toUpper(str)){
                    myStatement->error=ECE141::Errors::syntaxError;
                    valid = false;
                }
                else myTokenizer.next();
            }
            return *this;
        }

        TokenSequencer& captureList(std::vector<std::string>& aList, RawDataMode theMode){
            if(valid) {
                std::vector<std::vector<std::string>> rawData = getRawArguments(theMode);
                aList=rawData[0];
            }
            return *this;
        }

        TokenSequencer& captureListValues(std::vector<std::variant<int, std::string, float, bool>>& values, RawDataMode theMode){
            if(valid) {
                std::vector<std::vector<std::string>> rawData = getRawArguments(theMode);
                std::vector<std::variant<int,std::string,float,bool>> local;
                for (auto &x: rawData) {
                    for (auto &y: x){
                        std::variant<int, std::string, float, bool> aValue;
                        if (toUpper(y) == "TRUE")
                            aValue = true;
                        else if (toUpper(y) == "FALSE")
                            aValue = false;
                        else if (std::all_of(y.begin(), y.end(), ::isdigit))
                            aValue = std::stoi(y);
                        else if (isFloat(y))
                            aValue = std::stof(y);
                        else {
                            aValue=y;
                        }
                        local.push_back(aValue);
                    }
                }
                for(auto& i:local)
                    values.push_back(i);
            }
            return *this;
        }

        TokenSequencer& captureAttributes(std::vector<AttributeData>& attributes, RawDataMode theMode){
            if(valid){
                std::vector<std::vector<std::string>> theRawData = getRawArguments(theMode);
                auto temp = captureIntoAttributes(theRawData);
                for(auto i : temp){
                    attributes.push_back(i);
                }
            }
            return *this;
        }

        std::vector<AttributeData> captureIntoAttributes(std::vector<std::vector<std::string>> raw){
            std::vector<AttributeData> attributes;
            if(valid){
                size_t count;
                for(auto &x : raw){
                    AttributeData theAttributeData;
                    count=0;
                    if(isValidID(x[count]))
                        theAttributeData.name=x[count++];
                    if(isValidType(x[count]))
                        theAttributeData.type=ValidAttributeKeywords.at(toUpper(x[count++]));
                    while(count<x.size()){
                        if(x[count]=="(")
                        {theAttributeData.length=std::stoi(x[++count]); count++;}
                        else if(toUpper(x[count])=="AUTO_INCREMENT")
                            theAttributeData.autoinc=true;
                        else if(toUpper(x[count])=="PRIMARY"&&toUpper(x[count+1])=="KEY")
                        {theAttributeData.primarykey=true;count++;}
                        else if(toUpper(x[count])=="NOT"&&toUpper(x[count+1])=="NULL")
                        {theAttributeData.nullable=false;count++;}
                        else if(toUpper(x[count])=="NULL")
                            ; //to avoid incurring error if user decides to explicitly declare null
                        else
                        {myStatement->error=Errors::unexpectedIdentifier;valid=false;break;}
                        count++;
                    }
                    attributes.push_back(theAttributeData);
                }
            }
            return attributes;
        }

        //TokenSequencer& captureInto

        TokenSequencer& captureIf(std::string &theName){
            if(valid){
                if(isValidID(myTokenizer.current().data)){
                    theName=myTokenizer.current().data;
                    myTokenizer.next();
                }
                else {
                    myStatement->error=ECE141::Errors::identifierExpected;
                    valid=false;
                }
            }
            return *this;
        }

        TokenSequencer& captureNumber(size_t& theNum){
            if(valid){
                if(!std::isdigit(myTokenizer.current().data[0]))
                {valid=false;return*this;}
                theNum=std::stoi(myTokenizer.current().data);
                myTokenizer.next();
            }
            return*this;
        }

        std::vector<std::vector<std::string>> getRawArguments(RawDataMode theMode) {
            std::vector<std::vector<std::string>> theRawData;
            std::vector<std::string> temp;
            while (myTokenizer.more() && myTokenizer.current().data != ")"
                   && myTokenizer.current().data != ";") {
                while (myTokenizer.current().data!=","&&(myTokenizer.current().data!=")"
                &&myTokenizer.current().data!=";"||std::find(temp.begin(),temp.end(),"(")!=temp.end()
                &&std::find(temp.begin(),temp.end(),")")==temp.end())) {
                    temp.push_back(myTokenizer.current().data);
                    if (myTokenizer.current().data == ";" || !myTokenizer.next()) {
                        myStatement->error = Errors::identifierExpected;
                        valid = false;
                        break;
                    }
                }
                myTokenizer.next();
                if(RawDataMode::createMode==theMode) {
                    if(myTokenizer.peek(-1).data==";"){
                        myStatement->error = Errors::identifierExpected;
                        valid = false;
                        break;
                    }
                    theRawData.push_back(temp); temp.clear();
                }
                else if(RawDataMode::insertMode==theMode){
                    if(myTokenizer.current().keyword==Keywords::values_kw) {
                        theRawData.push_back(temp);temp.clear();
                        break;
                    }
                    else if(myTokenizer.current().data==","
                    &&myTokenizer.peek().data=="("){
                        myTokenizer.next(2);
                        theRawData.push_back(temp);temp.clear();
                    }
                    else if(myTokenizer.current().data==";"||myTokenizer.peek(-1).data==";")
                        theRawData.push_back(temp);
                    else if(myTokenizer.current().data=="(")
                    {theRawData.push_back(temp);temp.clear();myTokenizer.next();}
                }
            }
            return theRawData;
        }

        bool isValid(){
            bool old = valid;
            valid=true;
            return old;
        }
    private:
        std::string toUpper(std::string theString){
            for(char &c : theString){
                if(isalpha(c))
                    c=std::toupper(static_cast<unsigned char>(c));
            }
            return theString;
        }
        bool isValidID(std::string &theName){
            if(std::isdigit(theName[0]))
                return false;
            for(const char &c : theName){
                if(!(std::isalnum(c))&&c!='_')
                    return false;
            }
            return true;
        }
        bool isValidType(std::string &aName){
            if(ValidAttributeKeywords.find(toUpper(aName))==ValidAttributeKeywords.end()){
                myStatement->error=Errors::identifierExpected;
                valid=false;
                return false;
            }
            return true;
        }
        bool isFloat(std::string aStr){
            for(auto c : aStr){
                if(!std::isdigit(c)){
                    if(c != '.')
                        return false;
                }
            }
            return true;
        }

        Tokenizer &myTokenizer;
        bool valid;
        std::shared_ptr<Statement> myStatement;
    };
}

#endif //TokenSequencer_hpp