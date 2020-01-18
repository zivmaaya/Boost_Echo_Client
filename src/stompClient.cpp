#include <stdlib.h>
#include <connectionHandler.h>
#include "../include/bookClubClient.h"
#include <sstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>



void userInputProcess(std::vector<std::string>, bookClubClient *clientHandler);
std::vector<std::string> getUserInput();
void msgReceivedProcess(bookClubClient *clientHandler, ConnectionHandler *connectionHandler, std::vector<std::string> frame);
std::vector<std::string> bodyString (std::string body);
std::string getBookName(std::vector<std::string> bookVector,  int fIndex, int lIndex);

///----------------------------------------------------------------------------------------

class UserHandler{
private:
    std::mutex & _mutex;
    ConnectionHandler *_connectionHandler;
    bookClubClient *clientHandler;

public:
    UserHandler (std::mutex& mutex, ConnectionHandler &connectionHandler, bookClubClient &clientHandler)
    :  _mutex(mutex), _connectionHandler(&connectionHandler), clientHandler(&clientHandler) {}

    void run(){
        std::vector<std::string> userInput;
        userInput.push_back("");
        while (_connectionHandler->getConnectionStatus()&& userInput.at(0)!="logout"){
            userInput = getUserInput();
            std::lock_guard<std::mutex> lock(_mutex);
            userInputProcess(userInput, clientHandler);
        }
    }
};
///----------------------------------------------------------------------------------------

class ServerHandler{
private:
    std::mutex & _mutex;
    ConnectionHandler *_connectionHandler;
    bookClubClient *clientHandler;
public:
    ServerHandler (std::mutex& mutex, ConnectionHandler &connectionHandler, bookClubClient &clientHandler)
    :  _mutex(mutex), _connectionHandler(&connectionHandler), clientHandler(&clientHandler) {}

    void run(){
        while(_connectionHandler->getConnectionStatus()){
            std::vector<std::string> frame = _connectionHandler->getStompframe();
            if(frame.size()>0){
                std::lock_guard<std::mutex> lock(_mutex);
                msgReceivedProcess(clientHandler,_connectionHandler, frame);
            }

        }

    }
};



///----------------------------------------------------------------------------------------


int main (int argc, char *argv[]) {
    std::vector<std::string> initString = getUserInput();
    while(initString.at(0)!="login"){
        std::cout<<"Enter login details first"<<std::endl;
        initString=getUserInput();
    }
    std::string connectionData = initString.at(1);
    std::string host = connectionData.substr(0,connectionData.find(":"));
    short port = std::stoi(connectionData.substr(connectionData.find(":")+1));





    ConnectionHandler connectionHandler(host, port);
    bookClubClient clientHandler(connectionHandler);
    std::mutex mutex;
    UserHandler userHandler(mutex, connectionHandler,clientHandler);
    ServerHandler serverHandler(mutex, connectionHandler,clientHandler);

    if (!connectionHandler.connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }
    clientHandler.logIn(host, initString.at(2),initString.at(3));
    std::thread th1(&UserHandler::run, &userHandler);
    std::thread th2(&ServerHandler::run, &serverHandler);
    th1.join();
    th2.join();

    return 0;
}


///----------------------------------------------------------------------------------------

std::vector<std::string> getUserInput(){
    std::cout<<"enter your input: ";
    std::string userInput;
    getline(std::cin, userInput);
    std::string tmp;
    std::stringstream str(userInput);
    std::vector<std::string> splitUserInput;
    while (str >> tmp){
        splitUserInput.push_back(tmp);
    }
    return splitUserInput;
}

///----------------------------------------------------------------------------------------

void userInputProcess(std::vector<std::string> userInput,bookClubClient *clientHandler){
    if(userInput.at(0)=="join"){
        clientHandler->subscribe(userInput.at(1));
    }
    else if(userInput.at(0)=="exit"){
        clientHandler->unsubscribe(userInput.at(1));
    }
    else if(userInput.at(0)=="add"){
        clientHandler->addBook(getBookName(userInput,2,userInput.size()),userInput.at(1),true);
    }
    else if(userInput.at(0)=="borrow"){
        clientHandler->borrowBook(getBookName(userInput,2,userInput.size()),userInput.at(1));
    }
    else if (userInput.at(0)=="return"){
        clientHandler->returnBookIBorrowed(getBookName(userInput,2,userInput.size()),userInput.at(1));
    }
    else if (userInput.at(0)=="status"){
        clientHandler->getStatus(userInput.at(1));
    }
    else if(userInput.at(0)=="logout"){
        clientHandler->logOut();
    }
    else{
        std::cout<<"unValid input"<<std::endl;
    }
}


///----------------------------------------------------------------------------------------


void msgReceivedProcess(bookClubClient *clientHandler, ConnectionHandler *connectionHandler,std::vector<std::string> stompMessage){
//    if(connectionHandler->getConnectionStatus()){
//        std::vector<std::string> stompMessage = connectionHandler->getStompframe();
        if(stompMessage.at(0)=="MESSAGE"){
            std::string genre = stompMessage.at(3).substr(12);
            //-----print the message-----
            std::cout<<genre<<":"<<stompMessage.at(5)<<std::endl;
            std::vector<std::string> body = bodyString(stompMessage.at(5));
            if (body.size() > 1) {
                if (body.at(1) == "wish") {
                    clientHandler->isBookAvailable(getBookName(body,4,body.size()), genre);
                } else if (body.at(0) == "Taking" && body.at(body.size()-1) == clientHandler->getName()) {
                    clientHandler->lendBook(getBookName(body,1,body.size()-2), genre);
                } else if (body.at(1) == "has" && body.size() == 3) {
                    clientHandler->borrowingBookFrom(body.at(0), genre, getBookName(body,2,body.size()));
                } else if (body.at(0) == "Returning" && body.at(body.size()-1) == clientHandler->getName()) {
                    clientHandler->acceptBookILent(getBookName(body,1,body.size()-2), genre);
                } else if (body.at(1) == "status") {
                    clientHandler->sendStatus(genre);
                }
            }
        }
        else if(stompMessage.at(0)=="CONNECTED"){
            std::cout<<"Login successful"<<std::endl;
        }
        else if(stompMessage.at(0)=="RECEIPT"){
            std::string receiptId = stompMessage.at(1).substr(11);
            std::string receiptMessage = clientHandler->getReceiptMessage(receiptId);
            if (receiptMessage == "disconnect") {
                connectionHandler->close();
            }
            else {
                std::cout<<receiptMessage<<std::endl;
            }
        }
        else if (stompMessage.at(0)=="ERROR"){
            std::cout<<stompMessage.at(1).substr(9)<<std::endl;
            connectionHandler->close();
        }
        else{
            std::cout<<"unValid frame received"<<std::endl;
        }
    }



///----------------------------------------------------------------------------------------

std::vector<std::string> bodyString (std::string body){
    std::vector<std::string> splitBody;
    std::string temp;
    for(auto ch : body){
        if(ch == ' '){
            splitBody.push_back(temp);
            temp="";
        }
        else{
            temp+=ch;
        }
    }
    splitBody.push_back(temp);
    return splitBody;
}

std::string getBookName(std::vector<std::string> bookVector, int fIndex, int lIndex){
    std::string bookName;
    for (int i = fIndex; i < lIndex ; i++){
        bookName += bookVector.at(i)+ " ";
    }
    return bookName.substr(0,bookName.size()-1);
}
