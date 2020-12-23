#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void game(int);
void guessedWord(char *, char *, char *);
void exitGameWithMessage(char *, char *, int);

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
    game(sockfd);

    close(sockfd);
    return 0;
}
/*Protocol:
TYPE|{message 1}|[message 2]...|[message N]

Server may send:
	MESSAGE|{message}                               -Sends a message string for client to print
	GUESS|{YES/NO}|{lives remaining}|{displayWord}	-Informs user whether the guess was correct and sends remaining lives
	EXIT|{YES/NO}|{word}	                        -Informs client that the game is over. YES if won, NO if lost
	
Client may send:
	GUESS|{character}	-Client will attempt to guess the word
	EXIT|[message]		-Client can terminate the game early. Can optionally send a message
*/
void game(int sock) 
{
    char buffer[256];
    char delim[] = "|";
    char *type, *displayWord, *yn, *lives, *message;
    bzero(buffer,256);
    //

    //Initial message
    read(sock,buffer,256);
    type = strtok(buffer, delim);
    displayWord = strtok(NULL, delim);
    printf("Welcome to The Hangman! Please use the following syntax:\n\tGUESS|{character}\n\tEXIT|[message]\n" 
        "The word is:\n\n%s\n\n", displayWord);

    while(1)
    {
        //Cleanup
        bzero(buffer,256);
        fflush(stdin);

        //Send stdin to server
        fgets(buffer,256,stdin);
        write(sock, buffer, 256);

        //Receive response and decode message
        bzero(buffer,256);
        read(sock, buffer, 256);
        type = strtok(buffer, delim);
        if(strcmp("GUESS",type) == 0) 
        {
            yn = strtok(NULL, delim);
            lives = strtok(NULL, delim);
            displayWord = strtok(NULL, delim);
            guessedWord(yn, lives, displayWord);
        }
        else if (strcmp("EXIT",type) == 0)
        {
            yn = strtok(NULL, delim);
            displayWord = strtok(NULL, delim);
            exitGameWithMessage(yn, displayWord, sock);
        }
        else if (strcmp("MESSAGE",type) == 0) 
        {
            int index;
            for(index = 0; buffer[index] != '\0'; index++);
            message = &buffer[index + 1];
            printf("%s", message);
        }
    }
}

void guessedWord(char *yn, char *lives, char *displayWord) 
{
    char message[100];
    if(strcmp("YES",yn) == 0)
    {
        strcpy(message, "That's correct!\n");
    }
    else if (strcmp("NO",yn) == 0)
    {
        strcpy(message, "That's incorrect!\n");
    }
    printf("%s%s lives remaining.\n\n%s\n\n", message, lives, displayWord);
}

void exitGameWithMessage(char *yn, char *displayWord, int sock)
{
    char message[100];
    if(strcmp("YES",yn) == 0)
    {
        strcpy(message, "Congratulations, you have won!\nYou have guessed the word: ");
    }
    else if (strcmp("NO",yn) == 0)
    {
        strcpy(message, "Game Over!\nThe correct word was: ");
    }
    printf("%s%s\n", message, displayWord);

    close(sock);
    exit(0);
}
