#pragma once
#include <variant>
#include <memory>
#include <thread>
#include <Container.h>

namespace SHM
{	

template<typename... UpdateChanelEventTypes>
class Publisher
{
  public: 
    using UPDATE_CHANEL   = SHMTransport<UpdateChanelEventTypes...>;
  public:
    Publisher(const std::string& name, size_t capacity) 
    {
      mUpdateChanel.reset(new UPDATE_CHANEL(name+"_update_", Mode::CREATE_RW, capacity)); 
    }
    ~Publisher()
    {
      stop();
    }
    void stop()
    {
      mUpdateChanel->stop();
    }	    
    template<typename EVENT>
    void publish( const EVENT& event)
    {
      mUpdateChanel->allocate(event);
    }
  private:
    std::unique_ptr<UPDATE_CHANEL> mUpdateChanel;
};


}
