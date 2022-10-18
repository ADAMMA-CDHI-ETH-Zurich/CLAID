namespace claid
{
    class PropertyNode
    {
        virtual void set(void* data) = 0;
        virtual void* get() = 0;
    };
}