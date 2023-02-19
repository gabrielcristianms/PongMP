#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define PORT 51234
#define BUFLEN 100
 
using namespace sf;

//função pra imprimir o array de informações, mais usado para testar o código
void printEstadoJogo( int * estadoJ){
    for (int i = 0; i < 2; i++)
    {
        std::cout << estadoJ[i] << " ";
    }
}
 
bool intersects (const RectangleShape & rect1,const RectangleShape & rect2)
{
    FloatRect r1=rect1.getGlobalBounds();
    FloatRect r2=rect2.getGlobalBounds();
    return r1.intersects (r2);
}
 
int clamp (const int x, const int a, const int b)
{
    return std::min(std::max(a,x),b);
}

//wrap simples pra copiar e converter os valores do array de informações do jogo em char
void wrap(char* buf, int* estadoJogo){
    for(int i=0; i < 2 ; ++i ){
        buf[i] = static_cast<char>(estadoJogo[i]);
    }
}

//unwrap simples pra converter os bytes e jogar no array de informações
void unwrap(char* buf, int* estadoJogo){
    for(int i=0; i < 2; ++i ){
        estadoJogo[i] = static_cast<float>(buf[i]);
    }
}
 
int main()
{
    const int width = 640;
    const int height= 480;
    const int borderSize= 12;
    const int margin = 50;
    const int moveDistance = 200;
    const int initialBallSpeed = 5;
    
 
    VideoMode videoMode(width, height);
    RenderWindow window(videoMode,"Pong SFML 2");
 
    Font font;
    if (!font.loadFromFile("tomb.ttf"))
        return EXIT_FAILURE;     
    
    RectangleShape top;
    RectangleShape left;
    RectangleShape right;
    RectangleShape bottom;    
 
    // setting up all items
    top.setPosition(0, 0);
    top.setSize(Vector2f(width, borderSize));
 
    left.setPosition(-borderSize, 0);
    left.setSize(Vector2f(borderSize, height));
 
    right.setPosition(width, 0);
    right.setSize(Vector2f(borderSize, height));
 
    bottom.setPosition(0, height-borderSize);
    bottom.setSize(Vector2f(width, borderSize));
             
    top.setFillColor(Color(100,100,100));
    top.setOutlineColor(Color::Blue);
    top.setOutlineThickness(3);
 
    left.setFillColor(Color(100,100,100));
    left.setOutlineColor(Color::Blue);
    left.setOutlineThickness(3);
 
    right.setFillColor(Color(100,100,100));
    right.setOutlineColor(Color::Blue);
    right.setOutlineThickness(3);
 
    bottom.setFillColor(Color(100,100,100));
    bottom.setOutlineColor(Color::Blue);
    bottom.setOutlineThickness(3);
 
    RectangleShape ball;
    ball.setPosition(width/2, height/2);
    ball.setSize(Vector2f(20, 20));
    ball.setFillColor(Color::Yellow);
    ball.setOutlineColor(Color::Red);
    ball.setOutlineThickness(2);
 
    Vector2<float> ballSpeed(initialBallSpeed,initialBallSpeed);
 
    RectangleShape player1;    
    player1.setSize(Vector2f(borderSize, 90));
    player1.setPosition(margin-borderSize, height/2-25);
    player1.setFillColor(Color(0,122,245));
    player1.setOutlineColor(Color::Red);
    player1.setOutlineThickness(3);
 
    RectangleShape player2;    
    player2.setSize(Vector2f(borderSize, 90));
    player2.setPosition(width-margin, height/2-25);
    player2.setFillColor(Color(0,122,245));
    player2.setOutlineColor(Color::Red);
    player2.setOutlineThickness(3);
     
    RectangleShape middleLine;
    middleLine.setFillColor(Color(100,100,100,30));
    middleLine.setOutlineColor(Color(0,0,100,30));
    middleLine.setOutlineThickness(2);
    middleLine.setPosition(width/2, 0);
    middleLine.setSize(Vector2f(0, height));
 
    Text title("Pong SFML 2",font,50);
    title.setPosition(width/2-title.getGlobalBounds().width/2,100);
    title.setFillColor(Color::Blue); 
     
    Text start("Press any key to start",font,30);
    start.setPosition(width/2-start.getGlobalBounds().width/2,400);
    start.setFillColor(Color::Red);
 
    Text won("You have won this game.\n\n Congratulations !",font,20);
    won.setPosition(width/2-won.getGlobalBounds().width/2,height/2-won.getGlobalBounds().height/2);
    won.setFillColor(Color::Green);
 
    Text lost("You have lost this game, \n better luck next time!",font,20);
    lost.setPosition(width/2-lost.getGlobalBounds().width/2,height/2-lost.getGlobalBounds().height/2);
    lost.setFillColor(Color::Red);
 
    Text score("0   0",font,50);
    score.setPosition(width/2-score.getGlobalBounds().width/2,40);
    score.setFillColor(Color::Blue);
 
    unsigned int p1Score=0, p2Score=0;
 
    enum states {INTRO, PLAYING, P1WON, P1LOST};
     
    int gameState=INTRO;

    // Código de sockets
    bool isP1 = true;
    int connSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << "Socket number: " << connSocketFD << std::endl;
    char buff[BUFLEN]; // Buffer de rede
    
    //Inicialização das informações pro socket
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    //Connect do socket
    if (connect(connSocketFD, (sockaddr*) &server_addr, sizeof(sockaddr_in))) {
        close(connSocketFD);
        return -2;
    }
    else{
        std::cout << "Cliente conectado com sucesso!!! Que comecem os jogos..." << std::endl;
        read(connSocketFD, buff, sizeof(buff));

        //Checagem para definir quem é player 1 e quem é player dois, para definir quem vai mexer nas setinhas ou no W/S
        if(buff[0] == '2') isP1 = false;
        if(isP1) std::cout << "jogador é Player 1" << std::endl;
        else{
            std::cout << "jogador é player 2" << std::endl;
        }
    }
    
    bzero(buff, BUFLEN);
    // estadoJogo [2] = { gamestate(enum), movimento(baixo, nenhum ou cima) }
    /// Vetor onde serão guardadas as infos do jogo para ser enviado via rede
    int estadoJogo[2] = { gameState, 0};

    //Enviando mensagem com estado padrão do jogo.
    if(isP1){
        wrap(buff, estadoJogo);
        write(connSocketFD, buff, sizeof(buff));
    }

    //Game Loop
    while (window.isOpen())
    {
        /// Ler estado do oponente
        read(connSocketFD, buff, sizeof(buff));
        unwrap(buff, estadoJogo);
        gameState = estadoJogo[0];

        // Desenhar na tela de acordo com o estado de jogo
        window.clear(Color::White);
        switch(gameState)
        {
            case INTRO:
                window.draw(title);
                window.draw(start);
                break;
            case PLAYING:
                window.draw(middleLine);
                window.draw(left);
                window.draw(right);            
                window.draw(player1);    
                window.draw(player2);    
                window.draw(ball);    
                window.draw(score);    
                window.draw(top);
                window.draw(bottom);        
                break;
            case P1WON:
                window.draw(won);
                //Linha pra inverter o estado de jogo para que na atualização a ser enviado seja o rolé correto pro outro jogador
                gameState = P1LOST;
                break;
            case P1LOST:
                window.draw(lost);
                //Linha pra inverter o estado de jogo para que na atualização a ser enviado seja o rolé correto pro outro jogador
                gameState = P1WON;
                break;
        }
        window.display();

        //Leitura de eventos do teclado 
        Event event;
        while (window.pollEvent(event))
        {
            if ( (event.type == Event::Closed) ||
            ((event.type == Event::KeyPressed) && (event.key.code==Keyboard::Escape)) )
                window.close();    
            else
            if ((event.type == Event::KeyPressed) && (gameState == INTRO))
                gameState=PLAYING;
        }

        //atualizar mudança no estado de jogo
        estadoJogo[0] = gameState;

        // Caso ninguém tenha pressionado nada do teclado, envia esse estado para o outro jogador e volto pro começo do loop 
        if (gameState!=PLAYING){
            wrap(buff, estadoJogo);
            write(connSocketFD, buff, sizeof(buff));
            continue;
        }

        /// MOVIMENTA Oponente
        if(isP1){
            if(estadoJogo[1] == 1){ //PRA CIMA
                player2.move(0,-moveDistance/50.0);
            }
            else if(estadoJogo[1] == -1){ //PRA BAIXO
                player2.move(0,moveDistance/50.0);
            }
        }
        else{
            //player1.setPosition( estadoJogo[3], estadoJogo[4] );
            if(estadoJogo[1] == 1){ //PRA CIMA
                player1.move(0,-moveDistance/50.0);
            }
            else if(estadoJogo[1] == -1){ //PRA BAIXO
                player1.move(0,moveDistance/50.0);
            }   
        }

        /// MOVIMENTA JOGADOR 1
        // move player 1 pad
        if(isP1){
            if (Keyboard::isKeyPressed(Keyboard::Up))
            {
                player1.move(0,-moveDistance/50.0);
                estadoJogo[1] = 1;
            }
            else if (Keyboard::isKeyPressed(Keyboard::Down)){
                player1.move(0,moveDistance/50.0);
                    estadoJogo[1] = -1;
            }
            else{
                    estadoJogo[1] = 0;
            }
        }
        else{
            if (Keyboard::isKeyPressed(Keyboard::W))
            {
                player2.move(0,-moveDistance/50.0);
                estadoJogo[1] = 1;
            }
            else if (Keyboard::isKeyPressed(Keyboard::S)){
                    player2.move(0,moveDistance/50.0);
                    estadoJogo[1] = -1;
            }
            else{
                estadoJogo[1] = 0;
            }
        }


        /// GARANTE QUE JOGADORES FIQUEM DENTRO DO TABULEIRO
        // block players pad inside the play area
        if ( intersects(player1,bottom) || intersects(player1,top) )
        {
            FloatRect t=top.getGlobalBounds();
            FloatRect b=bottom.getGlobalBounds();
            Vector2f p=player1.getPosition();
            p.y=clamp(p.y,t.top+t.height+5,b.top-player1.getSize().y-5);
            player1.setPosition(p);
        }
        if ( intersects(player2,bottom) || intersects(player2,top) )
        {
            FloatRect t=top.getGlobalBounds();
            FloatRect b=bottom.getGlobalBounds();
            Vector2f p=player2.getPosition();
            p.y=clamp(p.y,t.top+t.height+5,b.top-player2.getSize().y-5);
            player2.setPosition(p);
        }
                
        

        /// CHECA SE BOLA COLIDIU EM CIMA OU EM BAIXO DO TABULEIRO
        // ball collides with top and bottom
        if (intersects(ball,top))
        {
            FloatRect t=top.getGlobalBounds();
            FloatRect b=ball.getGlobalBounds();
            ballSpeed.y=-ballSpeed.y;
            int u = t.top + t.height - b.top;
            ball.move(0,2*u);            
        }
        if ( intersects(ball,bottom) )
        {
            FloatRect bot= bottom.getGlobalBounds();
            FloatRect b= ball.getGlobalBounds();
            ballSpeed.y=-ballSpeed.y;
            int u = bot.top - b.height - b.top;
            ball.move(0,2*u);   
        }
        
        /// CHECA SE BOLA COLIDIU COM JOGADOR
        // ball collides with player1 and player2
        if (intersects(ball,player1))
        {
            FloatRect p= player1.getGlobalBounds();
            FloatRect b= ball.getGlobalBounds();
            ballSpeed.x= -ballSpeed.x;
            ballSpeed.y= (b.top+b.height/2 - p.top - p.height/2) / 100;
            int u = p.left +  p.width - b.left;
            b.left = p.left +  p.width + u;
            ball.setPosition(b.left,b.top);
            //increase ball speed by 1%
            ballSpeed.x=ballSpeed.x*1.01;
            ballSpeed.y=ballSpeed.y*1.01;
        }
        if ( intersects(ball,player2) )
        {
            FloatRect p=player2.getGlobalBounds();
            FloatRect b=ball.getGlobalBounds();
            ballSpeed.x=-ballSpeed.x;
            ballSpeed.y= (b.top+b.height/2 - p.top - p.height/2) / 100;
            int u = b.left + b.width - p.left;
            b.left = p.left -  b.width - u;
            ball.setPosition(b.left,b.top);
            //increase ball speed by 1%
            ballSpeed.x=ballSpeed.x*1.01;
            ballSpeed.y=ballSpeed.y*1.01;
        }                        
         
        /// CHECA POR PONTUAÇÃO
        // check for scoring
        if (intersects(ball,left))
        {
            p2Score++;
            std::stringstream str;
            str << p1Score << "   " << p2Score;
            score.setString(str.str());
            score.setPosition(width/2-score.getGlobalBounds().width/2,40);
            FloatRect p=player2.getGlobalBounds();
            FloatRect b=ball.getGlobalBounds();
            ball.setPosition(p.left-b.width-5, height/2);
            ballSpeed.x=-initialBallSpeed;
            ballSpeed.y=initialBallSpeed;
 
        }
        if (intersects(ball,right))
        {
            p1Score++;
            std::stringstream str;
            str << p1Score << "   " << p2Score;
            score.setString(str.str());
            score.setPosition(width/2-score.getGlobalBounds().width/2,40);
            FloatRect p=player1.getGlobalBounds();
            FloatRect b=ball.getGlobalBounds();
            ball.setPosition(p.left+p.width+5, height/2);
            ballSpeed.x=initialBallSpeed;
            ballSpeed.y=initialBallSpeed;
        }

        /// CHECA SE JOGO ACABOU
        // detect if game is over
        if (p1Score >=2 && p1Score >= p2Score +2){
            if(isP1){
                gameState=P1WON;
                estadoJogo[0] = P1LOST;
            }
            else{
                gameState=P1LOST;
                estadoJogo[0] = P1WON;
            }

        }
        if (p2Score >=2 && p2Score >= p1Score +2){
            if(!isP1){
                gameState=P1WON;
                estadoJogo[0] = P1LOST;
            }
            else{
                gameState=P1LOST;
                estadoJogo[0] = P1WON;
            } 
        }
         
        ball.move(ballSpeed.x,ballSpeed.y);
 
        /// Enviar estado pro oponente
        wrap(buff, estadoJogo);
        write(connSocketFD, buff, sizeof(buff));

    }
    // Close connection
    close(connSocketFD);
    return EXIT_SUCCESS;
}