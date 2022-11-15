#pragma once 

#include <string>
#include <fstream>
#include <streambuf>

#include "Serialization/Serialization.hpp"


namespace claid
{
    class File
    {
        DECLARE_SERIALIZATION(File)

        protected:
            std::string path;

        private:
            std::string content;

        public:

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                r.member("Path",  path, "Path");
            }

            File();
            File(std::string path);
            virtual ~File() {}

            virtual bool load();

            void setContent(const std::string& content);

            bool saveTo(const std::string& path);
            bool loadFrom(const std::string& path);

            virtual const std::string& getPath();

            virtual const std::string& getContent() const;
    };
}