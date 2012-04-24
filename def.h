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
 */
class Connection{
public:
	/**
	 * \brief socketul pe care se realizeaza conexiunea
	 */
	int sockfd;

	Connection(char* hostname, char* server_port);

	/**
	 * \brief Trimite mesajul message de lungime length
	 */
	void sendMessage(char* message, int length);

	/**
	 * \brief Citeste mesajul message de lungime length
	 */
	void readMessage(char* message, int length);

	~Connection();
};


/**
 * \class AI 
 * Reprezinta memoria botului, metode pentru generarea 
 * tuturor posbilitatilor in care se poate muta o piesa , legatura catre
 * server si istoria mutarilor.
 * \publicsection
 */
class AI{

	/**
	 * \brief Obiect care realizeaza conexiunea cu serverul
	 */
	Connection *link;

	/**
	 * \brief Culoarea botului meu, tabla de joc, istoricul mutarilor
	 */
	int myColor, board[2][26], history[2][26];

	/**
	 * \brief mesajul primit de la server
	 */
	char* serv_msg;
	/**
	 * \brief lungimea mesajului
	 */
	char msg_size;

	/**
	 * \bief Starile in care se poate afla tabla
	 */
	enum BoardState{Home, Normal, Out};

	/** 
	 * \publicsection
	 * \brief Muta o piesa alba de la pozitia poz, k pasi
	 */
	void moveWhite(int poz, int k);

	/**
	 * \brief Muta o piesa neagra de la pozitia poz, k pasi
	 * \param @his structura in care va fi pastrat istoricul
	 */
	void moveBlack(int poz, int k, int his[2][26]);

	/**
	 * \brief Verifica daca jocul s-a terminat
	 */
	bool endGame();

	/**
	 * \brief Daca sunt jucatorul alb converteste intotdeauna invers(complementar)
	 * \param msg mesajul care va fi convertit
	 * \param @arrived true daca conversia este facuta la primire si false daca 
	 * conversia este facuta inainte de trimitere
	 */
	void convert(char* msg, bool arrived);

	/**
	 * \brief Sincronizeaza tabla cu noile mutari venite de la server
	 */
	void syncronizeBoard();

	/**
	 * \brief afla starea pieselor jucatorului negru
	 * \return returneaza <b>AI::Home AI::Normal, AI::Out</b>
	 */
	BoardState getTableState();


	/**
	 * \brief Genereaza toate mutarile posibile <b>cu un singur zar</b>
	 * pentru una din piesele scoase pe bara
	 */
	std::vector<std::pair <int, int> > genMovesOutState(int dice);

	/**
	 * \brief Genereaza toate mutarile posibile <b>cu un singur zar</b>
	 * pentru toate piesele in cazul in care toate se alfa in casa
	 */
	std::vector<std::pair <int, int> > genMovesHomeState(int p, int dice);

	/**
	 * \brief Genereaza toate mutarile posibile <b>cu un singur zar</b>
	 * pentru cazul in care piesele nu sunt scoase pe bara si nu se afla
	 * in casa
	 */
	std::vector<std::pair <int, int> > genMovesNormalState(int p, int dice);

	/**
	 * \brief Genereaza toate pozitiile posbile <b>pentru un singur zar</b>
	 */
	std::vector<std::pair <int, int> > genMovesForOneDice(int p, int dice);

	/**
	 * \brief Genereaza toate posibilitatile pentru cazul in care zarurile
	 * sunt diferite
	 */
	std::vector<std::vector<std::pair <int, int> > > genPosDifferentDices();

	/**
	 * \brief Genereaza toate posibilitatile pentru cazul in care zarurile
	 * sunt egale
	 */
	std::vector<std::vector<std::pair <int, int> > > genPosDoubleDice();

	/**
	 * \return Returneaza scorul pentru tabla intr-o anumita stare
	 */
	int getScore(int color);

	/**
	 * \return returneaza numarul de piese neacoperite
	 */
	int countSingleCheckers();

	/**
	 *\return returneaza pozitia cea mai indepartata de casa pe 
	 * care se afla o piesa
	 */
	inline int getFirstCheckerPosition();

	/**
	 * \return Returneaza cate pozitii sun acoperite
	 */
	int getCoverCount();

	/**
	 * \brief Filtreaza toate mutarile posibile si o returneaza pe cea
	 * favorabila
	 * \param possibleMoves sunt pozitiile din care se alege
	 */
	std::vector<std::pair<int, int> > getBestMove(std::vector<std::vector<std::pair <int, int> > > possibleMoves);

	/**
	 * \brief Sterge istoricul mutarilor
	 * \param his Structura in care va fi pastrat istoricul
	 */
	void clearHistory(int his[2][26]);

	/**
	 * \brief Revine la starea de cand istoricul era gol
	 */
	void rollBack(int his[2][26]);

	/**
	 * \brief Returneaza numarul de piese de pe bara ale adversarului
	 */
	inline int outWhiteCheckers();


	/**
	 * \brief Verifica daca jucatorul care "poarta in casa"
	 * \details Consider "poarta in casa" daca cel mult 2 spatii in casa
	 * sunt libere
	 */
	bool isHomeGate(int color=BLACK);

	/**
	 * \brief Verifica daca piesele oponentului sunt "aproape"
	 * \details Cele mai mari probabilitati sunt de a da cu zarul
	 * A = [1, 2, 3, 4, 5, 6]->[12/36, 13/36, 14/36, 15/36, 16/36, 17/36]
	 * \details Pentru celelate probabilitatea este relativ mai mica:
	 * B = [7, 8, 9, 10, 11, 12]->[6/36, 5/36, 4/36, 3/36, 2/36, 1/36]
	 * \details Consider apropiate piesele din mutimea A
	 */
	int isClose(int piece);

	/**
	 * \brief Verifica cate piese se pot ataca
	 */
	int common();

public:
	AI(char* opponent_level, Connection *link);

	~AI();

	/**
	 * \brief Metoda generala pentru desfasurarea jocului
	 */
	void mainloop();


};
#endif

/*! \mainpage Documentatie
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
*/
