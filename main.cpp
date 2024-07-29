#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void drawChildContours(cv::Mat& img, const std::vector<std::vector<cv::Point>>& contours,  
                       const std::vector<cv::Vec4i>& hierarchy, int idx, const cv::Scalar& color) {  
    // 检查是否有子轮廓  
    int nextIdx = hierarchy[idx][2];  
    while (nextIdx != -1) {  
        // 绘制子轮廓  
        cv::drawContours(img, contours, nextIdx, color, 2, cv::LINE_8);  
  
        // 递归地绘制更深层次的子轮廓  
        drawChildContours(img, contours, hierarchy, nextIdx, color);  
  
        // 移动到下一个兄弟轮廓  
        nextIdx = hierarchy[nextIdx][0];  
    }  
}



int main(int argc, char *argv[])
{
    VideoCapture cap(2);
    if(!cap.isOpened())
    {
        cout << "Can't open camera" << endl;
        return -1;
    }
    Mat frame;
    while(true)
    {
        cap >> frame;
        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        Mat thresh;
        Mat canny;
        
        threshold(gray, thresh, 50, 255, THRESH_BINARY_INV);
        Mat kernel=getStructuringElement(MORPH_RECT, Size(5, 5));
        morphologyEx(thresh, thresh, MORPH_DILATE, kernel, Point(-1,-1), 2);
        //adaptiveThreshold(gray, thresh, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 35, 0);
        //Canny(thresh, canny, 50, 200);
         std::vector<std::vector<cv::Point>> contours;  
    std::vector<cv::Vec4i> hierarchy;  
    cv::findContours(thresh, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);  
  
    // 绘制轮廓和子轮廓  
    cv::Mat dst = cv::Mat::zeros(frame.size(), CV_8UC3);  
    for (size_t i = 0; i < contours.size(); i++) {  
        int area = cv::contourArea(contours[i]);  
        if (area < 500) continue;  
        // 检查是否为外部轮廓（顶级轮廓）  
        int idx = hierarchy[i][3];  
        if (idx==-1&&hierarchy[i][2]!=-1) {  
            // 绘制外部轮廓，颜色为蓝色  
            RotatedRect rect = cv::minAreaRect(contours[i]);  
            double radiowh=rect.size.width/rect.size.height;
            double radioarea=rect.size.area()/area;
            if(radiowh>0.9&&radiowh<1.1&&radioarea>0.9)
            {
                cv::drawContours(dst, contours, static_cast<int>(i), cv::Scalar(255, 0, 0), 2, cv::LINE_8, hierarchy, 0);  
  
            // 递归或迭代地绘制子轮廓  
            drawChildContours(dst, contours, hierarchy, i, cv::Scalar(0, 255, 0)); // 假设drawChildContours是一个递归函数  
            }
        }  
    }  
        imshow("Gray", gray);
        imshow("Threshold", thresh);
        //imshow("Canny", canny);
        imshow("Contours", dst);
        imshow("Frame", frame);
        if(waitKey(30) >= 0) break;
    }


}