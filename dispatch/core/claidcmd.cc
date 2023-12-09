#include "dispatch/core/CLAID.hh"
// Include the CLAID ArgumentParser
#include "dispatch/core/Utilities/ArgumentParser.hh"
// Change main to inlcude argc and argv parameters.
int main(int argc, char** argv) 
{
    ArgumentParser parser(argc, argv);

    std::string configPath = "";
    // Register argument to the parser.
    // This automatically parses argv.
    // A default value can be specified by the last parameter of the function.
    parser.add_argument<std::string>("path", configPath, "");

    // If "path" was not specified in argv, configPath is empty.
    // In that case, we print an error message.
    if(configPath == "")
    {
        printf("Error, no path to JSON configuration specified.\n"
        "Please use ./claidcmd path path/to/json/file.\n");
        exit(0);
    }

    const char* socket_path = "/tmp/test_grpc.sock";
    const char* config_file = configPath.c_str();
    const char* host_id = "test_server";
    const char* user_id = "user42";
    const char* device_id = "something_else";

    claid::CLAID claid;
    bool result = claid.start(socket_path, config_file, host_id, user_id, device_id);
    while(true);
    return 0;
}
