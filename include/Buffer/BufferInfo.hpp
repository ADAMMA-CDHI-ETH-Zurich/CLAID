#pragma once
#include "CLAID.hpp"
#include "BufferDataType.hpp"

#ifdef __PYTHON_WRAPPERS__

#endif

namespace claid
{
    struct BufferInfo
    {       
        //DECLARE_SERIALIZATION(BufferInfo)

        private:
            std::vector<int32_t> shape;
            std::vector<int32_t> strides;
            BufferDataType bufferDataType;

            std::vector<int32_t> calculateStrides() const
            {
                // Strides: how much you need to add to get to next element of same dimension.
                // E.g., if 2D matrix and rowMajor, if you are in the first row, you need to add numColums elements
                // to reach second row.
                std::vector<int32_t> strides;

                for(int32_t i = 0; i < this->shape.size(); i++)
                {
                    // Start at i + 1, because stride always is based on the *next* dimension.
                    // E.g., the stride for dimension 0 is based on all subsequent dimensions.
                    // Take the matrix example: The stride (see definition above) for the row dimension
                    // is based on the columns.
                    // To calculate stride for one dimension, multiply shape of all subsequent dimensions.
                    int32_t stride = 1;
                    for(int32_t j = i + 1; j < this->shape.size(); j++)
                    {
                        stride *= this->shape[j];
                    }
                    strides.push_back(stride);
                }
                return strides;
            }

        public:

            BufferInfo()
            {

            }

            BufferInfo(BufferDataType bufferDataType, std::vector<int32_t> shape) : bufferDataType(bufferDataType), shape(shape)
            {
                this->strides = calculateStrides();
            }

            Reflect(BufferInfo,
                reflectMember(shape);
                reflectMember(strides);
                reflectMember(bufferDataType);
            )

       


            const std::vector<int32_t>& getShape() const
            {
                return this->shape;
            }

            const std::vector<int32_t>& getStrides() const
            {
                return this->strides;
            }

            int32_t numberOfDimensions() const
            {
                return this->shape.size();
            }

            int32_t getNumberOfBytes() const
            {
                int32_t numBytes = sizeOfOneElementInBytes();

                for(const int32_t& value : this->shape)
                {
                    numBytes *= value;
                }

                return numBytes;
            }

            std::vector<int32_t> stridesInBytes() const
            {
                std::vector<int32_t> tmp = this->strides;

                for(int32_t& value : tmp)
                {
                    value *= sizeOfOneElementInBytes();
                }

                return tmp;
            }

            int32_t sizeOfOneElementInBytes() const
            {
                switch(this->bufferDataType)
                {
                    case BufferDataType::UINT8:
                    {
                        return sizeof(uint8_t);
                    }
                    break;
                    case BufferDataType::INT8:
                    {
                        return sizeof(int8_t);
                    }
                    break;
                    case BufferDataType::UINT16:
                    {
                        return sizeof(uint16_t);
                    }
                    break;
                    case BufferDataType::INT16:
                    {
                        return sizeof(int16_t);
                    }
                    break;
                    case BufferDataType::UINT32:
                    {
                        return sizeof(uint32_t);
                    }
                    break;
                    case BufferDataType::INT32:
                    {
                        return sizeof(int32_t);
                    }
                    break;
                    case BufferDataType::UINT64:
                    {
                        return sizeof(uint64_t);
                    }
                    break;
                    case BufferDataType::INT64:
                    {
                        return sizeof(int64_t);
                    }
                    break;
                    case BufferDataType::FLOAT:
                    {
                        return sizeof(float);
                    }
                    break;
                    case BufferDataType::DOUBLE:
                    {
                        return sizeof(double);
                    }
                    break;  
                }
                CLAID_THROW(Exception, "Invalid data type in BufferInfo. Enum value out of bounds.");
  
            }

       #ifdef __PYTHON_WRAPPERS__
   
            virtual py::buffer_info getPyBufferInfo(void* data) const
            {
                py::buffer_info bufferInfo;
                std::vector<int32_t> byteStrides = stridesInBytes();
          
                bufferInfo.ptr = data;
                bufferInfo.itemsize = sizeOfOneElementInBytes();
                bufferInfo.format = bufferDataTypeToPyFormat();
                bufferInfo.ndim = this->numberOfDimensions();
                bufferInfo.shape = vectorToPyVector(this->shape);
                bufferInfo.strides = vectorToPyVector(byteStrides);

                return bufferInfo;
            }

            virtual std::string bufferDataTypeToPyFormat() const
            {
                switch(this->bufferDataType)
                {
                    case BufferDataType::UINT8:
                    {
                        return py::format_descriptor<uint8_t>::format();
                    }
                    break;
                    case BufferDataType::INT8:
                    {
                        return py::format_descriptor<int8_t>::format();
                    }
                    break;
                    case BufferDataType::UINT16:
                    {
                        return py::format_descriptor<uint16_t>::format();
                    }
                    break;
                    case BufferDataType::INT16:
                    {
                        return py::format_descriptor<int16_t>::format();
                    }
                    break;
                    case BufferDataType::UINT32:
                    {
                        return py::format_descriptor<uint32_t>::format();
                    }
                    break;
                    case BufferDataType::INT32:
                    {
                        return py::format_descriptor<int32_t>::format();
                    }
                    break;
                    case BufferDataType::UINT64:
                    {
                        return py::format_descriptor<uint64_t>::format();
                    }
                    break;
                    case BufferDataType::INT64:
                    {
                        return py::format_descriptor<int64_t>::format();
                    }
                    break;
                    case BufferDataType::FLOAT:
                    {
                        return py::format_descriptor<float>::format();
                    }
                    break;
                    case BufferDataType::DOUBLE:
                    {
                        return py::format_descriptor<double>::format();
                    }
                    break;  
                }
            }

            void fromPyBufferInfo(py::buffer_info& pyBufferInfo)
            {
                this->bufferDataType = this->findCorrespondingDataTypeFromPyBufferInfo(pyBufferInfo);
                this->shape = this->pyVectorToVector(pyBufferInfo.shape);
                this->strides = calculateStrides();

                if(pyBufferInfo.itemsize != sizeOfOneElementInBytes())
                {
                    CLAID_THROW(Exception, "Error in BufferInfo. Failed to connvert py::buffer_info to BufferInfo.\n"
                    << "The size of one element in the buffer is different between the two.\n"
                    << "In the py::buffer_info, the element size is " << pyBufferInfo.itemsize << ", but for BufferInfo it is " << sizeOfOneElementInBytes() << ".");
                }

                if(pyBufferInfo.ndim != this->numberOfDimensions())
                {
                    CLAID_THROW(Exception, "Error in BufferInfo. Failed to connvert py::buffer_info to BufferInfo.\n"
                    << "The number of dimensions is different between the two.\n"
                    << "In the py::buffer_info, the number of dimensions is " << pyBufferInfo.ndim << ", but for BufferInfo it is " << numberOfDimensions() << ".");
                }

                if(pyBufferInfo.strides.size() != this->strides.size())
                {
                    CLAID_THROW(Exception, "Error in BufferInfo. Failed to connvert py::buffer_info to BufferInfo.\n"
                    << "The number of strides is different between the two.\n"
                    << "In the py::buffer_info, the number of strides is " << pyBufferInfo.strides.size() << ", but for BufferInfo it is " << this->strides.size() << ".");
                }

                std::vector<int32_t> byteStrides = stridesInBytes();
                for(int32_t i = 0; i < pyBufferInfo.strides.size(); i++)
                {
                    if(pyBufferInfo.strides[i] != byteStrides[i])
                    {
                         CLAID_THROW(Exception, "Error in BufferInfo. Failed to connvert py::buffer_info to BufferInfo.\n"
                        << "The number strides for the " << i << "th dimension does not match between the two.\n"
                        << "In the py::buffer_info, the stride for dimension " << i << " is " << pyBufferInfo.strides[i] << ", but for the BufferInfo it is " << byteStrides[i] << ".");
                    }
                }
            }


       #endif

        private:

        #ifdef __PYTHON_WRAPPERS__
            std::vector<py::ssize_t> vectorToPyVector(const std::vector<int32_t>& vector) const
            {
                std::vector<py::ssize_t> pyVector;

                for(const int32_t& value : vector)
                {
                    pyVector.push_back(value);
                }
                return pyVector;
            }

            std::vector<int32_t> pyVectorToVector(const std::vector<py::ssize_t>& pyVector) const
            {
                std::vector<int32_t> vector;

                for(const py::ssize_t& value : pyVector)
                {
                    vector.push_back(value);
                }
                return vector;
            }

            BufferDataType findCorrespondingDataTypeFromPyBufferInfo(py::buffer_info& bufferInfo)
            {
                std::map<std::string, BufferDataType> formatToDataTypeMap;
                formatToDataTypeMap.insert(std::make_pair(py::format_descriptor<uint8_t>::format(), UINT8));
                formatToDataTypeMap.insert(std::make_pair(py::format_descriptor<int8_t>::format(), INT8));
                formatToDataTypeMap.insert(std::make_pair(py::format_descriptor<uint16_t>::format(), UINT16));
                formatToDataTypeMap.insert(std::make_pair(py::format_descriptor<int16_t>::format(), INT16));
                formatToDataTypeMap.insert(std::make_pair(py::format_descriptor<uint32_t>::format(), UINT32));
                formatToDataTypeMap.insert(std::make_pair(py::format_descriptor<int32_t>::format(), INT32));
                formatToDataTypeMap.insert(std::make_pair(py::format_descriptor<uint64_t>::format(), UINT64));
                formatToDataTypeMap.insert(std::make_pair(py::format_descriptor<int64_t>::format(), INT64));
                formatToDataTypeMap.insert(std::make_pair(py::format_descriptor<float>::format(), FLOAT));
                formatToDataTypeMap.insert(std::make_pair(py::format_descriptor<double>::format(), DOUBLE));

                auto it = formatToDataTypeMap.find(bufferInfo.format);

                if(it == formatToDataTypeMap.end())
                {
                    CLAID_THROW(Exception, "Error in BufferInfo. Cannot convert py::buffer_info to BufferInfo.\n"
                    << "Unable to find corresponding BufferDataType for py::buffer_info.format \"" << bufferInfo.format << "\"");
                }
                return it->second;


                // std::map<BufferDataType, std::string> dataTypeToFormatMap;
                // dataTypeToFormatMap.insert(std::make_pair(UINT8, py::format_descriptor<uint8_t>::format()));
                // dataTypeToFormatMap.insert(std::make_pair(INT8, py::format_descriptor<int8_t>::format()));
                // dataTypeToFormatMap.insert(std::make_pair(UINT16, py::format_descriptor<uint16_t>::format()));
                // dataTypeToFormatMap.insert(std::make_pair(INT16, py::format_descriptor<int16_t>::format()));
                // dataTypeToFormatMap.insert(std::make_pair(UINT32, py::format_descriptor<uint32_t>::format()));
                // dataTypeToFormatMap.insert(std::make_pair(INT32, py::format_descriptor<int32_t>::format()));
                // dataTypeToFormatMap.insert(std::make_pair(UINT64, py::format_descriptor<uint64_t>::format()));
                // dataTypeToFormatMap.insert(std::make_pair(INT64, py::format_descriptor<int64_t>::format()));
                // dataTypeToFormatMap.insert(std::make_pair(FLOAT, py::format_descriptor<float>::format()));
                // dataTypeToFormatMap.insert(std::make_pair(DOUBLE, py::format_descriptor<double>::format()));


            }
        #endif
    };
}