#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define PORT 51234
#define BUFLEN 100

// int estadoJogo [2] = { gamestate(enum), direção(int -1, 0 ou 1)}

void wrap(char* buf, int* estadoJogo){
    for(int i=0; i < 2 ; ++i ){
        buf[i] = static_cast<char>(estadoJogo[i]);
    }
}

void unwrap(char* buf, int* estadoJogo){
    for(int i=0; i < 2; ++i ){
        estadoJogo[i] = static_cast<float>(buf[i]);
    }
}

int main () {

    int listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);    
    std::cout << "Socket number: " << listenSocketFD << std::endl;
    char buff[BUFLEN];
    bzero(buff, BUFLEN);
    
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    
    if (bind(listenSocketFD, (sockaddr*) &addr, sizeof(sockaddr_in))) {
        std::cout << "Binding error" << std::endl;
        close(listenSocketFD);
        return -2;
    }


    if (listen(listenSocketFD, SOMAXCONN)) {
        std::cout << "Listen error" << std::endl;
        close(listenSocketFD);
        return -3;
    }

    sockaddr_in client1_addr;
    socklen_t addr1_len = sizeof(sockaddr_in);
    std::cout << "Waiting for client connection..." << std::endl;
    int conn1SocketFD = accept(listenSocketFD, (sockaddr*) &client1_addr, &addr1_len);
    if (conn1SocketFD <= 0) {
        close(listenSocketFD);
        return -4;
    }
    else{
        std::cout << "Cliente 1 conectado com sucesso!!!" << std::endl;
        //Trecho para definir e enviar a informação de que o cliente é o player1
        buff[0] = '1';
        write(conn1SocketFD, buff, 1);
    }

    if (listen(listenSocketFD, SOMAXCONN)) {
        std::cout << "Listen error" << std::endl;
        close(listenSocketFD);
        return -3;
    }

    sockaddr_in client2_addr;
    socklen_t addr2_len = sizeof(sockaddr_in);
    std::cout << "Waiting for client connection..." << std::endl;
    int conn2SocketFD = accept(listenSocketFD, (sockaddr*) &client2_addr, &addr2_len);
    if (conn2SocketFD <= 0) {
        close(listenSocketFD);
        return -4;
    }
    else{
        std::cout << "Cliente 2 conectado com sucesso!!! Que comecem os jogos..." << std::endl;
        //Trecho para definir e enviar a informação de que o cliente é o player2
        buff[0] = '2';
        write(conn2SocketFD, buff, 1);
    }

    //Vetor pra passar informações  
    int estadoaux1[2];
    int estadoaux2[2];

    //Inicialização padrão do estado de jogo
    read(conn1SocketFD, buff, sizeof(buff));
    unwrap(buff, estadoaux1);
    unwrap(buff, estadoaux2);

    // código de gerenciamento das ações dos jogadores aqui
    while (true) {
        //Escrita e recebimento dos estados de jogo dos clientes: envia p2 -> recebe p2 -> envia p1 -> recebe p1
        write(conn2SocketFD, buff, sizeof(buff));
        read(conn2SocketFD, buff, sizeof(buff));
        write(conn1SocketFD, buff, sizeof(buff));  
        read(conn1SocketFD, buff, sizeof(buff));

        //Verificação para sair do loop quando algum jogador ganhar
        if(estadoaux1[0] == 2 || estadoaux2[0] == 2)
            break;
    }

    // Close connection
    close(conn1SocketFD);
    close(conn2SocketFD);
    return EXIT_SUCCESS;
}