#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <fstream>
#define SHOW(name,x) cv::imshow(name, x)
#define LOG(x) std::cout << x << std::endl



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
    cv::Point center_;
    //double angle_;
    STATUS status_;
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
    double angle_=0;
    std::vector<cv::Scalar> chessboard_hsv={cv::Scalar(20,61,0), cv::Scalar(50,255,255)};
    std::vector<cv::Scalar> box_hsv= {cv::Scalar(0 ,92 ,141), cv::Scalar(66 ,255 ,255 )};
    std::vector<cv::Scalar> white_hsv= {cv::Scalar(89,0,0 ), cv::Scalar(179,81,211)};
    std::vector<cv::Scalar> black_hsv= {cv::Scalar(0, 0, 0), cv::Scalar(179, 255, 43)};
    cv::Mat hsv_;

    
public:
    chess(){}

    bool get_hsv(cv::Mat& img)
    {
        if(img.empty())
        {
            LOG("get_hsv():img is empty");
            return false;
        }
        cv::cvtColor(img, hsv_, cv::COLOR_BGR2HSV);
        return true;
    }
    bool update()
    {
        if(hsv_.empty())
        {
            LOG("update():hsv is empty");
            return false;
        }
        cv::Mat mask;
        cv::inRange(hsv_, box_hsv[0],box_hsv[1],mask);
        cv::erode(mask, mask, cv::Mat(), cv::Point(-1,-1), 2);
        SHOW("hsvmask",mask);
        std::vector<std::vector<cv::Point>> contours;  
        std::vector<cv::Vec4i> hierarchy;  
        cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        int boxnum=0;
        std::vector<cv::Point> temp;
        for (size_t i = 0; i < contours.size(); i++) 
        {  
            
            int area = cv::contourArea(contours[i]);  
            if (area < 500) continue;  
            cv::RotatedRect tempbox=cv::minAreaRect(contours[i]);
            double radio=tempbox.size.width/tempbox.size.height;
            if(radio>0.7&&radio<1.3)
            // 检查是否为外部轮廓（顶级轮廓）  
            {
                boxnum++;
                temp.push_back(tempbox.center);
            }
        }
                if(boxnum==9)
        {
            points_.clear();
            points_.insert(points_.end(), temp.begin(), temp.end());
            return true;
        }

        cv::inRange(hsv_, chessboard_hsv[0],chessboard_hsv[1],mask);
        cv::dilate(mask, mask, cv::Mat(), cv::Point(-1,-1), 2);
        cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::RotatedRect maxbox;
        for (size_t i = 0; i < contours.size(); i++) 
        {  
            int area = cv::contourArea(contours[i]);  
            if (area < 500) continue;  
            cv::RotatedRect tempbox=cv::minAreaRect(contours[i]);
            double radio=tempbox.size.width/tempbox.size.height;
            if(radio>0.8&&radio<1.2)
            {
                if(maxbox.center.x==0||area>maxbox.size.area()) maxbox=tempbox;
            }
        }  
        SHOW("hsvmask2",mask);

        if(maxbox.center.x==0) 
        {
            LOG("update():no chessboard");
            return false;
        }
        else
        {
            points_.clear();
            cv::Point2f corners[4];
            maxbox.points(corners);
            cv::Point lt=corners[1];
            cv::Point rt=corners[2];
            cv::Point lb=corners[0];
            cv::Point rb=corners[3];

            cv::Point center=maxbox.center;
            points_.push_back(center);
            cv::Point l1=lt+(cv::Point)(center-lt)/2.8;
            points_.push_back(l1);
            cv::Point l3=rt+(cv::Point)(center-rt)/2.8;
            points_.push_back(l3);
            cv::Point l2=l1+(cv::Point)(l3-l1)/2;
            points_.push_back(l2);
            cv::Point r3=rb+(cv::Point)(center-rb)/2.8;
            points_.push_back(r3);
            cv::Point r1=lb+(cv::Point)(center-lb)/2.8;
            points_.push_back(r1);
            cv::Point r2=r1+(cv::Point)(r3-r1)/2;
            points_.push_back(r2);
            cv::Point m1=l1+(cv::Point)(r1-l1)/2;
            points_.push_back(m1);
            cv::Point m3=l3+(cv::Point)(r3-l3)/2;
            points_.push_back(m3);

            return true;
            
        }
        
        return false;
    }
    void reorder()
    {
        if(points_.size()!=9)
        {
            LOG("reorder(): 9 points needed");
            return;
        }
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
            LOG("show_box():box size error");
            return;
        }
        for(int i=0;i<9;i++)
        {
            //std::cout<<box_[i].get_center_()<<" "<<std::endl;
            cv::Point center=box_[i].get_center_();
            cv::putText(img, std::to_string(i), center, cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
            if(box_[i].get_status_()==box::EMPTY) cv::circle(img, center, 5, cv::Scalar(255, 0, 0), -1);
            else if(box_[i].get_status_()==box::BLACK) cv::circle(img, center, 5, cv::Scalar(0,0, 255), -1);
            else if(box_[i].get_status_()==box::WHITE) cv::circle(img, center, 5, cv::Scalar(255,255, 0), -1);
        }
        cv::putText(img, std::to_string(angle_), cv::Point(100, 50), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255, 0, 0), 1);
    }
    int track()
    {       
        if(box_.empty()||points_.empty())
        {
            LOG("track ():box or points empty");
            return false;
        }
            std::vector<box> points;
            for(auto& box:box_)
            {
                cv::Point center=box.get_center_();
               std::sort(points_.begin(), points_.end(), [&](cv::Point a, cv::Point b){
                    return cv::norm(a-center)<cv::norm(b-center);
               });
               if(cv::norm(points_[0]-center)>50)
               {
                return false;
               }
               else
               {
                points.push_back({points_[0],box.status_});
               }
            }
            
            for(auto& nowpoint:points)
            {
                int flagcount=0;
                auto it = std::find_if(points.begin(), points.end(), [&](const box& point) {
                double dis = cv::norm(point.center_ - nowpoint.center_); // 计算点与目标点之间的距离
                if(dis < 10.0)
                {
                    flagcount++;
                } // 如果距离小于50，则返回true
                if(flagcount>=2)
                {
                    std::cout<<"find"<<std::endl;
                    return true;
                }
                else
                {
                    return false;
                }
            });

            if (it != points.end()) {
                return -1;
            }           
            }

            box_=points;
            return true;
    }

    void get_angle()
    {
        if(box_.empty())
        {
            LOG("get_angle():box empty");
            return;
        }
        cv::Point point1=box_[1].get_center_();
        cv::Point point2=box_[7].get_center_();
        int x=abs(point1.x-point2.x);
        int y=abs(point1.y-point2.y);
        LOG(x);
        LOG(y);
        if(x<1||y<1)
        {
            angle_=0;
            return;
        }
        angle_=abs(atan2(x,y)*180/3.14);
        if(point1.x>point2.x)
        {
            angle_=-angle_;
        } 
    }

    void get_chess_color()
    {
        if(box_.empty())
        {
            LOG("get_chess_color():box empty");
            return;
        }
        if(hsv_.empty())
        {
            LOG("get_chess_color():hsv empty");
            return;
        }
        cv::Mat mask=cv::Mat(hsv_.size(), CV_8UC1, cv::Scalar(0));
        int r=20;
        for(int i=0;i<9;i++)
        {
            cv::circle(mask, box_[i].get_center_(), r, cv::Scalar(255), -1);
            box_[i].set_status_(box::EMPTY);
        }
        cv::Mat whilt_mask,black_mask;
        cv::inRange(hsv_, white_hsv[0],white_hsv[1],whilt_mask);
        cv::inRange(hsv_, black_hsv[0],black_hsv[1],black_mask);
        cv::dilate(whilt_mask, whilt_mask, cv::Mat(),cv::Point(-1,-1),2);
        cv::dilate(black_mask, black_mask, cv::Mat(),cv::Point(-1,-1),3);

        cv::bitwise_and(whilt_mask, mask, whilt_mask);
        cv::bitwise_and(black_mask, mask, black_mask);

        std::vector<std::vector<cv::Point>> contours;  
        std::vector<cv::Vec4i> hierarchy;  
        std::vector<cv::Point> temp;
        cv::findContours(whilt_mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        for (size_t i = 0; i < contours.size(); i++) 
        {  
            
            int area = cv::contourArea(contours[i]);  
            if (area < 50) continue;  
            cv::RotatedRect tempbox=cv::minAreaRect(contours[i]);
            double radio=tempbox.size.width/tempbox.size.height;
            if(radio>0.5&&radio<2)
            // 检查是否为外部轮廓（顶级轮廓）  
            {
                temp.push_back(tempbox.center);
            }
        }
        if(!temp.empty())
        {
        for(int i=0;i<9;i++)
        {
            std::sort(temp.begin(), temp.end(), [&](cv::Point a, cv::Point b){
                    return cv::norm(a-box_[i].get_center_())<cv::norm(b-box_[i].get_center_());
            });
            if(cv::norm(temp[0]-box_[i].get_center_())>r)
            {
                continue;
                //std::cout<<"error"<<std::endl;
            }
            else
            {
                box_[i].set_status_(box::WHITE);
                //std::cout<<"white"<<std::endl;
                LOG("white");
            }
        }
        }

        temp.clear();
        cv::findContours(black_mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        for (size_t i = 0; i < contours.size(); i++) 
        {  
            
            int area = cv::contourArea(contours[i]);  
            if (area < 10) continue;  
            cv::RotatedRect tempbox=cv::minAreaRect(contours[i]);
            double radio=tempbox.size.width/tempbox.size.height;
            if(radio>0.5&&radio<2)
            
            {
                temp.push_back(tempbox.center);
            }
        }
        if(!temp.empty())
        {
        for(int i=0;i<9;i++)
        {
            std::sort(temp.begin(), temp.end(), [&](cv::Point a, cv::Point b){
                    return cv::norm(a-box_[i].get_center_())<cv::norm(b-box_[i].get_center_());
            });
            if(cv::norm(temp[0]-box_[i].get_center_())>r)
            {
                continue;
            }
            else
            {
                box_[i].set_status_(box::BLACK);
                LOG("black");
            }
        }
        }



        
        SHOW("mask", mask);
        SHOW("whilt_mask", whilt_mask);
        SHOW("black_mask", black_mask);


    }

    bool normal(cv::Mat& img)
    {
        if(img.empty())
        {
            LOG("normal():image is empty");
            return false;
        }
        if(!get_hsv(img))
        {
            LOG("normal():get hsv failed");
            return false;
        }
        if(update())
        {
            reorder();
        }

        get_chess_color();
        show_box(img);
        return true;
    }

    bool moving(cv::Mat& img)
    {
        if(img.empty())
        {
            LOG("moving():image is empty");
            return false;
        }
        if(!get_hsv(img))
        {
            LOG("moving():get hsv failed");
            return false;
        }
        if(update())
        {

            if(track()==-1)
            {
                LOG("track() lost!");
                reorder();
            }
            get_angle();
        }
        get_chess_color();
        show_box(img);
        return true;
    }

    bool init(cv::Mat& img)
    {
        if(img.empty())
        {
            LOG("init():image is empty");
            return false;
        }
        if(!get_hsv(img))
        {
            LOG("init():get hsv failed");
            return false;
        }

        if(!update())
        {
            LOG("init():update failed");
            return false;
        }
        reorder();
        show_box(img);
        return true;
    }

    cv::Mat return_mat()
    {
        
        cv::Mat chess_mat=cv::Mat(3,3,CV_8SC1,cv::Scalar(0));
        if(box_.size()!=9)
        {
            LOG("return_mat():box is empty");
            return chess_mat;
        }
        for(int i=0;i<9;i++)
        {
            if(box_[i].get_status_()==box::WHITE)
            {
                chess_mat.at<char>(i/3,i%3)=1;
            }
            else if(box_[i].get_status_()==box::BLACK)
            {
                chess_mat.at<char>(i/3,i%3)=-1;
            }
        }
        LOG(chess_mat);
        return chess_mat;
    }
};


int main(int argc, char** argv)
{
   cv::VideoCapture cap(2);
    if(!cap.isOpened()) return -1;
    
    chess chess_;
    cv::Mat frame;
    while( true)
    {
        cap >> frame;
        if(chess_.init(frame))
        {
            std::cout<<"init"<<std::endl;
            break;
        }
    }

    while( true)
    {
        cap >> frame;
        //chess_.moving(frame);
        if(chess_.moving(frame))
        {
            chess_.return_mat();
        }
        cv::imshow("frame",frame);
        cv::waitKey(1);
    }

}