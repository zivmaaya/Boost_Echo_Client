//
// Created by zivmaaya@wincs.cs.bgu.ac.il on 14/01/2020.
//

#ifndef BOOST_ECHO_CLIENT_BOOKCLUBCLIENT_H
#define BOOST_ECHO_CLIENT_BOOKCLUBCLIENT_H

#include <vector>
#include <map>
#include "../include/connectionHandler.h"

class bookClubClient {

public:
    bookClubClient(ConnectionHandler &connectionHandler_);
    bookClubClient(const bookClubClient &aBookClubClient);
    bookClubClient & operator=(const bookClubClient &aBookClubClient);
    void addBook(std::string bookName, std::string genre, bool sendFrame);
    void subscribe(std::string genre);
    void unsubscribe(std::string genre);
    void borrowBook(std::string bookName, std::string genre);
    void returnBookIBorrowed(std::string bookName, std::string genre);
    void lendBook(std::string bookName, std::string genre);
    void sendStatus(std::string genre);
    void logIn (std::string host, std::string userName, std::string password);
    void borrowingBookFrom(std::string lender, std::string genre, std::string bookName);
    void logOut();
    void acceptBookILent(std::string bookName, std::string genre);
    void isBookAvailable(std::string bookName, std::string genre);
    void getStatus(std::string genre);
    std::string getName();
    std::string getReceiptMessage(std::string receipt);
    ConnectionHandler *getConnectionHandler() const ;
    void copy (bookClubClient bcc);

private:
    std::map<std::string, std::vector<std::string>> myBooks;
    std::map<std::string, std::string> booksIBorrowed;
    std::vector<std::string> booksILent;
    std::map<std::string,int > subscribeId;
    std::vector<std::string> waitingList;
    int subscribeCounter;
    int receipt;
    std::string clientName;
    std::string clientPassword;
    ConnectionHandler *connectionHandler;
    std::map<std::string,std::string> receiptTopic;


};


#endif //BOOST_ECHO_CLIENT_BOOKCLUBCLIENT_H
