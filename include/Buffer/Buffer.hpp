#pragma once 
#include "CLAID.hpp"

#include "BufferInfo.hpp"
#include "Reflection/SplitReflectInType.hpp"



namespace claid
{
    struct Buffer
    {
        //DECLARE_SERIALIZATION(Buffer)
        private:
            std::shared_ptr<char> data;
            BufferInfo bufferInfo;

        public:

            Buffer() : bufferInfo(BufferDataType::UINT8, {0})
            {
            }

            Buffer(std::shared_ptr<char> data, BufferDataType dataType, std::vector<int32_t> shape) : data(data)
            {
                this->bufferInfo = BufferInfo(dataType, shape);
            }

            Buffer(std::shared_ptr<char>& data, BufferInfo bufferInfo): data(data), bufferInfo(bufferInfo)
            {

            }

            SplitReflect()

            ReflectRead(Buffer,
                reflector.member("BufferInfo", bufferInfo, "");
                int32_t bytes = bufferInfo.getNumberOfBytes();

                this->allocateNew(bytes);
                
                char* ptr = this->getData();
                reflector.read(ptr, bytes);
            )

            ReflectWrite(Buffer,
                reflector.member("BufferInfo", bufferInfo, "");
                int32_t bytes = this->getNumberOfBytes();

                char* ptr = this->getData();
                reflector.write(reinterpret_cast<const char*>(ptr), bytes);
            )

            #ifdef __JAVA_WRAPPERS__
                // Override reflect function above if the Reflector is JbindWrapperGenerator.
                // This allows us to create a custom Java wrapper for this data type.
                CustomReflectForReflector(Buffer, JavaWrapper::JbindWrapperGenerator<Buffer>,

                )
            #endif

            #ifdef __PYTHON_WRAPPERS__

            // Override reflect function above if the Reflector is PybindWrapperGenerator.
            // This allows us to create a custom python wrapper for this data type.
            CustomReflectForReflector(Buffer, PythonWrapper::PybindWrapperGenerator<Buffer>,

                reflector.initPyClassIfNecessary(py::buffer_protocol());
                PythonWrapper::PybindWrapperGenerator<Buffer>::PyClass* pyClass;
                pyClass = reflector.getPyClass();
                pyClass->def_buffer([](Buffer &m) -> py::buffer_info {
                    BufferInfo info = m.getBufferInfo();
                    

                    py::buffer_info pyBufferInfo = info.getPyBufferInfo(reinterpret_cast<void*>(m.getData()));
                    Logger::logInfo("Getting buffer %s", pyBufferInfo.format.c_str());
                    return pyBufferInfo;
                });

                pyClass->def(py::init([](py::buffer b) {
                    /* Request a buffer descriptor from Python */
                    py::buffer_info info = b.request();

                    BufferInfo bufferInfo;
                    bufferInfo.fromPyBufferInfo(info);

                    int32_t bytes = bufferInfo.getNumberOfBytes();
                    std::shared_ptr<char> dataCopy(new char[bytes]);

                    memcpy(dataCopy.get(), info.ptr, bytes);
                    
                    return Buffer(dataCopy, bufferInfo);
                }));


                reflectMember(bufferInfo);
            )
        #endif

            const BufferInfo& getBufferInfo()
            {
                return this->bufferInfo;
            }

            void allocateNew(const int32_t bytes)
            {
                this->data = std::shared_ptr<char>(new char[bytes]);
            }

            int32_t size() const;
            void setData();

            int32_t getNumberOfBytes() const
            {
                return this->bufferInfo.getNumberOfBytes();
            }

            char* getData()
            {
                return this->data.get();
            }




    };
}