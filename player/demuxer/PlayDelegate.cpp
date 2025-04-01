#include "PlayDelegate.h"
void PlayDelegate::play_0(){
    for(auto delegate:m_DelegateList){
        if(auto delegate_ptr=delegate.lock()){
            delegate_ptr->play();
        }
    }
}
void PlayDelegate::pause_0(){
    for(auto delegate:m_DelegateList){
        if(auto delegate_ptr=delegate.lock()){
            delegate_ptr->pause();
        }
    }
}
void PlayDelegate::seek_0(long position){
    for(auto delegate:m_DelegateList){
        if(auto delegate_ptr=delegate.lock()){
            delegate_ptr->seek(position);
        }
    }
}
void PlayDelegate::addDeleagte(std::shared_ptr<PlayDelegate> delegate){
    m_DelegateList.push_back(delegate);
}