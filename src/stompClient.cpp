#include <stdlib.h>
#include <connectionHandler.h>
#include "../include/bookClubClient.h"

/**
* This code assumes that the server replies the exact text the client sent it (as opposed to the practical session example)
*/

std::vector<std::string> getUserInput();
void userInputProcess(std::vector<std::string>, bookClubClient clientHandler);
void msgReceivedProcess(bookClubClient clientHandler, ConnectionHandler *connectionHandler);
std::vector<std::string> bodyString (std::string body);
void printFrame(std::vector<std::string> frame);


int main (int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
        return -1;
    }
    std::string host = "132.73.199.150";
    short port = 7777;//todo: change back to argv
    
    ConnectionHandler connectionHandler(host, port);
    if (!connectionHandler.connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }
    bookClubClient clientHandler(connectionHandler);//todo: check if correct.
    while(1){
        std::vector<std::string> userInput = getUserInput();
        userInputProcess(userInput, clientHandler);
        if(!connectionHandler.getConnectionStatus()){
            break;
        }
        msgReceivedProcess(clientHandler, &connectionHandler);

    }



//
//	//From here we will see the rest of the ehco client implementation:
//    while (1) {
//        const short bufsize = 1024;
//        char buf[bufsize];
//        std::cin.getline(buf, bufsize);
//		std::string line(buf);
//		int len=line.length();
//        if (!connectionHandler.sendLine(line)) {
//            std::cout << "Disconnected. Exiting...\n" << std::endl;
//            break;
//        }
//		// connectionHandler.sendLine(line) appends '\n' to the message. Therefor we send len+1 bytes.
//        std::cout << "Sent " << len+1 << " bytes to server" << std::endl;
//
//
//        // We can use one of three options to read data from the server:
//        // 1. Read a fixed number of characters
//        // 2. Read a line (up to the newline character using the getline() buffered reader
//        // 3. Read up to the null character
//        std::string answer;
//        // Get back an answer: by using the expected number of bytes (len bytes + newline delimiter)
//        // We could also use: connectionHandler.getline(answer) and then get the answer without the newline char at the end
//        if (!connectionHandler.getLine(answer)) {
//            std::cout << "Disconnected. Exiting...\n" << std::endl;
//            break;
//        }
//
//		len=answer.length();
//		// A C string must end with a 0 char delimiter.  When we filled the answer buffer from the socket
//		// we filled up to the \n char - we must make sure now that a 0 char is also present. So we truncate last character.
//        answer.resize(len-1);
//        std::cout << "Reply: " << answer << " " << len << " bytes " << std::endl << std::endl;
//        if (answer == "bye") {
//            std::cout << "Exiting...\n" << std::endl;
//            break;
//        }
//    }
//    return 0;
}

std::vector<std::string> getUserInput(){
    std::string userInput;
    std::cin>>userInput;
    std::vector<std::string> splitUserInput;
    std::string temp;
    for(auto ch : userInput){
        if(ch == ' '){
            splitUserInput.push_back(temp);
            temp="";
        } else{
            temp+=ch;
        }
    }
    return splitUserInput;
}

void process(std::vector<std::string> userInput,bookClubClient clientHandler ){
    if (userInput.at(0)=="login"){
        clientHandler.logIn(userInput.at(2),userInput.at(3));
    }
    else if(userInput.at(0)=="join"){
        clientHandler.subscribe(userInput.at(1));
    }
    else if(userInput.at(0)=="exit"){
        clientHandler.unsubscribe(userInput.at(1));
    }
    else if(userInput.at(0)=="add"){
        clientHandler.addBook(userInput.at(2),userInput.at(1),true);
    }
    else if(userInput.at(0)=="borrow"){
        clientHandler.borrowBook(userInput.at(2),userInput.at(1));
    }
    else if (userInput.at(0)=="return"){
        clientHandler.returnBookIBorrowed(userInput.at(2),userInput.at(1));
    }
    else if (userInput.at(0)=="status"){
        clientHandler.getStatus(userInput.at(1));
    } else if(userInput.at(0)=="logout"){
        clientHandler.logOut();
    }
    else{
        std::cout<<"unValid input"<<std::endl;
    }
}

void msgReceivedProcess(bookClubClient clientHandler, ConnectionHandler *connectionHandler){
    while(connectionHandler->getConnectionStatus()){
        std::vector<std::string> stompMessage = connectionHandler->getStompframe();
        if(stompMessage.at(0)=="MESSAGE"){
            std::string genre = stompMessage.at(3).substr(12);
            //-----print the message-----
            std::cout<<genre<<":"<<stompMessage.at(5)<<std::endl;
            std::vector<std::string> body = bodyString(stompMessage.at(5));
            if(body.at(1) == "wish"){
                clientHandler.isBookAvailable(body.at(4),genre);
            }
            else if(body.at(0)=="Taking" && body.at(3)==clientHandler.getName()){
                clientHandler.lendBook(body.at(1),genre);
            }
            else if(body.at(1) == "has" && body.size()==3){
                clientHandler.borrowingBookFrom(body.at(0),genre,body.at(2));
            }
            else if(body.at(0) == "Returning" && body.at(3)==clientHandler.getName()){
                clientHandler.acceptBookILent(body.at(1),genre);
            }
            else if(body.at(0) == "Book"){
                clientHandler.sendStatus(genre);
            }
        }
        else if(stompMessage.at(0)=="CONNECTED"){
            std::cout<<"Login successful"<<std::endl;
        }
        else if(stompMessage.at(0)=="RECEIPT"){
            std::string receiptMessage = clientHandler.getReceiptMessage(stompMessage.at(1).substr(11));
            if (receiptMessage == "disconnect") {
                connectionHandler->close();//TODO: check if this the only connectionHandler.
            }
            else {
                std::cout<<receiptMessage<<std::endl;
            }
        }
        else if (stompMessage.at(0)=="ERROR"){
            std::cout<<stompMessage.at(1).substr(10)<<std::endl;
        }
        else{
            std::cout<<"unValid frame"<<std::endl;
        }
    }
}


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
    return splitBody;
}

