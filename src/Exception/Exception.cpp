
#include "Exception/Exception.hpp"

namespace portaible {


const char* Exception::what() const noexcept
{
	const std::size_t INDENT = 4;
	const char* INDENT_STR = "    ";

	mMessage = "";

	// obtain max width
	std::size_t maxMessageSize = 0;
	for(std::list<Info>::const_iterator i = mInfos.begin();
		i!=mInfos.end(); ++i)
	{
		std::size_t msgSize = i->message.size();

		if(i!=mInfos.begin())
			msgSize+=INDENT;

		if(msgSize > maxMessageSize)
			maxMessageSize = msgSize;
	}

	// append all informations with correct indentation
	for(std::list<Info>::const_iterator i = mInfos.begin();
		i!=mInfos.end(); ++i)
	{
		if(i!=mInfos.begin()) {
			mMessage += std::string("\n") + INDENT_STR;
			mMessage += i->what(maxMessageSize-INDENT);
		} else
			mMessage += i->what(maxMessageSize);

	}

	return mMessage.c_str();
}

std::string Exception::message() const noexcept
{
	std::string s;
	// append all informations with correct indentation
	for(std::list<Info>::const_iterator i = mInfos.begin(); i!=mInfos.end(); ++i)
	{
		if(i!=mInfos.begin())
			s += ", " + i->message;
		else
			s = i->message;
	}

	return s;
}


std::string Exception::Info::what(std::size_t messageWidth) const
{
	std::string s = message;

	// fill with spaces
	for(std::size_t i=message.size(); i<messageWidth; ++i)
		s += " ";

	if(!file.empty())
    {
        std::stringstream ss;
		ss << " (" << file << ":" << line << ")";
        s += ss.str();
    }
	return s;
}


}
