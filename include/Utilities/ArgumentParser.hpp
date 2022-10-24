
#include <string>
#include <vector>

class ArgumentParser
{
    private:
        int argc;
        char** argv;
    
        int findArgumentIndex(const char* argument);

        bool invalidArgumentFound;

        std::vector<std::string> arguments;
        std::vector<std::string> descriptions;


    public:
        ArgumentParser(int argc, char** argv);

        void printHelpIfInvalidArgumentFound();

        template<typename T> void add_argument(std::string argument, T& var, T defaultValue, std::string description);
        template<typename T> void add_argument(std::string argument, T& var, T defaultValue)
        {
            add_argument(argument, var, defaultValue, "");
        }

   
};