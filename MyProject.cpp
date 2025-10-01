#include <graphics.h>
#include <vector>
#include <deque>
#include <ctime>
#include <cstdlib>
#include <conio.h>
#include <fstream>
#include <algorithm>
using namespace std;


enum GameState {
    MENU,
    PLAYING,
    PAUSED,
    EVOLUTION,
    RANKING,
    GAME_OVER,
    EXIT_GAME
};

enum Direction {
    UP, DOWN, LEFT, RIGHT
};

const int WINDOW_WIDTH = 750;
const int WINDOW_HEIGHT = 500;
const int GRID_SIZE = 25;
const int GAME_AREA_WIDTH = 500, GAME_AREA_HEIGHT = 500;
const int GAME_AREA_LEFT = 0, GAME_AREA_TOP = 0;
const int GAME_AREA_RIGHT = GAME_AREA_LEFT + GAME_AREA_WIDTH;
const int GAME_AREA_BOTTOM = GAME_AREA_TOP + GAME_AREA_HEIGHT;
const int INFO_AREA_LEFT = GAME_AREA_RIGHT, INFO_AREA_TOP = 0;
const int INFO_AREA_WIDTH = 250, INFO_AREA_HEIGHT = 500;
const int GRID_COUNT_X = GAME_AREA_WIDTH / GRID_SIZE;
const int GRID_COUNT_Y = GAME_AREA_HEIGHT / GRID_SIZE;

struct Button {
    int x, y, width, height;
    const char* text;
};

void saveScoreToRanking(int score) {
    ofstream fout("ranking.txt", ios::app);
    if (fout.is_open()) fout << score << endl;
    fout.close();
}

vector<int> loadRanking() {
    vector<int> scores;
    ifstream fin("ranking.txt");
    int s;
    while (fin >> s) scores.push_back(s);
    fin.close();
    sort(scores.begin(), scores.end(), greater<int>());
    return scores;
}

class Food {
public:
    int x, y;
    Food() { 
		x = y = 0; 
	}
    void generate(const deque<pair<int,int>>& snakeBody, const vector<pair<int,int>>& obstacles) {
        while (1) {
            int gx = rand() % GRID_COUNT_X, gy = rand() % GRID_COUNT_Y;
            int newX = GAME_AREA_LEFT + gx * GRID_SIZE, newY = GAME_AREA_TOP + gy * GRID_SIZE;
            bool fail = false;
            for (auto &p : snakeBody) if (p.first==newX&&p.second==newY) { fail=true; break; }
            if (fail) continue;
            for (auto &o : obstacles) if (o.first==newX&&o.second==newY) { fail=true; break; }
            if (fail) continue;
            x = newX; y = newY; break;
        }
    }
    void draw() {
        setfillcolor(RED);
        bar(x, y, x + GRID_SIZE, y + GRID_SIZE);
    }
};

class Snake {
public:
    deque<pair<int,int>> body;
    Direction direction;
    bool grow;
    Snake() {
        direction = RIGHT;
        int startX = GAME_AREA_LEFT+(GRID_COUNT_X/2)*GRID_SIZE, startY = GAME_AREA_TOP+(GRID_COUNT_Y/2)*GRID_SIZE;
        body.push_back(make_pair(startX, startY));
        body.push_back(make_pair(startX-GRID_SIZE, startY));
        body.push_back(make_pair(startX-2*GRID_SIZE, startY));
        grow = false;
    }
    void drawBodyBlock(int x, int y) {
        setfillcolor(GREEN);
        bar(x, y, x+GRID_SIZE, y+GRID_SIZE);
    }
    void drawHead(int x, int y) {
        setfillcolor(GREEN);
        bar(x, y, x+GRID_SIZE, y+GRID_SIZE);
        int eyeR = max(2, GRID_SIZE/6), pupilR = max(1, eyeR/2);
        int ex1 = x, ey1 = y, ex2 = x, ey2 = y;
        int offset_small = GRID_SIZE/6, offset_big = GRID_SIZE/4;
        if (direction==RIGHT) {
            ex1=x+GRID_SIZE-offset_big; ey1=y+offset_small+(GRID_SIZE/8);
            ex2=x+GRID_SIZE-offset_big; ey2=y+GRID_SIZE-offset_small-(GRID_SIZE/8);
        } else if (direction==LEFT) {
            ex1=x+offset_big; ey1=y+offset_small+(GRID_SIZE/8);
            ex2=x+offset_big; ey2=y+GRID_SIZE-offset_small-(GRID_SIZE/8);
        } else if (direction==UP) {
            ex1=x+offset_small+(GRID_SIZE/8); ey1=y+offset_big;
            ex2=x+GRID_SIZE-offset_small-(GRID_SIZE/8); ey2=y+offset_big;
        } else {
            ex1=x+offset_small+(GRID_SIZE/8); ey1=y+GRID_SIZE-offset_big;
            ex2=x+GRID_SIZE-offset_small-(GRID_SIZE/8); ey2=y+GRID_SIZE-offset_big;
        } 
        setfillcolor(WHITE); fillellipse(ex1,ey1,eyeR,eyeR); fillellipse(ex2,ey2,eyeR,eyeR);
        setfillcolor(BLACK); fillellipse(ex1,ey1,pupilR,pupilR); fillellipse(ex2,ey2,pupilR,pupilR);
    }
    void drawTail() {
        if (body.empty()) return;
        int tx = body.back().first, ty = body.back().second;
        int cx = tx + GRID_SIZE/2, cy = ty + GRID_SIZE/2;
        setfillcolor(EGERGB(255,255,0)); bar(tx,ty,tx+GRID_SIZE,ty+GRID_SIZE);
        int dotR = max(1, GRID_SIZE/6);
        setfillcolor(EGERGB(255,0,0)); fillellipse(cx,cy,dotR,dotR);
    }
    void draw() {
        for (size_t i = 0; i < body.size(); ++i) {
            int x = body[i].first, y = body[i].second;
            if (i==0) drawHead(x, y);
            else if (i==body.size()-1) drawTail();
            else drawBodyBlock(x, y);
        }
    }
    void changeDirection(Direction newDir) {
        if ((direction==UP&&newDir==DOWN)||(direction==DOWN&&newDir==UP)||
            (direction==LEFT&&newDir==RIGHT)||(direction==RIGHT&&newDir==LEFT)) return;
        direction = newDir;
    }
    void move() {
        pair<int,int> head = body.front();
        switch(direction) {
            case UP: head.second-=GRID_SIZE; break;
            case DOWN: head.second+=GRID_SIZE; break;
            case LEFT: head.first-=GRID_SIZE; break;
            case RIGHT: head.first+=GRID_SIZE; break;
        }
        body.push_front(head);
        if (grow) grow = false;
        else body.pop_back();
    }
    bool checkCollision() {
        auto head = body.front();
        if (head.first<GAME_AREA_LEFT||head.first>=GAME_AREA_RIGHT||
            head.second<GAME_AREA_TOP||head.second>=GAME_AREA_BOTTOM) return true;
        for (size_t i=1;i<body.size();i++) if (body[i]==head) return true;
        return false;
    }
    void drawGrid() {
        setcolor(EGERGB(60,60,60));
        for (int x=GAME_AREA_LEFT;x<=GAME_AREA_RIGHT;x+=GRID_SIZE)
            line(x,GAME_AREA_TOP,x,GAME_AREA_BOTTOM);
        for (int y=GAME_AREA_TOP;y<=GAME_AREA_BOTTOM;y+=GRID_SIZE)
            line(GAME_AREA_LEFT,y,GAME_AREA_RIGHT,y);
    }
};

class SnakeGame {
private:
    GameState state;
    Snake snake;
    Food food;
    int score, speed;
    bool isHardMode;
    vector<pair<int,int>> obstacles;
    unsigned long long lastObstacleRefreshMs;
    bool gameStarted;
public:
    SnakeGame() {
        state = MENU;
        score = 0; speed = 150;
        isHardMode = false;
        lastObstacleRefreshMs = 0;
        gameStarted = false;
    }
    unsigned long long currentMs() {
        return (unsigned long long)(clock()*1000/CLOCKS_PER_SEC);
    }
    void generateObstacles(int count=10) {
        obstacles.clear();
        while ((int)obstacles.size() < count) {
            int gx = rand()%GRID_COUNT_X, gy = rand()%GRID_COUNT_Y;
            int ox = GAME_AREA_LEFT+gx*GRID_SIZE, oy = GAME_AREA_TOP+gy*GRID_SIZE;
            if (ox==food.x&&oy==food.y) continue;
            bool collide = false;
            for (auto&p:snake.body) if (p.first==ox&&p.second==oy) { collide=true; break; }
            if (collide) continue;
            bool exists = false;
            for (auto&q:obstacles) if (q.first==ox&&q.second==oy) { exists=true; break; }
            if (exists) continue;
            obstacles.push_back(make_pair(ox,oy));
        }
        lastObstacleRefreshMs = currentMs();
    }
    void initGame() {
        snake = Snake(); food = Food(); score = 0; gameStarted = false;
        food.generate(snake.body, obstacles);
        speed = isHardMode ? 100 : 150;
        if (isHardMode) { generateObstacles(10); lastObstacleRefreshMs = currentMs(); }
        else obstacles.clear();
        state = PLAYING;
    }
    void showMenu() {
        PIMAGE bgImg = newimage();
        if (getimage(bgImg, "Pic/Covering.png") == 0) {
            int iw = getwidth(bgImg), ih = getheight(bgImg);
            putimage(0,0,WINDOW_WIDTH,WINDOW_HEIGHT,bgImg,0,0,iw,ih);
        }
        Button startBtn = {315,360,120,40,"开始游戏"},
               exitBtn = {315,410,120,40,"退出游戏"},
               rankBtn = {620,450,100,30,"排行榜"},
               simpleBtn = {240,310,100,30,"简单"},
               hardBtn = {410,310,100,30,"困难"};
        setfillcolor(EGERGB(0x70,0x70,0xA0));
        setcolor(WHITE); setfont(20,0,"黑体");
        if (!isHardMode) {
            setfillcolor(EGERGB(100,180,100));
            bar(simpleBtn.x,simpleBtn.y,simpleBtn.x+simpleBtn.width,simpleBtn.y+simpleBtn.height);
            setfillcolor(EGERGB(140,140,170));
            bar(hardBtn.x,hardBtn.y,hardBtn.x+hardBtn.width,hardBtn.y+hardBtn.height);
        } else {
            setfillcolor(EGERGB(140,140,170));
            bar(simpleBtn.x,simpleBtn.y,simpleBtn.x+simpleBtn.width,simpleBtn.y+simpleBtn.height);
            setfillcolor(EGERGB(200,100,100));
            bar(hardBtn.x,hardBtn.y,hardBtn.x+hardBtn.width,hardBtn.y+hardBtn.height);
        }
        setcolor(WHITE);
        outtextxy(simpleBtn.x+10,simpleBtn.y+5,simpleBtn.text);
        outtextxy(hardBtn.x+10,hardBtn.y+5,hardBtn.text);
        setfillcolor(EGERGB(0x70,0x70,0xA0));
        bar(startBtn.x,startBtn.y,startBtn.x+startBtn.width,startBtn.y+startBtn.height);
        outtextxy(startBtn.x+10,startBtn.y+10,startBtn.text);
        bar(exitBtn.x,exitBtn.y,exitBtn.x+exitBtn.width,exitBtn.y+exitBtn.height);
        outtextxy(exitBtn.x+10,exitBtn.y+10,exitBtn.text);
        bar(rankBtn.x,rankBtn.y,rankBtn.x+rankBtn.width,rankBtn.y+rankBtn.height);
        outtextxy(rankBtn.x+10,rankBtn.y+5,rankBtn.text);
        if (bgImg) delimage(bgImg);
    }
    GameState handleMenuInput() {
        mouse_msg msg = {0};
        while (1) {
            while (mousemsg()) {
                msg = getmouse();
                if (msg.is_left() && msg.is_down()) {
                    int x = msg.x, y = msg.y;
                    if (x>=315&&x<=435 && y>=360&&y<=400) return PLAYING;
                    else if (x>=315&&x<=435 && y>=410&&y<=450) return EXIT_GAME;
                    else if (x>=620&&x<=720 && y>=450&&y<=480) return RANKING;
                    else if (x>=240&&x<=340 && y>=310&&y<=340) { isHardMode=false; return MENU; }
                    else if (x>=410&&x<=510 && y>=310&&y<=340) { isHardMode=true; return MENU; }
                }
            }
            delay_ms(10);
        }
    }
    void handleGameInput() {
        if (kbhit()) {
            int key = getch();
            if (state==EVOLUTION||state==GAME_OVER) { state=MENU; return; }
            switch(key) {
                case 'W': case 'w': case 72: snake.changeDirection(UP); return;
                case 'S': case 's': case 80: snake.changeDirection(DOWN); return;
                case 'A': case 'a': case 75: snake.changeDirection(LEFT); return;
                case 'D': case 'd': case 77: snake.changeDirection(RIGHT); return;
                case 27: state=MENU; return;
                case ' ':
                    if (state==PLAYING && !gameStarted) { gameStarted=true; return; }
                    if (state==PLAYING && gameStarted) { state=PAUSED; return; }
                    else if (state==PAUSED) { state=PLAYING; return; }
                    return;
                default: return;
            }
        }
    }
    void updateGame() {
        snake.move();
        if (isHardMode) {
            unsigned long long now = currentMs();
            if (now-lastObstacleRefreshMs>=5000) generateObstacles(10);
        }
        if (snake.body.front().first==food.x && snake.body.front().second==food.y) {
            snake.grow = true;
            food.generate(snake.body, obstacles);
            if (!isHardMode) {
                score += 10;
                if (score%50==0 && speed>50) speed -= 50;
            } else {
                score += 20;
                if (score%50==0 && speed>30) speed -= 50;
            }
        }
        if (score>=100 && state==PLAYING) {
            saveScoreToRanking(score);
            state=EVOLUTION; return;
        }
        if (snake.checkCollision()) {
            saveScoreToRanking(score);
            state=GAME_OVER; return;
        }
        if (isHardMode) {
            auto head = snake.body.front();
            for (auto&obs:obstacles)
                if (head.first==obs.first&&head.second==obs.second) {
                    saveScoreToRanking(score);
                    state=GAME_OVER; return;
                }
        }
    }
    void drawEvolutionOverlay() {
        PIMAGE evoImg = newimage();
        bool loaded = (getimage(evoImg, "Pic/AFT Revolution.png")==0);
        if (loaded) {
            int iw = getwidth(evoImg), ih = getheight(evoImg);
            putimage(0,0,WINDOW_WIDTH,WINDOW_HEIGHT,evoImg,0,0,iw,ih);
        }
        if (evoImg) delimage(evoImg);
        if (kbhit()) { getch(); state=MENU; }
    }
    void drawGameOverOverlay() {
        setfillcolor(EGERGB(20,20,20));
        bar(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
        setfont(48,0,"黑体");
        setcolor(EGERGB(255,80,80));
        outtextxy(WINDOW_WIDTH/2-80,WINDOW_HEIGHT/2-60,"你死了");
        char buf[64];
        sprintf(buf,"得分: %d",score);
        setfont(28,0,"黑体");
        setcolor(WHITE);
        outtextxy(WINDOW_WIDTH/2-60,WINDOW_HEIGHT/2-10,buf);
        setfont(18,0,"黑体");
        outtextxy(WINDOW_WIDTH/2-120,WINDOW_HEIGHT/2+30,"按任意键返回菜单（分数已保存）");
    }
    void drawGame() {
        cleardevice();
        setfillcolor(EGERGB(20,20,20));
        bar(GAME_AREA_LEFT,GAME_AREA_TOP,GAME_AREA_RIGHT,GAME_AREA_BOTTOM);
        setfillcolor(EGERGB(40,40,60));
        bar(INFO_AREA_LEFT,INFO_AREA_TOP,INFO_AREA_LEFT+INFO_AREA_WIDTH,INFO_AREA_TOP+INFO_AREA_HEIGHT);
        setcolor(WHITE);
        rectangle(GAME_AREA_LEFT,GAME_AREA_TOP,GAME_AREA_RIGHT,GAME_AREA_BOTTOM);
        rectangle(INFO_AREA_LEFT,INFO_AREA_TOP,INFO_AREA_LEFT+INFO_AREA_WIDTH,INFO_AREA_TOP+INFO_AREA_HEIGHT);
        snake.drawGrid();
        if (isHardMode) for (auto&obs:obstacles) {
            setfillcolor(EGERGB(139,69,19));
            bar(obs.first,obs.second,obs.first+GRID_SIZE,obs.second+GRID_SIZE);
        }
        snake.draw(); food.draw();
        setcolor(WHITE); setfont(20,0,"黑体");
        xyprintf(INFO_AREA_LEFT+20,INFO_AREA_TOP+50,"分数: %d",score);
        xyprintf(INFO_AREA_LEFT+20,INFO_AREA_TOP+100,"操作方式:");
        xyprintf(INFO_AREA_LEFT+20,INFO_AREA_TOP+130,"按下WASD来控制方向");
        xyprintf(INFO_AREA_LEFT+20,INFO_AREA_TOP+160,"ESC返回菜单");
        xyprintf(INFO_AREA_LEFT+20,INFO_AREA_TOP+190,"空格：开始/暂停");
        if (state==PLAYING && !gameStarted) {
            setfont(18,0,"黑体");
            setcolor(EGERGB(200,200,50));
            outtextxy(INFO_AREA_LEFT+20,INFO_AREA_TOP+230,"按 空格 键 开始游戏");
        }
        if (isHardMode)
            outtextxy(INFO_AREA_LEFT+20,INFO_AREA_TOP+260,"模式: 困难 (食物 20 分)");
        else
            outtextxy(INFO_AREA_LEFT+20,INFO_AREA_TOP+260,"模式: 简单 (食物 10 分)");
        setfont(16,0,"黑体");
        outtextxy(GAME_AREA_LEFT+10,GAME_AREA_TOP-25,"游戏区域");
        outtextxy(INFO_AREA_LEFT+10,INFO_AREA_TOP-25,"信息区域");
        if (state==PAUSED) {
            PIMAGE pauseImg = newimage();
            bool ok = (getimage(pauseImg,"Pic/Pause.png")==0);
            if (ok) {
                int iw = getwidth(pauseImg), ih = getheight(pauseImg);
                int px = GAME_AREA_LEFT + (GAME_AREA_WIDTH-iw)/2, py = GAME_AREA_TOP + (GAME_AREA_HEIGHT-ih)/2;
                putimage_withalpha(NULL, pauseImg, px, py);
            }
            if (pauseImg) delimage(pauseImg);
        }
        if (state==EVOLUTION) drawEvolutionOverlay();
        if (state==GAME_OVER) drawGameOverOverlay();
    }
    void showRanking() {
        cleardevice();
        setcolor(WHITE); setfont(30,0,"黑体");
        outtextxy(0,80,"排行榜（前10名）");
        vector<int> scores = loadRanking();
        setfont(24,0,"黑体");
        for (size_t i=0;i<scores.size()&&i<10;++i) {
            char buf[64];
            sprintf(buf,"第%zu名：%d分",i+1,scores[i]);
            outtextxy(300,130+i*35,buf);
        }
        outtextxy(500,500-60,"按任意键返回菜单");
        getch(); state=MENU;
    }
    void run() {
        while (state != EXIT_GAME) {
            switch(state) {
                case MENU:
                    showMenu();
                    state = handleMenuInput();
                    if (state==PLAYING) initGame();
                    break;
                case PLAYING: case PAUSED: case EVOLUTION: case GAME_OVER:
                    while (state==PLAYING||state==PAUSED||state==EVOLUTION||state==GAME_OVER) {
                        handleGameInput();
                        if (state==PLAYING && gameStarted) updateGame();
                        drawGame();
                        delay_ms(speed);
                    }
                    break;
                case RANKING:
                    showRanking();
                    if (state==PLAYING) initGame();
                    break;
            }
        }
    }
};

int main() {
    initgraph(WINDOW_WIDTH,WINDOW_HEIGHT);
    srand((unsigned)time(NULL));
    SnakeGame game;
    game.run();
    closegraph();
    return 0;
}
