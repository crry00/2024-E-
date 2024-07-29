#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>

class chess;

class box
{
public:
    enum STATUS
    {
        EMPTY,
        BLACK,
        WHITE
    };
private:
    cv::Point center_;
    STATUS status_;

public:
    box(cv::Point center, STATUS status=EMPTY)
        : center_(center), status_(status) {}
    cv::Point get_center_() const { return center_; }
    STATUS get_status_() const { return status_; }
    void set_status_(STATUS status) { status_ = status; }
    friend chess;
};

class chess
{
public:
    std::vector<box> box_;
    std::vector<cv::Point> points_ ;
    cv::Mat frame_;
    cv::Mat gray_;
    cv::Mat thresh_;
    cv::Mat kernel_=cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::Mat dst_;
    cv::Mat hsv_;
    std::vector<cv::Scalar> w_color= {cv::Scalar(84, 0, 128), cv::Scalar(179, 255, 255)};
    std::vector<cv::Scalar> b_color= {cv::Scalar(0, 0, 0), cv::Scalar(179, 255, 43)};
    std::vector<box> whilt ;
    std::vector<box> black ;
public:
    chess(){}
    bool get_init_chess_box(cv::Mat& img)
    {   
        whilt.clear();
        cv::cvtColor(img, hsv_, cv::COLOR_BGR2HSV);
        cv::Mat whilt_mask;
        cv::Mat black_mask;
        cv::Mat mask=cv::Mat(img.size(), CV_8UC1, cv::Scalar(0));
        cv::Point temp;
        cv::Point center=box_[4].get_center_();
        cv::inRange(hsv_, w_color[0], w_color[1], whilt_mask);
        cv::inRange(hsv_, b_color[0], b_color[1], black_mask);
        std::vector<std::vector<cv::Point>> contours;  
        std::vector<cv::Vec4i> hierarchy;  
        cv::findContours(whilt_mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        for(int i=0;i<contours.size();i++)
        {
            int area=cv::contourArea(contours[i]);
            if(area>1000||area<100) continue;
            cv::RotatedRect tempbox=cv::minAreaRect(contours[i]);
            double radio=tempbox.size.width/tempbox.size.height;
            if(radio>0.8&&radio<1.2)
            {
               whilt.push_back(box(tempbox.center, box::WHITE));
               temp.x=center.x+(center.x-tempbox.center.x);
               temp.y=tempbox.center.y;
               cv::circle(mask, temp, tempbox.size.width+5, cv::Scalar(255, 0, 0), -1);
               cv::drawContours(img, contours, i, cv::Scalar(255,0, 0), 1);  
            }

        }
        cv::bitwise_and(mask, black_mask, black_mask);
        cv::morphologyEx(black_mask,black_mask, cv::MORPH_DILATE, kernel_, cv::Point(-1,-1), 1);
        cv::findContours(black_mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        for(int i=0;i<contours.size();i++)
        {
            int area=cv::contourArea(contours[i]);
            if(area>1000||area<100) continue;
            cv::RotatedRect tempbox=cv::minAreaRect(contours[i]);
            double radio=tempbox.size.width/tempbox.size.height;
            if(radio>0.6&&radio<1.5)
            {
               black.push_back(box(tempbox.center, box::BLACK));

               cv::drawContours(img, contours, i, cv::Scalar(0,255, 0), 1);  
            }

        }
        cv::imshow("mask", black_mask);
        if(whilt.size()==5&&black.size()==5) return true;
        return false;
        
    }

    bool update(cv::Mat& img)
    {

        frame_=img.clone();
        cv::cvtColor(frame_, gray_, cv::COLOR_BGR2GRAY);
        cv::threshold(gray_, thresh_, 60, 255, cv::THRESH_BINARY_INV);
        cv::morphologyEx(thresh_, thresh_, cv::MORPH_DILATE, kernel_, cv::Point(-1,-1), 2);
        
        std::vector<std::vector<cv::Point>> contours;  
        std::vector<cv::Vec4i> hierarchy;  
        cv::findContours(thresh_, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);  
  
    // 绘制轮廓和子轮廓  
        //cv::Mat dst = cv::Mat::zeros(frame_.size(), CV_8UC3);  
        for (size_t i = 0; i < contours.size(); i++) {  
            if(hierarchy[i][2]==-1) continue;
            // 计算轮廓面积（单位：像素
            int area = cv::contourArea(contours[i]);  
            if (area < 500) continue;  
            cv::RotatedRect tempbox=cv::minAreaRect(contours[i]);
            double radio=tempbox.size.width/tempbox.size.height;
            if(radio>0.7&&radio<1.3)
            // 检查是否为外部轮廓（顶级轮廓）  
            {
                int boxnum=0;
            for(int j=hierarchy[i][2];j!=-1;j=hierarchy[j][0]) {  
                cv::RotatedRect tempbox=cv::minAreaRect(contours[j]);
                double radio=tempbox.size.width/tempbox.size.height;
                if(radio>0.7&&radio<1.3) boxnum++;
            }
            if(boxnum==9)
            {
                points_.clear();
                cv::drawContours(img, contours, i, cv::Scalar(0, 255, 0), 1);  
                for(int j=hierarchy[i][2];j!=-1;j=hierarchy[j][0]) {  
                    cv::RotatedRect tempbox=cv::minAreaRect(contours[j]);
                    points_.push_back(tempbox.center);
                    cv::drawContours(img, contours, j, cv::Scalar(255,0, 0), 1);  
                }
                return true;
            }
            }
        }  
        return false;
    }
    void reorder()
    {
        box_.clear();
        std::vector<cv::Point> up;
        std::vector<cv::Point> middle;
        std::vector<cv::Point> down;
        std::sort(points_.begin(), points_.end(), [](cv::Point a, cv::Point b){
            return a.y<b.y;
        });
        for(int i=0;i<3;i++)
        {
            up.push_back(points_[i]);
        }
        std::sort(up.begin(), up.end(), [](cv::Point a, cv::Point b){
            return a.x<b.x;
        });
        for(int i=0;i<3;i++)
        {
            box_.push_back(box(up[i], box::EMPTY));
        }

        for(int i=3;i<6;i++)
        {
            middle.push_back(points_[i]);
        }
        std::sort(middle.begin(), middle.end(), [](cv::Point a, cv::Point b){
            return a.x<b.x;
        });
        for(int i=3;i<6;i++)
        {
            box_.push_back(box(middle[i-3], box::EMPTY));
        }
        for(int i=6;i<9;i++)
        {
            down.push_back(points_[i]);
        }
        std::sort(down.begin(), down.end(), [](cv::Point a, cv::Point b){
            return a.x<b.x;
        });
        for(int i=6;i<9;i++)
        {
            box_.push_back(box(down[i-6], box::EMPTY));
        }
    }
    void show_box(cv::Mat& img)
    {
        if(box_.size()!=9) 
        {
            std::cout<<box_.size()<<"error"<<std::endl;
            return;
        }
        for(int i=0;i<9;i++)
        {
            std::cout<<box_[i].get_center_()<<" "<<std::endl;
            cv::Point center=box_[i].get_center_();
            cv::putText(img, std::to_string(i), center, cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
            if(box_[i].get_status_()==box::EMPTY) cv::circle(img, center, 5, cv::Scalar(255, 0, 0), -1);
            else if(box_[i].get_status_()==box::BLACK) cv::circle(img, center, 5, cv::Scalar(0,0, 255), -1);
            else if(box_[i].get_status_()==box::WHITE) cv::circle(img, center, 5, cv::Scalar(255,255, 0), -1);
        }
    }
    int task_three(cv::Mat img)
    {
        if(update(img))
        {
            std::vector<box> points;
            for(auto& box:box_)
            {
                cv::Point center=box.get_center_();
               std::sort(points_.begin(), points_.end(), [&](cv::Point a, cv::Point b){
                    return cv::norm(a-center)<cv::norm(b-center);
               });
               if(cv::norm(points_[0]-center)>50)
               {
                return -1;
               }
               else
               {
                box.center_=points_[0];
               }
            }
            return true;
        }
        return false;
    }
};


int main(int argc, char** argv)
{
    cv::VideoCapture cap(2);
    if(!cap.isOpened()) return -1;
    chess chess_;
    cv::Mat frame;
    cap >> frame;
    while(true)
    {
        if(chess_.update(frame))
        {
            std::cout<<"update"<<std::endl;
            chess_.reorder();
            chess_.show_box(frame);
            cv::destroyAllWindows();
            break;
        }
        cv::imshow("frame", chess_.frame_);
        cv::imshow("thresh", chess_.thresh_);
        cv::imshow("img",frame);
        //cv::imshow("dst", chess_.dst_);
        cv::imshow("gray", chess_.gray_);
        cv::imshow("frame", frame);
        int key=cv::waitKey(1000);
    }
    while (true) {
        
        cap >> frame;
        if(frame.empty()) break;

        if(chess_.task_three(frame)==1)
        {
            std::cout<<"task_three"<<std::endl;
            chess_.get_init_chess_box(frame);

            chess_.show_box(frame);
        }
        else if(chess_.task_three(frame)==-1)
        {
            std::cout<<"task_three error"<<std::endl;
            chess_.reorder();
            chess_.show_box(frame);
        }
        cv::imshow("frame", chess_.frame_);
        cv::imshow("thresh", chess_.thresh_);
        cv::imshow("img",frame);
        //cv::imshow("dst", chess_.dst_);
        cv::imshow("gray", chess_.gray_);
        cv::imshow("frame", frame);

        int key=cv::waitKey(30);
        if(key==27) break;
    }
}