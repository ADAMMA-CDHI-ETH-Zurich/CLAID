#include "RecursiveReflector.hpp"
#include "RunTime/RunTime.hpp"
namespace portaible
{
    class SimpleReflector : public RecursiveReflector<SimpleReflector>
    {
        public:
            template<typename T>
            void callFloatOrDouble(const char* property, T& member)
            {
                printf("I am float, val is %f\n", member);

                RunTime runtime;
                printf("And runtime val is %d\n",     portaible::RunTime::getInstance()->getVal());
            }   

            // Also includes any variants of signed, unsigned, short, long, long long, ...
            template<typename T>
            void callInt(const char* property, T& member)
            {
                printf("I am int, val is <%s>%d</%s>\n", property, member, property);
            }

            void callBool(const char* property, bool& member)
            {
                printf("I am int, val is <%s>%d</%s>\n", property, member, property);
            }

            template<typename T>
            void callString(const char* property, T& member)
            {
                printf("I am string, val is %s %s\n", member.c_str(), property);
            }

            template<typename T>
            void callBeginClass(const char* property, T& member)
            {

            }

            template<typename T>
            void callEndClass(const char* property, T& member)
            {

            }
            
    };
}