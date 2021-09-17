// CircleDetector.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#include <iostream>
#include <thread>
#include <list>
#include <mutex>

#include <stdlib.h>
#include <stdio.h>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

list<int> datas;

double RValue(Mat image)
{
    list <int> bluel_ist;
    list <int> green_list;
    list <int> red_list;

    //전달받은 Material 값에서 R채널 값만 따로 저장함
    for (int row = 0; row < image.rows; row++) 
    {
        for (int col = 0; col < image.cols; col++)
        {
            //opencv는 BGR 채널을 쓰므로 
            //0,1,2 중 가장 마지막 2채널의 image.at Material 값을 Vec3b 로 형변환하여 리스트에 저장
            red_list.push_back(image.at<Vec3b>(row, col)[2]);
        }

        //한 col을 다 돌고 난 후에는 다음 col로 넘어가기 위해 \n 을 적용
        cout << "\n" << endl;
    }

    int result = 0;

    red_list.sort();
    red_list.reverse();

    //list 반복자 선언
    list<int>::iterator it;
    int i = 0;

    //red_list 안을 반복하는 iterator it이 
    //red_list의 시작에서 끝까지 돌게 될 동안
    for (it = red_list.begin(); it != red_list.end(); it++) 
    {
        //왜 굳이 4*3으로 나눈다고 썼을까?
        //리턴값도 동일한 형태이므로, 채널 값을 조절하기 위한 방편이 아닐까? 생각됨
        if (i < red_list.size() / 4 * 3)
        {
            result = result + *it;
        }
    }

    return result / (red_list.size() / 4 * 3);

}

//교체된 테스트 이미지의 원 좌표를 확인할 수 있는 함수
void printXY(int event, int x, int y, int flags, void* userdata)
{

    int start_x = -1;
    int start_y = -1;

    bool Is_mouseleftclicked = false;

    if (event == EVENT_MOUSEMOVE) 
    {
        if (Is_mouseleftclicked)
        {
            char buf[256];
            stringstream _x, _y;
            
            _x << start_x;
            _y << start_y;

            cout << "X, Y좌표 " << endl;
            cout << _x.str() << endl;
        }
    }
    else if (event == EVENT_LBUTTONDOWN)
    {
        Is_mouseleftclicked = true;

        start_x = x;
        start_y = y;

        char buf[256];
        stringstream _x, _y;

        _x << start_x;
        _y << start_y;

        cout << "X, Y좌표 " << endl;
        cout << _x.str() + ", " + _y.str() << endl;

    }
    else if (event == EVENT_LBUTTONUP)
    {
        Is_mouseleftclicked = false;
    }

    
}

//카트리지 구역 안의 원 개수와 평균 색깔을 반환하는 함수
string CatridgeType(Mat CatRegion)
{
    //최종 결과인 갯수, 타입
    int catCount=0;
    string catColor="";

    //그레이 스케일로 변환
    Mat catGray;
    cvtColor(CatRegion, catGray, COLOR_BGR2GRAY);

    //바이너리 이미지로 변환
    Mat catBin;
    threshold(catGray, catBin, 30, 255, THRESH_BINARY_INV|THRESH_OTSU);

    //컨투어를 찾음
    vector<vector<Point>> contours;
    findContours(catGray, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    //컨투어 내에서, 포인트의 근사 위치를 가져다 놓는다.
    vector<Point2f> approx;
    Mat image_result;
    image_result = catBin.clone();

    for (size_t i = 0; i < contours.size(); i++) 
    {
        //contour를 근사화
        approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true) * 0.005, true);

        int size = approx.size();
        string figureType = "";

        line(catBin, approx[0], approx[approx.size() - 1], Scalar(0,255,0), 3);

        for (int k = 0; k < size - 1; k++)
        {
            line(catBin, approx[k], approx[k+1], Scalar(0, 255, 0), 3);
        }

        //도형 판정
        if (isContourConvex(Mat(approx)))
        {
            if (size < 3)
            {
                cout << "서클" + catCount << endl;
                figureType += " /Circle";
                catCount += 1;
            }
            else if (size == 4) 
            {
                cout << "사각형" + catCount << endl;
            }
            else 
            {
                cout << "서클 아님" << endl;
                figureType += " /NotCircle";
            }

        }
        else 
        {
            figureType = to_string(approx.size());
        }

        imshow(figureType, catBin);
        waitKey(0);


        stringstream _count;
        _count << catCount;

        string ThisFunctionResult = figureType + " & " + _count.str();

        return ThisFunctionResult;
    }




}

string CatrigdeHough(Mat CatRegion)
{
    Mat blurred;
    blur(CatRegion, blurred, Size(3, 3));

    cout << blurred.rows << endl;
    vector<Vec3f> circles;

    //잘못된 라이브러리 문제일 수 있음
    HoughCircles(blurred, circles, HOUGH_GRADIENT, 1, 5, 40, 21, 2, 500);

    Mat dst;
    //cvtColor(CatRegion, dst, COLOR_GRAY2BGR);

    int count = 0;

    for (Vec3f c : circles)
    {
        Point center(cvRound(c[0]), cvRound(c[1]));
        int radius = cvRound(c[2]);

        circle(dst, center, radius, Scalar(0, 0, 233), 2, LINE_AA);
        count++;
    }

   /* imshow("dst", dst);
    waitKey(0);*/

    string _count = "";
    _count = to_string(count);

    return _count;
}

int main(int ac, char** av)
{
    
    //테스트용 파일을 읽어와 Mat 변환
    Mat image_origin;
    image_origin = imread("test/blue_three.png");

    //정해진 위치에 테스트 파일이 없을 경우를 대비한 예외처리
    if (image_origin.rows < 0 || image_origin.cols < 0) { cout << "[SYSTEM] Please Check Test file. Test File Error : Null" << endl; }


#pragma region  테스트 이미지 내의 원 좌표 식별 함수

    /*imshow("image", image_origin);
    setMouseCallback("image", printXY, 0);

    waitKey(0);*/
#pragma endregion
   
#pragma region 검출 좌표들(Contol)

    int endofimage_x = 536;
    int endofimage_y = 824;

    //notion 페이지 표 참조
    //시약 반응이 나타나는 5개의 점 좌표를 다음과 같이 고정 입력합니다.
    //각 2개가 한 쌍의 범위입니다. (1,2   3,4   5,6   7,8   9,10)
    int x_1 = 179;
    int y_1 = 127;
    int x_2 = 273;
    int y_2 = 218;

    int x_3 = 96;
    int y_3 = 358;
    int x_4 = 189;
    int y_4 = 452;

    int x_5 = 324;
    int y_5 = 340;
    int x_6 = 414;
    int y_6 = 441;

    int x_7 = 91;
    int y_7 = 576;
    int x_8 = 188;
    int y_8 = 685;

    int x_9 = 321;
    int y_9 = 583;
    int x_10 = 432;
    int y_10 = 680;

#pragma endregion

#pragma region 카트리지 좌표들

    //위의 검출 좌표와 마찬가지로,
    //카트리지 영역 내의 좌표를 (a,b   c,d   e,f) 쌍으로 고정합니다.

    int a_x = 20;
    int a_y = 18;
    int b_x = 109;
    int b_y = 92;

    int c_x = 16;
    int c_y = 107;
    int d_x = 105;
    int d_y = 183;

    int e_x = 25;
    int e_y = 201;
    int f_x = 111;
    int f_y = 273;


#pragma endregion

    ////오리지널 이미지를 gray 이미지로 변환하여 
    ////5구획으로 나눔
    //Mat gray_image;
    //cvtColor(image_origin, gray_image, COLOR_BGR2GRAY);

    //Mat gray_region;
    //gray_region = gray_image(Range(0, endofimage_y), Range(0, endofimage_x));

    //Mat region_1 = gray_region(Range(y_1, y_2), Range(x_1, x_2));
    //Mat region_2 = gray_region(Range(y_3, y_4), Range(x_3, x_4));
    //Mat region_3 = gray_region(Range(y_5, y_6), Range(x_5, x_6));
    //Mat region_4 = gray_region(Range(y_7, y_8), Range(x_7, x_8));
    //Mat region_5 = gray_region(Range(y_9, y_10), Range(x_9, x_10));

#pragma region RValue 확인 및 콘솔 출력 파트

    //double rValue1 = RValue(region_1);
    //double rValue2 = RValue(region_2);
    //double rValue3 = RValue(region_3);
    //double rValue4 = RValue(region_4);
    //double rValue5 = RValue(region_5);

    /*printf("Circle 1:  %f\n", 255 - rValue1);*/
    //printf("Circle 2:  %f\n", 255 - rValue2);
    //printf("Circle 3:  %f\n", 255 - rValue3);
    //printf("Circle 4:  %f\n", 255 - rValue4);
    //printf("Circle 5:  %f\n", 255 - rValue5);

#pragma endregion 

#pragma region 카트리지 색 및 타입 출력

    Mat cat;
    cat = image_origin(Range(10, f_y), Range(10, f_x));
    string CatResult = CatrigdeHough(image_origin);

    printf("RESULT : %s\n", CatResult);

#pragma endregion





}
