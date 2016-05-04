//////////////////////////////////////////////
  //            AlphaJo_2.7.5            //
//                  by_JZL                  //
//////////////////////////////////////////////

#include <windows.h>
#include <iostream>
#include <fstream>


//////////////////////////////////////////////
//                 设置颜色                 //
//////////////////////////////////////////////


void SetColor(unsigned short ForeColor,unsigned short BackGroundColor){
    HANDLE hCon=GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hCon,(ForeColor%16)|(BackGroundColor%16*16));
}



//////////////////////////////////////////////
//                 局面表示                 //
//////////////////////////////////////////////

#define  NOPIECE     0 //无子
#define  KING        1 //王
#define  ARCHER      2 //弓箭手
#define  KNIGHT      3 //骑士
#define  FIGHTER     4 //步兵
#define  WHITE       0 //白方
#define  BLACK       1 //黑方

#define MAX_GEN_MOVES    32 //走法的最大生成数
#define SEARCH_DEPTH      7 //设置搜索深度
#define RANDOM_VALUE     10 //随机因子
#define INFINITY_VALUE  1000 //设置局面价值无穷大为1000

//全局变量
int currentPlayer = WHITE;   //初始化为白方走棋，BLACK表示黑方走棋
int theMoves[MAX_GEN_MOVES]; //定义一个走法数组用来保存生成的所有走法
int bestMove;                //最佳走法，搜索结果
int theDepth;                //当前搜索深度

//棋盘数组(带开局棋子位置，下白上黑)
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

//用来标记棋子是否在棋盘上的数组
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

//用来标记王是否在未过河和步兵是否后退的数组
static const int isAtHome[2][64] = {
  { //白王(步兵)
    0, 0, 0, 0, 0, 0, 0, 8, //下标为7的元素用来判断步兵是否后退
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  }, { //黑王(步兵)
    0, 0, 0, 0, 0, 0, 0,-8, //下标为7的元素用来判断步兵是否后退
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  }
};

//这几个数组用来表示棋子的走子方向
static const char kingMovesTable[8] = {-8, 8, -1, 1, -9, -7, 9, 7}; //王
static const char archerMovesTable[4] = {-9, -7, 9, 7}; //弓手
static const char knightMovesTable[4] = {-8, 8, -1, 1}; //骑士
static const char fightMovesTable[4] = {-8, 8, -1, 1}; //步兵

//棋子的价值数组
static const char pieceValue[4] = {9, 5, 5, 3}; //王9 弓5 骑5 步3

//由棋盘数组下标获得棋子X坐标
int getXFromLocation(int location){
  return (location & 7) - 1;
}

//由棋盘数组下标获得棋子Y坐标
int getYFromLocation(int location){
  return (location >> 3) - 1;
}

//由棋子X坐标，Y坐标获得棋盘数组下标
int getLocationFromXY(int x, int y){
  return (x + 1) + (y + 1 << 3);
}

//判断是否为己方棋子
int isMyPiece(char piece){
  int tag;
  tag = (currentPlayer << 3) + 8;
  return piece & tag;
}

//改变走棋方，不是0 就是1
void changePlayer(){
  currentPlayer = 1 - currentPlayer;
}

//////////////////////////////////////////////
//                 走法生成                 //
//////////////////////////////////////////////

//在棋盘上放一枚棋子的函数
void addPiece(int location, char piece){
  board[location] = piece;
}

//在棋盘上拿走一枚棋子的函数
void delPiece(int location){
  board[location] = NOPIECE;
}

//根据走法生成走法起点位置(起点位置的棋盘数组下标)
int generateMoveFrom(int move){
  return move & 255;
}

//根据走法生成走法目的位置(目的位置的棋盘数组下标)
int generateMoveTo(int move){
  return move >> 8;
}

//由起点位置和目的位置合成走法
int composeMove(int locationFrom, int locationTo){
  return locationFrom + locationTo * 256;
}

//走法生成函数，产生一个局面的所有走法，传递一个走法列表数组指针，返回生成的所有走法
int generateAllMoves(int *moves){
  int i, genCount, locationFrom, locationTo, sideTag;
  char pieceFrom, pieceTo;

  //走法计数器清零
  genCount = 0;
  //走棋方标记
  sideTag = (currentPlayer << 3) + 8;

  //遍历棋盘找到当前走棋方棋子
  for(locationFrom = 0; locationFrom < 64; locationFrom ++){
    //取得当前位置的棋子
    pieceFrom = board[locationFrom];

    //1.如果找到的不是本方棋子，继续找
    if(!isMyPiece(pieceFrom)){
      continue;
    }

    //2.找到棋子，根据棋子类型生成走法
    switch (pieceFrom - sideTag + 1){

      case KING:
        for(i = 0; i < 8; i++){
          locationTo = locationFrom + kingMovesTable[i];
          if(isAtHome[currentPlayer][locationTo]){ //目标位置是否未出国界
            pieceTo = board[locationTo];
            if(!isMyPiece(pieceTo)){ //目标位置是否无本方棋子
              moves[genCount] = composeMove(locationFrom, locationTo); //保存走法
              genCount++; //计数
            }
          }
        }
        break;

      case ARCHER:
        for(i = 0; i < 4; i++){
          locationTo = locationFrom + archerMovesTable[i];
          if(isInBoard[locationTo]){ //目标位置是否在棋盘内
            pieceTo = board[locationTo];
            if(!isMyPiece(pieceTo)){ //目标位置是否无本方棋子
              moves[genCount] = composeMove(locationFrom, locationTo); //保存走法
              genCount++; //计数
            }
          }
        }
        break;

      case KNIGHT:
        for(i = 0; i < 4; i++){
          locationTo = locationFrom + knightMovesTable[i];
          if(isInBoard[locationTo]){ //目标位置是否在棋盘内
            pieceTo = board[locationTo];
            if(!isMyPiece(pieceTo)){ //目标位置是否无本方棋子
              moves[genCount] = composeMove(locationFrom, locationTo); //保存走法
              genCount++; //计数
            }
          }
        }
        break;

      case FIGHTER:
        for(i = 0; i < 4; i++){
          locationTo = locationFrom + kingMovesTable[i];
          if(isInBoard[locationTo]){ //目标位置是否在棋盘内
            if(locationFrom + isAtHome[currentPlayer][7] != locationTo){ //检查兵是否后退
              pieceTo = board[locationTo];
              if(!isMyPiece(pieceTo)){ //目标位置是否无本方棋子
                moves[genCount] = composeMove(locationFrom, locationTo); //保存走法
                genCount++; //计数
              }
            }
          }
        }
        break;
    }
  }
  return genCount; //返回生成的走法数
}

//能根据走法走一步棋的函数
int makeOneMove(int move, char *captured){ //传递一个用来保存吃子位置的数组
  int i, genCount, isLegalMove, locationFrom, locationTo;
  char pieceFrom;

  isLegalMove = 1; //初始化走法为不合法
  genCount = generateAllMoves(theMoves); //生成所有走法
  //在所有走法中查找当前走法是否存在
  for(i = 0; i < genCount; i++){
    //如果找到一个走法等于当前走法
    if(theMoves[i] == move){
      isLegalMove = 0; //所有走法中有此走法，说明走法合法
      break;
    }
  }

  //1.首先判断走法是否合法
  if(isLegalMove == 1){
    return 0; //返回走棋失败
  }

  //2.分解走法，取得起点位置和目的位置
  locationFrom = generateMoveFrom(move);
  locationTo = generateMoveTo(move);

  //3.取得要走的子
  pieceFrom = board[locationFrom];

  //4.保存被吃的子
  *captured = board[locationTo];

  //5.移动棋子
  if(*captured != NOPIECE){
    delPiece(locationTo);
  }
  delPiece(locationFrom);
  addPiece(locationTo, pieceFrom);

  //6.交换走棋方
  changePlayer();

  return 1; //返回走棋成功
}

//撤销走一步棋的函数
void undoOneMove(int move, char captured){
  int locationFrom, locationTo;
  char pieceFrom;

  //1.分解走法，取得起点位置和目的位置
  locationFrom = generateMoveFrom(move);
  locationTo = generateMoveTo(move);

  //3.取得要还原的子
  pieceFrom = board[locationTo];

  //4.交换走棋方
  changePlayer();

  //5.移动棋子
  delPiece(locationTo);   //删除棋子
  addPiece(locationFrom, pieceFrom); //添加己方棋子
  if(captured != NOPIECE){
    addPiece(locationTo, captured);
  }
}

//////////////////////////////////////////////
//                 局面评估                 //
//////////////////////////////////////////////

//判断指定走棋方是否分出胜负的函数
int isThePlayerDie(int thePlayer){
  int i, theKing, isDie;

  //初始化标记
  isDie = 1;

  //取得当前走棋方的王
  theKing = (thePlayer << 3) + 8;

  //如果生成零个走法，则已被困死，将返回1
  if(generateAllMoves(theMoves)){
    //检测王是否死去
    for(i = 0; i < 64; i ++){
      if(board[i] == theKing){
        isDie = 0;
      }
    }
  }

  return isDie;
}

//局面评估函数
int evaluatePosition(){

  int i, whiteValue, blackValue, value;

  //自己已死
  if(isThePlayerDie(currentPlayer)){
    return -INFINITY_VALUE + theDepth;
  }

  //初始化双方的总价值
  whiteValue = blackValue = 0;

  //遍历棋盘，找到棋子
  for(i = 0; i < 64; i++){
    if(board[i] <= 15 && board[i] >= 1){
      whiteValue += pieceValue[board[i] % 8];
    }
    if(board[i] >= 16){
      blackValue += pieceValue[board[i] % 8];
    }
  }

  //计算局面价值
  value = whiteValue - blackValue;

  return currentPlayer == WHITE ? value : -value;
}

//////////////////////////////////////////////
//                 局面搜索                 //
//////////////////////////////////////////////

//Alpha-Jo搜索函数
int AlphaBetaSearch(int depth, int alpha, int beta){
  int i, genCount, value;
  int allMoves[MAX_GEN_MOVES];
  char captured;

  //如果搜索到指定深度，则返回局面评估值
  if(depth == 0){
    return evaluatePosition();
  }

  //如果是杀棋，就根据距杀棋的步数给出评价
  if(isThePlayerDie(currentPlayer)){ //自己已死
    return -INFINITY_VALUE + theDepth;
  }

  genCount = generateAllMoves(allMoves);

  for(i = 0; i < genCount; i++){
    if(makeOneMove(allMoves[i], &captured)){ //如果走棋成功
      theDepth++;
      value = -AlphaBetaSearch(depth - 1, -beta, -alpha); //递归
      undoOneMove(allMoves[i], captured); //还原
      theDepth--;

      if(value >= beta){
        return beta;
      }
      if(value > alpha){
        alpha = value;
        if(theDepth == 0){ //如果是根节点保存最佳走法
          bestMove = allMoves[i];
          alpha += (rand() & RANDOM_VALUE) - (rand() & RANDOM_VALUE); //随机性
        }
      }
    }
  }

  return alpha;
}

//让电脑走棋
void computerThink(){
  char captured;
  theDepth = 0; //距根结点的距离
  AlphaBetaSearch(SEARCH_DEPTH, -INFINITY_VALUE, INFINITY_VALUE);
  makeOneMove(bestMove, &captured);
}

//定义个变量用来设置电脑(引擎)是白方或者是黑方
int engine = BLACK;

//翻转棋盘
int flipLocation(int location){
  location = 62 - location;
  return location;
}
////////////////////////////
//界面
//////////////////////////
void showBoard(){


  std::cout << std::endl;
  std::cout << "                    y **********" << std::endl;
  for(int i = 0; i < 6; i++){
    std::cout << "                    " << i << " ";
    int location;
    for(int j = 0; j < 5; j++){
   	  location = getLocationFromXY(j, i); //取得数组下标

      if(engine == WHITE){ //如果引擎持白方，翻转棋盘位置
        location = flipLocation(location);
      }
      int piece;
      piece = board[location];
      if(piece == 8){
        std::cout << "★";
      }else if(piece == 9){
        std::cout << "◆";
      }else if(piece == 10){
        std::cout << "■";
      }else if(piece == 11){
        std::cout << "●";
      }else if(piece == 16){
        std::cout << "☆";
      }else if(piece == 17){
        std::cout << "◇";
      }else if(piece == 18){
        std::cout << "□";
      }else if(piece == 19){
        std::cout << "○";
      }else if(piece == NOPIECE){
        std::cout << "╋";
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
   	  location = getLocationFromXY(j, i); //取得数组下标

      if(engine == WHITE){ //如果引擎持白方，翻转棋盘位置
        location = flipLocation(location);
      }
      int piece;
      piece = board[location];
      if(piece == 8){
        outFile << "★";
      }else if(piece == 9){
        outFile << "◆";
      }else if(piece == 10){
        outFile << "■";
      }else if(piece == 11){
        outFile << "●";
      }else if(piece == 16){
        outFile << "☆";
      }else if(piece == 17){
        outFile << "◇";
      }else if(piece == 18){
        outFile << "□";
      }else if(piece == 19){
        outFile << "○";
      }else if(piece == NOPIECE){
        outFile << "╋";
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
  std::cout << "Nice to see you,  o(∩_∩)o ";
  std::cout << std::endl << "开始：\n\n";


  outFile << "/////////////////////////////\n";
  outFile << "//    AlphaJo_2.7.5   //\n//     by Zhaolun_J     //\n";
  outFile << "/////////////////////////////\n";
  outFile << "Nice to see you,  o(∩_∩)o ";
  outFile << std::endl << "开始：\n\n";
  showBoard();
  ///////////////////////////////////////////////////////////////////////
  /////////////////////////文件输出/////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  outFile << std::endl;
  outFile << "                    y **********" << std::endl;
  for(int i = 0; i < 6; i++){
    outFile << "                    " << i << " ";
    int location;
    for(int j = 0; j < 5; j++){
   	  location = getLocationFromXY(j, i); //取得数组下标

      if(engine == WHITE){ //如果引擎持白方，翻转棋盘位置
        location = flipLocation(location);
      }
      int piece;
      piece = board[location];
      if(piece == 8){
        outFile << "★";
      }else if(piece == 9){
        outFile << "◆";
      }else if(piece == 10){
        outFile << "■";
      }else if(piece == 11){
        outFile << "●";
      }else if(piece == 16){
        outFile << "☆";
      }else if(piece == 17){
        outFile << "◇";
      }else if(piece == 18){
        outFile << "□";
      }else if(piece == 19){
        outFile << "○";
      }else if(piece == NOPIECE){
        outFile << "╋";
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

    std::cout << "\n输入Q退出，输入D下棋：";
    std::cin.get(command);
    std::cin.get();
    if(command == 'q' || command == 'Q'){
      return 0;
    }

    if(command == 'd' || command == 'D'){ //选择了下棋
      std::cout << std::endl << "输入W选择白棋，输入B选择黑棋，输入C观看电脑对弈：";
      std::cin.get(command);
      std::cin.get();
      if(command == 'w' || command == 'W'){ //人选择了白棋
        engine = BLACK; //设置引擎为黑棋
        std::cout << "\n您选择了白棋！\n";
        //////配置套装1////////////////////////
        /////////////////////////////////////
        /////////////////////////////////////
        std::cout << "引擎配置中...10%";
        Sleep(1000);
        std::cout << "\r引擎配置中...60%";
        Sleep(1000);
        std::cout << "\r引擎配置中...99%";
        Sleep(1000);
        std::cout << "\r配置成功！战斗将在3秒后开始！";
        Sleep(1000);
        std::cout << "\r配置成功！战斗将在2秒后开始！";
        Sleep(1000);
        std::cout << "\r配置成功！战斗将在1秒后开始！";
        Sleep(1000);
        std::cout << "\r战斗开始！！！！！！！！！！！！";
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        //////////////////////////////////////////////////
      }else if(command == 'b' || command == 'B'){ //人选择了黑棋
        engine = WHITE; //设置引擎为白棋
        std::cout << "\n您选择了黑棋！\n";
        //////配置套装1////////////////////////
        /////////////////////////////////////
        /////////////////////////////////////
        std::cout << "引擎配置中...50%";
        Sleep(1000);
        std::cout << "\r引擎配置中...80%";
        Sleep(1000);
        std::cout << "\r引擎配置中...100%";
        Sleep(1000);
        std::cout << "\r配置成功！战斗将在3秒后开始！";
        Sleep(1000);
        std::cout << "\r配置成功！战斗将在2秒后开始！";
        Sleep(1000);
        std::cout << "\r配置成功！战斗将在1秒后开始！";
        Sleep(1000);
        std::cout << "\r战斗开始！！！！！！！！！！！！";
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        //////////////////////////////////////////////////
      }else if(command == 'c' || command == 'C'){ //人选择了黑棋
        engine = 2; //设置电脑对弈标志
        std::cout << "\n电脑对弈开始！\n";
        //////配置套装1////////////////////////
        /////////////////////////////////////
        /////////////////////////////////////
        std::cout << "引擎配置中...1%";
        Sleep(1000);
        std::cout << "\r引擎配置中...48%";
        Sleep(1000);
        std::cout << "\r引擎配置中...99%";
        Sleep(1000);
        std::cout << "\r配置成功！战斗将在3秒后开始！";
        Sleep(1000);
        std::cout << "\r配置成功！战斗将在2秒后开始！";
        Sleep(1000);
        std::cout << "\r配置成功！战斗将在1秒后开始！";
        Sleep(1000);
        std::cout << "\r战斗开始！！！！！！！！！！！！";
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        //////////////////////////////////////////////////
      }
      else{
        std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n重来重来，看指示来阿亲\n\n\n\n";
        continue;
      }

      while(1){
        if(isThePlayerDie(currentPlayer) && currentPlayer != engine){
          std::cout << "\n不好意思，你已战败！再来一局请重新打开程序！\n";
          std::cout << "\n输入q退出：";
          // std::cin.clear();
          std::cin.get(command);
          // std::cin.get();
          if(command == 'q' || command == 'Q'){
            return 0;
          }
        }else if(isThePlayerDie(currentPlayer) && currentPlayer == engine){
          std::cout << "\n恭喜你，电脑被你打败！再来一局请重新打开程序！\n";
          std::cout << "\n输入q退出：";
          //std::cin.clear();
          std::cin.get(command);
          //std::cin.get();
          if(command == 'q' || command == 'Q'){
            return 1;
          }
        }else{
          if(engine == WHITE){ //引擎为白棋
            showBoard();

            /////////////////////////////////////////////////////////////
            ///////////////////文件输出////////////////////////////////////
            /////////////////////////////////////////////////////////////
            outFile << std::endl;
            outFile << "                    y **********" << std::endl;
            for(int i = 0; i < 6; i++){
              outFile << "                    " << i << " ";
              int location;
              for(int j = 0; j < 5; j++){
   	            location = getLocationFromXY(j, i); //取得数组下标

                if(engine == WHITE){ //如果引擎持白方，翻转棋盘位置
                location = flipLocation(location);
                }
                int piece;
                piece = board[location];
               if(piece == 8){
                 outFile << "★";
               }else if(piece == 9){
                 outFile << "◆";
               }else if(piece == 10){
                 outFile << "■";
               }else if(piece == 11){
                 outFile << "●";
               }else if(piece == 16){
                 outFile << "☆";
               }else if(piece == 17){
                 outFile << "◇";
               }else if(piece == 18){
                 outFile << "□";
               }else if(piece == 19){
                 outFile << "○";
               }else if(piece == NOPIECE){
                 outFile << "╋";
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





            if(1){ //如果电脑持白方则先走
              std::cout << "\n电脑A正在思考...\n";
              computerThink();
              move = bestMove;
              from = generateMoveFrom(move); //分解走法
              to = generateMoveTo(move);
              from = flipLocation(from); //翻转位置
              to = flipLocation(to);
              fromX = getXFromLocation(from); //取得坐标
			  fromY = getYFromLocation(from);
			  toX = getXFromLocation(to);
			  toY = getYFromLocation(to);
              std::cout << "\n电脑所走的棋是：from (" << fromX
                        << "," << fromY << ") to (" << toX << "," << toY << ")";
              showBoard();
              //////////////////////////////////////////////////////////////////////
              //////////////////////////////////文件输出/////////////////////////////
              ///////////////////////////////////////////////////////////////////////
            outFile << std::endl;
            outFile << "                    y **********" << std::endl;
            for(int i = 0; i < 6; i++){
              outFile << "                    " << i << " ";
              int location;
              for(int j = 0; j < 5; j++){
             	  location = getLocationFromXY(j, i); //取得数组下标

                if(engine == WHITE){ //如果引擎持白方，翻转棋盘位置
                  location = flipLocation(location);
                }
                int piece;
                piece = board[location];
                if(piece == 8){
                  outFile << "★";
                }else if(piece == 9){
                  outFile << "◆";
                }else if(piece == 10){
                  outFile << "■";
                }else if(piece == 11){
                  outFile << "●";
                }else if(piece == 16){
                  outFile << "☆";
                }else if(piece == 17){
                  outFile << "◇";
                }else if(piece == 18){
                  outFile << "□";
                }else if(piece == 19){
                  outFile << "○";
                }else if(piece == NOPIECE){
                  outFile << "╋";
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
            if(isThePlayerDie(currentPlayer)){ //如果玩家已败
              continue;
            }
            if(1){ //轮到人走棋
              std::cout << std::endl << "轮到您下棋，请输入坐标 x,y x,y ：";
              scanf("%d,%d %d,%d", &fromX, &fromY, &toX, &toY);
              std::cout << std::endl << "您走的棋是：from ("
                        << fromX << "," << fromY << ") to ("
                        << toX << toY << ")";
              from = getLocationFromXY(fromX, fromY); //合成位置
              to = getLocationFromXY(toX, toY);
              from = flipLocation(from); //翻转位置
              to = flipLocation(to);
              move = composeMove(from, to); //合成走法
              while(!makeOneMove(move, &captured)){
                std::cout << std::endl << "输入坐标错误，请重新输入坐标：";
                // scanf("%d,%d %d,%d", &fromX, &fromY, &toX, &toY);
                std::cin >> fromX;
                std::cin.get();
                std::cin >> fromY;
                std::cin >> toX;
                std::cin.get();
                std::cin >> toY;
                // std::cin.get();
                from = getLocationFromXY(fromX, fromY);//合成位置
                to = getLocationFromXY(toX, toY);
                from = flipLocation(from); //翻转位置
                to = flipLocation(to);
                move = composeMove(from, to); //合成走法
                getchar();
              }
            }
          }

          if(engine == BLACK){ //引擎为黑方
            showBoard();
            //////////////////////////文件输出////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////
            outFile << std::endl;
            outFile << "                    y **********" << std::endl;
            for(int i = 0; i < 6; i++){
              outFile << "                    " << i << " ";
              int location;
              for(int j = 0; j < 5; j++){
             	  location = getLocationFromXY(j, i); //取得数组下标

                if(engine == WHITE){ //如果引擎持白方，翻转棋盘位置
                  location = flipLocation(location);
                }
                int piece;
                piece = board[location];
                if(piece == 8){
                  outFile << "★";
                }else if(piece == 9){
                  outFile << "◆";
                }else if(piece == 10){
                  outFile << "■";
                }else if(piece == 11){
                  outFile << "●";
                }else if(piece == 16){
                  outFile << "☆";
                }else if(piece == 17){
                  outFile << "◇";
                }else if(piece == 18){
                  outFile << "□";
                }else if(piece == 19){
                  outFile << "○";
                }else if(piece == NOPIECE){
                  outFile << "╋";
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
            if(1){ //轮到人走棋
              std::cout << std::endl << "轮到您下棋，请输入坐标 x,y x,y ：";
              // scanf("%d,%d %d,%d", &fromX, &fromY, &toX, &toY);
              std::cin >> fromX;
              std::cin.get();
              std::cin >> fromY;
              std::cin >> toX;
              std::cin.get();
              std::cin >> toY;
              std::cout << std::endl << "您走的棋是：from (" << fromX << ", " << fromY
                        << " to (" << toX << ", " << toY << ")"
                        << std::endl;
              from = getLocationFromXY(fromX, fromY);//合成位置
              to = getLocationFromXY(toX, toY);
              move = composeMove(from, to); //合成走法
              while(!makeOneMove(move, &captured)){
                // std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n输入坐标错误，请重新输入坐标：";
                std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
                showBoard();///////////////////////////////////////////////////////////////////////////
                std::cout << "输入坐标错误，请重新输入坐标：";
                MessageBox(NULL,"坐标输入错误","AlphaJo",1);
                // scanf("%d,%d %d,%d", &fromX, &fromY, &toX, &toY);
                std::cin >> fromX;
                std::cin.get();
                std::cin >> fromY;
                std::cin >> toX;
                std::cin.get();
                std::cin >> toY;
                from = getLocationFromXY(fromX, fromY);//合成位置
                to = getLocationFromXY(toX, toY);
                move = composeMove(from, to); //合成走法
                getchar();

              }
              showBoard();
            }
            if(isThePlayerDie(currentPlayer)){ //如果电脑已败
              continue;
            }
            if(1){ //如果电脑黑方则后走
              // std::cout << std::endl << "电脑正在思考..." << std::endl;
              //////////配置套装2///////////////////////////
              /////////////////////////////////////////////
              /////////////////////////////////////////////
              std::cout << std::endl << "电脑正在思考...1%";
              for (int v = 1; v < 101; v++){
                  std::cout << "\r电脑正在思考...." ;
                  std::cout << v;
                  std::cout << "%";
                  Sleep(10);
              }
              /////////////////////////////////////////////
              //////////////////////////////////////////////
              ///////////////////////////////////////////////
              computerThink();
              move = bestMove;
              from = generateMoveFrom(move); //分解走法
              to = generateMoveTo(move);
              fromX = getXFromLocation(from); //取得坐标
			  fromY = getYFromLocation(from);
			  toX = getXFromLocation(to);
			  toY = getYFromLocation(to);
              std::cout << std::endl << "电脑所走的棋是：from (" << fromX << ","
                        << fromY << ") to (" << toX << "," << toY << ")"
                        << std::endl;
            }
          }

          if(engine == 2){ //引擎对战标志
            showBoard();
            if(1){ //电脑A先走棋
              std::cout << "\n电脑正在思考...";
              //////////配置套装2///////////////////////////
              /////////////////////////////////////////////
              /////////////////////////////////////////////
              std::cout << std::endl << "电脑正在思考...1%";
              for (int v = 1; v < 101; v++){
                  std::cout << "\r电脑正在思考....";
                  std::cout << v;
                  std::cout  << "%";
                  Sleep(10);
              }
              ///////////////////////////////////////////////
              ///////////////////////////////////////////////
              ////////////////////////////////////////////
              computerThink();
              move = bestMove;
              from = generateMoveFrom(move); //分解走法
              to = generateMoveTo(move);
              fromX = getXFromLocation(from); //取得坐标
			  fromY = getYFromLocation(from);
			  toX = getXFromLocation(to);
			  toY = getYFromLocation(to);
              // printf("\n电脑A所走的棋是：from (%d,%d) to (%d,%d)\n", fromX, fromY, toX, toY);
              std::cout << std::endl
                        << "电脑B所走的棋是：from ("
                        << fromX << "," << fromY << ") to (" << toX << "," << toY << ")";
              showBoard();
              //////////////////////////文件输出////////////////////////////////////////
              //////////////////////////////////////////////////////////////////////////
              /////////////////////////////////////////////////////////////////////////
            outFile << std::endl;
            outFile << "                    y **********" << std::endl;
            for(int i = 0; i < 6; i++){
              outFile << "                    " << i << " ";
              int location;
              for(int j = 0; j < 5; j++){
             	  location = getLocationFromXY(j, i); //取得数组下标

                if(engine == WHITE){ //如果引擎持白方，翻转棋盘位置
                  location = flipLocation(location);
                }
                int piece;
                piece = board[location];
                if(piece == 8){
                  outFile << "★";
                }else if(piece == 9){
                  outFile << "◆";
                }else if(piece == 10){
                  outFile << "■";
                }else if(piece == 11){
                  outFile << "●";
                }else if(piece == 16){
                  outFile << "☆";
                }else if(piece == 17){
                  outFile << "◇";
                }else if(piece == 18){
                  outFile << "□";
                }else if(piece == 19){
                  outFile << "○";
                }else if(piece == NOPIECE){
                  outFile << "╋";
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
            if(isThePlayerDie(currentPlayer)){ //如果玩家已败
              continue;
            }
            if(1){ //轮到电脑B走棋
              std::cout << std::endl << "电脑B正在思考..." << std::endl;
              //////////配置套装2///////////////////////////
              /////////////////////////////////////////////
              /////////////////////////////////////////////
              std::cout << std::endl << "电脑正在思考...1%";
              for (int v = 1; v < 101; v++)
              {
                  std::cout << "\r电脑正在思考....";
                  std::cout << v;
                  std::cout << "%";
                  Sleep(10);
              }
              /////////////////////////////////////////////
              //////////////////////////////////////////////
              ///////////////////////////////////////////////
              computerThink();
              move = bestMove;
              from = generateMoveFrom(move); //分解走法
              to = generateMoveTo(move);
              fromX = getXFromLocation(from); //取得坐标
			  fromY = getYFromLocation(from);
			  toX = getXFromLocation(to);
			  toY = getYFromLocation(to);
              std::cout << std::endl
                        << "电脑B所走的棋是：from ("
                        << fromX << "," << fromY << ") to (" << toX << "," << toY << ")";
            }
          }
        }
      }
    } else{
      std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nd敲几个回车再按D就能玩啦！！！\n";
    }
  }
  outFile.close();
  return 0;
}
