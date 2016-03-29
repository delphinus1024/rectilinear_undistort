// Lens profile rectilinear undistortion

#include <math.h>
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>

// Lens Params
double FocalLengthX              ;    // fx / Dmax, so multiply Dmax when in use
double FocalLengthY              ;    // fy / Dmax, so multiply Dmax when in use
double ImageXCenter              ;    // ux / Dmax, so multiply Dmax when in use
double ImageYCenter              ;    // uy / Dmax, so multiply Dmax when in use
double RadialDistortParam1       ;    // k1 (unit none)
double RadialDistortParam2       ;    // k2 (unit none)
double RadialDistortParam3       ;    // k3 (unit none)
double TangentialDistortParam1   ;    // k4 (unit none)
double TangentialDistortParam2   ;    // k5 (unit none)

// Lens Params multiplied by Dmax
double FocalLengthXD;
double FocalLengthYD;
double ImageXCenterD;
double ImageYCenterD;

// point, before and after
typedef struct {
    double du;
    double dv;
    double ud;
    double vd;
} coords;

double	sEnlage = 2.;	// when processing, add extra space to include whole image. Usually, this value is too large so that you can adjust this value for faster processing.

int init(double pWidth,double pHeight) {

    // multiply by Dmax
    if(pWidth > pHeight) {	
        FocalLengthXD = pWidth * FocalLengthX;
        FocalLengthYD = pWidth * FocalLengthY;
    } else {
        FocalLengthXD = pHeight * FocalLengthX;
        FocalLengthYD = pHeight * FocalLengthY;
    }

    ImageXCenterD = pWidth * ImageXCenter;
    ImageYCenterD = pHeight * ImageYCenter;

    std::cout <<
        "FocalLengthXD=" << FocalLengthXD << "," << 
        "FocalLengthYD=" << FocalLengthYD << "," << 
        "ImageXCenterD=" << ImageXCenterD << "," << 
        "ImageYCenterD=" << ImageYCenterD << std::endl;

    return 0;
}

coords distort_point(double pU,double pV) {
    coords ret;

    double x = pU / FocalLengthXD;
    double y = pV / FocalLengthYD;
    double r_pow2 = x * x + y * y;

    double ud = pU + FocalLengthXD * (
        ((RadialDistortParam1 * r_pow2)
        + (RadialDistortParam2 * r_pow2 * r_pow2)
        + (RadialDistortParam3 * r_pow2 * r_pow2 * r_pow2)) * x
        + 2. * (
        (TangentialDistortParam1 * y)
        + (TangentialDistortParam2 * x)) * x
        + TangentialDistortParam2 * r_pow2);

    double vd = pV + FocalLengthYD * (
        ((RadialDistortParam1 * r_pow2)
        + (RadialDistortParam2 * r_pow2 * r_pow2)
        + (RadialDistortParam3 * r_pow2 * r_pow2 * r_pow2)) * y
        + 2. * (
        (TangentialDistortParam1 * y)
        + (TangentialDistortParam2 * x)) * y
        + TangentialDistortParam1 * r_pow2);

    ret.ud = ud;
    ret.vd = vd;

    return ret;
} 

int calc_remap(cv::Mat &src,cv::Mat &canvas,cv::Mat &map_x,cv::Mat &map_y,cv::Rect &org_rect,cv::Rect &max_rect) {
	
    int canvas_centerx = canvas.cols/2;
    int canvas_centery = canvas.rows/2;
    int xorg = canvas_centerx - static_cast<int>(ImageXCenterD);
    int yorg = canvas_centery - static_cast<int>(ImageYCenterD);

    assert(xorg >= 0 && yorg >= 0);

    org_rect = cv::Rect(xorg, yorg, 
        (xorg + src.cols) > canvas.cols ? canvas.cols - xorg : src.cols, 
        (yorg + src.rows) > canvas.rows ? canvas.rows - yorg : src.rows);
    
    cv::Mat src_region(canvas, org_rect);
    cv::Rect srcorg_rect = cv::Rect(0,0,org_rect.width,org_rect.height);
    cv::Mat srcorg_region(src, srcorg_rect); 
    srcorg_region.copyTo(src_region);
    
    int valid_left = canvas.cols,valid_right = 0,valid_top = canvas.rows,valid_bottom = 0;
    
    for(int y = 0;y < canvas.rows;++y) {
        for(int x = 0;x < canvas.cols;++x) {
            coords co = distort_point(static_cast<double>(x - canvas_centerx) ,static_cast<double>(y - canvas_centery));

            map_x.at<float>(y,x) = static_cast<float>(co.ud + canvas_centerx);
            map_y.at<float>(y,x) = static_cast<float>(co.vd + canvas_centery);
            
            // check minimum rect which includes whole valid image
            if((co.ud >= -ImageXCenterD) && (co.ud <= (src.cols - ImageXCenterD))
                && (co.vd >= -ImageYCenterD) && (co.vd <= (src.rows - ImageYCenterD))) {
                if(x < valid_left)
                    valid_left = x;
                else if(valid_right < x)
                    valid_right = x;
                
                if(y < valid_top)
                    valid_top = y;
                else if(valid_bottom < y)
                    valid_bottom = y;
            }
        }
    }

    max_rect.x = valid_left;
    max_rect.y = valid_top;
    max_rect.width = valid_right - valid_left;
    max_rect.height = valid_bottom - valid_top;
    
    std::cout << "maxrect:" << max_rect << std::endl;

    return 0;
}

// as default, using lens params from Samyang 14mm f2.8 for Canon @ F8
const cv::String keys =
    "{help h usage ? |          | show help                }"
    "{@image         |          | input image file         }"
    "{@result        |result.tif| result image file        }"
    "{S s            |          | show image when finish   }"
    "{FX fx          |0.403752  | FocalLengthX             }"
    "{FY fy          |0.403752  | FocalLengthY             }"
    "{IX ix          |0.500000  | ImageXCenter             }"
    "{IY iy          |0.499867  | ImageYCenter             }"
    "{R1 r1          |-0.071356 | RadialDistortParam1      }"
    "{R2 r2          |0.022986  | RadialDistortParam2      }"
    "{R3 r3          |0.0       | RadialDistortParam3      }"
    "{T1 t1          |0.0       | TangentialDistortParam1  }"
    "{T2 t2          |0.0       | TangentialDistortParam2  }"
    ;

int main(int argc,char *argv[]) {
    cv::CommandLineParser parser(argc, argv, keys);
    parser.about("Rectilinear lens undistortion.");

    bool should_show = parser.has("S") ? true: false;
    cv::String imagefile = parser.get<cv::String>("@image");
    cv::String resultfile = parser.get<cv::String>("@result");
    FocalLengthX = parser.get<double>("FX");
    FocalLengthY = parser.get<double>("FY");
    ImageXCenter = parser.get<double>("IX");
    ImageYCenter = parser.get<double>("IY");
    RadialDistortParam1 = parser.get<double>("R1");
    RadialDistortParam2 = parser.get<double>("R2");
    RadialDistortParam3 = parser.get<double>("R3");
    TangentialDistortParam1 = parser.get<double>("T1");
    TangentialDistortParam2 = parser.get<double>("T2");

    if (parser.has("h") || !parser.check() || imagefile.empty()) {
        parser.printMessage();
        return -1;
    }

    CV_Assert(sEnlage >= 1.);
    CV_Assert(FocalLengthX > 0.);
    CV_Assert(FocalLengthY > 0.);

    cv::Mat src = cv::imread(imagefile);
    cv::Mat canvas = cv::Mat::zeros(src.rows * static_cast<int>(sEnlage),src.cols * static_cast<int>(sEnlage), CV_8UC3);
    cv::Mat tar;
    cv::Mat map_x = cv::Mat::zeros(canvas.size(), CV_32FC1);
    cv::Mat map_y = cv::Mat::zeros(canvas.size(), CV_32FC1);

    std::cout << "Processing " << imagefile << "..." << std::endl;
    std::cout << "source image size:" << src.cols << "," << src.rows << std::endl;

    init(src.cols , src.rows);

    cv::Rect org_rect,max_rect;
    calc_remap(src,canvas,map_x,map_y,org_rect,max_rect);
    cv::remap(canvas, tar, map_x, map_y, CV_INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0,0, 0));
    cv::Mat clip_max = cv::Mat(tar, max_rect); 
    
    std::cout << "result image size:" << tar.cols << "," << tar.rows << std::endl;
    
    imwrite(resultfile, clip_max);
    if(should_show) {
        cv::namedWindow( "Result", cv::WINDOW_NORMAL );
        cv::imshow("Result",clip_max);
        cv::waitKey();
    }

    return 0;
}
