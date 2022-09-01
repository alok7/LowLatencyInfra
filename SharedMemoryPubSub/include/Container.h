#pragma once
#include <variant>
#include <SHM.h>

namespace SHM 
{

template<class... Ts> struct visitors : Ts... { using Ts::operator()...; };
template<class... Ts> visitors(Ts...) -> visitors<Ts...>;


template<typename ...CONTENTS>
class SHMTransport : public Store<std::variant<CONTENTS...>>
{
  private: 
    using RECORD_TYPE = std::variant<CONTENTS...> ;
    using Base = Store<RECORD_TYPE> ;
  public:
    SHMTransport(std::string _name, Mode _mode, size_t recordCount_=1048576) : Store<std::variant<CONTENTS...>>(_name, _mode, recordCount_), mName(_name)
    {
      mReaderPos = 0; 
    }
    ~SHMTransport()
    {
      mStop = true;
    }
    template<typename CONTENT_TYPE>
    CONTENT_TYPE& allocate()
    {
      auto& slot = Base::allocate();
      CONTENT_TYPE dummy;
      auto content = new(&slot) RECORD_TYPE(dummy);
      return std::get<CONTENT_TYPE>(*content);
    }
    template<typename CONTENT_TYPE>
    void allocate(const CONTENT_TYPE& content) noexcept 
    {
      auto& master = masterNode();
      SpinLock lock(master);
      auto& slot = Base::allocate();
      auto record = new(&slot) RECORD_TYPE(content);
      this->finalize();
    }
    template<typename CONTENT_TYPE>
    CONTENT_TYPE& read(size_t pos)
    {
      auto& record = Base::read(pos);
      return std::get<CONTENT_TYPE>(record);
    }
    inline MasterRecord& masterNode()
    {
      return Base::masterNode();
    }
    template<typename... Fs>
    void readAsync(Fs... fs)
    {
      auto& master = masterNode();
      auto visitor = visitors<Fs...>{fs...};
      while(!mStop)
      {      
        if(mReaderPos != master.mLatestFinalizedPos.load(std::memory_order_acquire))
        {
          SpinLock lock(master);
	  size_t current = mReaderPos;
          mReaderPos  = this->increment(current);
          auto& record = Base::read(current);
          try
          {
            std::visit(visitor, record);
          }
          catch(...) // shouldn't ever happen
          {
            std::cerr << "bad variant read error " << std::endl;
            mReaderPos = this->increment(mReaderPos);
          }
        }
      }
    }
    void setReaderPosition()
    {
      auto& master = masterNode();
      mReaderPos = master.mLatestFinalizedPos.load();
      std::cout << "[INFO]: SHMReader " << mName << " startReadPos " << mReaderPos << std::endl;
    }
    void setReaderPosition(size_t pos)
    {
      mReaderPos = pos;
      std::cout << "[INFO]: SHMReader " << mName << " startReadPos " << mReaderPos << std::endl;
    }
    void stop()
    {
      mStop = true;
    }
    void readSnapShot( bool enable)
    {
      auto& master = masterNode();
      master.isSnapShotPublished.store(enable);
    }
    private:
      size_t mReaderPos;   
      bool mStop = false; 
      std::string mName;
  };

}
