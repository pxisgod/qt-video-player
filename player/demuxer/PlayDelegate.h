#ifndef PLAY_DELEGATE_H
#define PLAY_DELEGATE_H
#include <list>
#include <memory>
class PlayDelegate{
public:
    void play_0();
    void pause_0();
    void seek_0(long position);
    void addDeleagte(std::shared_ptr<PlayDelegate> delegate);
    virtual void play()=0;
    virtual void pause()=0;
    virtual void seek(long position)=0;
private:
    std::list<std::weak_ptr<PlayDelegate>> m_DelegateList;
};
#endif