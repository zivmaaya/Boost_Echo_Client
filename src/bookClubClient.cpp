//
// Created by zivmaaya@wincs.cs.bgu.ac.il on 14/01/2020.
//

#include "../include/bookClubClient.h"
#include "../include/connectionHandler.h"
#include <vector>

bookClubClient::bookClubClient(ConnectionHandler &connectionHandler_) : connectionHandler(&connectionHandler_) {}

void bookClubClient::addBook(std::string bookName, std::string genre, bool sendFrame) {
    if(myBooks.find(genre) != myBooks.end()){
        myBooks.find(genre)->second.push_back(bookName);
    } else{
        std::vector<std::string> t;
        t.push_back(bookName);
        myBooks.insert({genre,t});
    }
    if(sendFrame){
        std::vector<std::string> stompFrame;
        stompFrame.push_back("SEND");
        stompFrame.push_back("destination:" + genre);
        stompFrame.push_back("");
        stompFrame.push_back(this->clientName+" has added the book " + bookName);
        connectionHandler->sendStompFrame(stompFrame);
    }
}

void bookClubClient::subscribe(std::string genre) {
    subscribeId.insert({genre,subscribeCounter});
    std::vector<std::string> stompFrame;
    stompFrame.push_back("SUBSCRIBE");
    stompFrame.push_back("destination:"+genre);
    stompFrame.push_back("id:"+std::to_string(subscribeCounter));
    stompFrame.push_back("receipt:"+std::to_string(receipt));
    stompFrame.push_back("\n");
    receiptTopic.insert({std::to_string(receipt),genre});
    subscribeCounter++;
    receipt++;
    connectionHandler->sendStompFrame(stompFrame);
}

void bookClubClient::unsubscribe(std::string genre) {
    std::vector<std::string> stompFrame;
    stompFrame.push_back("UNSUBSCRIBE");
    stompFrame.push_back("destination:"+genre);
    int id = subscribeId.find(genre)->second;
    stompFrame.push_back("id:"+std::to_string(id));
    stompFrame.push_back("receipt:"+std::to_string(receipt));
    stompFrame.push_back("\n");
    connectionHandler->sendStompFrame(stompFrame);
    subscribeId.erase(genre);
    receiptTopic.insert({std::to_string(receipt),genre});
    receipt++;
}

void bookClubClient::borrowBook(std::string bookName, std::string genre) {
    waitingList.push_back(bookName);
    std::vector<std::string> stompFrame;
    stompFrame.push_back("SEND");
    stompFrame.push_back("destination:"+genre);
    stompFrame.push_back("");
    stompFrame.push_back(clientName+" wish to borrow " + bookName);
    connectionHandler->sendStompFrame(stompFrame);
}

void bookClubClient::borrowingBookFrom(std::string lender, std::string genre, std::string bookName) {
    auto index = std::find(waitingList.begin(),waitingList.end(),bookName);
    if (index != waitingList.end()){
        addBook(bookName,genre, false);
        booksIBorrowed.insert({bookName,lender});
        waitingList.erase(index);
        std::vector<std::string> stompFrame;
        stompFrame.push_back("SEND");
        stompFrame.push_back("destination:"+genre);
        stompFrame.push_back("");
        stompFrame.push_back("Taking " + bookName + " from " + lender);
        connectionHandler->sendStompFrame(stompFrame);
    }

}
void bookClubClient::returnBookIBorrowed(std::string bookName, std::string genre) {
    std::vector<std::string> v = myBooks.find(genre)->second;
    auto index = std::find(v.begin(),v.end(),bookName);
    if(index != v.end()) {
        myBooks.find(genre)->second.erase(index);
        std::vector<std::string> stompFrame;
        std::string lender = booksIBorrowed.find(bookName)->second;
        stompFrame.push_back("SEND");
        stompFrame.push_back("destination:" + genre);
        stompFrame.push_back("");
        stompFrame.push_back("Returning " + bookName + " to " + lender);
        connectionHandler->sendStompFrame(stompFrame);
        booksIBorrowed.erase(bookName);
    }
}

void bookClubClient::sendStatus(std::string genre) {
    std::vector<std::string> stompFrame;
    stompFrame.push_back("SEND");
    stompFrame.push_back("destination:" + genre);
    std::string myBooksList = clientName+":";
    int i=0;
    for(std::string book : myBooks.find(genre)->second){
        if(i==0){
            myBooksList = myBooksList + book;
        } else{
            myBooksList = myBooksList +", "+ book;
        }
        i++;
    }
    stompFrame.push_back("");
    stompFrame.push_back(myBooksList);
    connectionHandler->sendStompFrame(stompFrame);
}

void bookClubClient::logIn(std::string userName, std::string password) {
    clientName = userName;
    clientPassword = password;
    std::vector<std::string> stompFrame;
    stompFrame.push_back("CONNECT");
    stompFrame.push_back("accept-version:1.2");
    stompFrame.push_back("host:stomp.cs.bgu.ac.il");//TODO: host should be server machine address
    stompFrame.push_back("login:"+userName);
    stompFrame.push_back("passcode:"+password);
    stompFrame.push_back("\n");
    connectionHandler->sendStompFrame(stompFrame);
}

void bookClubClient::logOut() {
    std::vector<std::string> stompFrame;
    stompFrame.push_back("DISCONNECT");
    stompFrame.push_back("receipt:"+receipt);
    stompFrame.push_back("\n");
    receiptTopic.insert({std::to_string(receipt),"disconnect"});
    receipt++;
    connectionHandler->sendStompFrame(stompFrame);
    //connectionHandler->close();//TODO: wait for receipt
}

void bookClubClient::acceptBookILent(std::string bookName, std::string genre) {
    addBook(bookName,genre,false);
    auto index = std::find(booksILent.begin(),booksILent.end(),bookName);
    if (index != booksILent.end()){
        booksILent.erase(index);
    }
}

void bookClubClient::lendBook(std::string bookName, std::string genre) {
    std::vector<std::string> v = myBooks.find(genre)->second;
    auto index = std::find(v.begin(),v.end(),bookName);
    if(index != v.end()) {
        myBooks.find(genre)->second.erase(index);
        booksILent.push_back(bookName);
    }
}

void bookClubClient::isBookAvailable(std::string bookName, std::string genre) {
    std::vector<std::string> v = myBooks.find(genre)->second;
    auto it = std::find(v.begin(),v.end(),bookName);
    if(it != v.end()) {
        std::vector<std::string> stompFrame;
        stompFrame.push_back("SEND");
        stompFrame.push_back("destination:" + genre);
        stompFrame.push_back("");
        stompFrame.push_back(clientName+" has " + bookName);
        connectionHandler->sendStompFrame(stompFrame);
    }
}

void bookClubClient::getStatus(std::string genre) {
    std::vector<std::string> stompFrame;
    stompFrame.push_back("SEND");
    stompFrame.push_back("destination:" + genre);
    stompFrame.push_back("");
    stompFrame.push_back("book status");
    connectionHandler->sendStompFrame(stompFrame);
}

std::string bookClubClient::getName() {
    return clientName;
}

std::string bookClubClient::getReceiptMessage(std::string receipt) {
    std::string topic = receiptTopic.find(receipt)->second;//TODO: think if need to delete receipt after
    if (topic == "disconnect"){
        return topic;
    }
    else if (subscribeId.find(topic) != subscribeId.end()){
        return ("Joined club "+topic);
    }
    else {
        return ("Exited club "+topic);
    }
}




