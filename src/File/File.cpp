#include "File/File.hpp"


namespace portaible
{
    File::File()
    {

    }

    File::File(std::string path) : path(path)
    {
        
    }

    bool File::load()
    {
        return this->loadFrom(this->path);
    }

    void File::setContent(const std::string& content)
    {
        this->content = content;
    }

    bool File::saveTo(const std::string& path)
    {
        std::ofstream file(path);

        if(!file.is_open())
        {
            return false;
        }


        file << this->content;
        return true;
    }

    bool File::loadFrom(const std::string& path)
    {
        this->path = path;
        std::ifstream file(path);

        if(!file.is_open())
        {
            return false;
        }

        this->content = std::string ((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());


        return true;
    }

    const std::string& File::getPath()
    {
        return this->path;
    }


    const std::string& File::getContent() const
    {
        return this->content;
    }
}

REGISTER_SERIALIZATION(portaible::File)
