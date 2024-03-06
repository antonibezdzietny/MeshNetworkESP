// messageQueue.hpp - FIFO QUEUE
#ifndef MESSAGE_QUEUE_HPP
#define MESSAGE_QUEUE_HPP

#include "Arduino.h"

class MessageQueue
{
    // Class of FIFO queue for message in string type
private:
    struct messageItem
    {
        String message;
        messageItem * nextItem;
    };
    unsigned int maxMessage_;
    bool overwrite_;
    int itemsNumber_;
    messageItem * firstItem_;
    messageItem * lastItem_;

    void addMessage( String msg ){
        messageItem * adrMsg = new messageItem;
        adrMsg -> message  = msg;
        adrMsg -> nextItem = nullptr;

        if( isEmpty() ){
            firstItem_ = adrMsg;
            lastItem_  = adrMsg;
        }else{
            lastItem_ -> nextItem = adrMsg;
            lastItem_ = adrMsg;
        }

        ++itemsNumber_; 
    }

    void rewriteMessage( String msg ){
        firstItem_ -> message  = msg;
        lastItem_  -> nextItem = firstItem_;
        firstItem_ = firstItem_ -> nextItem;
        lastItem_ = lastItem_ -> nextItem;
        lastItem_ ->nextItem =  nullptr;
    }  


public:
    MessageQueue( unsigned int maxMessage = 10, bool overwrite = false ){
        maxMessage_  = maxMessage;
        overwrite_   = overwrite; 
        itemsNumber_ = 0;
        firstItem_   = nullptr;
        lastItem_    = nullptr;
    }

    ~MessageQueue(){}

    bool push( String & msg ){
        if( isFull() ){ // If queue is full
            if( overwrite_ == false ) // If queue is full overwrite is not available
                return false;

            rewriteMessage( msg ); // If queue is full overwrite is available
            return true;
        }

        addMessage( msg ); 
        return true;  
    }

    String front(){
        if( isEmpty() )
            return "";
        return firstItem_ -> message;
    }

    bool pop(){
        if( isEmpty() )
            return false;
        
        if( itemsNumber_ == 1 ){
            delete firstItem_;
            firstItem_ = nullptr;
            lastItem_  = nullptr;
        }else{
            messageItem * item = firstItem_ -> nextItem;
            delete firstItem_;
            firstItem_ = item;
        }
        --itemsNumber_;   
        return true;
    }

    bool isEmpty(){
        return itemsNumber_ == 0;
    }

    bool isFull(){
        return itemsNumber_ >= maxMessage_;
    }

    int getItemNumber(){
        return itemsNumber_;
    }
};
#endif