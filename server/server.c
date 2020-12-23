#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/inet.h>

#define START_LIVES 10
#define WORD_COUNT 14

void hangman(int, struct sockaddr_in);
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
int generateRandomWord(char*);
void fillWithBlanks(char*);
int checkCharacter(int*, int*, char c, char*, char*);
void exitMessage(char*, struct sockaddr_in);

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) 
    {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    while (1) 
    {
        newsockfd = accept(sockfd, 
            (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0)  
        {
            close(sockfd);
            printf("Client %s:%d has connected\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
            hangman(newsockfd, cli_addr);
            printf("Client %s:%d has disconnected\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
            exit(0);
        }
        else close(newsockfd);
    }
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
void hangman (int client, struct sockaddr_in client_addr)
{
    int n;
    char word[256], displayWord[256];
    char delim[] = "|";
    int lives = START_LIVES, charsGuessed = 0;
    char *character, *type, *message;
    
    int size = generateRandomWord(word);
    strcpy(displayWord, word);
    fillWithBlanks(displayWord);
   
    char buffer[256];
    bzero(buffer,256);	//Set buffer values to zero
	
	strcpy(buffer, "MESSAGE|");
    strcat(buffer, displayWord);
    write(client, buffer, 256);	//Writes initial message to the client

    while(1) 
    {
        bzero(buffer, 256);
        if(read(client, buffer, 256) == 0) break;
        type = strtok(buffer, delim);
        if(strcmp("GUESS",type) == 0)
        {
            character = strtok(NULL, delim);
            if(checkCharacter(&lives, &charsGuessed, character[0], word, displayWord) == 1) {
			    if(charsGuessed == size) {
                    bzero(buffer, 256);
                    strcpy(buffer, "EXIT|YES|");
                    strcat(buffer, word);
                    if(write(client, buffer, 256) == 0) break;
                    break;
			    }
                sprintf(buffer, "GUESS|YES|%d|%s", lives, displayWord);
                if(write(client, buffer, 256) == 0) break;
            }
            else {
                if(lives == 0) {
                    strcpy(buffer, "EXIT|NO|");
                    strcat(buffer, word);
                    if(write(client, buffer, 256) == 0) break;
                    break;
                }
                sprintf(buffer, "GUESS|NO|%d|%s", lives, displayWord);
                if(write(client, buffer, 256) == 0) break;
            }
        }
        else if(strcmp("EXIT",type) == 0 || ("EXIT\n",type) == 0)
        {
            message = strtok(NULL, delim);
            if(message[0] != '\n') exitMessage(message, client_addr);
            bzero(buffer, 256);
            strcpy(buffer, "EXIT|NO|");
            strcat(buffer, word);
            if(write(client, buffer, 256) == 0) break;
            break;
        }
        else
        {
            bzero(buffer, 256);
            strcpy(buffer, "MESSAGE|Invalid input. Please follow the correct protocol:\n\tGUESS|{character}\n\tEXIT|[message]\n\n");
            if(write(client, buffer, 256) == 0) break;
        }
    }
}

int generateRandomWord (char * word)
{
	srand(time(NULL));
    int random = rand() % WORD_COUNT;
    switch(random) {
        case 0:
            strcpy(word, "APPLE\0");
            return 5;
        case 1:
            strcpy(word, "LEMON\0");
            return 5;
        case 2:
            strcpy(word, "CARROT\0");
            return 6;
        case 3:
            strcpy(word, "SHIP\0");
            return 4;
        case 4:
            strcpy(word, "TRUCK\0");
            return 5;
        case 5:
            strcpy(word, "PERSON\0");
            return 6;
        case 6:
            strcpy(word, "DOG\0");
            return 3;
        case 7:
            strcpy(word, "CAT\0");
            return 3;
        case 8:
            strcpy(word, "KEYBOARD\0");
            return 8;
        case 9:
            strcpy(word, "MOUSE\0");
            return 5;
        case 10:
            strcpy(word, "MOONLIGHT\0");
            return 9;
        case 11:
            strcpy(word, "SUNSET\0");
            return 6;
        case 12:
            strcpy(word, "SOFA\0");
            return 4;
        case 13:
            strcpy(word, "CHRISTMAS\0");
			return 9;
    }
    strcpy(word, "\0");
}

void fillWithBlanks(char* word) {
    for(int i = 0; word[i] != '\0'; i++) {
        word[i] = '_';
    }
}

int checkCharacter(int *lives, int *charsGuessed, char c, char* word, char* displayWord) {
    c = toupper(c);
    int guessed = 0;
    for(int i = 0; word[i] != '\0'; i++) {
        if(c == word[i]) {
            guessed = 1;
            displayWord[i] = c;
            (*charsGuessed)++;
        }
    }
    if(guessed == 0) {
        (*lives)--;
    }
    return guessed;
}

void exitMessage(char *message, struct sockaddr_in client_addr)
{
    printf("Client %s:%d - %s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), message);
}