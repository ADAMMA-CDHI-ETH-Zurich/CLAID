#ifndef SYNC_UTIL_HH_
#define SYNC_UTIL_HH_

#include <list>
#include <memory>
#include <mutex>

namespace claid {

template <class T>
class SharedQueue {
  public:
    void push_front(std::shared_ptr<T> val);  // adds in the front 
    void push_back(std::shared_ptr<T> val); // adds at the end 
    std::shared_ptr<T> pop_front();  // returns null_ptr if empty 

  private:
    std::mutex cMutex; 
    std::list<std::shared_ptr<T>> container;  // stores the elements 
};

template <class T>
void SharedQueue<T>::push_front(std::shared_ptr<T> val) {
    std::lock_guard<std::mutex> lock(cMutex);
    container.push_front(val);
}

template <class T>
void SharedQueue<T>::push_back(std::shared_ptr<T> val) {
   std::lock_guard<std::mutex> lock(cMutex);
   container.push_back(val);
}

template <class T>
std::shared_ptr<T> SharedQueue<T>::pop_front() {
    std::lock_guard<std::mutex> lock(cMutex);
    if (!container.empty()) {
        std::shared_ptr<T> ret = container.front();
        container.pop_front();
        return ret; 
    }
    return std::shared_ptr<T>(nullptr); 
}

}  // namespace claid  

#endif  // SYNC_UTIL_HH
