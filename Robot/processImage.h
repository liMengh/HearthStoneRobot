/*
使用opencv库对图像进行处理
*/
#pragma once
#include <iostream>
#include <fstream>
#include <opencv2\opencv.hpp>
#include <time.h>
#include "opencv2/legacy/legacy.hpp"    //BruteForceMatcher实际在该头文件中  
#include "gameInfo.h"
#include "ControlMouse.h"
using namespace cv;
using namespace std;
//int num[11][9] = { { 0, 1, 1, 1, 0, 1, 1, 1, 1 }, { 0, 1, 1, 0, 1, 1, 0, 1, 1 }, { 1, 0, 1, 1, 0, 1, 0, 1, 0 }, { 0, 1, 0, 0, 1, 1, 0, 0, 1 },
//{ 0, 0, 1, 0, 0, 1, 0, 1, 1 }, { 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 0, 1, 0, 1, 1, 1, 1 }, { 1, 1, 1, 0, 1, 0, 0, 1, 0 },
//{ 0, 0, 1, 0, 1, 1, 1, 0, 0 }, { 1, 1, 1, 1, 0, 1, 0, 0, 0 }, { 0, 1, 1, 1, 0, 1, 1, 1, 0 } };
bool contoursCmp(std::vector<Point> a, std::vector<Point> b)
{
	return a[0].x < b[0].x;
}
vector<vector<vector<bool>>> numModel;
class ProcessImage
{
	vector<Card> allCards;
	
	HWND hWnd;
	VideoWriter outputVideo;
	static ProcessImage* self;
	int gameOverNum = 0;
	ProcessImage(){
		numModel.resize(11);//读取数字模型
		ifstream f;
		f.open("numModel.txt", ios::in);

		bool b;
		char c;
		for (int i = 0; i < 11; ++i)
		{
			vector<vector<bool>> v;
			while (1)
			{
				vector<bool> vb;
				for (int j = 0; j < 9; ++j)
				{
					f >> b >> c;
					cout << b << c;
					vb.push_back(b);
				}
				cout << endl;
				v.push_back(vb);
				if (c != '#') break;
			}
			numModel[i] = v;
		}
		f.close();

		f.open("cardInfo.txt", ios::in);//获取卡组信息
		while (!f.eof())
		{
			Card temp;
			f >> temp.name;
			if (temp.name[0] == '#')
			{
				char c[255];
				f.getline(c, 255);
				continue;
			}
			f >> temp.type >> temp.spend >> temp.attack >> temp.life >> temp.behavior >> temp.explain;
			cout << temp.name << " " << temp.type << " " << temp.spend << " " << temp.attack << " " << temp.life << " " << temp.behavior << " " << temp.explain << endl;
			allCards.push_back(temp);
		}
		f.close();

		hWnd = ::FindWindow("UnityWndClass", ("炉石传说"));
	}

	Mat selfFloorBG;
	Mat otherFloorBG;
	Mat weaponBG;
	Mat gameOverOtherBG;
	Mat gameOverSelfBG;
	Mat startBG;
public:
	static ProcessImage* getInstance()
	{
		if (self == NULL)
		{
			self = new ProcessImage();
		}
		return self;
	}
	void initVideo(Mat &src, GameInfo &gameInfo)
	{
		
		string fname = "HS/video/" + std::to_string(gameInfo.currentTimes) + ".avi";
		gameInfo.currentTimes++;
		outputVideo.open(fname, CV_FOURCC('M', 'J', 'P', 'G'), 1, Size(src.cols, src.rows), true);
	}
	void saveVideo(Mat &src, GameInfo &gameInfo)
	{
		outputVideo << src;
	}
	void releaseVideo()
	{
		outputVideo.release();
	}
	void isGameOver(GameInfo &gameInfo)
	{
		ControlMouse *cm = ControlMouse::getInstance();
		cm->moveToPosition(OutSideX, OutSideY);//移动到场景外，防止干扰
		Sleep(1000);
		Mat src = getCurrentImage();
		Mat sub,sub2;
		src(Rect(460, 75, 110, 120)).copyTo(sub);
		src(Rect(455, 515, 110, 120)).copyTo(sub2);
		
		double b = compareImageBySub(gameOverOtherBG, sub);
		double b2 = compareImageBySub(gameOverSelfBG, sub2);
		cout << "b:" << b << endl;
		cout << "b2:" << b2 << endl;
		if (b < 0.1 || b2 < 0.1)
		{
			++gameOverNum;
			Sleep(1000);
		}
		else 
		{
			gameOverNum = 0;
		}
		if (gameOverNum>3)
		{
			gameInfo.state = STATE_GAMEOVE;
			cout << "游戏结束" << endl;
			/*imshow("1", sub2);
			imshow("2", sub);
			waitKey(0);*/
		}
	}
	void gameOver(Mat& src, GameInfo& gameInfo)
	{
		Mat sub;
		float f;
		saveVideo(src, gameInfo);
		src(Rect(790, 590, 70, 75)).copyTo(sub);
		startBG = imread("HS/startBG.png");
		f = compareImageBySub(startBG, sub);
		cout << f << endl;
		if (f > 0.9)
		{
			cout << "开始新的一局" << endl;
			releaseVideo();
			gameInfo.currentTimes += 1;
			gameInfo.state = STATE_STARTGAME;
		}
	
	}
	void startGame(Mat &src,GameInfo &gameInfo)
	{
		
	}
	void process(GameInfo &gameInfo)
	{
		Mat src = getCurrentImage();
		
		
		switch (gameInfo.state)
		{
		case STATE_STARTGAME:
			initVideo(src, gameInfo);
			break;
		case STATE_CHANGECARDSTART:
			saveVideo(src, gameInfo);
			changeCardStart(src, gameInfo);
			break;
		case STATE_FIGHTSTART:
			saveVideo(src, gameInfo);
			fightStart(src);
			break;
		case STATE_SELFTURN_PLAY:
			saveVideo(src, gameInfo);
			selfTurn(src, gameInfo);
			break;
		case STATE_SELFTURN_FIGHT:
			saveVideo(src, gameInfo);
			selfTurnFight(src, gameInfo);
			break;
		case STATE_OTHERTURN:
			saveVideo(src, gameInfo);
			otherTrun(src, gameInfo);
			isGameOver(gameInfo);
			break;
		case STATE_GAMEOVE:
			gameOver(src,gameInfo);
			break;
		default:
			break;
		}
		
	
	}
	void fightStart(Mat &src)
	{

		//截取背景
		src(Rect(166, 363, 680, 115)).copyTo(selfFloorBG);
		imwrite("HS/self_floor.png", selfFloorBG);
		src(Rect(204, 229, 630, 118)).copyTo(otherFloorBG);
		imwrite("HS/other_floor.png", otherFloorBG);
		src(Rect(337, 534, 112, 110)).copyTo(weaponBG);
		imwrite("HS/weaponBG.png", weaponBG);
		src(Rect(460, 75, 110, 120)).copyTo(gameOverOtherBG);
		imwrite("HS/gameOverOtherBG.png", gameOverOtherBG);
		src(Rect(455, 515, 110, 120)).copyTo(gameOverSelfBG);
		imwrite("HS/gameOverSelfBG.png", gameOverSelfBG);
		
	}
	void changeCardStart(Mat src, GameInfo &gameInfo)//预处理发牌阶段图像
	{
		Mat sure;
		src(Rect(475, 590, 80, 25)).copyTo(sure);
		Mat sureBG = imread("HS/sure.png");
		float ratio = compareImageBySub(sure, sureBG);
		cout <<"有确认按钮可能性：" <<ratio*100 <<"%"<< endl;
		/*imshow("1", sure);
		imshow("2", sureBG);
		waitKey(0);*/
		if (ratio < 0.5) return;//知道有确认按钮（动画结束）时才进行下一步
		for (int i = 0; i<src.rows; i++)//阈值处理，白色留下，其余改为黑色
		{
			for (int j = 0; j<src.cols; j++)
			{
				uchar r = src.at<Vec3b>(i, j)[0];
				uchar g = src.at<Vec3b>(i, j)[1];
				uchar b = src.at<Vec3b>(i, j)[2];
				if (r < 30 && g >= 70 &&g<=140 && b < 30)
				{
					src.at<Vec3b>(i, j)[0] = 0;
					src.at<Vec3b>(i, j)[1] = 0;
					src.at<Vec3b>(i, j)[2] = 0;
				}
				
			}
		}
		Mat gray;
		cvtColor(src, gray, CV_BGR2GRAY);
		//imshow("灰度",gray);
		Mat thr;
		threshold(gray, thr, 50, 255, CV_THRESH_BINARY);
		//imshow("gray", thr);
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
		dilate(thr, thr, element);
		erode(thr, thr, element);
		//erode(thr, thr, element);
		//dilate(thr, thr, element);
		//imshow("fushi", thr);
		//waitKey(0);
		vector<std::vector<Point>> contours;
		findContours(thr,			//图像
			contours,				//轮廓点
			//包含图像拓扑结构的信息（可选参数，这里没有选）
			CV_RETR_EXTERNAL,			//获取轮廓的方法（这里获取外围轮廓）
			CV_CHAIN_APPROX_NONE);		//轮廓近似的方法（这里不近似，获取全部轮廓）
		//打印轮廓信息
		//std::cout << "共有外围轮廓：" << contours.size() << "条" << std::endl;
		//std::vector<std::vector<Point>>::const_iterator itContours = contours.begin();
		/*for (; itContours != contours.end(); ++itContours)
		{
			std::cout << "每个轮廓的长度: " << itContours->size() << std::endl;
		}*/




		Mat result(thr.size(), CV_8U, Scalar(255));
		result.setTo(Scalar(255));
		drawContours(result, contours, -1, Scalar(0), 2);
		//imshow("提取外围轮廓", result);

		//除去太长或者太短的轮廓
		int cmin = 700;
		int cmax = 1000;
		std::vector<std::vector<Point>>::const_iterator itc = contours.begin();
		while (itc != contours.end())
		{
			if (itc->size() < cmin || itc->size() > cmax)
				itc = contours.erase(itc);
			else
				++itc;
		}

		//画出轮廓

		//画出轮廓，参数为：画板，轮廓，轮廓指示（这里画出所有轮廓），颜色，线粗
		//	drawContours(result, contours, -1, Scalar(0), 2);
		//将轮廓重绘于白板上
		result.setTo(Scalar(255));
		drawContours(result, contours, -1, Scalar(0), 1);
		//imshow("去除较小的", result); //还要注意提取轮廓的方法还有很多种，比如CV_RETR_LIST代表所有轮廓
		//waitKey(0);
		//轮廓的形状描述子
		//外接矩形
		Mat matio[10];
		int num = contours.size();
		gameInfo.currentNum = num;
		for (int i = 0; i < contours.size(); ++i)
		{
			Rect r0 = boundingRect(Mat(contours[i]));
			src(r0).copyTo(matio[i]);
			rectangle(src, r0, Scalar(0), 2);
			/*imshow("" + i, matio[i]);
			waitKey(0);*/
			//gameInfo.handCard[i].spend = getNumByFullCard(matio[i]);
			
			int cardID = compareImageByORB(matio[i]);
			gameInfo.handCard[i] = allCards[cardID];
			gameInfo.handCard[i].x = r0.x + r0.width / 2;
			gameInfo.handCard[i].y = r0.y + r0.height / 2;
			cout << "换牌阶段：识别出的卡牌信息：名称：" << gameInfo.handCard[i].name << " 费用：" << gameInfo.handCard[i].spend << " 位置:" << gameInfo.handCard[i].x << " " << gameInfo.handCard[i].y << endl;


		}
	//	imshow("外接矩形", src);
	}
	int getNumByImage(Mat src)
	{
		Mat gray;
		cvtColor(src, gray, CV_BGR2GRAY);
		Mat thr;
		threshold(gray, thr, 250, 255, CV_THRESH_BINARY);
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
		Mat cz;
		dilate(thr, cz, element);
		erode(cz, cz, element);
		//	imshow("gray2", cz);
		//	waitKey(0);
		vector<std::vector<Point>> contours;
		findContours(cz,			//图像
			contours,				//轮廓点
			//包含图像拓扑结构的信息（可选参数，这里没有选）
			CV_RETR_EXTERNAL,			//获取轮廓的方法（这里获取外围轮廓）
			CV_CHAIN_APPROX_NONE);		//轮廓近似的方法（这里不近似，获取全部轮廓）
		//打印轮廓信息
		/*std::cout << "共有外围轮廓：" << contours.size() << "条" << std::endl;
		for (int i = 0; i < contours.size(); ++i)
		{
			cout << contours[i].size() << endl;;
		}*/
		Rect r0 = boundingRect(Mat(contours[contours.size() - 1]));
		Mat matio;
		thr(r0).copyTo(matio);
		rectangle(src, r0, Scalar(0), 2);
		//	imshow("222", matio);
		//	waitKey(0);
		return recoNum(matio);
	}
	int getNumByFullCard(Mat src)//提取数字部分轮廓
	{
		Mat image;
		src(Rect(0, 0, src.rows / 4, src.cols / 3)).copyTo(image);
		return getNumByImage(image);		
	}
	int recoNum(Mat src)//数字识别
	{
		Mat sub[9];
		int h = src.rows / 3;
		int w = src.cols / 3;
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				src(Rect(j*w, i*h, w, h)).copyTo(sub[i * 3 + j]);
				/*imshow("sub", sub[i*3 + j]);
				waitKey(0);*/
			}
		}
		bool temp[9] = { 0 };
		for (int i = 0; i < 9; ++i)
		{
			int num = 0;
			for (int j = 0; j < sub[i].cols; ++j)
			{
				for (int k = 0; k < sub[i].rows; ++k)
				{
					if (sub[i].at<uchar>(k, j) == 255)
					{
						num++;
					}
				}
			}
			
			if (num>w*h / 2) temp[i] = 1, cout << "1" << ",";
			else cout << "0" << ",";
			//if (i % 3 == 2) cout << endl;
			//cout << num << " "<<endl;
		}
		int maxsum = 0, maxn = 0;
		for (int i = 0; i < 11; ++i)
		{
			for (int k = 0; k < numModel[i].size(); ++k)
			{
				int sum = 0;
				for (int j = 0; j < 9; ++j)
				{
					if (temp[j] == numModel[i][k][j])
					{
						++sum;
					}
				}
				if (sum>maxsum)
				{
					maxsum = sum;
					maxn = i;
				}
			}
		}
		cout << "识别出的数字是：" << maxn << endl;
		return maxn;
	}
	void recoHandCrad(Mat src, GameInfo &gameInfo)
	{
		//识别手牌
		clock_t startT = clock();
		Mat image, src2;
		src(Rect(260, 660, 440, src.rows - 660)).copyTo(src2);
		src2.copyTo(image);
		for (int i = 0; i<src2.rows; i++)//阈值处理，白色留下，其余改为黑色
		{
			for (int j = 0; j<src2.cols; j++)
			{
				uchar r = src2.at<Vec3b>(i, j)[0];
				uchar g = src2.at<Vec3b>(i, j)[1];
				uchar b = src2.at<Vec3b>(i, j)[2];
				if (r == 255 && g == 255 && b == 255)
				{
					image.at<Vec3b>(i, j)[0] = 255;
					image.at<Vec3b>(i, j)[1] = 255;
					image.at<Vec3b>(i, j)[2] = 255;
				}
				else
				{
					image.at<Vec3b>(i, j)[0] = 0;
					image.at<Vec3b>(i, j)[1] = 0;
					image.at<Vec3b>(i, j)[2] = 0;
				}
			}
		}
		Mat gray;
		cvtColor(image, gray, CV_BGR2GRAY);//转换为单通道便于进行轮廓提取
		//imshow("gray", gray);

		//Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));//腐蚀膨胀效果不好
		//dilate(thr, thr, element);
		//erode(thr, thr, element);
		//dilate(thr, thr, element);
		//erode(thr, thr, element);
		//imshow("fushi", thr);
		vector<std::vector<Point>> contours;
		findContours(gray,			//图像
			contours,				//轮廓点
			//包含图像拓扑结构的信息（可选参数，这里没有选）
			CV_RETR_EXTERNAL,			//获取轮廓的方法（这里获取外围轮廓）
			CV_CHAIN_APPROX_NONE);		//轮廓近似的方法（这里不近似，获取全部轮廓）
		double durationT = (double)(clock() - startT) / CLOCKS_PER_SEC;
		cout << "手牌识别阶段，提取出数字部分用时:" << durationT << "秒" << endl;
		//打印轮廓信息
		//std::cout << "共有外围轮廓：" << contours.size() << "条" << std::endl;
		//std::vector<std::vector<Point>>::const_iterator itContours = contours.begin();
		/*for (; itContours != contours.end(); ++itContours)
		{
		std::cout << "每个轮廓的长度: " << itContours->size() << std::endl;
		}*/




		/*	Mat result(gray.size(), CV_8U, Scalar(255));
		result.setTo(Scalar(255));
		drawContours(result, contours, -1, Scalar(0), 2);
		imshow("提取外围轮廓", result);*/

		//除去太长或者太短的轮廓
		int cmin = 30;
		int cmax = 100;
		std::vector<std::vector<Point>>::const_iterator itc = contours.begin();
		while (itc != contours.end())
		{
			cout << "手牌数字部分轮廓大小：" << itc->size()<<endl;
			if (itc->size() < cmin || itc->size() > cmax)
				itc = contours.erase(itc);
			else
				++itc;
		}

		//画出轮廓，参数为：画板，轮廓，轮廓指示（这里画出所有轮廓），颜色，线粗
		//	drawContours(result, contours, -1, Scalar(0), 2);
		//将轮廓重绘于白板上
		//result.setTo(Scalar(255));
		//drawContours(result, contours, -1, Scalar(0), 1);
		//imshow("去除较小的", result); //还要注意提取轮廓的方法还有很多种，比如CV_RETR_LIST代表所有轮廓
		//waitKey(0);
		//轮廓的形状描述子
		//外接矩形
		Mat matio;
		gameInfo.currentNum = 0;
		for (int i = 0; i < contours.size(); ++i)
		{
			//cout << contours[i].size() << endl;
			Rect r0 = boundingRect(Mat(contours[i]));
			if (r0.width >= r0.height) continue;
			src2(r0).copyTo(matio);
			//rectangle(src2, r0, Scalar(0), 2);
			/*imshow("iiiiiiiii", matio);
			waitKey(0);*/
			//gameInfo.handCard[gameInfo.currentNum].spend = getNumByImage(matio[i]);
			int x = 260 + r0.x + r0.width / 2 + 10;
			int y = 660 + r0.y + r0.height / 2 + 30;
			ControlMouse *con = ControlMouse::getInstance();
			con->moveToPosition(x, y);
			Rect r1(x - 100 - 10, y - 320 - 30, 250, 350);

			//cout << gameInfo.handCard[gameInfo.currentNum].x << " " << gameInfo.handCard[gameInfo.currentNum].y << endl;
			//cout << r1.x << " " << r1.y << " " << r1.width << " " << r1.height << endl;
			Mat image = getCurrentImage();
			//imshow("1111111111", image); waitKey(0);
			/*imshow("test1", image);
			waitKey(0);*/
			Mat image1;
			image(r1).copyTo(image1);
			/*imshow("test", image1);
			waitKey(0);*/
			clock_t start = clock();
			int cardID = compareImageByORB(image1);
			double duration = (double)(clock() - start) / CLOCKS_PER_SEC;
			cout << "比较一张卡牌用时:" << duration << "秒" << endl;
			gameInfo.handCard[gameInfo.currentNum] = allCards[cardID];
			gameInfo.handCard[gameInfo.currentNum].x = x;
			gameInfo.handCard[gameInfo.currentNum].y = y;
			cout << "出牌阶段：识别出的卡牌信息：名称：" << gameInfo.handCard[gameInfo.currentNum].name << " 费用：" << gameInfo.handCard[gameInfo.currentNum].spend << " 位置:" << gameInfo.handCard[gameInfo.currentNum].x << " " << gameInfo.handCard[gameInfo.currentNum].y << endl;
			gameInfo.currentNum++;

		}
		//imshow("外接矩形", src);

		//	findContours(thr,			//图像 
		//	contours,				//轮廓点
		//	//包含图像拓扑结构的信息（可选参数，这里没有选）
		//	CV_RETR_LIST,			//获取轮廓的方法（这里获取所有轮廓）
		//	CV_CHAIN_APPROX_NONE);		//轮廓近似的方法（这里不近似，获取全部轮廓
		////画出轮廓
		//drawContours(result, contours, -1, Scalar(0), 2);
		//imshow("提取所有轮廓", result);
	}
	void recoSelfMonster(Mat src, GameInfo &gameInfo)
	{
		Mat floor = imread("HS/self_floor.png",1);
		Mat out;
		Mat gray,floor_gray;
		cvtColor(src, gray, CV_BGR2GRAY);
		cvtColor(floor, floor_gray, CV_BGR2GRAY);
		//imshow("yuan", src);
		//imshow("floor", floor);
		subtract(floor_gray, gray, out);
		cout << "当前场面减去场面背景成功！" << endl;
		//imshow("test", out);
		//waitKey(0);
		Mat thr;
		threshold(out, thr, 40, 255, CV_THRESH_BINARY);
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));//腐蚀膨胀效果不好
		dilate(thr, thr, element);
		erode(thr, thr, element);
		dilate(thr, thr, element);
		erode(thr, thr, element);
		cout << "腐蚀膨胀结束！" << endl;
		/*imshow("thr", thr);
		waitKey(0);*/

		int sum = 0, bg = 0, ed = 0;
		for (int i = 0; i < thr.cols; ++i)
		{
			if (thr.at<uchar>(thr.rows / 2, i) == 255)
			{
				if (bg == 0) bg = i;
				else ed = i;
			}
		}
		cout << bg << " " << ed << endl;
		int count = (ed - bg + 95) / 96;
		gameInfo.selfMonsterNum = 0;
		for (int i = 0; i < count; ++i)
		{
			Rect r0(bg + i * 96, 0, 96, thr.rows);
			Mat temp;
			if (r0.x + r0.width>src.cols || r0.y+r0.height>src.rows) continue;
			src(r0).copyTo(temp);
			/*imshow("tests", temp);
			waitKey(0);*/
			int x = r0.x + r0.width / 2;
			int y = r0.y + r0.height / 2;
			x += 10 + 166;
			y += 30 + 363;


			ControlMouse *cm = ControlMouse::getInstance();
			cm->moveToPosition(x, y);
			Sleep(1000);
			Mat image = getCurrentImage();
			Mat sub;
			image(Rect(r0.x + r0.width + 166 + 10, r0.y + 363 - r0.height / 2 - 30, 205, 275)).copyTo(sub);
			/*imshow("sub",sub);
			waitKey(0);*/
			int cardID = compareImageByORB(sub);
			gameInfo.selfMonster[gameInfo.selfMonsterNum] = allCards[cardID];
			gameInfo.selfMonster[gameInfo.selfMonsterNum].x = x;
			gameInfo.selfMonster[gameInfo.selfMonsterNum].y = y;

			//识别数值
			Mat attackImage;
			temp(Rect(0, 0, temp.cols / 2, temp.rows)).copyTo(attackImage);//读取左侧数字
			gameInfo.otherMonster[gameInfo.selfMonsterNum].attack = recoImageNum(attackImage);
			Mat lifeImage;
			temp(Rect(temp.cols / 2, 0, temp.cols / 2, temp.rows)).copyTo(lifeImage);//读取左侧数字
			gameInfo.otherMonster[gameInfo.selfMonsterNum].life = recoImageNum(lifeImage);

			cout << "出牌阶段：识别己方场上随从：个数：" << gameInfo.selfMonsterNum + 1 << " 名称：" << gameInfo.selfMonster[gameInfo.selfMonsterNum].name << " 攻击：" << gameInfo.otherMonster[gameInfo.selfMonsterNum].attack << " 生命：" << gameInfo.otherMonster[gameInfo.selfMonsterNum].life << " 费用：" << gameInfo.selfMonster[gameInfo.selfMonsterNum].spend << " 位置:" << gameInfo.selfMonster[gameInfo.selfMonsterNum].x << " " << gameInfo.selfMonster[gameInfo.selfMonsterNum].y << endl;
			gameInfo.selfMonsterNum++;
		}

		//vector<std::vector<Point>> contours;
		//findContours(thr,			//图像
		//	contours,				//轮廓点
		//	//包含图像拓扑结构的信息（可选参数，这里没有选）
		//	CV_RETR_EXTERNAL,			//获取轮廓的方法（这里获取外围轮廓）
		//	CV_CHAIN_APPROX_NONE);		//轮廓近似的方法（这里不近似，获取全部轮廓）
		//cout << "获取轮廓结束！" << endl;
		//int cmin = 300;
		//Mat matio;
		//gameInfo.selfMonsterNum = 0;
		//for (int i = 0; i < contours.size(); ++i)
		//{
		//	cout << "识别自己场上随从：" << contours[i].size() << endl;
		//	if ((int)contours[i].size() > cmin)
		//	{
		//		//cout <<contours[i].size() << endl;
		//		Rect r0 = boundingRect(Mat(contours[i]));
		//		//cout << r0.x << " " << r0.y << " " << r0.width << " " << r0.height << endl;
		//	//	if (r0.width >= r0.height) continue;
		//		src(r0).copyTo(matio);
		//		/*imshow("tests",matio);
		//		waitKey(0);*/
		//		int x = r0.x + r0.width / 2;
		//		int y = r0.y + r0.height / 2;
		//		x += 10 + 166;
		//		y += 30 + 363;
		//		ControlMouse *cm = ControlMouse::getInstance();
		//		cm->moveToPosition(x, y);
		//		Sleep(1000);
		//		Mat image = getCurrentImage();
		//		Mat sub;
		//		image(Rect(r0.x + r0.width+166+10, r0.y+363-r0.height/2-30, 205, 275)).copyTo(sub);
		//		/*imshow("sub",sub);
		//		waitKey(0);*/
		//		int cardID = compareImageByORB(sub);
		//		gameInfo.selfMonster[gameInfo.selfMonsterNum] = allCards[cardID];
		//		gameInfo.selfMonster[gameInfo.selfMonsterNum].x = x;
		//		gameInfo.selfMonster[gameInfo.selfMonsterNum].y = y;
		//		cout << "出牌阶段：识别己方场上随从：个数：" << gameInfo.selfMonsterNum+1<<" 名称：" << gameInfo.selfMonster[gameInfo.selfMonsterNum].name << " 费用：" << gameInfo.selfMonster[gameInfo.selfMonsterNum].spend << " 位置:" << gameInfo.selfMonster[gameInfo.selfMonsterNum].x << " " << gameInfo.selfMonster[gameInfo.selfMonsterNum].y << endl;
		//		gameInfo.selfMonsterNum++;
		//		
		//	}
		//}
		cout << "识别自己场上随从结束！" << endl;
		
	}
	bool isTaunt(Mat src)
	{
		Mat src2,image;
		src.copyTo(src2);
		src.copyTo(image);
		double sum = 0;
		for (int i = 0; i<src2.rows; i++)//阈值处理，白色留下，其余改为黑色
		{
			for (int j = 0; j<src2.cols; j++)
			{
				uchar r = src2.at<Vec3b>(i, j)[0];
				uchar g = src2.at<Vec3b>(i, j)[1];
				uchar b = src2.at<Vec3b>(i, j)[2];
				if (r<200 && g<200 && b<200 && r>40 && g>40 && b>40 && abs(r - g)<25 && abs(r - b)<25 && abs(g - b)<25)
				{
					sum++;
					image.at<Vec3b>(i, j)[0] = 255;
					image.at<Vec3b>(i, j)[1] = 255;
					image.at<Vec3b>(i, j)[2] = 255;
				}
				else
				{
					image.at<Vec3b>(i, j)[0] = 0;
					image.at<Vec3b>(i, j)[1] = 0;
					image.at<Vec3b>(i, j)[2] = 0;
				}
			}
		}
		cout << "sum:" << sum << "宽 高:" << src2.rows << " "<<src2.cols << endl;
		/*imshow("image", image);
		waitKey(0);*/
		return sum>2500;
	}
	
	int recoImageNum(Mat src)
	{
		//imshow("b", src);
		for (int i = 0; i<src.rows; i++)//阈值处理，白色留下，其余改为黑色
		{
			for (int j = 0; j<src.cols; j++)
			{
				uchar b = src.at<Vec3b>(i, j)[0];
				uchar g = src.at<Vec3b>(i, j)[1];
				uchar r = src.at<Vec3b>(i, j)[2];
				//cout << (int)r << " " << (int)g << " " << (int)b << endl;
				if ((r >= 230 && g >= 230 && b >= 230) || (r == 0 && g == 255 && b == 0) || (r == 255 && g == 0 && b == 0))
				{
					src.at<Vec3b>(i, j)[0] = 255;
					src.at<Vec3b>(i, j)[1] = 255;
					src.at<Vec3b>(i, j)[2] = 255;
				}
				else
				{
					src.at<Vec3b>(i, j)[0] = 0;
					src.at<Vec3b>(i, j)[1] = 0;
					src.at<Vec3b>(i, j)[2] = 0;
				}
			}
		}
		Mat gray;
		cvtColor(src, gray, CV_BGR2GRAY);//转换为单通道便于进行轮廓提取
		//imshow("a", gray);
		//waitKey(0);
		vector<std::vector<Point>> contours;
		findContours(gray,			//图像
			contours,				//轮廓点
			//包含图像拓扑结构的信息（可选参数，这里没有选）
			CV_RETR_EXTERNAL,			//获取轮廓的方法（这里获取外围轮廓）
			CV_CHAIN_APPROX_NONE);		//轮廓近似的方法（这里不近似，获取全部轮廓）
		//打印轮廓信息
		std::cout << "识别图像数值：共有外围轮廓：" << contours.size() << "条" << std::endl;
		std::vector<std::vector<Point>>::const_iterator itContours = contours.begin();
		for (; itContours != contours.end(); ++itContours)
		{
			std::cout << "每个轮廓的长度: " << itContours->size() << std::endl;
		}
		//除去太长或者太短的轮廓
		int cmin = 30;
		int cmax = 100;
		std::vector<std::vector<Point>>::const_iterator itc = contours.begin();
		while (itc != contours.end())
		{
			cout << "手牌数字部分轮廓大小：" << itc->size() << endl;
			if (itc->size() < cmin || itc->size() > cmax)
				itc = contours.erase(itc);
			else
				++itc;
		}



		Mat result(gray.size(), CV_8U, Scalar(255));
		result.setTo(Scalar(255));
		drawContours(result, contours, -1, Scalar(0), 2);
	//	imshow("提取外围轮廓", result);
	//	waitKey(0);
		Mat matio;
		int num = contours.size();
		int sum = 0;
		sort(contours.begin(), contours.begin() + contours.size(), contoursCmp);
		for (int i = 0; i < contours.size(); ++i)
		{
			Rect r0 = boundingRect(Mat(contours[i]));
			if (r0.width >= r0.height) continue;
			src(r0).copyTo(matio);
			rectangle(src, r0, Scalar(255), 2);
			//imshow("", matio);
			
			sum = sum*10+recoNum(matio);
			//waitKey(0);
		}
		return sum;
	}
	void recoOtherMonster(Mat src, GameInfo &gameInfo)
	{
		Mat floor = imread("HS/other_floor.png", 0);
		Mat out;
		Mat gray;
		cvtColor(src, gray, CV_BGR2GRAY);
		/*imshow("yuan", gray);
		imshow("floor", floor);*/
		subtract(floor, gray, out);
		/*imshow("test", out);
		waitKey(0);*/
		Mat thr;
		threshold(out, thr, 50, 255, CV_THRESH_BINARY);
		//Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));//腐蚀膨胀效果不好
		//dilate(thr, thr, element);
		//erode(thr, thr, element);
		//dilate(thr, thr, element);
		//erode(thr, thr, element);
		

		int sum = 0,bg=0,ed=0;
		for (int i = 0; i < thr.cols; ++i)
		{
			if (thr.at<uchar>(thr.rows / 2, i) == 255)
			{
				if (bg == 0) bg = i;
				else ed = i;
			}
		}
		cout << bg << " " << ed << endl;
		int count = (ed - bg+95) / 96;
		gameInfo.otherMonsterNum = 0;
		for (int i = 0; i < count; ++i)
		{
			Rect r0(bg + i * 96, 0, 96, thr.rows);
			Mat temp;
			if (r0.x + r0.width>src.cols || r0.y + r0.height>src.rows) continue;
			src(r0).copyTo(temp);
			/*imshow("tests", temp);
			waitKey(0);*/
			int x = r0.x+r0.width / 2+10;
			int y = r0.height/2;
			x += 204;
			y +=  229+40;
			
			//imshow("1",temp);
			//waitKey(0);
			gameInfo.otherMonster[gameInfo.otherMonsterNum].taugh = isTaunt(temp);
			gameInfo.otherMonster[gameInfo.otherMonsterNum].x = x;
			gameInfo.otherMonster[gameInfo.otherMonsterNum].y = y;

			//识别数值
			Mat attackImage;
			temp(Rect(0, 0, temp.cols / 2, temp.rows)).copyTo(attackImage);//读取左侧数字
			
			gameInfo.otherMonster[gameInfo.otherMonsterNum].attack = recoImageNum(attackImage);
			Mat lifeImage;
			temp(Rect(temp.cols / 2, 0, temp.cols/2, temp.rows)).copyTo(lifeImage);//读取左侧数字
			gameInfo.otherMonster[gameInfo.otherMonsterNum].life = recoImageNum(lifeImage);

			cout << "出牌阶段：对方场上卡牌信息：是否嘲讽：" << gameInfo.otherMonster[gameInfo.otherMonsterNum].taugh << " 攻击：" << gameInfo.otherMonster[gameInfo.otherMonsterNum].attack << " 生命：" << gameInfo.otherMonster[gameInfo.otherMonsterNum].life<< " 位置:" << gameInfo.otherMonster[gameInfo.otherMonsterNum].x << " " << gameInfo.otherMonster[gameInfo.otherMonsterNum].y << endl;
			gameInfo.otherMonsterNum++;
			/*imshow("sub",sub);
			waitKey(0);*/
		}

		//vector<std::vector<Point>> contours;
		//findContours(thr,			//图像
		//	contours,				//轮廓点
		//	//包含图像拓扑结构的信息（可选参数，这里没有选）
		//	CV_RETR_EXTERNAL,			//获取轮廓的方法（这里获取外围轮廓）
		//	CV_CHAIN_APPROX_NONE);		//轮廓近似的方法（这里不近似，获取全部轮廓）

		//int cmin = 100;
		//Mat matio[10];
		//std::vector<std::vector<Point>>::const_iterator itc = contours.begin();
		//gameInfo.otherMonsterNum = 0;
		//for (int i = 0; i < contours.size(); ++i)
		//{
		//	cout << itc->size() << endl;
		//	if (itc->size() > cmin)
		//	{
		//		Rect r0 = boundingRect(Mat(contours[i]));
		//		if (r0.width >= r0.height) continue;
		//		src(r0).copyTo(matio[0]);
		//		imshow("tests",matio[0]);
		//		waitKey(0);
		//		int x = r0.width/2 ;
		//		int y = r0.height/2;
		//		x += 204;
		//		y +=  229+40;
		//		/*ControlMouse *cm = ControlMouse::getInstance();
		//		cm->moveToPosition(x, y);
		//		Sleep(1000);
		//		HWND hWnd = ::FindWindow("UnityWndClass", ("炉石传说"));
		//		Mat image = getCurrentImage(hWnd);
		//		Mat sub;
		//		image(Rect(x, y, 102, 126)).copyTo(sub);
		//		imshow("sub", sub);
		//		waitKey(0);*/
		//	
		//		gameInfo.otherMonster[gameInfo.otherMonsterNum].taugh = isTaunt(matio[0]);
		//		gameInfo.otherMonster[gameInfo.otherMonsterNum].x = x;
		//		gameInfo.otherMonster[gameInfo.otherMonsterNum].y = y;
		//		cout << "出牌阶段：对方场上卡牌信息：是否嘲讽：" << gameInfo.otherMonster[gameInfo.otherMonsterNum].taugh << " 位置:" << gameInfo.otherMonster[gameInfo.otherMonsterNum].x << " " << gameInfo.otherMonster[gameInfo.otherMonsterNum].y << endl;
		//		gameInfo.otherMonsterNum++;
		//		/*imshow("sub",sub);
		//		waitKey(0);*/
		//	}
		//	++itc;
		//}
	}
	void selfTurn(Mat src, GameInfo &gameInfo)//处理出牌阶段场景
	{
		clock_t start = clock();
		recoHandCrad(src, gameInfo);
		double duration = (double)(clock() - start) / CLOCKS_PER_SEC;
		cout << "识别手牌总用时:"<<duration<<"秒"<< endl;
		cout << "判断英雄是否有武器" << endl;
		clock_t start2 = clock();
		Mat weapon;
		src(Rect(337, 534, 112, 110)).copyTo(weapon);
		Mat weaponBG = imread("HS/weaponBG.png");
		double rs = compareImageBySub(weapon, weaponBG);
		/*imshow("wuqi", weapon);
		imshow("bg", weaponBG);

		waitKey(0);*/
		cout << "武器匹配度：" << (1-rs) * 100 << "%" << endl;
		double duration2 = (double)(clock() - start2) / CLOCKS_PER_SEC;
		cout << "判断是否有武器用时:" << duration2 << "秒" << endl;
		gameInfo.haveWeapon = false;
		if (rs < 0.9)
		{
			cout << "英雄有武器:" << rs << endl;
			gameInfo.haveWeapon = true;
		}
		/*ControlMouse *controlMouse = ControlMouse::getInstance();
		controlMouse->moveToPosition(1000, 700);
		Sleep(1000);*/
		////识别已方场上随从
		//Mat selfFloor;
		//src(Rect(166, 363, 680, 115)).copyTo(selfFloor);
		//recoSelfMonster(selfFloor, gameInfo);
		//
		////识别对方场上随从
		//Mat otherFloor;
		//src(Rect(204, 229, 630, 118)).copyTo(otherFloor);
		//recoOtherMonster(otherFloor, gameInfo);
	}
	void selfTurnFight(Mat src, GameInfo &gameInfo)
	{
		cout << "识别已方场上随从" << endl;
		Mat selfFloor;
		src(Rect(166, 363, 680, 115)).copyTo(selfFloor);
		recoSelfMonster(selfFloor, gameInfo);
		//cout << "判断英雄是否有武器" << endl;
		//clock_t start = clock();
		//Mat weapon;
		//src(Rect(337, 534, 112, 110)).copyTo(weapon);
		//Mat weaponBG = imread("HS/weaponBG.png");
		//double rs = compareImageBySub(weapon, weaponBG);
		///*imshow("wuqi", weapon);
		//imshow("bg", weaponBG);
		//
		//waitKey(0);*/
		//cout << "武器匹配度：" << rs*100<<"%" << endl;
		//double duration = (double)(clock() - start) / CLOCKS_PER_SEC;
		//cout << "判断是否有武器用时:" << duration << "秒" << endl;
		if (gameInfo.haveWeapon )//if (rs < 0.9)
		{
			//cout << "英雄有武器:"<<rs << endl;
			gameInfo.selfMonster[gameInfo.selfMonsterNum].x = 520;
			gameInfo.selfMonster[gameInfo.selfMonsterNum].y = 620;
			gameInfo.selfMonsterNum++;
		}
		cout << "识别对方场上随从" << endl;
		Mat otherFloor;
		src(Rect(204, 229, 630, 118)).copyTo(otherFloor);
		recoOtherMonster(otherFloor, gameInfo);
		Sleep(1000);
	}
	Mat getCurrentImage()
	{
		HDC hDC = ::GetWindowDC(hWnd);//得到句柄
		assert(hDC);

		HDC hMemDC = ::CreateCompatibleDC(hDC);//建立兼容dc

		assert(hMemDC);

		RECT rc;
		//	::GetWindowRect(hWnd, &rc);//此矩形包括程序外边框
		::GetClientRect(hWnd, &rc);//得到界面部分矩形大小
		HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, rc.right - rc.left, rc.bottom - rc.top);//兼容位图
		assert(hBitmap);

		HBITMAP hOldBmp = (HBITMAP)::SelectObject(hMemDC, hBitmap);

		::PrintWindow(hWnd, hMemDC, PW_CLIENTONLY);//得到画面

		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;
		BITMAPINFOHEADER bi = { 0 };
		CONST int nBitCount = 24;
		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = width;
		bi.biHeight = height;
		bi.biPlanes = 1;
		bi.biBitCount = nBitCount;
		bi.biCompression = BI_RGB;
		DWORD dwSize = width * 3 * height;
		unsigned char *data = new unsigned char[dwSize];
		::GetDIBits(hMemDC, hBitmap, 0, height, data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);//将画面输出到data中
		Mat image(height, width, CV_8UC3, data);//将uchar转换为mat类型，用于opencv
		flip(image, image, 0);//图像上下反转

		::SelectObject(hMemDC, hOldBmp);
		::DeleteObject(hBitmap);
		::DeleteObject(hMemDC);
		::ReleaseDC(hWnd, hDC);
		
		return image;
	}
	void otherTrun(Mat src, GameInfo &gameInfo)
	{
		Sleep(1000);
		Mat image;
		src(Rect(924, 340, 45, 10)).copyTo(image);
		int i = 0, j = 0;
		uchar r = image.at<Vec3b>(i, j)[0];
		uchar g = image.at<Vec3b>(i, j)[1];
		uchar b = image.at<Vec3b>(i, j)[2];
		if (abs(r - b) > 100 || abs(g - b) > 100)
		{
			Sleep(5000);
			gameInfo.state = STATE_SELFTURN_PLAY;
			//process(gameInfo);
			Mat src = getCurrentImage();
			selfTurn(src, gameInfo);
		}
	}
	float compareImageBySub(Mat &a, Mat &a2)
	{
		int sum = 0;
		for (int i = 0; i<a.rows; i++)//阈值处理，白色留下，其余改为黑色
		{
			for (int j = 0; j<a.cols; j++)
			{
				uchar r = a.at<Vec3b>(i, j)[0];
				uchar g = a.at<Vec3b>(i, j)[1];
				uchar b = a.at<Vec3b>(i, j)[2];

				uchar r2 = a2.at<Vec3b>(i, j)[0];
				uchar g2 = a2.at<Vec3b>(i, j)[1];
				uchar b2 = a2.at<Vec3b>(i, j)[2];
				if (abs(r - r2)<20 && abs(b2 - b)<20 && abs(g - g2)<20)
				{
					sum++;	
				}
				
			}
		}
		return (float)sum/(a.rows*a.cols);
	}
	int compareImage(Mat &a, Mat &b)
	{
		ORB orb;
		vector<KeyPoint> keyPoints_1, keyPoints_2;
		Mat descriptors_1, descriptors_2;
		orb(a, Mat(), keyPoints_1, descriptors_1);
		orb(b, Mat(), keyPoints_2, descriptors_2);
		//	cout << clock() - t_start << endl;
		//2.BruteForceMatcher匹配
		BruteForceMatcher<HammingLUT> matcher;  //也可以使用ruteForce<Hamming>
		vector<DMatch> matches;
		matcher.match(descriptors_1, descriptors_2, matches);
		//	cout << matches.size() << endl;
		//	cout << clock() - t_start << endl;
		//3.过滤匹配点
		double max_dist = 0; double min_dist = 100;
		//-- Quick calculation of max and min distances between keypoints
		for (int i = 0; i < descriptors_1.rows; i++)
		{
			double dist = matches[i].distance;
			//cout << ":"<<dist << endl;
			if (dist < min_dist) min_dist = dist;
			if (dist > max_dist) max_dist = dist;
		}
		//	printf("-- Max dist : %f \n", max_dist);
		//	printf("-- Min dist : %f \n", min_dist);
		//	cout << clock() - t_start << endl;
		//-- Draw only "good" matches (i.e. whose distance is less than 0.6*max_dist )
		//-- PS.- radiusMatch can also be used here.
		std::vector< DMatch > good_matches;
		for (int i = 0; i < descriptors_1.rows; i++)
		{
			if (matches[i].distance <= 0.6*max_dist)
			{
				good_matches.push_back(matches[i]);
			}
		}
		return (int)good_matches.size();
	}
	int compareImageByORB(Mat img_1)
	{
		clock_t t_start = clock();
		Mat img_2;
		int maxi=1, maxn=0;
		for (int i = 0; i < 30; ++i)
		{
			//cout << "弟" << i << "张图片" << endl;
			stringstream ss;
			ss << i;
			string s = ss.str();
			img_2 = imread("HS/T7/"+s+".png");
			if (!img_1.data || !img_2.data)
			{
		//		cout << "error reading images " << endl;
				break;
			}
		//	cout << clock() - t_start << endl;
			//1.orb检测特征点并提取特征值
			ORB orb;
			vector<KeyPoint> keyPoints_1, keyPoints_2;
			Mat descriptors_1, descriptors_2;
			orb(img_1, Mat(), keyPoints_1, descriptors_1);
			orb(img_2, Mat(), keyPoints_2, descriptors_2);
		//	cout << clock() - t_start << endl;
			//2.BruteForceMatcher匹配
			BruteForceMatcher<HammingLUT> matcher;  //也可以使用ruteForce<Hamming>
			vector<DMatch> matches;
			matcher.match(descriptors_1, descriptors_2, matches);
		//	cout << matches.size() << endl;
		//	cout << clock() - t_start << endl;
			//3.过滤匹配点
			double max_dist = 0; double min_dist = 100;
			//-- Quick calculation of max and min distances between keypoints
			for (int i = 0; i < descriptors_1.rows; i++)
			{
				double dist = matches[i].distance;
				if (dist < min_dist) min_dist = dist;
				if (dist > max_dist) max_dist = dist;
			}
		//	printf("-- Max dist : %f \n", max_dist);
		//	printf("-- Min dist : %f \n", min_dist);
		//	cout << clock() - t_start << endl;
			//-- Draw only "good" matches (i.e. whose distance is less than 0.6*max_dist )
			//-- PS.- radiusMatch can also be used here.
			std::vector< DMatch > good_matches;
			for (int i = 0; i < descriptors_1.rows; i++)
			{
				if (matches[i].distance < 0.6*max_dist)
				{
					good_matches.push_back(matches[i]);
				}
			}
		//	cout << "goodMatches:" << good_matches.size() << endl;
		//	cout << clock() - t_start << endl;
			if (good_matches.size()>maxn)
			{
				maxn = good_matches.size();
				maxi = i;
			}
		}
		/*imshow("原图像", img_1);
		stringstream ss;
		ss << maxi;
		string s = ss.str();
		img_2 = imread("HS/T7/" + s + ".png");
		imshow("比较最接近图像", img_2);
		waitKey(0);*/
		return maxi;
	}
	
private:
	const int MAXV = 1;
	
};
ProcessImage* ProcessImage::self = NULL;