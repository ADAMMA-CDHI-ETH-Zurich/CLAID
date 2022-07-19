#pragma once

#include <thread>

#include "ITCChannel.hpp"
#include "SmartPointerDataVector.hpp"
#include "SocketClient.hpp"

namespace Socket
{
	class Network
	{
		private:
			ITCChannel<std::pair<SocketClient*, SmartPointerDataVector<char>>>* dataChannel;
			bool active = false;
			bool blocking = false;

			std::thread* thread = nullptr;


			SocketClient* socketClient;

			void run();

		public:
			ITCChannel<std::pair<SocketClient*, SmartPointerDataVector<char>>>* getDataChannel();
			void start(bool blocking = false);

			SocketClientReaderWorker(ITCChannel<std::pair<SocketClient*, SmartPointerDataVector<char>>>* dataChannel, SocketClient* socketClient);
			~SocketClientReaderWorker();
	};
}
