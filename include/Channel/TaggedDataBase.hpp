#pragma once
#include "Utilities/Time.hpp"
#include <string>
#include <vector>
namespace claid 
{

    class TaggedDataBase
    {
    public:

        TaggedDataBase() : sequenceID(0), timestamp(Time::min()) {}


        TaggedDataBase(const Time& timestamp, uint64_t sequenceID = 0) :
            timestamp(timestamp),
            sequenceID(sequenceID) 
        {
        }


    public:

        template<typename Reflector>
        void reflect(Reflector& r)
        {
            r.member("Timestamp",  timestamp, "The timestamp", Time::now());
            r.template member<uint64_t>("SequenceID", sequenceID, "A user defined sequence ID", 0);
        }

    public:

        Time timestamp;
        uint64_t sequenceID;

        // Can hold additional tags to store meta information about the data.
        // Is only used internally (hence, not added in reflection).
        // An example use case would be the remote capabilities of CLAID.
        // When data is received from a remotely connected RunTime, it is tagged in order to prevent loops.
        // (I.e., when a Channel has subscribers both locally and remotely and data is received from the remote RunTime, 
        // a loop might happen as the RemoteObserver would think the data was posted locally).)
        std::vector<std::string> tags;
        
        void addTag(const std::string& tag)
        {
            this->tags.push_back(tag);
        }

        bool hasTag(const std::string& tag) const
        {
            return std::find(this->tags.begin(), this->tags.end(), tag) != this->tags.end();
        }

    };
}