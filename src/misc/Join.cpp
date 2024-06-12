#include <misc/Join.hpp>

namespace ECE141{

    Join::Join() {
        initMap();
    }

    StatusResult Join::applyLeftJoin(RowCollection& aCollection, std::vector<std::string> &aColumns){
        return leftAndRightJoin(aCollection,aColumns,leftOrRight::left);
    }

    StatusResult Join::applyRightJoin(RowCollection& aCollection, std::vector<std::string> &aColumns){
        return leftAndRightJoin(aCollection,aColumns,leftOrRight::right);
    }

    StatusResult Join::leftAndRightJoin(ECE141::RowCollection &aCollection, std::vector<std::string> &aColumns, leftOrRight type) {
        StatusResult theResult;
        auto primaryTableField = (type==leftOrRight::left) ? conditions.first: conditions.second;
        auto secondaryTableField = (primaryTableField==conditions.first) ? conditions.second : conditions.first;
        bool noMatch;
        RowCollection newCollection;
        for(auto& row : aCollection){
            noMatch=true;
            if(row->getSchema()->name==primaryTableField->table){
                if(!row->getSchema()->hasAttribute(primaryTableField->fieldName))
                {theResult.error=Errors::unexpectedIdentifier;return theResult;}
                for(auto& innerRow : aCollection){
                    if(innerRow->getSchema()->name==secondaryTableField->table){
                        if(!innerRow->getSchema()->hasAttribute(secondaryTableField->fieldName))
                        {theResult.error=Errors::unexpectedIdentifier;return theResult;}
                        if(matches(innerRow,row,primaryTableField,secondaryTableField))
                        {noMatch=false;auto tempCol=updateRow(aColumns,innerRow,row,noMatch);std::unique_ptr<Row> tempRow=std::make_unique<Row>(*row);newCollection.push_back(std::move(tempRow));
                            for(const auto& name : tempCol){row->getData().erase(name);}}
                    }
                }
                if(noMatch)
                {std::unique_ptr<Row> null(nullptr);updateRow(aColumns,null,row,noMatch);auto tempRow=std::make_unique<Row>(*row);newCollection.push_back(std::move(tempRow));}
            }
        }
        for(auto vit = aCollection.begin(); vit != aCollection.end(); vit++){
            vit->reset();
        }
        aCollection.clear();
        aCollection=std::move(newCollection);
        return theResult;
    }

    bool Join::matches(std::unique_ptr<Row>& innerRow, std::unique_ptr<Row>& outerRow, std::shared_ptr<TableField> primaryField,
                       std::shared_ptr<TableField> secondaryField) {
        if(outerRow->getData().find(primaryField->fieldName)!=outerRow->getData().end()){
            if(innerRow->getData().find(secondaryField->fieldName)!=innerRow->getData().end()){
                return outerRow->getData()[primaryField->fieldName]==innerRow->getData()[secondaryField->fieldName];
            }else{
                return std::get_if<int>(&outerRow->getData()[primaryField->fieldName])&&
                        *std::get_if<int>(&outerRow->getData()[primaryField->fieldName])==static_cast<int>(innerRow->getID());
            }
        }
        else{
            if(innerRow->getData().find(secondaryField->fieldName)!=innerRow->getData().end()){
                return std::get_if<int>(&innerRow->getData()[secondaryField->fieldName])&&
                       *std::get_if<int>(&innerRow->getData()[secondaryField->fieldName])==static_cast<int>(outerRow->getID());
            }else{
                return outerRow->getID()==innerRow->getID();
            }
        }
    }

    std::vector<std::string> Join::updateRow(std::vector<std::string> aColumns, std::unique_ptr<Row>& innerRow,
                         std::unique_ptr<Row>& outerRow, bool noMatch) {
        std::vector<std::string> columns;
        for(const auto& column : aColumns){
            if(outerRow->getData().count(column)==0){
                outerRow->getData()[column] = (noMatch) ? "NULL" : innerRow->getData()[column];
                columns.push_back(column);
            }
            else if(outerRow->getData().count(column)&&std::count(aColumns.begin(), aColumns.end(),column)>1){
                outerRow->getData()[column+"."+std::to_string(outerRow->getData().count(column)+1)] = (noMatch) ? "NULL" : innerRow->getData()[column];
                columns.push_back(column+"."+std::to_string(outerRow->getData().count(column)+1));
            }
        }
        return columns;
    }

    void Join::initMap() {
        joinMap={
                {Keywords::left_kw,&Join::applyLeftJoin},
                {Keywords::right_kw,&Join::applyRightJoin}
        };
    }

    StatusResult Join::apply(ECE141::RowCollection &aCollection, std::vector<std::string> &aColumns) {
        return (this->*joinMap[joinType])(aCollection, aColumns);
    }

}