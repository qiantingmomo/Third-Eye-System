#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <queue>
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SerialClass.h"	
#include "constants.h"
#include "findEyeCenter.h"
#include "findEyeCorner.h"

/** Temp **/
struct temp{
	int x;
	int y;
};

//Serial
Serial* SP = new Serial("\\\\.\\COM5");
//

/** Constants **/


/** Function Headers */
void detectAndDisplay( cv::Mat frame );

/** Global variables */
//-- Note, either copy these two files from opencv/data/haarscascades to your current folder, or change these locations
cv::String face_cascade_name = "../res/haarcascade_frontalface_alt.xml";
cv::CascadeClassifier face_cascade;
std::string main_window_name = "Capture - Face detection";
std::string face_window_name = "Capture - Face";
cv::RNG rng(12345);
cv::Mat debugImage;
cv::Mat skinCrCbHist = cv::Mat::zeros(cv::Size(256, 256), CV_8UC1);

//
struct temp temp_right ={0,0};
struct temp temp_left = {0,0};

/**
 * @function main
 */
int main( int argc, const char** argv ) {
  CvCapture* capture;
  cv::Mat frame;

  if (SP->IsConnected())
	  printf("We're connected\n");


  // Load the cascades
  if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading face cascade, please change face_cascade_name in source code.\n"); return -1; };

  cv::namedWindow(main_window_name,CV_WINDOW_NORMAL);
  cv::moveWindow(main_window_name, 400, 100);
  cv::namedWindow(face_window_name,CV_WINDOW_NORMAL);
  cv::moveWindow(face_window_name, 10, 100);
  cv::namedWindow("Right Eye",CV_WINDOW_NORMAL);
  cv::moveWindow("Right Eye", 10, 600);
  cv::namedWindow("Left Eye",CV_WINDOW_NORMAL);
  cv::moveWindow("Left Eye", 10, 800);
  cv::namedWindow("aa",CV_WINDOW_NORMAL);
  cv::moveWindow("aa", 10, 800);
  cv::namedWindow("aaa",CV_WINDOW_NORMAL);
  cv::moveWindow("aaa", 10, 800);

  createCornerKernels();
  ellipse(skinCrCbHist, cv::Point(113, 155.6), cv::Size(23.4, 15.2),
          43.0, 0.0, 360.0, cv::Scalar(255, 255, 255), -1);

   // Read the video stream
  capture = cvCaptureFromCAM(0);
  if( capture ) {
    while( true ) {
      frame = cvQueryFrame( capture );
      // mirror it
      cv::flip(frame, frame, 1);
      frame.copyTo(debugImage);

      // Apply the classifier to the frame
      if( !frame.empty() ) {
        detectAndDisplay( frame );
      }
      else {
        printf(" --(!) No captured frame -- Break!");
        break;
      }

      imshow(main_window_name,debugImage);

      int c = cv::waitKey(10);
      if( (char)c == 'c' ) { break; }
      if( (char)c == 'f' ) {
        imwrite("frame.png",frame);
      }

    }
  }

  releaseCornerKernels();

  return 0;
}

void findEyes(cv::Mat frame_gray, cv::Rect face) {
  cv::Mat faceROI = frame_gray(face);
  cv::Mat debugFace = faceROI;

  //
  /*if (SP->IsConnected())
		printf("We're connected\n");*/
  //

  if (kSmoothFaceImage) {
    double sigma = kSmoothFaceFactor * face.width;
    GaussianBlur( faceROI, faceROI, cv::Size( 0, 0 ), sigma);
  }
  //-- Find eye regions and draw them
  int eye_region_width = face.width * (kEyePercentWidth/100.0);
  int eye_region_height = face.width * (kEyePercentHeight/100.0);
  int eye_region_top = face.height * (kEyePercentTop/100.0);
  printf("facesize:(%d * %d * %d)\n", face.width, face.width, face.height);

  cv::Rect leftEyeRegion(face.width*(kEyePercentSide/100.0),
                         eye_region_top,eye_region_width,eye_region_height);
  cv::Rect rightEyeRegion(face.width - eye_region_width - face.width*(kEyePercentSide/100.0),
                          eye_region_top,eye_region_width,eye_region_height);

  //-- Find Eye Centers
  cv::Point leftPupil = findEyeCenter(faceROI,leftEyeRegion,"Left Eye");
  cv::Point rightPupil = findEyeCenter(faceROI,rightEyeRegion,"Right Eye");


  // get corner regions
  cv::Rect leftRightCornerRegion(leftEyeRegion);
  leftRightCornerRegion.width -= leftPupil.x;
  leftRightCornerRegion.x += leftPupil.x;
  leftRightCornerRegion.height /= 2;
  leftRightCornerRegion.y += leftRightCornerRegion.height / 2;
  cv::Rect leftLeftCornerRegion(leftEyeRegion);
  leftLeftCornerRegion.width = leftPupil.x;
  leftLeftCornerRegion.height /= 2;
  leftLeftCornerRegion.y += leftLeftCornerRegion.height / 2;
  cv::Rect rightLeftCornerRegion(rightEyeRegion);
  rightLeftCornerRegion.width = rightPupil.x;
  rightLeftCornerRegion.height /= 2;
  rightLeftCornerRegion.y += rightLeftCornerRegion.height / 2;
  cv::Rect rightRightCornerRegion(rightEyeRegion);
  rightRightCornerRegion.width -= rightPupil.x;
  rightRightCornerRegion.x += rightPupil.x;
  rightRightCornerRegion.height /= 2;
  rightRightCornerRegion.y += rightRightCornerRegion.height / 2;
  rectangle(debugFace,leftRightCornerRegion,200);
  rectangle(debugFace,leftLeftCornerRegion,200);
  rectangle(debugFace,rightLeftCornerRegion,200);
  rectangle(debugFace,rightRightCornerRegion,200);

  // change eye centers to face coordinates
  rightPupil.x += rightEyeRegion.x;
  rightPupil.y += rightEyeRegion.y;
  leftPupil.x += leftEyeRegion.x;
  leftPupil.y += leftEyeRegion.y;
  printf("rightPupil:(%d,%d)\n;leftPupil:(%d,%d)\n", rightPupil.x, rightPupil.y,leftPupil.x,leftPupil.y);

  // control servos
  if(temp_right.x != 0 && temp_right.y !=0){
	  temp_right.x = temp_right.x - rightPupil.x;
	  temp_right.y = temp_right.y - rightPupil.y;
	  printf("difference righteye:x = %d, y = %d\n",temp_right.x, temp_right.y);

	  temp_left.x = temp_left.x - leftPupil.x;
	  temp_left.y = temp_left.y - leftPupil.y;
	  printf("difference lefteye:x = %d, y = %d\n\n",temp_left.x, temp_left.y);

	  int dataLength = 4;
	  int i=5;
	  int k=10;
	  
	  if(temp_right.x >= k||temp_left.x >= k||temp_right.x <= -k||temp_left.x <= -k){
		  //SP->WriteData("X",dataLength);
		  char str[5]={};
		  if(temp_right.x >= k||temp_left.x >= k){
			  //
			  //Sleep(100);
			  //
			  //SP->WriteData("+",dataLength);
			  if(temp_right.x >= temp_left.x){
				  sprintf(str,"%s%d","X+",temp_right.x);
				  printf("%s\n",str);
				  SP->WriteData(str,dataLength);
			  }
			  else if(temp_right.x <= temp_left.x){
				  sprintf(str,"%s%d","X+",temp_left.x);
				  printf("%s\n",str);
				  SP->WriteData(str,dataLength);
			  }

			  //
			  printf("yes\n\n");
			  /*char incomingData[2] = "";
			  int dataLength = 2;
			  SP->ReadData(incomingData,dataLength);
			  printf("%s\n",incomingData);*/
			  //
		  }
		  else if(temp_right.x <= -k||temp_left.x <= -k){
			  //
			  //Sleep(100);
			  //
			  //SP->WriteData("-",dataLength);
			  if(temp_right.x >= temp_left.x){
				  sprintf(str,"%s%d","X-",abs(temp_right.x));
				  printf("%s\n",str);
				  SP->WriteData(str,dataLength);
			  }
			  else if(temp_right.x <= temp_left.x){
				  sprintf(str,"%s%d","X-",abs(temp_left.x));
				  printf("%s\n",str);
				  SP->WriteData(str,dataLength);
			  }

			  //
			  printf("yes\n\n");
			  /*char incomingData[2] = "";
			  int dataLength = 2;
			  SP->ReadData(incomingData,dataLength);
			  printf("%s\n",incomingData);*/
			  //
		  }
	  }


	  if(temp_right.y >= i||temp_left.y >= i||temp_right.y <= -i||temp_left.y <= -i){
		  //SP->WriteData("Y",dataLength);
		  char str[5]={};
		  if(temp_right.y >= i||temp_left.y >= i){
			  //SP->WriteData("+",dataLength);
			  if(temp_right.y >= temp_left.y){
				  sprintf(str,"%s%d","Y+",temp_right.y);
				  printf("%s\n",str);
				  SP->WriteData(str,dataLength);
			  }
			  else if(temp_left.y >= temp_left.y){
				  sprintf(str,"%s%d","Y+",temp_left.y);
				  printf("%s\n",str);
				  SP->WriteData(str,dataLength);
			  }

			  //
			  printf("yes\n\n");
			  /*char incomingData[2] = "";
			  int dataLength = 2;
			  SP->ReadData(incomingData,dataLength);
			  printf("%s\n",incomingData);*/
			  //
		  }
		  else if(temp_right.y <= -i||temp_left.y <= -i){
			  //SP->WriteData("-",dataLength);
			  if(temp_right.y <= temp_left.y){
				  sprintf(str,"%s%d","Y-",abs(temp_right.y));
				  printf("%s\n",str);
				  SP->WriteData(str,dataLength);
			  }
			  else if(temp_left.y <= temp_left.y){
				  sprintf(str,"%s%d","Y-",abs(temp_left.y));
				  printf("%s\n",str);
				  SP->WriteData(str,dataLength);
				  //Sleep(2);
			  }

			  //
			  printf("yes\n\n");
			  /*char incomingData[2] = "";
			  int dataLength = 2;
			  SP->ReadData(incomingData,dataLength);
			  printf("%s\n",incomingData);*/
			  //
		  }
	  }

  }

  temp_right.x = rightPupil.x;
  temp_right.y = rightPupil.y;
  temp_left.x = leftPupil.x;
  temp_left.y = leftPupil.y;
  //printf("rightPupil:(%d,%d)\n;leftPupil:(%d,%d)\n", rightPupil.x, rightPupil.y,leftPupil.x,leftPupil.y);

  //Sleep(500);

  // draw eye centers
  circle(debugFace, rightPupil, 3, 1234);
  circle(debugFace, leftPupil, 3, 1234);

  //-- Find Eye Corners
  if (kEnableEyeCorner) {
    cv::Point2f leftRightCorner = findEyeCorner(faceROI(leftRightCornerRegion), true, false);
    leftRightCorner.x += leftRightCornerRegion.x;
    leftRightCorner.y += leftRightCornerRegion.y;
    cv::Point2f leftLeftCorner = findEyeCorner(faceROI(leftLeftCornerRegion), true, true);
    leftLeftCorner.x += leftLeftCornerRegion.x;
    leftLeftCorner.y += leftLeftCornerRegion.y;
    cv::Point2f rightLeftCorner = findEyeCorner(faceROI(rightLeftCornerRegion), false, true);
    rightLeftCorner.x += rightLeftCornerRegion.x;
    rightLeftCorner.y += rightLeftCornerRegion.y;
    cv::Point2f rightRightCorner = findEyeCorner(faceROI(rightRightCornerRegion), false, false);
    rightRightCorner.x += rightRightCornerRegion.x;
    rightRightCorner.y += rightRightCornerRegion.y;
    circle(faceROI, leftRightCorner, 3, 200);
    circle(faceROI, leftLeftCorner, 3, 200);
    circle(faceROI, rightLeftCorner, 3, 200);
    circle(faceROI, rightRightCorner, 3, 200);
  }

  imshow(face_window_name, faceROI);
//  cv::Rect roi( cv::Point( 0, 0 ), faceROI.size());
//  cv::Mat destinationROI = debugImage( roi );
//  faceROI.copyTo( destinationROI );
}


cv::Mat findSkin (cv::Mat &frame) {
  cv::Mat input;
  cv::Mat output = cv::Mat(frame.rows,frame.cols, CV_8U);

  cvtColor(frame, input, CV_BGR2YCrCb);

  for (int y = 0; y < input.rows; ++y) {
    const cv::Vec3b *Mr = input.ptr<cv::Vec3b>(y);
//    uchar *Or = output.ptr<uchar>(y);
    cv::Vec3b *Or = frame.ptr<cv::Vec3b>(y);
    for (int x = 0; x < input.cols; ++x) {
      cv::Vec3b ycrcb = Mr[x];
//      Or[x] = (skinCrCbHist.at<uchar>(ycrcb[1], ycrcb[2]) > 0) ? 255 : 0;
      if(skinCrCbHist.at<uchar>(ycrcb[1], ycrcb[2]) == 0) {
        Or[x] = cv::Vec3b(0,0,0);
      }
    }
  }
  return output;
}

/**
 * @function detectAndDisplay
 */
void detectAndDisplay( cv::Mat frame ) {
  std::vector<cv::Rect> faces;
  //cv::Mat frame_gray;

  std::vector<cv::Mat> rgbChannels(3);
  cv::split(frame, rgbChannels);
  cv::Mat frame_gray = rgbChannels[2];

  //cvtColor( frame, frame_gray, CV_BGR2GRAY );
  //equalizeHist( frame_gray, frame_gray );
  //cv::pow(frame_gray, CV_64F, frame_gray);
  //-- Detect faces
  face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150) );
//  findSkin(debugImage);

  for( int i = 0; i < faces.size(); i++ )
  {
    rectangle(debugImage, faces[i], 1234);
  }
  //-- Show what you got
  if (faces.size() > 0) {
    findEyes(frame_gray, faces[0]);
  }
}
