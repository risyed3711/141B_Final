//
//  Helpers.hpp
//  Database3
//
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.
//


#ifndef Helpers_h
#define Helpers_h

#include "BasicTypes.hpp"
#include "tokenizer/keywords.hpp"
#include "database/Attribute.hpp"
#include <algorithm>
#include <utility>
#include "Errors.hpp"
#include <set>
#include <stdexcept>

namespace ECE141 {

    template<typename T>
    bool contains(std::initializer_list<T> list, T& value) {
        return std::find(list.begin(), list.end(), value) != list.end(); //What is the timesave here?
    }

    static inline bool checkLogical(const Operators& anOp) {
        return (anOp == Operators::and_op ||
                anOp == Operators::or_op ||
                anOp == Operators::notequal_op);
    }

    static inline bool checkOperator(const Operators& anOp) {
        return (anOp == Operators::equal_op ||
                anOp == Operators::gte_op ||
                anOp == Operators::lte_op ||
                anOp == Operators::lt_op ||
                anOp == Operators::gt_op ||
                anOp == Operators::notequal_op);
    }

    static bool isValidIdentifier(const std::string& anIdentifier) {
        if (anIdentifier.empty()) {
            return false;  // An identifier cannot be an empty string
        }

        if (!std::isalpha(anIdentifier[0]) && anIdentifier[0] != '_') {
            return false;  // First character must be a letter or underscore
        }

        for (size_t i = 1; i < anIdentifier.length(); i++) {
            if (!std::isalpha(anIdentifier[i]) && !std::isdigit(anIdentifier[i]) && anIdentifier[i] != '_') {
                return false;  // Subsequent characters: letters, digits, or underscores
            }
        }
        return true;
    };

    static bool isValidNumber(const std::string& aNumber) {
        int theValue;
        bool theResult = false;
        try {
            theValue = std::stoi(aNumber);
            if (theValue > 0) theResult = true;
        }
        catch (const std::invalid_argument &) {
            theResult = false; // Conversion failed
        }
        return theResult;
    }

    static std::map<std::string, Operators> gExpressionOps = {
            {"=",   Operators::equal_op},
            {"<",   Operators::lt_op},
            {"<=",  Operators::lte_op},
            {">",   Operators::gt_op},
            {">=",  Operators::gte_op},
            {"!=",  Operators::notequal_op},
            {"not", Operators::notequal_op},
    };

    static std::map<std::string, Operators> gOperators = {
            std::make_pair(".", Operators::dot_op),
            std::make_pair("+", Operators::add_op),
            std::make_pair("-", Operators::subtract_op),
            std::make_pair("*", Operators::multiply_op),
            std::make_pair("/", Operators::divide_op),
            std::make_pair("^", Operators::power_op),
            std::make_pair("%", Operators::mod_op),
            std::make_pair("=", Operators::equal_op),
            std::make_pair("!=", Operators::notequal_op),
            std::make_pair("not", Operators::notequal_op),
            std::make_pair("<", Operators::lt_op),
            std::make_pair("<=", Operators::lte_op),
            std::make_pair(">", Operators::gt_op),
            std::make_pair(">=", Operators::gte_op),
            std::make_pair("and", Operators::and_op),
            std::make_pair("or", Operators::or_op),
            std::make_pair("nor", Operators::nor_op),
            std::make_pair("between", Operators::between_op),
    };

    static ECE141::Keywords gJoinTypes[]={
            ECE141::Keywords::cross_kw,ECE141::Keywords::full_kw, ECE141::Keywords::inner_kw,
            ECE141::Keywords::left_kw, ECE141::Keywords::right_kw
    };

    //a list of known functions...
    static std::map<std::string,int> gFunctions = {
            std::make_pair("avg", 10),
            std::make_pair("count", 20),
            std::make_pair("max", 30),
            std::make_pair("min", 40)
    };


    //This map binds a keyword string with a Keyword (token)...
    static std::map<std::string,  Keywords> gDictionary = {
            std::make_pair("about",     Keywords::about_kw),
            std::make_pair("activate",  Keywords::activate_kw),
            std::make_pair("add",       Keywords::add_kw),
            std::make_pair("all",       Keywords::all_kw),
            std::make_pair("alter",     Keywords::alter_kw),
            std::make_pair("and",       Keywords::and_kw),
            std::make_pair("as",        Keywords::as_kw),
            std::make_pair("asc",       Keywords::asc_kw),
            std::make_pair("avg",       ECE141::Keywords::avg_kw),
            std::make_pair("auto_increment", Keywords::auto_increment_kw),
            std::make_pair("backup", Keywords::backup_kw),
            std::make_pair("between",   ECE141::Keywords::between_kw),
            std::make_pair("boolean",   ECE141::Keywords::boolean_kw),
            std::make_pair("by",        ECE141::Keywords::by_kw),
            std::make_pair("change",    ECE141::Keywords::change_kw),
            std::make_pair("changed",    ECE141::Keywords::changed_kw),
            std::make_pair("char",      ECE141::Keywords::char_kw),
            std::make_pair("column",    ECE141::Keywords::column_kw),
            std::make_pair("count",     ECE141::Keywords::count_kw),
            std::make_pair("create",    ECE141::Keywords::create_kw),
            std::make_pair("cross",     ECE141::Keywords::cross_kw),
            std::make_pair("current_date",  Keywords::current_date_kw),
            std::make_pair("current_time",  Keywords::current_time_kw),
            std::make_pair("current_timestamp", Keywords::current_timestamp_kw),
            std::make_pair("database",  ECE141::Keywords::database_kw),
            std::make_pair("databases", ECE141::Keywords::databases_kw),
            std::make_pair("datetime",  ECE141::Keywords::datetime_kw),
            std::make_pair("decimal",   ECE141::Keywords::decimal_kw),
            std::make_pair("desc",      ECE141::Keywords::desc_kw),
            std::make_pair("delete",    ECE141::Keywords::delete_kw),
            std::make_pair("describe",  ECE141::Keywords::describe_kw),
            std::make_pair("default",   ECE141::Keywords::default_kw),
            std::make_pair("distinct",  ECE141::Keywords::distinct_kw),
            std::make_pair("double",    ECE141::Keywords::double_kw),
            std::make_pair("drop",      ECE141::Keywords::drop_kw),
            std::make_pair("dump",      ECE141::Keywords::dump_kw),
            std::make_pair("enum",      ECE141::Keywords::enum_kw),
            std::make_pair("error",     ECE141::Keywords::error_kw),
            std::make_pair("explain",   ECE141::Keywords::explain_kw),
            std::make_pair("extension", ECE141::Keywords::extension_kw),
            std::make_pair("false",     ECE141::Keywords::false_kw),
            std::make_pair("float",     ECE141::Keywords::float_kw),
            std::make_pair("foreign",   ECE141::Keywords::foreign_kw),
            std::make_pair("from",      ECE141::Keywords::from_kw),
            std::make_pair("full",      ECE141::Keywords::full_kw),
            std::make_pair("group",     ECE141::Keywords::group_kw),
            std::make_pair("help",      ECE141::Keywords::help_kw),
            std::make_pair("in",        ECE141::Keywords::in_kw),
            std::make_pair("index",     ECE141::Keywords::index_kw),
            std::make_pair("indexes",   ECE141::Keywords::indexes_kw),
            std::make_pair("inner",     ECE141::Keywords::inner_kw),
            std::make_pair("insert",    ECE141::Keywords::insert_kw),
            std::make_pair("int",       ECE141::Keywords::integer_kw),
            std::make_pair("integer",   ECE141::Keywords::integer_kw),
            std::make_pair("into",      ECE141::Keywords::into_kw),
            std::make_pair("join",      ECE141::Keywords::join_kw),
            std::make_pair("key",       ECE141::Keywords::key_kw),
            std::make_pair("last",      ECE141::Keywords::last_kw),
            std::make_pair("left",      ECE141::Keywords::left_kw),
            std::make_pair("like",      ECE141::Keywords::like_kw),
            std::make_pair("limit",     ECE141::Keywords::limit_kw),
            std::make_pair("max",       ECE141::Keywords::max_kw),
            std::make_pair("min",       ECE141::Keywords::min_kw),
            std::make_pair("modify",    ECE141::Keywords::modify_kw),
            std::make_pair("not",       ECE141::Keywords::not_kw),
            std::make_pair("null",      ECE141::Keywords::null_kw),
            std::make_pair("on",        ECE141::Keywords::on_kw),
            std::make_pair("or",        ECE141::Keywords::or_kw),
            std::make_pair("order",     ECE141::Keywords::order_kw),
            std::make_pair("outer",     ECE141::Keywords::outer_kw),
            std::make_pair("primary",   ECE141::Keywords::primary_kw),
            std::make_pair("query",     ECE141::Keywords::query_kw),
            std::make_pair("quit",      ECE141::Keywords::quit_kw),
            std::make_pair("references",ECE141::Keywords::references_kw),
            std::make_pair("restore",   ECE141::Keywords::restore_kw),
            std::make_pair("right",     ECE141::Keywords::right_kw),
            std::make_pair("rows",      ECE141::Keywords::rows_kw),
            std::make_pair("run",       ECE141::Keywords::run_kw),
            std::make_pair("select",    ECE141::Keywords::select_kw),
            std::make_pair("self",      ECE141::Keywords::self_kw),
            std::make_pair("set",       ECE141::Keywords::set_kw),
            std::make_pair("show",      ECE141::Keywords::show_kw),
            std::make_pair("success",   ECE141::Keywords::success_kw),
            std::make_pair("sum",       ECE141::Keywords::sum_kw),
            std::make_pair("table",     ECE141::Keywords::table_kw),
            std::make_pair("tables",    ECE141::Keywords::tables_kw),
            std::make_pair("timestamp", ECE141::Keywords::timestamp_kw),
            std::make_pair("true",      ECE141::Keywords::true_kw),
            std::make_pair("unique",    ECE141::Keywords::unique_kw),
            std::make_pair("update",    ECE141::Keywords::update_kw),
            std::make_pair("use",       ECE141::Keywords::use_kw),
            std::make_pair("values",    ECE141::Keywords::values_kw),
            std::make_pair("varchar",   ECE141::Keywords::varchar_kw),
            std::make_pair("version",   ECE141::Keywords::version_kw),
            std::make_pair("where",     ECE141::Keywords::where_kw)
    };

    class Helpers {
    public:

        static Keywords getKeywordId(const std::string aKeyword) {
            auto theIter = gDictionary.find(aKeyword);
            if (theIter != gDictionary.end()) {
                return theIter->second;
            }
            return Keywords::unknown_kw;
        }

        //convert from char to keyword...
        static Keywords charToKeyword(char aChar) {
            switch(toupper(aChar)) {
                case 'I': return Keywords::integer_kw;
                case 'T': return Keywords::datetime_kw;
                case 'B': return Keywords::boolean_kw;
                case 'F': return Keywords::float_kw;
                case 'V': return Keywords::varchar_kw;
                default:  return Keywords::unknown_kw;
            }
        }

        static std::string dataTypeToString(DataTypes aType) {
            switch(aType) {
                case DataTypes::no_type:        return "none";
                case DataTypes::bool_type:      return "boolean";
                case DataTypes::timestamp_type: return "date";
                case DataTypes::float_type:     return "float";
                case DataTypes::int_type:       return "integer";
                case DataTypes::varchar_type:   return "varchar";
                default:                        return "none";
            }
        }

        static std::string boolToString(bool aBool) {
            if (aBool) {
                return "YES";
            } else {
                return "NO";
            }
        }

        static std::string boolToStringBlank(bool aBool) {
            if (aBool) {
                return "YES";
            } else return " ";
        }

        static const char* keywordToString(Keywords aType) {
            switch(aType) {
                case Keywords::boolean_kw:    return "bool";
                case Keywords::create_kw:     return "create";
                case Keywords::database_kw:   return "database";
                case Keywords::databases_kw:  return "databases";
                case Keywords::datetime_kw:   return "datetime";
                case Keywords::describe_kw:   return "describe";
                case Keywords::drop_kw:       return "drop";
                case Keywords::float_kw:      return "float";
                case Keywords::integer_kw:    return "integer";
                case Keywords::show_kw:       return "show";
                case Keywords::table_kw:      return "table";
                case Keywords::tables_kw:     return "tables";
                case Keywords::use_kw:        return "use";
                case Keywords::varchar_kw:    return "varchar";
                default:                      return "unknown";
            }
        }
        //New method used by error view class
        static const char* errorToString(Errors anError) {
            switch (anError) {
                case Errors::noError: return "No Error";

                    //Parsing related errors
                case Errors::keywordExpected: return "Keyword expected";
                case Errors::identifierExpected: return "Identifier expected";
                case Errors::illegalIdentifier: return "Illegal identifier";
                case Errors::unknownIdentifier: return "Unknown identifier";
                case Errors::unexpectedIdentifier: return "Unexpected identifier";
                case Errors::joinTypeExpected: return "Join type expected";
                case Errors::keyExpected: return "Key expected";
                case Errors::syntaxError: return "Syntax error";
                case Errors::unexpectedKeyword: return "Unexpected keyword";
                case Errors::unexpectedValue: return "Unexpected value";
                case Errors::valueExpected: return "Value expected";
                case Errors::operatorExpected: return "Operator expected";
                case Errors::punctuationExpected: return "Punctuation expected";
                case Errors::eofError: return "End of file error";
                    // Statement related errors
                case Errors::statementExpected: return "Statement expected";
                case Errors::noDatabaseSpecified: return "No database specified";

                    // Table/DB related errors
                case Errors::tableExists: return "Table already exists";
                case Errors::unknownEntity: return "Unknown entity";
                case Errors::unknownTable: return "Unknown table";
                case Errors::unknownDatabase: return "Unknown database";
                case Errors::databaseExists: return "Database already exists";
                case Errors::databaseCreationError: return "Error creating database";
                case Errors::primaryKeyRequired: return "Primary key required";
                case Errors::invalidTableName: return "Invalid table name";
                    // Type related errors
                case Errors::unknownType: return "Unknown type";
                case Errors::unknownAttribute: return "Unknown attribute";
                case Errors::invalidAttribute: return "Invalid attribute";
                case Errors::invalidArguments: return "Invalid arguments";
                case Errors::keyValueMismatch: return "Key/value mismatch";

                    // Storage/IO related errors
                case Errors::readError: return "Read error";
                case Errors::writeError: return "Write error";
                case Errors::seekError: return "Seek error";
                case Errors::storageFull: return "Storage full";

                    // Index related errors
                case Errors::indexExists: return "Index already exists";
                case Errors::cantCreateIndex: return "Cannot create index";
                case Errors::unknownIndex: return "Unknown index";

                    // Command related errors
                case Errors::unknownCommand: return "Unknown command";
                case Errors::invalidCommand: return "Invalid command";

                    // General purpose errors
                case Errors::userTerminated: return "User terminated";
                case Errors::notImplemented: return "Not implemented";

                    // Default case for unknown errors
                case Errors::unknownError: return "Unknown error";
                default: return "Unknown error code";
            }
        }


        // USE: ---validate that given keyword is a datatype...
        static bool isDatatype(Keywords aKeyword) {
            switch(aKeyword) {
                case Keywords::char_kw:
                case Keywords::datetime_kw:
                case Keywords::float_kw:
                case Keywords::integer_kw:
                case Keywords::varchar_kw:
                    return true;
                default: return false;
            }
        }

        static Operators toOperator(std::string aString) {
            auto theIter = gOperators.find(aString);
            if (theIter != gOperators.end()) {
                return theIter->second;
            }
            return Operators::unknown_op;
        }

        static int getFunctionId(const std::string anIdentifier) {
            auto theIter = gFunctions.find(anIdentifier);
            if (theIter != gFunctions.end()) {
                return theIter->second;
            }
            return 0;
        }

        static bool isNumericKeyword(Keywords aKeyword) {
            static Keywords theTypes[]={Keywords::decimal_kw, Keywords::double_kw, Keywords::float_kw, Keywords::integer_kw};
            for(auto k : theTypes) {
                if(aKeyword==k) return true;
            }
            return false;
        }

        static const DataTypes getTypeForKeyword(Keywords aKeyword) {
            switch (aKeyword) {
                case Keywords::boolean_kw:
                    return DataTypes::bool_type;
                case Keywords::datetime_kw:
                    return DataTypes::datetime_type;
                case Keywords::float_kw:
                    return DataTypes::float_type;
                case Keywords::integer_kw:
                    return DataTypes::int_type;
                case Keywords::varchar_kw:
                    return DataTypes::varchar_type;
                default:
                    return DataTypes::no_type;
            }
        }

        template<typename T, size_t aSize>
        static bool in_array(T (&anArray)[aSize], const T &aValue) {
            return std::find(std::begin(anArray), std::end(anArray), aValue) != std::end(anArray);
        };
    };
}
#endif /* Helpers_h */