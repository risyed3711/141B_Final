#ifndef Storable_h
#define Storable_h

#include "misc/Errors.hpp"
#include "storage/BlockIO.hpp"
#include <iostream>

namespace ECE141{
    class Storable {
    public:
        virtual ~Storable() {}
        virtual StatusResult  encode(std::ostream &anOutput) const=0;
        virtual StatusResult  decode(std::istream &anInput) =0;
        virtual bool          initHeader(Block &aBlock) const=0;
    };
}


#endif //Storable_hpp