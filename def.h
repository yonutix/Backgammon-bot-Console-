#ifndef _DEF_
#define _DEF_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <utility>
#include <vector>
#include <iostream>

#define WHITE 1
#define BLACK 0
#define INF 0x7FFFFFFF
#define MIN(a, b) ((a>b)?b:a)

/**
 * \class Connection
 * \brief Clasa connection realizeaza conexiunea cu serverul
 * are metode de trimitere si primire a pachetelor
 * \brief Class Connection realise the server connection
 * it has methods for sending and receiving packages
 */
class Connection{
public:
	/**
	 * \brief socketul pe care se realizeaza conexiunea
	 * \brief The connection is realised trough this socket
	 */
	int sockfd;

	Connection(char* hostname, char* server_port);

	/**
	 * \brief Trimite mesajul message de lungime length
	 * \brief Send the message of length length
	 */
	void sendMessage(char* message, int length);

	/**
	 * \brief Citeste mesajul message de lungime length
	 * \brief Read the message of length length
	 */
	void readMessage(char* message, int length);

	~Connection();
};


/**
 * \class AI 
 * Reprezinta memoria botului, metode pentru generarea 
 * tuturor posbilitatilor in care se poate muta o piesa , legatura catre
 * server si istoria mutarilor.
 * Represents the bot memory, it has methods for generating
 * all the possibilities in wich a piece can be moved, 
 * the link between server and the moves history
 * \publicsection
 */
class AI{

	/**
	 * \param link Obiect care realizeaza conexiunea cu serverul
	 * \param link The object wich realise the server connection
	 */
	Connection *link;

	/**
	 * \brief Culoarea botului meu, tabla de joc, istoricul mutarilor
	 * \brief The color of my bot, the game board, moves hostory
	 */
	int myColor, board[2][26], history[2][26];

	/**
	 * \brief mesajul primit de la server
	 * \brief The message recived from the server
	 */
	char* serv_msg;
	/**
	 * \brief lungimea mesajului
	 * \brief the length of the message
	 */
	char msg_size;

	/**
	 * \bief Starile in care se poate afla tabla
	 * \brief States wich in the board can be
	 */
	enum BoardState{Home, Normal, Out};

	/** 
	 * \publicsection
	 * \brief Muta o piesa alba de la pozitia poz, k pasi
	 * \brief Move a white piece from the position poz k steps
	 */
	void moveWhite(int poz, int k);

	/**
	 * \brief Muta o piesa neagra de la pozitia poz, k pasi
	 * \param his structura in care va fi pastrat istoricul
	 * \brief Move a black piece from the position k, k steps
	 * \param his The structure wich in will be kept the hostory
	 */
	void moveBlack(int poz, int k, int his[2][26]);

	/**
	 * \brief Verifica daca jocul s-a terminat
	 * \brief Verify if the game is over
	 */
	bool endGame();

	/**
	 * \brief Daca sunt jucatorul alb converteste intotdeauna invers(complementar)
	 * \param msg mesajul care va fi convertit
	 * \param @arrived true daca conversia este facuta la primire si false daca 
	 * conversia este facuta inainte de trimitere
	 * \brief It switches always the current player
	 * \param msg The message to be converted
	 * \param arrived Contains true if the conversion is made for arrival or false
	 * if the conversion is made before sending
	 */
	void convert(char* msg, bool arrived);

	/**
	 * \brief Sincronizeaza tabla cu noile mutari venite de la server
	 * \brief Syncronize the board with the new moves arrived from server
	 */
	void syncronizeBoard();

	/**
	 * \brief afla starea pieselor jucatorului negru
	 * \return returneaza <b>AI::Home AI::Normal, AI::Out</b>
	 * \brief Aknoledge the state of the black player's pieces
	 * \return Returns  <b>AI::Home AI::Normal, AI::Out</b>
	 */
	BoardState getTableState();


	/**
	 * \brief Genereaza toate mutarile posibile <b>cu un singur zar</b>
	 * pentru una din piesele scoase pe bara
	 * \brief Generate all the possible moves <b>with a single dice</b>
	 * for one of the out pieces
	 */
	std::vector<std::pair <int, int> > genMovesOutState(int dice);

	/**
	 * \brief Genereaza toate mutarile posibile <b>cu un singur zar</b>
	 * pentru toate piesele in cazul in care toate se alfa in casa
	 * \brief Generate all the possible moves <b>with a single dice</b>
	 * for all pieces when all are in home place
	 */
	std::vector<std::pair <int, int> > genMovesHomeState(int p, int dice);

	/**
	 * \brief Genereaza toate mutarile posibile <b>cu un singur zar</b>
	 * pentru cazul in care piesele nu sunt scoase pe bara si nu se afla
	 * in casa
	 * \brief Generate all possible moves <b>with a single dice</b>
	 * when the pieces are nor out nor home
	 */
	std::vector<std::pair <int, int> > genMovesNormalState(int p, int dice);

	/**
	 * \brief Genereaza toate pozitiile posbile <b>pentru un singur zar</b>
	 * \brief Generate all possible position <b>with a single dice</b>
	 */
	std::vector<std::pair <int, int> > genMovesForOneDice(int p, int dice);

	/**
	 * \brief Genereaza toate posibilitatile pentru cazul in care zarurile
	 * sunt diferite
	 * \brief Generate all possibilities when the dices unequal
	 */
	std::vector<std::vector<std::pair <int, int> > > genPosDifferentDices();

	/**
	 * \brief Genereaza toate posibilitatile pentru cazul in care zarurile
	 * sunt egale
	 * \brief Generate all posibilities when the dices are equals
	 */
	std::vector<std::vector<std::pair <int, int> > > genPosDoubleDice();

	/**
	 * \return Returneaza scorul pentru tabla intr-o anumita stare
	 * \return Returns the score for the board in a specific state
	 */
	int getScore(int color);

	/**
	 * \return returneaza numarul de piese neacoperite
	 * \return the number of "uncovered" pieces
	 */
	int countSingleCheckers();

	/**
	 *\return returneaza pozitia cea mai indepartata de casa pe 
	 * care se afla o piesa
	 * \return Returns the position the most departed from home
	 * wich in a piece is located
	 */
	inline int getFirstCheckerPosition();

	/**
	 * \return Returneaza cate pozitii sun acoperite
	 * \return Returns how much positions are "covered"
	 */
	int getCoverCount();

	/**
	 * \brief Filtreaza toate mutarile posibile si o returneaza pe cea
	 * favorabila
	 * \param possibleMoves sunt pozitiile din care se alege
	 * \brief Filter all the possible moves and returns the best one
	 * \param possibleMoves are the position to choose from
	 */
	std::vector<std::pair<int, int> > getBestMove(std::vector<std::vector<std::pair <int, int> > > possibleMoves);

	/**
	 * \brief Sterge istoricul mutarilor
	 * \param his Structura in care va fi pastrat istoricul
	 * \brief Delete the moves history
	 * \param his The structure wich in is kept the history
	 */
	void clearHistory(int his[2][26]);

	/**
	 * \brief Revine la starea de cand istoricul era gol
	 * \brief Clears the history stack
	 */
	void rollBack(int his[2][26]);

	/**
	 * \brief Returneaza numarul de piese de pe bara ale adversarului
	 * \brief Returns the opponent out pieces number
	 */
	inline int outWhiteCheckers();


	/**
	 * \brief Verifica daca jucatorul are "poarta in casa"
	 * \details Consider "poarta in casa" daca cel mult 2 spatii in casa
	 * sunt libere
	 * \brief Checks if the player has "house gate"
	 * \details "house gate" is when at most 2 spaces in house are clear
	 */
	bool isHomeGate(int color=BLACK);

	/**
	 * \brief Verifica daca piesele oponentului sunt "aproape"
	 * \details Cele mai mari probabilitati sunt de a da cu zarul
	 * A = [1, 2, 3, 4, 5, 6]->[12/36, 13/36, 14/36, 15/36, 16/36, 17/36]
	 * \details Pentru celelate probabilitatea este relativ mai mica:
	 * B = [7, 8, 9, 10, 11, 12]->[6/36, 5/36, 4/36, 3/36, 2/36, 1/36]
	 * \details Consider apropiate piesele din mutimea A
	 * \brief Checks if the opponent pieces are close
	 */
	int isClose(int piece);

	/**
	 * \brief Verifica cate piese se pot ataca
	 * \brief Checks how much pieces can attack each other
	 */
	int common();

public:
	AI(char* opponent_level, Connection *link);

	~AI();

	/**
	 * \brief Metoda generala pentru desfasurarea jocului
	 * \brief General method for game development
	 */
	void mainloop();


};
#endif

/*! \mainpage Documentatie \ Documentation
* \details <b>Proiectarea algoritmilor</b>
* \details <b>Tema 2</b>
* \details <b>Mihai Cosmin 322CC</b>
*
*
* \details Am folosit 2 clase: Connection, AI
* \details Connection este o interfata intre scheletul de cod pus la dispozitie pentru tema si algoritmul botului.
*
* \details Clasa AI este practic tema.
* \details In construcotr "asez" piesele pe tabla, stabilesc conexiunea(aflu ce culoare sunt, intializez istoricul, aloc memorie 
*pentru bufferul in care citesc) Folosesc doua buffere din motive de simplitate in implementare, as fi putut folosi unul.
*
* \details In destructor eliberez memoria bufferului si a linkului catre clasa cu conexiunea.
*
* \details Tot algoritmul se desfasoara in metoda mainloop().
* \details In mainloop() exista un while care se termina doar atunci cand pierd sau castig.
* \details La fiecare pas citesc mesajul, il convertesc astfel incat sa joc mereu cu negrul(am ales asa ca sa merg crescator, as fi
* putut alege la fel de bine albul).
* \details Sincronizez tabla cu mutarea oponentului.
* \details Generez toate mutarile posibile.
* \details Filtrez mutarile si o aleg pe cea mai buna in functie de o evaluare.
* \details Convertesc mutarea in caz ca oponentul e negru.
* \details Trimit mesajul.
* \details Pentru detalii vezi README
*
* \details <b>Algorithms design</b>
* \details <b>Homework 2</b>
* \details <b>Mihai Cosmin 322CC</b>
*
*
* \details I have used two classes: Connection, AI
* \details Connection is a interface between code scheleton made avialible for homework and the bot algorithm
*
* \details AI class practically is the homework
* \details In the constructor I place the pieces on the board, I make the connection(I find out what color I am , I inistialize the hystory,
* allocate memory for the arrival buffer).I use two buffers because of simplicity in implementation, I could use one.
*
* \details In the destructor I free the buffer and the link to connecction's class memory
*
* \details The algorithm happends in mainloop() method.
* \details In mainloop() exists a "while" wich is finished only when I lose or win.
* \details At each step I read the message , I convert it so I can play always with black (I have chosen to go ascending, I could choose
* as well the white player)
* \details I Syncronize the board with the opponent move
* \details I generate all the possibile moves
* \details I filter the moves and I choose the best one based to an evaluation.
* \details I convert the move in case that tha opponent is black
* \details I send the message
* \details FOrm more details see README
*/
