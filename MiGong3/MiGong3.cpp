#include "StdAfx.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <iomanip>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
using namespace std;

#define KEY_DOWN(VK_NONAME)((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
COORD pos;

HWND hwnd=FindWindow(L"ConsoleWindowClass",NULL);    //控制台窗口句柄 
HWND h=GetForegroundWindow();
POINT pt;

//将string字符串转换成LPCWSTR字符串
LPCWSTR stringToLPCWSTR(string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t *wcstring = (wchar_t *)malloc(sizeof(wchar_t)*(orig.length()-1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);
	return wcstring;
}

//设置控制台相关参数
void SetConsole()
{
	SetConsoleTextAttribute(hOut, 240);  //设置前景与背景颜色
	//隐藏光标
	CONSOLE_CURSOR_INFO curInfo;
	curInfo.dwSize = 1;
	curInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hOut,&curInfo);
}

struct Way
{
	int x;   //存放每一步所在的行数
	int y;   //存放每一步所在的列数
	int parent;   //层序寻找最短路径时用到的变量，表示当前步的前一步
};

class labyrinth        //迷宫类
{
public:
	labyrinth();            //构造函数
	void ReadFile();        //读取相关文件

	void LoadUI(LPCWSTR lpszName,int X,int Y,int Z,int K);  //加载封面图片
	void GameMenu();          //第一层菜单
	
	void SetMenu();          //游戏设置的菜单
	void SetModes();         //设置游戏模式
	void ChooseLevel();      //编辑迷宫前选关
	void ReviseMap();        //编辑迷宫

	void ShowMap();              //显示地图
	void ShowTime(int start);    //显示时间
	void ShowInstructions();    //显示其他信息
  
	void PlayGame();           //玩游戏
	void BeginUI();            //开始界面  
	void MoveRat(int &X,int &Y);     //老鼠移动
	void GoNextLevel(int start,int finish);  //进入下一关
	void ShowDialogBox(int score);    //当前关卡成功时弹出的对话框

	void ShowNearestWay();  //当前关卡的最短路径
	void ShowFindWay();     //老鼠自动查找路线

	void QuitGame();        //退出时保存相关数据的函数

private:
	int map[40][40];     //迷宫地图矩阵       
	string Path[10];     //每一关的相关文件夹的路径    
	int HighestScore;    //历史最高分
	int TimeLimit;        //没一关卡的时间限制
	int NowLevel;        //当前关卡数
	int Unlocked[10];     //每一关卡的解锁状态
	char LevelName[30];    //当前关卡的名字

	void ShowFindWay(int x,int y);    //自动查找路线
};

//主函数
int main()
{		
	SetConsole();     //设置控制台的大小，隐藏光标
	labyrinth maze;     //定义一个迷宫对象
	maze.GameMenu();    //迷宫的朱功能表
	return 0;
}

//构造函数
labyrinth::labyrinth()
{
	ifstream ReadPath("LevelPath.txt");
	char reading[30];
	for(int i=0; i<3; i++) //逐行读取
	{
		ReadPath.getline(reading, 30, '\n');
		Path[i] = reading;
	}
	ReadPath.close();

	ifstream GetLevel("NowLevel.txt");
	GetLevel >> NowLevel;
	GetLevel.get();
	GetLevel.close();

	ReadFile();
	TimeLimit = 20000;
}

//读取相关文件
void labyrinth::ReadFile()
{
	string path = Path[NowLevel];
	path.append("MapMatrix");
	ifstream GetMap(path,ios::binary);
	for(int i=0; i<31; i++)
	{
		for(int j=0; j<31; j++)
			GetMap.read((char *)&map[i][j],4);
	}
	GetMap.close();

	path = Path[NowLevel];
	ifstream Get1(path.append("HighestScore.txt"));
	Get1 >> HighestScore;
	Get1.get();
	Get1.close();

	path = Path[NowLevel];
	ifstream ReadLevelName(path.append("LevelName.txt"));
	ReadLevelName.getline(LevelName, 30, '\n');
	ReadLevelName.close();

	ifstream Get_UnlockedLevel("UnlockedLevel",ios::binary);
	for(int i=0; i<10; i++)
		Get_UnlockedLevel.read((char *)&Unlocked[i],4);
	Get_UnlockedLevel.close();
}

//加载关卡封面图片
void labyrinth::LoadUI(LPCWSTR lpszName,int X,int Y,int Z,int K)
{
	HDC dc = GetDC(h);    //检索一指定窗口的客户区域或整个屏幕的显示设备上下文环境的句柄
	HBITMAP hBitmap;       //HBITMAP是加载位图需要用到的类
	//LoadImage的返回值是相关资源的句柄。因为加载的是位图所以返回的句柄是HBITMAP型的(需要强制转换)
	hBitmap=(HBITMAP)LoadImage(NULL,lpszName,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_CREATEDIBSECTION);
	HDC cmdmem = CreateCompatibleDC(dc);      //创建一个与指定设备兼容的内存设备上下文环境，即cmdmem
	SelectObject(cmdmem , hBitmap);     //选择hBitmap到指定的设备上下文环境中，该新对象替换先前的相同类型的对象。
	BitBlt(dc , X , Y , Z , K , cmdmem , 0 , 0 , SRCCOPY); //对指定的源设备环境区域中的像素进行位块(bit_block)转换，以传送到目标设备环境
}

//游戏的菜单
void labyrinth::GameMenu()
{
	LoadUI(L"UI.bmp",0,0,799,799);
	Sleep(2000);
	LoadUI(L"FirstMenu.bmp",0,0,799,799);
	int X=220,Y=280,i=0;
	while(TRUE)
	{
		Sleep(80);
		if(kbhit())
		{
			if(GetKeyState(VK_TAB)<0)
			{
				switch(i)
				{
				case 0: PlayGame(); break;
				case 1: SetMenu(); break;
				case 2: QuitGame(); break;
				}
				LoadUI(L"FirstMenu.bmp",0,0,799,799);
				i=0;
				Y=280;
			}
			LoadUI(L"Blank.bmp",X,Y,X+86,Y+86);       //输出空白
			if(GetKeyState(VK_UP)<0){
				i = (--i)%3;
				if(i==-1) i=2;
				Y = 280 + i*80;
			}
			else if(GetKeyState(VK_DOWN)<0){      
				i = (++i)%3;
				Y = 280 + i*80;
			}
			LoadUI(L"Rat.bmp",X,Y,X+86,Y+86);       //输出新的老鼠
		}
	}
}


//游戏设置的菜单
void labyrinth::SetMenu()
{
	LoadUI(L"SetMenu.bmp",0,0,799,799);
	int X=250,Y=290,i=0;
	while(TRUE)
	{
		Sleep(80);
		if(kbhit())
		{
			if(GetKeyState(VK_TAB)<0)
			{
				switch(i)
				{
				case 0: ReviseMap(); break;
				case 1: SetModes(); break;
				case 2: return; break;
				}
				LoadUI(L"SetMenu.bmp",0,0,799,799);
				i=0;
				Y=290;
			}

			LoadUI(L"SetBlank.bmp",X,Y,X+71,Y+64);
			if(GetKeyState(VK_UP)<0){
				i = (--i)%3;
				if(i==-1) i=2;
				Y = 290 + i*80;
			}
			else if(GetKeyState(VK_DOWN)<0){
				i = (++i)%3;
				Y = 290 + i*80;
			}
			LoadUI(L"SetRat.bmp",X,Y,X+71,Y+64);
		}
	}
}

//选取游戏模式
void labyrinth::SetModes()
{
	LoadUI(L"ModesMenu.bmp",0,0,799,799);
	int X=250,Y=285,i=0;
	while(TRUE)
	{
		Sleep(80);
		if(kbhit())
		{
			if(GetKeyState(VK_TAB)<0)
			{
				switch(i)
				{
				case 0: TimeLimit=30000; break;
				case 1: TimeLimit=20000; break;
				case 2: TimeLimit=15000; break;
				}
				MessageBox(NULL, TEXT("修改成功"),TEXT("迷宫"),MB_OK);
				return;
			}

			LoadUI(L"ModesBlank.bmp",X,Y,X+71,Y+64);
			if(GetKeyState(VK_UP)<0){
				i = (--i)%3;
				if(i==-1) i=2;
				Y = 285 + i*80;
			}
			else if(GetKeyState(VK_DOWN)<0){
				i = (++i)%3;
				Y = 285 + i*80;
			}
			LoadUI(L"ModesRat.bmp",X,Y,X+71,Y+64);
		}
	}
}

//修改迷宫前选取游戏关卡
void labyrinth::ChooseLevel()
{
	SetConsoleTextAttribute(hOut, 240);
	system("cls");
	pos.X = 35;
	pos.Y = 18;
	SetConsoleCursorPosition(hOut,pos);
	cout << " 请输入第几关： ";
	char choose;
	do{
		choose = getch();
		switch(choose)
		{
		case '1': NowLevel = 0; break;
		case '2': NowLevel = 1; break;
		case '3': NowLevel = 2; break;
		}
	}while('1'>choose || choose>'3');
	ReadFile();
}

//修改迷宫地图
void labyrinth::ReviseMap()
{  
	int temp = NowLevel;
	ChooseLevel();
	ShowMap();
	SetConsoleTextAttribute(hOut, 40);
	while(true)
	{
		if(KEY_DOWN(VK_LBUTTON))
		{
			MessageBox(NULL, TEXT("修改成功"),TEXT("迷宫"),MB_OK);
			keybd_event(VK_RETURN,0,0,0);
			break;
		}
		if(KEY_DOWN(VK_RBUTTON))   //鼠标右键是否按下 
		{
			GetCursorPos(&pt);
			ScreenToClient(h,&pt);
			pos.X = pt.x/8/2*2;
			pos.Y = pt.y/16;
			SetConsoleCursorPosition(hOut,pos);
			if(map[pos.Y-4][pt.x/8/2-4] == 0)
			{
				cout << "  ";
				map[pos.Y-4][pt.x/8/2-4] = 1;
			}
			else if(map[pos.Y-4][pt.x/8/2-4] == 1)
			{
				cout << "█";      //■ ※ █
				map[pos.Y-4][pt.x/8/2-4] = 0;
			}
		}	
		Sleep(115);
	}

	string path = Path[NowLevel];
	path.append("MapMatrix");
	ofstream SaveMap(path,ios::binary);
	for(int i=0; i<31; i++)
	{
		for(int j=0; j<31; j++)
			SaveMap.write((char *)&map[i][j],4);
	}
	SaveMap.close();

	NowLevel = temp;
	ReadFile();
}


//玩游戏，手动操作老鼠移动及其他相关操作
void labyrinth::PlayGame()
{
	SetConsoleTextAttribute(hOut, 240);
	string path1 = Path[NowLevel];
	path1.append("LevelUI.bmp");
	LPCWSTR path = stringToLPCWSTR(path1);
	LoadUI(path,0,0,799,799);
	Sleep(2000);
	system("cls");
	
	BeginUI();
	ShowInstructions();
	int X = 15,Y = 15;
	map[X][Y] = 1;
	clock_t start=0, finish=0;  
	
	while(true)
	{

		finish = clock();
		if(start!=0 && finish-start>=TimeLimit){
			MessageBox(NULL,L"游戏失败！\n 得分：0",L"迷宫",MB_OK);
			SetConsoleTextAttribute(hOut, 240);
			return;	
		}

		if(start != 0)
			MoveRat(X,Y);   //老鼠移动

		if(X==29 && Y==29){		//到达终点
			GoNextLevel(start,finish);     //本关结束进入下一关
			return;
		}

		if(GetKeyState(VK_SPACE)<0 && start==0){      //按空格开始
			ShowMap();
			start = clock();
		}
		if(GetKeyState(74)<0){   //按J上一关
			if(NowLevel-1<0){
				MessageBox(NULL,L"前面没有关卡了！",L"Maze",MB_OK);
				return;
			}
			Sleep(10);
			NowLevel--;
			ReadFile();
			PlayGame();
			return;
		}
		if(GetKeyState(75)<0){    //按K下一关
			if(NowLevel+1>2){
				MessageBox(NULL,L"后面没有关卡了！",L"Maze",MB_OK);
				return;
			}
			if(Unlocked[NowLevel+1]==0){
				MessageBox(NULL,L"下一关未解锁！！！\n完成本关将自动解锁下一关",L"Maze",MB_OK);
				return;
			}		
			Sleep(10);
			NowLevel++;
			ReadFile();
			PlayGame();
			return;
		}

		if(GetKeyState(0xA4)<0 || GetKeyState(0xA5)<0){     //按ALT重新开始
			Sleep(10);
			PlayGame();
			return;
		}
	
		if(GetKeyState(8)<0){      //按退格返回
			Sleep(10);
			return;
		}
		Sleep(50);  
		ShowTime(start);
	}
}

//开始游戏的界面
void labyrinth::BeginUI()
{
	SetConsoleTextAttribute(hOut, 46);
	pos.X = 4;
	for(int i=2; i<37; i++)
	{
		pos.Y = i;
		SetConsoleCursorPosition(hOut,pos);
		if(i==2)
			cout << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓";
		else if(i==14)
			cout << "┃               游戏开始后，按方向键可移动老鼠的位置               ┃";
		else if(i==16)
			cout << "┃          若进行游戏的其他操作，则当前取得的成绩不予保存          ┃";
		else if(i==18)
			cout << "┃                 你还可以在未开始本关之前切换关卡                 ┃";
		else if(i==24)
			cout << "┃                     ╔══════════╗                     ┃";
		else if(i==25)
			cout << "┃                     ║  确认开始：Space   ║                     ┃";
		else if(i==26)
			cout << "┃                     ╚══════════╝                     ┃";
		else if(i==36)
			cout << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛";
		else
			cout << "┃                                                                  ┃";
	}
}

//老鼠移动的函数
void labyrinth::MoveRat(int &X,int &Y)
{
			pos.X = 2*Y+8;
			pos.Y = X+4;
			SetConsoleCursorPosition(hOut,pos);
			SetConsoleTextAttribute(hOut, 40);
			cout << "  ";

			if(GetKeyState(VK_UP)<0 && map[X-1][Y]==1)  X--;
			else if(GetKeyState(VK_DOWN)<0 && map[X+1][Y]==1)  X++;
			else if(GetKeyState(VK_LEFT)<0 && map[X][Y-1]==1)  Y--;
			else if(GetKeyState(VK_RIGHT)<0 && map[X][Y+1]==1)  Y++;

			pos.X = 2*Y+8;
			pos.Y = X+4;
			SetConsoleCursorPosition(hOut,pos);
			SetConsoleTextAttribute(hOut, 44);
			cout << "◆";
}

//当前关卡成功时进入下一关的函数
void labyrinth::GoNextLevel(int start,int finish)
{
	int score = (TimeLimit-finish+start)/100;
	ShowDialogBox(score);
	if(HighestScore<score)
	{
		string path = Path[NowLevel];
		path.append("HighestScore.txt");
		ofstream SaveScore(path,ios::out);
		SaveScore << score << " ";
		SaveScore.close();
	}
	if(NowLevel==2)
	{
		MessageBoxA(NULL,"你已通关！！！","迷宫",MB_OK);
		NowLevel = 0;
		ReadFile();
		return;
	}
	Unlocked[++NowLevel] = 1;
	
	ofstream SaveUnlockedLevel("UnlockedLevel",ios::binary);
	for(int i=0; i<10; i++)
		SaveUnlockedLevel.write((char *)&Unlocked[i],4);
	SaveUnlockedLevel.close();

	ReadFile();
	SetConsoleTextAttribute(hOut, 240);
	PlayGame();
}

//当前关卡成功时弹出的对话框
void labyrinth::ShowDialogBox(int score)
{
	int i,j;
	pos.X = 16;
	for(i=14; i<27; i++)
	{
		pos.Y = i;
		SetConsoleCursorPosition(hOut,pos);
		SetConsoleTextAttribute(hOut, 55);
		if(pos.Y==14) cout << "┏━━━━━━━━━━━━━━━━━━━━━━┓";
		else if(pos.Y==26) cout << "┗━━━━━━━━━━━━━━━━━━━━━━┛";
		else cout << "┃                                            ┃";   
	}
	pos.X = 14;
	for(i=13; i<26; i++)
	{
		pos.Y = i;
		SetConsoleCursorPosition(hOut,pos);
		SetConsoleTextAttribute(hOut, 176);
		if(pos.Y==13) cout << "┏━━━━━━━━━━━━━━━━━━━━━━┓";
		else if(pos.Y==25) cout << "┗━━━━━━━━━━━━━━━━━━━━━━┛";
		else if(pos.Y==15) cout << "┃             成功找到出口！！！             ┃";
		else if(pos.Y==17) cout << "┃             历史最高分数："<< HighestScore << "              ┃";
		else if(pos.Y==19) cout << "┃               你的分数：" << setw(3) << score << "                ┃";
		else if(pos.Y==22) cout << "┃ ╔═════╗ ╔═════╗ ╔════╗ ┃";
		else if(pos.Y==23) cout << "┃ ║最短路径 F║ ║自动寻路 H║ ║下一关 K║ ┃";
		else if(pos.Y==24) cout << "┃ ╚═════╝ ╚═════╝ ╚════╝ ┃";
		else cout << "┃                                            ┃";  
	}
	while(true)
	{
		if(GetKeyState(70)<0)
		{
			SetConsoleTextAttribute(hOut, 240);
			ShowNearestWay();
			return;
		}
		if(GetKeyState(72)<0)
		{
			SetConsoleTextAttribute(hOut, 240);
			ShowFindWay();
			return;
		}
		if(GetKeyState(75)<0)
			return;
		Sleep(50);
	}
}



//显示迷宫地图
void labyrinth::ShowMap()
{
	SetConsoleTextAttribute(hOut, 40);

	pos.X = 4;
	pos.Y = 2;
	SetConsoleCursorPosition(hOut,pos);
	cout << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓";
	pos.Y = 3;
	SetConsoleCursorPosition(hOut,pos);
	cout << "┃                                                                  ┃";
	pos.Y = 35;
	SetConsoleCursorPosition(hOut,pos);
	cout << "┃                                                                  ┃";
	pos.Y = 36;
	SetConsoleCursorPosition(hOut,pos);
	cout << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛";
	for(int i=0; i<31; i++)
	{
		pos.Y = i+4;
		pos.X = 4;
		SetConsoleCursorPosition(hOut,pos);
		cout << "┃  ";
		for(int j=0; j<31; j++)
		{
			if(map[i][j] == 0)
				cout << "█";   //用"※"表示障碍物 ■ █
			else 
				cout << "  ";  //用"  "表示迷宫中能行走的地方
		}
		cout << "  ┃";
	}

	pos.Y = 19;
	pos.X = 38;
	SetConsoleCursorPosition(hOut,pos);
	SetConsoleTextAttribute(hOut, 44);
	cout << "◆";

	pos.Y = 33;
	pos.X = 68;
	SetConsoleCursorPosition(hOut,pos);
	SetConsoleTextAttribute(hOut, 45);
	cout << "★";
	
	SetConsoleTextAttribute(hOut, 240);
}

//显示当前关卡的其他信息
void labyrinth::ShowInstructions()
{
	SetConsoleTextAttribute(hOut, 62);
	pos.X = 78;
	for(int i=2; i<39; i++)
	{
		pos.Y = i;
		SetConsoleCursorPosition(hOut,pos);
		if(i==2||i==12||i==22||i==32)
			cout << "┏━━━━━━━┓";
		else if(i==6||i==16||i==26||i==36)
			cout << "┗━━━━━━━┛";
		else if(i==3)
			cout << "┃  剩余时间：  ┃";
		else if(i==5)
			cout<<"┃    "<<setw(4)<<setiosflags(ios::fixed)<<setprecision(1)<<(double)TimeLimit/1000<<" s    ┃";
		else if(i==13)
			cout << "┃  当前关卡：  ┃";
		else if(i==15)
			cout << "┃  " << LevelName << "  ┃";  
		else if(i==23)
			cout << "┃  当前模式：  ┃";
		else if(i==25)
		{
			if(TimeLimit==30000) cout << "┃   简单模式   ┃";
			else if(TimeLimit==20000) cout << "┃   普通模式   ┃";
			else if(TimeLimit==15000) cout << "┃   困难模式   ┃";
		}
		else if(i==33)
			cout << "┃ 历史最高分： ┃";
		else if(i==35)
			cout << "┃    " << setw(3) << HighestScore << " 分    ┃";
		else if(i==4||i==14||i==24||i==34)
			cout << "┃              ┃";
	}

	for(int i=39; i<=45; i++)
	{
		SetConsoleTextAttribute(hOut, 143);
		for(int j=4; j<98; j++)
		{
			pos.X = j;
			pos.Y = i;
			SetConsoleCursorPosition(hOut,pos);
			if((i==39||i==43)&&(j==4||j==38||j==72))
				cout << "╔══════════╗";
			else if((i==41||i==45)&&(j==4||j==38||j==72))
				cout << "╚══════════╝";
			else if(i==40 && j==4)
				cout << "║    开始： Space    ║";
			else if(i==44 && j==4)
				cout << "║   返回： BkSpace   ║";
			else if(i==40 && j==38)
				cout << "║     上一关： J     ║";
			else if(i==44 && j==38)
				cout << "║     下一关： K     ║";
			else if(i==40 && j==72)
				cout << "║    移动：方向键    ║";
			else if(i==44 && j==72)
				cout << "║   重新开始： Alt   ║";
		}
	}
	SetConsoleTextAttribute(hOut, 240);
}

//在指定的位置显示倒计时
void labyrinth::ShowTime(int start)
{
		clock_t finish = clock();
		double time;
		if(start == 0)
			time = (double)TimeLimit/1000;
		else 
			time = (double)(TimeLimit-finish+start)/1000;

		pos.Y = 5;
		pos.X = 78;
		SetConsoleCursorPosition(hOut,pos);
		SetConsoleTextAttribute(hOut, 62);
		cout<<"┃    "<<setw(4)<<setiosflags(ios::fixed)<<setprecision(1)<< time <<" s    ┃"; 
}

//显示电脑自动寻路的过程
void labyrinth::ShowFindWay()
{
	ShowMap();
	ShowFindWay(15,15);
	SetConsoleTextAttribute(hOut, 240);
	MessageBoxA(NULL,"即将进入下一关！","迷宫",MB_OK);
}

//私有的递归函数，被ShowFindWay()调用
void labyrinth::ShowFindWay(int X,int Y)
{
	static int Arrive = 0;
	Sleep(120);
	pos.X = 2*Y+8;
	pos.Y = X+4;
	SetConsoleCursorPosition(hOut,pos);
	SetConsoleTextAttribute(hOut, 44);
	cout << "⊕";
	//向上试探
	if(map[X-1][Y] == 1 &&  Arrive == 0){
		map[X][Y] = 2;  //将通路的矩阵值标记为2
		ShowFindWay(X-1,Y);
	}
	//向下试探
	if(map[X+1][Y] == 1 &&  Arrive == 0){
		map[X][Y] = 2;  //将通路的矩阵值标记为2
		ShowFindWay(X+1,Y);
	}
	//向左试探
	if(map[X][Y-1] == 1 &&  Arrive == 0){
		map[X][Y] = 2;   //将通路的矩阵值标记为2
		ShowFindWay(X,Y-1);
	}
	//向右试探
	if(map[X][Y+1] == 1 &&  Arrive == 0){
		map[X][Y] = 2;  //将通路的矩阵值标记为2
		ShowFindWay(X,Y+1);
	}
	if(Arrive==1) return;
	Sleep(80);

	//到达终点就直接返回
	if(X==29 && Y==29)   //当到达(19,19)时，表示找到通路
		Arrive = 1;
	//如果上下左右均为死路，则回退。
	 if(Arrive == 0){
		map[X][Y] = 3;  //将死路的矩阵值标记为3 
		pos.X = 2*Y+8;
		pos.Y = X+4;
		SetConsoleCursorPosition(hOut,pos);
		SetConsoleTextAttribute(hOut, 40);
		cout << "  ";
	}
}

//在地图上显示寻找最短路径的过程
void labyrinth::ShowNearestWay()
{
	ShowMap();
	Way step[1000];       //存放栈元素的结构数组，表示每一步的坐标
	int front,rear;  
	int visited[31][31] = {0};
	rear = front = -1;
	step[++rear].parent = -1;
	step[rear].x = step[rear].y = 15;
	visited[step[rear].x][step[rear].y] = 1;

	while(front<rear)  
	{	
		front++;
		if(visited[step[front].x-1][step[front].y]==0 && map[step[front].x-1][step[front].y]==1)
		{
			rear++;
			step[rear].parent = front;
			step[rear].x = step[front].x - 1;
			step[rear].y = step[front].y;
			visited[step[rear].x][step[rear].y] = 1;
			if(step[rear].x==29 && step[rear].y==29) break;
		}
		if(visited[step[front].x+1][step[front].y]==0 && map[step[front].x+1][step[front].y]==1)
		{
			rear++;
			step[rear].parent = front;
			step[rear].x = step[front].x + 1;
			step[rear].y = step[front].y;
			visited[step[rear].x][step[rear].y] = 1;
			if(step[rear].x==29 && step[rear].y==29) break;
		}
		if(visited[step[front].x][step[front].y-1]==0 && map[step[front].x][step[front].y-1]==1)
		{
			rear++;
			step[rear].parent = front;
			step[rear].x = step[front].x;
			step[rear].y = step[front].y - 1;
			visited[step[rear].x][step[rear].y] = 1;
			if(step[rear].x==29 && step[rear].y==29) break;
		}
		if(visited[step[front].x][step[front].y+1]==0 && map[step[front].x][step[front].y+1]==1)
		{
			rear++;
			step[rear].parent = front;
			step[rear].x = step[front].x;
			step[rear].y = step[front].y + 1;
			visited[step[rear].x][step[rear].y] = 1;
			if(step[rear].x==29 && step[rear].y==29) break;
		}
	}

	Way NearestWay[500];
	int i = rear,top=-1;
	while(step[i].parent!=-1)
	{
		NearestWay[++top] = step[i];
		i = step[i].parent;
	}
	NearestWay[++top] = step[i];
	while(top!=-1)
	{
		Sleep(200);
		pos.X = 2*NearestWay[top].y+8;
		pos.Y = NearestWay[top].x+4;
		SetConsoleCursorPosition(hOut,pos);
		SetConsoleTextAttribute(hOut, 112);
		cout << "  ";

		pos.X = 2*NearestWay[--top].y+8;
		pos.Y = NearestWay[top].x+4;
		SetConsoleCursorPosition(hOut,pos);
		SetConsoleTextAttribute(hOut, 44);
		cout << "◆";
	}
	SetConsoleTextAttribute(hOut, 240);
	MessageBoxA(NULL,"即将进入下一关！","迷宫",MB_OK);
}

//退出游戏的函数
void labyrinth::QuitGame()
{
	ofstream SaveNowLevel("NowLevel.txt",ios::out);
	SaveNowLevel << NowLevel << " ";
	SaveNowLevel.close();

	exit(0);
}


