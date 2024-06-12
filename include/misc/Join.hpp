#ifndef Join_hpp
#define Join_hpp

#include <iostream>
#include "Helpers.hpp"
#include "database/Attribute.hpp"
#include "functional"
#include "tokenizer/TokenSequencer.hpp"
#include "database/Row.hpp"

namespace ECE141{

    enum class leftOrRight{
        left,right
    };

    struct Join{
        Join();
        StatusResult apply(RowCollection& aCollection,std::vector<std::string>& aColumns);
        Keywords joinType;
        std::pair<std::shared_ptr<TableField>,std::shared_ptr<TableField>> conditions;
    protected:
        StatusResult applyLeftJoin(RowCollection& aCollection, std::vector<std::string> &aColumns);
        StatusResult applyRightJoin(RowCollection& aCollection, std::vector<std::string> &aColumns);
        StatusResult leftAndRightJoin(RowCollection& aCollection, std::vector<std::string> &aColumns, leftOrRight type);
        std::vector<std::string> updateRow(std::vector<std::string> aColumns, std::unique_ptr<Row>& innerRow, std::unique_ptr<Row>& outerRow, bool noMatch);
        bool matches(std::unique_ptr<Row>& innerRow, std::unique_ptr<Row>& outerRow, std::shared_ptr<TableField> primaryField, std::shared_ptr<TableField> secondaryField);


        typedef StatusResult (Join::*FuncPtr)(RowCollection&,std::vector<std::string>&);
        std::map<Keywords,FuncPtr> joinMap;
        void initMap();
    };
}


#endif //Join_hpp