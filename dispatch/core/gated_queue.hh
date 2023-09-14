#ifndef SYNC_UTIL_HH_
#define SYNC_UTIL_HH_


#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>

namespace claid {

class CommonGate {
	private:
		std::mutex m;
		std::condition_variable cv;

	friend class GatedQueue; 
};

template<class T>
class GatedQueue
{
	private:
		std::list<std::shared_ptr<T>> queue;
		shared_ptr<SharedGate> gate;
	public:
		GatedQueue(shared_ptr<CommonGate> gate) : gate(gate) {}

		GatedQueue() : gate(make_shared<CommonGate>()) {}

		shared_ptr<CommonGate> gate() { return gate; }

        void push_front(std::shared_ptr<T> val)  // adds in the front 
        {
            std::unique_lock<std::mutex> lock(gate->m);
			queue.push_front(val);
			gate->cv.notify_one();
        }

        void push_back(std::shared_ptr<T> val) // adds at the end 
        {
            std::unique_lock<std::mutex> lock(gate->m);
			queue.push_back(val);
			gate->cv.notify_one();
        }
        
        std::shared_ptr<T> pop_front() 
        {
            std::unique_lock<std::mutex> lock(gate->m);
			
            // Have to account for spurious wakeups?
			cv.wait(lock, [&]() { return !queue.empty(); });

			std::shared_ptr<T> out = queue.front();
			queue.pop_front();

            return out;
        }

		size_t size()
		{
			std::unique_lock<std::mutex> lock(m);
			return queue.size();
		}
};




}  // namespace claid  

#endif  // SYNC_UTIL_HH
