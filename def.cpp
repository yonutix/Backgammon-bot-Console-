
#include "def.h"

Connection::Connection(char* hostname, char* server_port)
{
  int portno = atoi(server_port);
  this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    perror("ERROR opening socket");
    exit(0);
  }
  struct hostent *server;

  server = gethostbyname(hostname);
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }
  struct sockaddr_in serv_addr;

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
      (char *)&serv_addr.sin_addr.s_addr,
      server->h_length);
  serv_addr.sin_port = htons(portno);

  //Conectare la server
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
    perror("ERROR connecting");
    exit(0);
  }
}

Connection::~Connection()
{
  close(sockfd);
}

void Connection::sendMessage(char* message, int length) {
  int sent_so_far = 0, ret = 0;
  while (sent_so_far < length) {
    ret = send(this->sockfd, message + sent_so_far, length - sent_so_far, 0);
    if (ret == -1) {
      perror("Can't send message");
      exit(0);
    }
    sent_so_far += ret;
  }
}

void Connection::readMessage(char* message, int length) {
  int read_so_far = 0, ret = 0;
  while (read_so_far < length) {
    ret = recv(this->sockfd, message + read_so_far, length - read_so_far, 0);
    if (ret == -1) {
      perror("Can't read message");
      exit(0);
    }
    read_so_far += ret;
  }
}

AI::AI(char* opponent_level, Connection *link)
{
  /*Initializez tabla*/
  for(int i = 0; i < 26; i++)
    board[0][i] = board[1][i] = 0;
  board[WHITE][6] = 5;
  board[WHITE][8] = 3;
  board[WHITE][13] = 5;
  board[WHITE][24] = 2;
  board[BLACK][1] = 2;
  board[BLACK][12] = 5;
  board[BLACK][17] = 3;
  board[BLACK][19] = 5;

  //Conexiunea
  this->link = link;
  //Trimitere nivel oponent
  char buff = opponent_level[0] - '0';
  char msg_size = 1;
  link->sendMessage(&msg_size, 1);
  link->sendMessage(&buff, 1);
  //Citire culoare
  char buffer[2];
  bzero(buffer, 2);
  link->readMessage(&msg_size, 1);
  link->readMessage(buffer, msg_size);
  if (buffer[0] == 0){
    // sunt jucatorul alb
    this->myColor = WHITE;
    printf("Sunt alb!\n");
  }
  else{ 
    if (buffer[0] == 1){
      // sunt jucatorul negru
      this->myColor = BLACK;
      printf("Sunt negru!\n");
    }
    else{
      printf("Eroare citind de la server\n");
      // mesaj invalid; eroare!
    }
  }
  clearHistory(history);
  serv_msg = (char*)malloc(sizeof(char)* 100);
}

AI::~AI()
{
  delete link;
  delete serv_msg;
}

void AI::moveWhite(int poz, int k){
  if(poz-k > 0){
    /*Daca destinatia mutarii este pe tabla*/
    if(board[BLACK][poz-k] == 1){
      /*Daca la destinatie se afla o piesa neagra ea trebuie scoasa pe bara*/
      board[BLACK][0]++;
      board[BLACK][poz-k]--;
      board[WHITE][poz]--;
      board[WHITE][poz-k]++;
    }
    else{
      /*Daca destinatia e libera*/
      board[WHITE][poz]--;
      board[WHITE][poz-k]++;
    }
  }
  else{
    /*Daca piesa este scoasa din joc*/
    board[WHITE][poz]--;
    board[WHITE][0]++;
  }
}

void AI::moveBlack(int poz, int k, int his[2][26])
{
  if(poz+k < 25){
    /*Daca destinatia este pe tabla*/
    if(board[WHITE][poz+k] == 1){
      /*Daca la destinatie se afla o piesa alba*/
      board[WHITE][25]++;
      board[WHITE][poz+k]--;
      board[BLACK][poz]--;
      board[BLACK][poz+k]++;

      his[WHITE][25]++;
      his[WHITE][poz+k]--;
      his[BLACK][poz]--;
      his[BLACK][poz+k]++;


    }
    else{
      board[BLACK][poz]--;
      board[BLACK][poz+k]++;

      his[BLACK][poz]--;
      his[BLACK][poz+k]++;
    }
  }
  else{
    board[BLACK][poz]--;
    board[BLACK][25]++;

    his[BLACK][poz]--;
    his[BLACK][25]++;
  }
}

void AI::clearHistory(int his[2][26])
{
  for(int i = 0; i < 26; i++)
    his[WHITE][i] = his[BLACK][i] = 0;
}

void AI::rollBack(int his[2][26])
{
  for(int i = 0; i < 26; i++){
    board[WHITE][i] -= his[WHITE][i];
    board[BLACK][i] -= his[BLACK][i];
  }
  clearHistory(his);
}

bool AI::endGame()
{
  if(msg_size == 1){
    if (serv_msg[0] == 'W') {
        printf("WE WON\n");
        return true;
      } else {
        printf("WE LOST\n");
        return true;
      }
  }
  return false;
}

void AI::convert(char* msg, bool arrived)
{
  int n, start;
  if(arrived == true){
    n = msg_size-2;
    start = 0;
  }
  else{
    n = msg_size;
    start = 1;
  }

  for(int i = start; i < n; i+=2){
    if(myColor == WHITE && msg[i] != 30 && msg[i] != 0){
      /*Daca oponentul e negru, si nu sunt mesaje de scoatere sau introducere pe
      tabla*/
      msg[i] = 25 - msg[i];
    }
    if(msg[i] == 30)
      msg[i] = 25;
    if(msg[i] == 0)
      msg[i] = 30;
  }
}

void AI::syncronizeBoard()
{
  for(int i = 0; i < msg_size-2; i+=2)
    moveWhite(serv_msg[i], serv_msg[i+1]);
}

enum AI::BoardState AI::getTableState()
{
  if(board[BLACK][0] > 0)
    return AI::Out;

  for(int i = 1; i < 19; i++)
    if(board[BLACK][i] > 0)
      return AI::Normal;

  return AI::Home;
}

std::vector<std::pair <int, int> > AI::genMovesOutState(int dice)
{
  std::vector<std::pair <int, int> > pos;
  std::pair<int, int> move;
  if(board[WHITE][dice] < 2){
    move.first = 0;
    move.second = dice;
    pos.push_back(move);
  }
  return pos;
}

inline int AI::getFirstCheckerPosition()
{
  for(int i = 0; i < 25; i++)
    if(board[BLACK][i] > 0)
      return i;
  return 25;
}

std::vector<std::pair <int, int> > AI::genMovesHomeState(int p, int dice)
{
  std::vector<std::pair <int, int> > pos;
  std::pair<int, int> move;
  for(int i = p; i < 25; i++){
    if(board[BLACK][i] > 0){
      if(i+dice > 25 && getFirstCheckerPosition() == i){
        /*Pentru cazul in care zarul este destul de mare incat sa scoata piesa pe
        bara trebuie generata doar prima mutare*/
        move.first = i;
        move.second = dice;
        pos.push_back(move);
        break;
      }
      if((i+dice < 25 && board[WHITE][i+dice] < 2) || i+dice == 25){
        move.first = i;
        move.second = dice;
        pos.push_back(move);
      }
    }
  }
  return pos;
}

std::vector<std::pair <int, int> > AI::genMovesNormalState(int p, int dice)
{
  std::vector<std::pair <int, int> > pos;
  std::pair<int, int> step;
  for(int i = p; i < 25-dice; i++){
    if(board[BLACK][i] > 0 && board[WHITE][i+dice] < 2){
      step.first = i;
      step.second = dice;
      pos.push_back(step);
      
    }
  }
  return pos;
}

std::vector<std::pair<int, int> > AI::genMovesForOneDice(int p, int dice)
{
  if(getTableState() == Home)
    return genMovesHomeState(p, dice);
  if(getTableState() == Normal)
    return genMovesNormalState(p, dice);
  if(getTableState() == Out)
    return genMovesOutState(dice);
  return genMovesNormalState(p, dice);
}

std::vector<std::vector<std::pair <int, int> > >  AI::genPosDifferentDices()
{
  int dice1 = serv_msg[msg_size-2], dice2 = serv_msg[msg_size-1];
  std::vector<std::pair <int, int> > v1, v2, v3, v_aux;
  std::vector<std::vector<std::pair <int, int> > > V;

  v1 = genMovesForOneDice(1, dice1);
  for(unsigned int i = 0; i < v1.size(); i++){
    moveBlack(v1[i].first, v1[i].second, history);
    v2 = genMovesForOneDice(v1[i].first, dice2);
    for(unsigned int j = 0; j < v2.size(); j++){
      v_aux.clear();
      v_aux.push_back(v1[i]);
      v_aux.push_back(v2[j]);
      V.push_back(v_aux);
    }
    rollBack(history);
  }

  v3 = genMovesForOneDice(1, dice2);
  for(unsigned int i = 0; i < v3.size(); i++){
    moveBlack(v3[i].first, v3[i].second, history);
    v2 = genMovesForOneDice(v3[i].first, dice1);
    for(unsigned int j = 0; j < v2.size(); j++){
      v_aux.clear();
      v_aux.push_back(v3[i]);
      v_aux.push_back(v2[j]);
      V.push_back(v_aux);
    }
    rollBack(history);
  }
  if(V.empty())
  {
    v_aux.clear();
    for(unsigned int i = 0; i < v1.size(); i++){
      std::vector<std::pair<int, int> > v_aux;
      v_aux.push_back(v1[i]);
      V.push_back(v_aux);
    }
    for(unsigned int i = 0; i < v3.size(); i++){
      std::vector<std::pair<int, int> > v_aux;
      v_aux.push_back(v3[i]);
      V.push_back(v_aux);
    }
  }
  return V;
}

std::vector<std::vector<std::pair <int, int> > > AI::genPosDoubleDice()
{
  int dice = serv_msg[msg_size-1];
  std::vector<std::pair <int, int> > v1, v2, v3, v4, v_aux;
  std::vector<std::vector<std::pair <int, int> > > V;
  int history1[2][26], history2[2][26], history3[2][26];

  clearHistory(history1);
  clearHistory(history2);
  clearHistory(history3);

  v1 = genMovesForOneDice(1, dice);
  for(unsigned int i = 0; i < v1.size(); i++){
    v_aux.clear();
    v_aux.push_back(v1[i]);
    V.push_back(v_aux);
    moveBlack(v1[i].first, v1[i].second, history1);
    v2 = genMovesForOneDice(v1[i].first, v1[i].second);
    for(unsigned int j = 0; j < v2.size(); j++){
      v_aux.clear();
      v_aux.push_back(v1[i]);
      v_aux.push_back(v2[j]);
      V.push_back(v_aux);
      moveBlack(v2[j].first, v2[j].second, history2);
      v3 = genMovesForOneDice(v2[j].first, v2[j].second);
      for(unsigned int k = 0; k < v3.size(); k++){
        v_aux.clear();
        v_aux.push_back(v1[i]);
        v_aux.push_back(v2[j]);
        v_aux.push_back(v3[k]);
        V.push_back(v_aux);
        moveBlack(v3[k].first, v3[k].second, history3);
        v4 = genMovesForOneDice(v3[k].first, v3[k].second);
        for(unsigned int l = 0; l < v4.size(); l++){
          v_aux.clear();
          v_aux.push_back(v1[i]);
          v_aux.push_back(v2[j]);
          v_aux.push_back(v3[k]);
          v_aux.push_back(v4[l]);
          V.push_back(v_aux);
        }
        rollBack(history3);
      }
      rollBack(history2);
    }
    rollBack(history1);
  }
  unsigned int maxSize = 0;

  for(unsigned int i = 0; i < V.size(); i++){
    if(V[i].size() > maxSize)
      maxSize = V[i].size();
  }

  for(unsigned int i = 0; i < V.size(); i++){
    if(V[i].size() < maxSize){
      V.erase(V.begin() + i);
      i--;
    }
  }
  return V;
}

int AI::common()
{
  int p, common = 0;
  for(int i = 0; i < 25; i++){
    p = i;
    if(board[BLACK][i] > 0)
      break;
  }
  for(int i = p+1; i < 25; i++){
    if(board[WHITE][i] > 0)
      common += board[WHITE][i];
  }
  return common;
}

int AI::isClose(int piece)
{
  int nr = 0;

  for(int i = piece+1; i < MIN(24, piece+6); i++)
  {
    if(board[WHITE][i] > 0)
      nr++;
  }
  return nr;

}

int AI::countSingleCheckers()
{
  int nr = 0;
  for(int i = 1; i < 25; i++){
    if(board[BLACK][i] == 1){
      nr+=1;
    }
  }
  return nr;
}

inline int AI::outWhiteCheckers()
{
  return board[WHITE][25];
}

bool AI::isHomeGate(int color)
{
  int nr = 0;
  if(color == BLACK){
    for(int i = 19; i < 25; i++){
      if(board[BLACK][i] < 2)
        nr++;
    }
  }
  else{
    for(int i = 1; i < 7; i++){
      if(board[WHITE][i] < 2)
        nr++;
    }
  }

  if(nr < 3)
    return true;
  return false;
}

int AI::getScore(int color)
{
  int score = 0;
  
  for(int i = 1; i < 26; i++)
    if(board[color][i] > 1){
      if(common() == 0){
        score += 1000*i;
        continue;
      }
      if(isHomeGate(WHITE) && common()){
        if(i > 18)
          score += i*i*20*board[color][i];
        else score += i*i*10*board[color][i];
      }
      else{
        if(i > 18)
          score += i*20*board[color][i];
        else score += i*20*board[color][i];
      }
    }
  return score;
}

int AI::getCoverCount()
{
  int score = 0;
  for (int i = 1; i < 25; i++)
    if(board[BLACK][i] > 1)
      score++;
  for(int i = 1; i < 7; i++){
    if(board[BLACK][i] > 1)
      score -= (7-i)*board[BLACK][i];
  }
  return score;
}


std::vector<std::pair<int, int> > AI::getBestMove(std::vector<std::vector<std::pair <int, int> > > possibleMoves)
{
  std::vector<std::pair<int, int> > bestMove;
  int bestMoveScore = 0, moveScore, 
  nrSingle, bestSingle = INF, 
  bestCover = 0, cover,
  bestOut = 0, out;
  for(unsigned int i = 0; i < possibleMoves.size(); i++){

    for(unsigned int j = 0; j < possibleMoves[i].size(); j++)
      moveBlack(possibleMoves[i][j].first, possibleMoves[i][j].second, history);

    int c = common();

    nrSingle = countSingleCheckers();
    if(nrSingle < bestSingle && !isHomeGate() && c != 0)
    {
      bestOut = outWhiteCheckers();
      bestCover = getCoverCount();
      bestMoveScore = getScore(BLACK);
      bestMove = possibleMoves[i];
      bestSingle = nrSingle;
    }
    else{
      if(nrSingle == bestSingle || c == 0){
        out = outWhiteCheckers();
        if(out > bestOut && c != 0 ){
          bestCover = getCoverCount();
          bestMoveScore = getScore(BLACK);
          bestMove = possibleMoves[i];
          bestOut = out;
        }
        else{
          if(out == bestOut || c == 0){
            cover = getCoverCount();
            if(cover > bestCover && c != 0){
              bestMoveScore = getScore(BLACK);
              bestMove = possibleMoves[i];
              bestCover = cover;
            }
            else{
              if(cover == bestCover || c == 0){
                moveScore = getScore(BLACK);
                if(moveScore > bestMoveScore){
                  bestMove = possibleMoves[i];
                  bestMoveScore = moveScore;
                }
              }
            }
          }
        }
      }
    }

    if(isHomeGate() &&  c != 0){
      out = outWhiteCheckers();
      if(out > bestOut && c != 0){
        bestCover = getCoverCount();
        bestSingle = countSingleCheckers();
        bestMoveScore = getScore(BLACK);
        bestMove = possibleMoves[i];
        bestOut = out;
      }
      else{
        if(out == bestOut || c == 0){
          nrSingle = countSingleCheckers();
          if(nrSingle < bestSingle && c != 0){
            bestCover = getCoverCount();
            bestMoveScore = getScore(BLACK);
            bestMove = possibleMoves[i];
            bestSingle = nrSingle;
          }
          else{
            if(nrSingle == bestSingle || c == 0){
              cover = getCoverCount();
              if(cover > bestCover && c != 0){
                bestMoveScore = getScore(BLACK);
                bestMove = possibleMoves[i];
                bestCover = cover;
              }
              else{
                if(cover == bestCover || c == 0){
                  moveScore = getScore(BLACK);
                  if(moveScore > bestMoveScore){
                    bestMove = possibleMoves[i];
                    bestMoveScore = moveScore;
                  }
                }
              }
            }
          }
        }
      }
    }
    rollBack(history);
  }
  return bestMove;
}

void AI::mainloop()
{
  char buffer[100];
  while(1){
    link->readMessage(&msg_size, 1);
    link->readMessage(buffer, msg_size);
    strcpy(serv_msg, buffer);
    if(endGame()) 
      break;
    convert(this->serv_msg, true);

    syncronizeBoard();

    std::vector<std::vector<std::pair <int, int> > > dicePossibilities;
    dicePossibilities = 
      (serv_msg[msg_size-1] != serv_msg[msg_size-2])?genPosDifferentDices():genPosDoubleDice();
    std::vector<std::pair<int, int> > bestMove;
    bestMove = getBestMove(dicePossibilities);

    for(unsigned int i = 0; i < bestMove.size(); i++){
      moveBlack(bestMove[i].first, bestMove[i].second, history);
    }
    clearHistory(history);

    msg_size = bestMove.size()*2;
    buffer[0] = msg_size;
    for(unsigned int i = 0; i < bestMove.size(); i++){
      buffer[2*i+1] = bestMove[i].first;
      buffer[2*i+2] = bestMove[i].second;
    }
    msg_size++;

    convert(buffer, false);
    link->sendMessage(buffer, msg_size);
  }
  return;
}
