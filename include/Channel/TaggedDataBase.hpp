#pragma once
#include "Utilities/Time.hpp"
#include <string>
namespace portaible 
{

    class TaggedDataBase
    {
    public:

        TaggedDataBase() : sequenceID(0), timestamp(Time::min()) {}


        TaggedDataBase(const Time& timestamp, uint32_t sequenceID = 0) :
            timestamp(timestamp),
            sequenceID(sequenceID) 
        {
                
        }


    public:

        template<typename Reflector>
        void reflect(Reflector& r)
        {
            r.member("Timestamp",  timestamp, "The timestamp", Time::now());
            r.member("SequenceID", sequenceID, "A user defined sequence ID", 0);
        }

    public:

        Time timestamp;
        uint32_t sequenceID;
    };
}