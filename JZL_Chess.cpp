//////////////////////////////////////////////
  //            AlphaJo_2.7.5            //
//                  by_JZL                  //
//////////////////////////////////////////////

#include <windows.h>
#include <iostream>
#include <fstream>


//////////////////////////////////////////////
//                 ������ɫ                 //
//////////////////////////////////////////////


void SetColor(unsigned short ForeColor,unsigned short BackGroundColor){
    HANDLE hCon=GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hCon,(ForeColor%16)|(BackGroundColor%16*16));
}



//////////////////////////////////////////////
//                 �����ʾ                 //
//////////////////////////////////////////////

#define  NOPIECE     0 //����
#define  KING        1 //��
#define  ARCHER      2 //������
#define  KNIGHT      3 //��ʿ
#define  FIGHTER     4 //����
#define  WHITE       0 //�׷�
#define  BLACK       1 //�ڷ�

#define MAX_GEN_MOVES    32 //�߷������������
#define SEARCH_DEPTH      7 //�����������
#define RANDOM_VALUE     10 //�������
#define INFINITY_VALUE  1000 //���þ����ֵ�����Ϊ1000

//ȫ�ֱ���
int currentPlayer = WHITE;   //��ʼ��Ϊ�׷����壬BLACK��ʾ�ڷ�����
int theMoves[MAX_GEN_MOVES]; //����һ���߷����������������ɵ������߷�
int bestMove;                //����߷����������
int theDepth;                //��ǰ�������

//��������(����������λ�ã��°��Ϻ�)
char board[64] = {
  0,  0,  0,  0,  0,  0,  0,  0,
  0, 18, 17, 16, 17, 18,  0,  0,
  0, 19, 19, 19, 19, 19,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,
  0, 11, 11, 11, 11, 11,  0,  0,
  0, 10,  9,  8,  9, 10,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0
};

//������������Ƿ��������ϵ�����
static const int isInBoard[64] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

//����������Ƿ���δ���ӺͲ����Ƿ���˵�����
static const int isAtHome[2][64] = {
  { //����(����)
    0, 0, 0, 0, 0, 0, 0, 8, //�±�Ϊ7��Ԫ�������жϲ����Ƿ����
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  }, { //����(����)
    0, 0, 0, 0, 0, 0, 0,-8, //�±�Ϊ7��Ԫ�������жϲ����Ƿ����
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  }
};

//�⼸������������ʾ���ӵ����ӷ���
static const char kingMovesTable[8] = {-8, 8, -1, 1, -9, -7, 9, 7}; //��
static const char archerMovesTable[4] = {-9, -7, 9, 7}; //����
static const char knightMovesTable[4] = {-8, 8, -1, 1}; //��ʿ
static const char fightMovesTable[4] = {-8, 8, -1, 1}; //����

//���ӵļ�ֵ����
static const char pieceValue[4] = {9, 5, 5, 3}; //��9 ��5 ��5 ��3

//�����������±�������X����
int getXFromLocation(int location){
  return (location & 7) - 1;
}

//�����������±�������Y����
int getYFromLocation(int location){
  return (location >> 3) - 1;
}

//������X���꣬Y���������������±�
int getLocationFromXY(int x, int y){
  return (x + 1) + (y + 1 << 3);
}

//�ж��Ƿ�Ϊ��������
int isMyPiece(char piece){
  int tag;
  tag = (currentPlayer << 3) + 8;
  return piece & tag;
}

//�ı����巽������0 ����1
void changePlayer(){
  currentPlayer = 1 - currentPlayer;
}

//////////////////////////////////////////////
//                 �߷�����                 //
//////////////////////////////////////////////

//�������Ϸ�һö���ӵĺ���
void addPiece(int location, char piece){
  board[location] = piece;
}

//������������һö���ӵĺ���
void delPiece(int location){
  board[location] = NOPIECE;
}

//�����߷������߷����λ��(���λ�õ����������±�)
int generateMoveFrom(int move){
  return move & 255;
}

//�����߷������߷�Ŀ��λ��(Ŀ��λ�õ����������±�)
int generateMoveTo(int move){
  return move >> 8;
}

//�����λ�ú�Ŀ��λ�úϳ��߷�
int composeMove(int locationFrom, int locationTo){
  return locationFrom + locationTo * 256;
}

//�߷����ɺ���������һ������������߷�������һ���߷��б�����ָ�룬�������ɵ������߷�
int generateAllMoves(int *moves){
  int i, genCount, locationFrom, locationTo, sideTag;
  char pieceFrom, pieceTo;

  //�߷�����������
  genCount = 0;
  //���巽���
  sideTag = (currentPlayer << 3) + 8;

  //���������ҵ���ǰ���巽����
  for(locationFrom = 0; locationFrom < 64; locationFrom ++){
    //ȡ�õ�ǰλ�õ�����
    pieceFrom = board[locationFrom];

    //1.����ҵ��Ĳ��Ǳ������ӣ�������
    if(!isMyPiece(pieceFrom)){
      continue;
    }

    //2.�ҵ����ӣ������������������߷�
    switch (pieceFrom - sideTag + 1){

      case KING:
        for(i = 0; i < 8; i++){
          locationTo = locationFrom + kingMovesTable[i];
          if(isAtHome[currentPlayer][locationTo]){ //Ŀ��λ���Ƿ�δ������
            pieceTo = board[locationTo];
            if(!isMyPiece(pieceTo)){ //Ŀ��λ���Ƿ��ޱ�������
              moves[genCount] = composeMove(locationFrom, locationTo); //�����߷�
              genCount++; //����
            }
          }
        }
        break;

      case ARCHER:
        for(i = 0; i < 4; i++){
          locationTo = locationFrom + archerMovesTable[i];
          if(isInBoard[locationTo]){ //Ŀ��λ���Ƿ���������
            pieceTo = board[locationTo];
            if(!isMyPiece(pieceTo)){ //Ŀ��λ���Ƿ��ޱ�������
              moves[genCount] = composeMove(locationFrom, locationTo); //�����߷�
              genCount++; //����
            }
          }
        }
        break;

      case KNIGHT:
        for(i = 0; i < 4; i++){
          locationTo = locationFrom + knightMovesTable[i];
          if(isInBoard[locationTo]){ //Ŀ��λ���Ƿ���������
            pieceTo = board[locationTo];
            if(!isMyPiece(pieceTo)){ //Ŀ��λ���Ƿ��ޱ�������
              moves[genCount] = composeMove(locationFrom, locationTo); //�����߷�
              genCount++; //����
            }
          }
        }
        break;

      case FIGHTER:
        for(i = 0; i < 4; i++){
          locationTo = locationFrom + kingMovesTable[i];
          if(isInBoard[locationTo]){ //Ŀ��λ���Ƿ���������
            if(locationFrom + isAtHome[currentPlayer][7] != locationTo){ //�����Ƿ����
              pieceTo = board[locationTo];
              if(!isMyPiece(pieceTo)){ //Ŀ��λ���Ƿ��ޱ�������
                moves[genCount] = composeMove(locationFrom, locationTo); //�����߷�
                genCount++; //����
              }
            }
          }
        }
        break;
    }
  }
  return genCount; //�������ɵ��߷���
}

//�ܸ����߷���һ����ĺ���
int makeOneMove(int move, char *captured){ //����һ�������������λ�õ�����
  int i, genCount, isLegalMove, locationFrom, locationTo;
  char pieceFrom;

  isLegalMove = 1; //��ʼ���߷�Ϊ���Ϸ�
  genCount = generateAllMoves(theMoves); //���������߷�
  //�������߷��в��ҵ�ǰ�߷��Ƿ����
  for(i = 0; i < genCount; i++){
    //����ҵ�һ���߷����ڵ�ǰ�߷�
    if(theMoves[i] == move){
      isLegalMove = 0; //�����߷����д��߷���˵���߷��Ϸ�
      break;
    }
  }

  //1.�����ж��߷��Ƿ�Ϸ�
  if(isLegalMove == 1){
    return 0; //��������ʧ��
  }

  //2.�ֽ��߷���ȡ�����λ�ú�Ŀ��λ��
  locationFrom = generateMoveFrom(move);
  locationTo = generateMoveTo(move);

  //3.ȡ��Ҫ�ߵ���
  pieceFrom = board[locationFrom];

  //4.���汻�Ե���
  *captured = board[locationTo];

  //5.�ƶ�����
  if(*captured != NOPIECE){
    delPiece(locationTo);
  }
  delPiece(locationFrom);
  addPiece(locationTo, pieceFrom);

  //6.�������巽
  changePlayer();

  return 1; //��������ɹ�
}

//������һ����ĺ���
void undoOneMove(int move, char captured){
  int locationFrom, locationTo;
  char pieceFrom;

  //1.�ֽ��߷���ȡ�����λ�ú�Ŀ��λ��
  locationFrom = generateMoveFrom(move);
  locationTo = generateMoveTo(move);

  //3.ȡ��Ҫ��ԭ����
  pieceFrom = board[locationTo];

  //4.�������巽
  changePlayer();

  //5.�ƶ�����
  delPiece(locationTo);   //ɾ������
  addPiece(locationFrom, pieceFrom); //��Ӽ�������
  if(captured != NOPIECE){
    addPiece(locationTo, captured);
  }
}

//////////////////////////////////////////////
//                 ��������                 //
//////////////////////////////////////////////

//�ж�ָ�����巽�Ƿ�ֳ�ʤ���ĺ���
int isThePlayerDie(int thePlayer){
  int i, theKing, isDie;

  //��ʼ�����
  isDie = 1;

  //ȡ�õ�ǰ���巽����
  theKing = (thePlayer << 3) + 8;

  //�����������߷������ѱ�������������1
  if(generateAllMoves(theMoves)){
    //������Ƿ���ȥ
    for(i = 0; i < 64; i ++){
      if(board[i] == theKing){
        isDie = 0;
      }
    }
  }

  return isDie;
}

//������������
int evaluatePosition(){

  int i, whiteValue, blackValue, value;

  //�Լ�����
  if(isThePlayerDie(currentPlayer)){
    return -INFINITY_VALUE + theDepth;
  }

  //��ʼ��˫�����ܼ�ֵ
  whiteValue = blackValue = 0;

  //�������̣��ҵ�����
  for(i = 0; i < 64; i++){
    if(board[i] <= 15 && board[i] >= 1){
      whiteValue += pieceValue[board[i] % 8];
    }
    if(board[i] >= 16){
      blackValue += pieceValue[board[i] % 8];
    }
  }

  //��������ֵ
  value = whiteValue - blackValue;

  return currentPlayer == WHITE ? value : -value;
}

//////////////////////////////////////////////
//                 ��������                 //
//////////////////////////////////////////////

//Alpha-Jo��������
int AlphaBetaSearch(int depth, int alpha, int beta){
  int i, genCount, value;
  int allMoves[MAX_GEN_MOVES];
  char captured;

  //���������ָ����ȣ��򷵻ؾ�������ֵ
  if(depth == 0){
    return evaluatePosition();
  }

  //�����ɱ�壬�͸��ݾ�ɱ��Ĳ�����������
  if(isThePlayerDie(currentPlayer)){ //�Լ�����
    return -INFINITY_VALUE + theDepth;
  }

  genCount = generateAllMoves(allMoves);

  for(i = 0; i < genCount; i++){
    if(makeOneMove(allMoves[i], &captured)){ //�������ɹ�
      theDepth++;
      value = -AlphaBetaSearch(depth - 1, -beta, -alpha); //�ݹ�
      undoOneMove(allMoves[i], captured); //��ԭ
      theDepth--;

      if(value >= beta){
        return beta;
      }
      if(value > alpha){
        alpha = value;
        if(theDepth == 0){ //����Ǹ��ڵ㱣������߷�
          bestMove = allMoves[i];
          alpha += (rand() & RANDOM_VALUE) - (rand() & RANDOM_VALUE); //�����
        }
      }
    }
  }

  return alpha;
}

//�õ�������
void computerThink(){
  char captured;
  theDepth = 0; //������ľ���
  AlphaBetaSearch(SEARCH_DEPTH, -INFINITY_VALUE, INFINITY_VALUE);
  makeOneMove(bestMove, &captured);
}

//����������������õ���(����)�ǰ׷������Ǻڷ�
int engine = BLACK;

//��ת����
int flipLocation(int location){
  location = 62 - location;
  return location;
}
////////////////////////////
//����
//////////////////////////
void showBoard(){


  std::cout << std::endl;
  std::cout << "                    y **********" << std::endl;
  for(int i = 0; i < 6; i++){
    std::cout << "                    " << i << " ";
    int location;
    for(int j = 0; j < 5; j++){
   	  location = getLocationFromXY(j, i); //ȡ�������±�

      if(engine == WHITE){ //�������ְ׷�����ת����λ��
        location = flipLocation(location);
      }
      int piece;
      piece = board[location];
      if(piece == 8){
        std::cout << "��";
      }else if(piece == 9){
        std::cout << "��";
      }else if(piece == 10){
        std::cout << "��";
      }else if(piece == 11){
        std::cout << "��";
      }else if(piece == 16){
        std::cout << "��";
      }else if(piece == 17){
        std::cout << "��";
      }else if(piece == 18){
        std::cout << "��";
      }else if(piece == 19){
        std::cout << "��";
      }else if(piece == NOPIECE){
        std::cout << "��";
      }else{
        std::cout << " ";
      }
    }
    std::cout << std::endl;
  }
  std::cout << "                      **********" << std::endl;
  std::cout << "                      0 1 2 3 4x" << std::endl;
}
/*
void fshowBoard(){
  std::ofstream outFile;
  outFile.open("qipu.txt");
  outFile << std::endl;
  outFile << "                    y **********" << std::endl;
  for(int i = 0; i < 6; i++){
    outFile << "                    " << i << " ";
    int location;
    for(int j = 0; j < 5; j++){
   	  location = getLocationFromXY(j, i); //ȡ�������±�

      if(engine == WHITE){ //�������ְ׷�����ת����λ��
        location = flipLocation(location);
      }
      int piece;
      piece = board[location];
      if(piece == 8){
        outFile << "��";
      }else if(piece == 9){
        outFile << "��";
      }else if(piece == 10){
        outFile << "��";
      }else if(piece == 11){
        outFile << "��";
      }else if(piece == 16){
        outFile << "��";
      }else if(piece == 17){
        outFile << "��";
      }else if(piece == 18){
        outFile << "��";
      }else if(piece == 19){
        outFile << "��";
      }else if(piece == NOPIECE){
        outFile << "��";
      }else{
        outFile << " ";
      }
    }
    outFile << std::endl;
  }
  outFile << "                      **********" << std::endl;
  outFile << "                      0 1 2 3 4x" << std::endl;
  outFile.close();
}
*/

int main(void){
  int move, from, to, fromX, fromY, toX, toY;
  char command, captured;
  system("color fc");
  MessageBox(NULL,"             Welcome to the world of\n                            MIND","AlphaJo",32);
  std::ofstream outFile;
  outFile.open("qipu.txt");
  //SetColor(1,2);
  std::cout << "/////////////////////////////\n";
  std::cout << "//    AlphaJo_2.7.5   //\n//     by Zhaolun_J     //\n";
  std::cout << "/////////////////////////////\n";
  std::cout << "Nice to see you,  o(��_��)o ";
  std::cout << std::endl << "��ʼ��\n\n";


  outFile << "/////////////////////////////\n";
  outFile << "//    AlphaJo_2.7.5   //\n//     by Zhaolun_J     //\n";
  outFile << "/////////////////////////////\n";
  outFile << "Nice to see you,  o(��_��)o ";
  outFile << std::endl << "��ʼ��\n\n";
  showBoard();
  ///////////////////////////////////////////////////////////////////////
  /////////////////////////�ļ����/////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  outFile << std::endl;
  outFile << "                    y **********" << std::endl;
  for(int i = 0; i < 6; i++){
    outFile << "                    " << i << " ";
    int location;
    for(int j = 0; j < 5; j++){
   	  location = getLocationFromXY(j, i); //ȡ�������±�

      if(engine == WHITE){ //�������ְ׷�����ת����λ��
        location = flipLocation(location);
      }
      int piece;
      piece = board[location];
      if(piece == 8){
        outFile << "��";
      }else if(piece == 9){
        outFile << "��";
      }else if(piece == 10){
        outFile << "��";
      }else if(piece == 11){
        outFile << "��";
      }else if(piece == 16){
        outFile << "��";
      }else if(piece == 17){
        outFile << "��";
      }else if(piece == 18){
        outFile << "��";
      }else if(piece == 19){
        outFile << "��";
      }else if(piece == NOPIECE){
        outFile << "��";
      }else{
        outFile << " ";
      }
    }
    outFile << std::endl;
  }
  outFile << "                      **********" << std::endl;
  outFile << "                      0 1 2 3 4x" << std::endl;
  //outFile.close();

////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

  while(1){

    std::cout << "\n����Q�˳�������D���壺";
    std::cin.get(command);
    std::cin.get();
    if(command == 'q' || command == 'Q'){
      return 0;
    }

    if(command == 'd' || command == 'D'){ //ѡ��������
      std::cout << std::endl << "����Wѡ����壬����Bѡ����壬����C�ۿ����Զ��ģ�";
      std::cin.get(command);
      std::cin.get();
      if(command == 'w' || command == 'W'){ //��ѡ���˰���
        engine = BLACK; //��������Ϊ����
        std::cout << "\n��ѡ���˰��壡\n";
        //////������װ1////////////////////////
        /////////////////////////////////////
        /////////////////////////////////////
        std::cout << "����������...10%";
        Sleep(1000);
        std::cout << "\r����������...60%";
        Sleep(1000);
        std::cout << "\r����������...99%";
        Sleep(1000);
        std::cout << "\r���óɹ���ս������3���ʼ��";
        Sleep(1000);
        std::cout << "\r���óɹ���ս������2���ʼ��";
        Sleep(1000);
        std::cout << "\r���óɹ���ս������1���ʼ��";
        Sleep(1000);
        std::cout << "\rս����ʼ������������������������";
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        //////////////////////////////////////////////////
      }else if(command == 'b' || command == 'B'){ //��ѡ���˺���
        engine = WHITE; //��������Ϊ����
        std::cout << "\n��ѡ���˺��壡\n";
        //////������װ1////////////////////////
        /////////////////////////////////////
        /////////////////////////////////////
        std::cout << "����������...50%";
        Sleep(1000);
        std::cout << "\r����������...80%";
        Sleep(1000);
        std::cout << "\r����������...100%";
        Sleep(1000);
        std::cout << "\r���óɹ���ս������3���ʼ��";
        Sleep(1000);
        std::cout << "\r���óɹ���ս������2���ʼ��";
        Sleep(1000);
        std::cout << "\r���óɹ���ս������1���ʼ��";
        Sleep(1000);
        std::cout << "\rս����ʼ������������������������";
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        //////////////////////////////////////////////////
      }else if(command == 'c' || command == 'C'){ //��ѡ���˺���
        engine = 2; //���õ��Զ��ı�־
        std::cout << "\n���Զ��Ŀ�ʼ��\n";
        //////������װ1////////////////////////
        /////////////////////////////////////
        /////////////////////////////////////
        std::cout << "����������...1%";
        Sleep(1000);
        std::cout << "\r����������...48%";
        Sleep(1000);
        std::cout << "\r����������...99%";
        Sleep(1000);
        std::cout << "\r���óɹ���ս������3���ʼ��";
        Sleep(1000);
        std::cout << "\r���óɹ���ս������2���ʼ��";
        Sleep(1000);
        std::cout << "\r���óɹ���ս������1���ʼ��";
        Sleep(1000);
        std::cout << "\rս����ʼ������������������������";
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        //////////////////////////////////////////////////
      }
      else{
        std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n������������ָʾ������\n\n\n\n";
        continue;
      }

      while(1){
        if(isThePlayerDie(currentPlayer) && currentPlayer != engine){
          std::cout << "\n������˼������ս�ܣ�����һ�������´򿪳���\n";
          std::cout << "\n����q�˳���";
          // std::cin.clear();
          std::cin.get(command);
          // std::cin.get();
          if(command == 'q' || command == 'Q'){
            return 0;
          }
        }else if(isThePlayerDie(currentPlayer) && currentPlayer == engine){
          std::cout << "\n��ϲ�㣬���Ա����ܣ�����һ�������´򿪳���\n";
          std::cout << "\n����q�˳���";
          //std::cin.clear();
          std::cin.get(command);
          //std::cin.get();
          if(command == 'q' || command == 'Q'){
            return 1;
          }
        }else{
          if(engine == WHITE){ //����Ϊ����
            showBoard();

            /////////////////////////////////////////////////////////////
            ///////////////////�ļ����////////////////////////////////////
            /////////////////////////////////////////////////////////////
            outFile << std::endl;
            outFile << "                    y **********" << std::endl;
            for(int i = 0; i < 6; i++){
              outFile << "                    " << i << " ";
              int location;
              for(int j = 0; j < 5; j++){
   	            location = getLocationFromXY(j, i); //ȡ�������±�

                if(engine == WHITE){ //�������ְ׷�����ת����λ��
                location = flipLocation(location);
                }
                int piece;
                piece = board[location];
               if(piece == 8){
                 outFile << "��";
               }else if(piece == 9){
                 outFile << "��";
               }else if(piece == 10){
                 outFile << "��";
               }else if(piece == 11){
                 outFile << "��";
               }else if(piece == 16){
                 outFile << "��";
               }else if(piece == 17){
                 outFile << "��";
               }else if(piece == 18){
                 outFile << "��";
               }else if(piece == 19){
                 outFile << "��";
               }else if(piece == NOPIECE){
                 outFile << "��";
               }else{
                 outFile << " ";
               }
             }
             outFile << std::endl;
           }
           outFile << "                      **********" << std::endl;
           outFile << "                      0 1 2 3 4x" << std::endl;
           // outFile.close();
             /////////////////////////////////////////////////////////////
             /////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////
            /////////////////////////////////////////////////////////////





            if(1){ //������Գְ׷�������
              std::cout << "\n����A����˼��...\n";
              computerThink();
              move = bestMove;
              from = generateMoveFrom(move); //�ֽ��߷�
              to = generateMoveTo(move);
              from = flipLocation(from); //��תλ��
              to = flipLocation(to);
              fromX = getXFromLocation(from); //ȡ������
			  fromY = getYFromLocation(from);
			  toX = getXFromLocation(to);
			  toY = getYFromLocation(to);
              std::cout << "\n�������ߵ����ǣ�from (" << fromX
                        << "," << fromY << ") to (" << toX << "," << toY << ")";
              showBoard();
              //////////////////////////////////////////////////////////////////////
              //////////////////////////////////�ļ����/////////////////////////////
              ///////////////////////////////////////////////////////////////////////
            outFile << std::endl;
            outFile << "                    y **********" << std::endl;
            for(int i = 0; i < 6; i++){
              outFile << "                    " << i << " ";
              int location;
              for(int j = 0; j < 5; j++){
             	  location = getLocationFromXY(j, i); //ȡ�������±�

                if(engine == WHITE){ //�������ְ׷�����ת����λ��
                  location = flipLocation(location);
                }
                int piece;
                piece = board[location];
                if(piece == 8){
                  outFile << "��";
                }else if(piece == 9){
                  outFile << "��";
                }else if(piece == 10){
                  outFile << "��";
                }else if(piece == 11){
                  outFile << "��";
                }else if(piece == 16){
                  outFile << "��";
                }else if(piece == 17){
                  outFile << "��";
                }else if(piece == 18){
                  outFile << "��";
                }else if(piece == 19){
                  outFile << "��";
                }else if(piece == NOPIECE){
                  outFile << "��";
                }else{
                  outFile << " ";
                }
              }
              outFile << std::endl;
            }
            outFile << "                      **********" << std::endl;
            outFile << "                      0 1 2 3 4x" << std::endl;
            // outFile.close();
            //////////////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////
            }
            if(isThePlayerDie(currentPlayer)){ //�������Ѱ�
              continue;
            }
            if(1){ //�ֵ�������
              std::cout << std::endl << "�ֵ������壬���������� x,y x,y ��";
              scanf("%d,%d %d,%d", &fromX, &fromY, &toX, &toY);
              std::cout << std::endl << "���ߵ����ǣ�from ("
                        << fromX << "," << fromY << ") to ("
                        << toX << toY << ")";
              from = getLocationFromXY(fromX, fromY); //�ϳ�λ��
              to = getLocationFromXY(toX, toY);
              from = flipLocation(from); //��תλ��
              to = flipLocation(to);
              move = composeMove(from, to); //�ϳ��߷�
              while(!makeOneMove(move, &captured)){
                std::cout << std::endl << "������������������������꣺";
                // scanf("%d,%d %d,%d", &fromX, &fromY, &toX, &toY);
                std::cin >> fromX;
                std::cin.get();
                std::cin >> fromY;
                std::cin >> toX;
                std::cin.get();
                std::cin >> toY;
                // std::cin.get();
                from = getLocationFromXY(fromX, fromY);//�ϳ�λ��
                to = getLocationFromXY(toX, toY);
                from = flipLocation(from); //��תλ��
                to = flipLocation(to);
                move = composeMove(from, to); //�ϳ��߷�
                getchar();
              }
            }
          }

          if(engine == BLACK){ //����Ϊ�ڷ�
            showBoard();
            //////////////////////////�ļ����////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////
            outFile << std::endl;
            outFile << "                    y **********" << std::endl;
            for(int i = 0; i < 6; i++){
              outFile << "                    " << i << " ";
              int location;
              for(int j = 0; j < 5; j++){
             	  location = getLocationFromXY(j, i); //ȡ�������±�

                if(engine == WHITE){ //�������ְ׷�����ת����λ��
                  location = flipLocation(location);
                }
                int piece;
                piece = board[location];
                if(piece == 8){
                  outFile << "��";
                }else if(piece == 9){
                  outFile << "��";
                }else if(piece == 10){
                  outFile << "��";
                }else if(piece == 11){
                  outFile << "��";
                }else if(piece == 16){
                  outFile << "��";
                }else if(piece == 17){
                  outFile << "��";
                }else if(piece == 18){
                  outFile << "��";
                }else if(piece == 19){
                  outFile << "��";
                }else if(piece == NOPIECE){
                  outFile << "��";
                }else{
                  outFile << " ";
                }
              }
              outFile << std::endl;
            }
            outFile << "                      **********" << std::endl;
            outFile << "                      0 1 2 3 4x" << std::endl;
            // outFile.close();
  /////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
            if(1){ //�ֵ�������
              std::cout << std::endl << "�ֵ������壬���������� x,y x,y ��";
              // scanf("%d,%d %d,%d", &fromX, &fromY, &toX, &toY);
              std::cin >> fromX;
              std::cin.get();
              std::cin >> fromY;
              std::cin >> toX;
              std::cin.get();
              std::cin >> toY;
              std::cout << std::endl << "���ߵ����ǣ�from (" << fromX << ", " << fromY
                        << " to (" << toX << ", " << toY << ")"
                        << std::endl;
              from = getLocationFromXY(fromX, fromY);//�ϳ�λ��
              to = getLocationFromXY(toX, toY);
              move = composeMove(from, to); //�ϳ��߷�
              while(!makeOneMove(move, &captured)){
                // std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n������������������������꣺";
                std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
                showBoard();///////////////////////////////////////////////////////////////////////////
                std::cout << "������������������������꣺";
                MessageBox(NULL,"�����������","AlphaJo",1);
                // scanf("%d,%d %d,%d", &fromX, &fromY, &toX, &toY);
                std::cin >> fromX;
                std::cin.get();
                std::cin >> fromY;
                std::cin >> toX;
                std::cin.get();
                std::cin >> toY;
                from = getLocationFromXY(fromX, fromY);//�ϳ�λ��
                to = getLocationFromXY(toX, toY);
                move = composeMove(from, to); //�ϳ��߷�
                getchar();

              }
              showBoard();
            }
            if(isThePlayerDie(currentPlayer)){ //��������Ѱ�
              continue;
            }
            if(1){ //������Ժڷ������
              // std::cout << std::endl << "��������˼��..." << std::endl;
              //////////������װ2///////////////////////////
              /////////////////////////////////////////////
              /////////////////////////////////////////////
              std::cout << std::endl << "��������˼��...1%";
              for (int v = 1; v < 101; v++){
                  std::cout << "\r��������˼��...." ;
                  std::cout << v;
                  std::cout << "%";
                  Sleep(10);
              }
              /////////////////////////////////////////////
              //////////////////////////////////////////////
              ///////////////////////////////////////////////
              computerThink();
              move = bestMove;
              from = generateMoveFrom(move); //�ֽ��߷�
              to = generateMoveTo(move);
              fromX = getXFromLocation(from); //ȡ������
			  fromY = getYFromLocation(from);
			  toX = getXFromLocation(to);
			  toY = getYFromLocation(to);
              std::cout << std::endl << "�������ߵ����ǣ�from (" << fromX << ","
                        << fromY << ") to (" << toX << "," << toY << ")"
                        << std::endl;
            }
          }

          if(engine == 2){ //�����ս��־
            showBoard();
            if(1){ //����A������
              std::cout << "\n��������˼��...";
              //////////������װ2///////////////////////////
              /////////////////////////////////////////////
              /////////////////////////////////////////////
              std::cout << std::endl << "��������˼��...1%";
              for (int v = 1; v < 101; v++){
                  std::cout << "\r��������˼��....";
                  std::cout << v;
                  std::cout  << "%";
                  Sleep(10);
              }
              ///////////////////////////////////////////////
              ///////////////////////////////////////////////
              ////////////////////////////////////////////
              computerThink();
              move = bestMove;
              from = generateMoveFrom(move); //�ֽ��߷�
              to = generateMoveTo(move);
              fromX = getXFromLocation(from); //ȡ������
			  fromY = getYFromLocation(from);
			  toX = getXFromLocation(to);
			  toY = getYFromLocation(to);
              // printf("\n����A���ߵ����ǣ�from (%d,%d) to (%d,%d)\n", fromX, fromY, toX, toY);
              std::cout << std::endl
                        << "����B���ߵ����ǣ�from ("
                        << fromX << "," << fromY << ") to (" << toX << "," << toY << ")";
              showBoard();
              //////////////////////////�ļ����////////////////////////////////////////
              //////////////////////////////////////////////////////////////////////////
              /////////////////////////////////////////////////////////////////////////
            outFile << std::endl;
            outFile << "                    y **********" << std::endl;
            for(int i = 0; i < 6; i++){
              outFile << "                    " << i << " ";
              int location;
              for(int j = 0; j < 5; j++){
             	  location = getLocationFromXY(j, i); //ȡ�������±�

                if(engine == WHITE){ //�������ְ׷�����ת����λ��
                  location = flipLocation(location);
                }
                int piece;
                piece = board[location];
                if(piece == 8){
                  outFile << "��";
                }else if(piece == 9){
                  outFile << "��";
                }else if(piece == 10){
                  outFile << "��";
                }else if(piece == 11){
                  outFile << "��";
                }else if(piece == 16){
                  outFile << "��";
                }else if(piece == 17){
                  outFile << "��";
                }else if(piece == 18){
                  outFile << "��";
                }else if(piece == 19){
                  outFile << "��";
                }else if(piece == NOPIECE){
                  outFile << "��";
                }else{
                  outFile << " ";
                }
              }
              outFile << std::endl;
            }
            outFile << "                      **********" << std::endl;
            outFile << "                      0 1 2 3 4x" << std::endl;
            // outFile.close();
            }
            if(isThePlayerDie(currentPlayer)){ //�������Ѱ�
              continue;
            }
            if(1){ //�ֵ�����B����
              std::cout << std::endl << "����B����˼��..." << std::endl;
              //////////������װ2///////////////////////////
              /////////////////////////////////////////////
              /////////////////////////////////////////////
              std::cout << std::endl << "��������˼��...1%";
              for (int v = 1; v < 101; v++)
              {
                  std::cout << "\r��������˼��....";
                  std::cout << v;
                  std::cout << "%";
                  Sleep(10);
              }
              /////////////////////////////////////////////
              //////////////////////////////////////////////
              ///////////////////////////////////////////////
              computerThink();
              move = bestMove;
              from = generateMoveFrom(move); //�ֽ��߷�
              to = generateMoveTo(move);
              fromX = getXFromLocation(from); //ȡ������
			  fromY = getYFromLocation(from);
			  toX = getXFromLocation(to);
			  toY = getYFromLocation(to);
              std::cout << std::endl
                        << "����B���ߵ����ǣ�from ("
                        << fromX << "," << fromY << ") to (" << toX << "," << toY << ")";
            }
          }
        }
      }
    } else{
      std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nd�ü����س��ٰ�D��������������\n";
    }
  }
  outFile.close();
  return 0;
}
