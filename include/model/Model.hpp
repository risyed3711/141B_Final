#ifndef Model_hpp
#define Model_hpp

#include <iostream>
#include "database/Database.hpp"
#include "filesystem"

namespace ECE141{
    struct Model{
        Model() : currentDatabase{nullptr} {}

        bool doesExist(std::string& dbName){
            return std::filesystem::exists(Config::getDBPath(dbName));
        }

        std::shared_ptr<Database> setDB(std::string& dbName){
            if(doesExist(dbName))
                currentDatabase=std::make_shared<Database>(dbName,OpenFile{});
            else
                currentDatabase= nullptr;
            return currentDatabase;
        }

        std::shared_ptr<Database> dropDB(std::string& dbName){
            std::filesystem::remove(Config::getDBPath(dbName));
            if(currentDatabase&&currentDatabase->getName()==dbName)
                currentDatabase= nullptr;
            return currentDatabase;
        }

        std::vector<std::string> getDBList(){
            std::filesystem::path thePath = ECE141::Config::getStoragePath();
            std::vector<std::string> ans;
            std::string tempStr;
            for(const auto &db : std::filesystem::directory_iterator(thePath)){
                if(db.path().string().find(".db")!=std::string::npos) {
                    tempStr=db.path().string();
                    tempStr.erase(0,5);
                    tempStr.erase(tempStr.size() - 3);
                    ans.push_back(tempStr);
                }
            }
            return ans;
        }

        void addTokens(ECE141::Tokenizer& aTokenizer){
            auto toks = aTokenizer.getTokens();
            for(const auto& tok:toks){
                allTokens.push_back(tok);
            }
        }

        std::shared_ptr<Database> currentDatabase;
        std::vector<ECE141::Token> allTokens;
    };
}

#endif //Model_hpp