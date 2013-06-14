//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <exception>

#include <opencv2/opencv.hpp>
#include "NiTE.h"
#include "OpenNI.h"
using namespace nite;
using namespace openni;
using namespace std;
using namespace cv;

/*//////////////////////////////////////////////////////////////////////
// constants
//////////////////////////////////////////////////////////////////////*/

const Size frameSize(640, 480);
const unsigned int maxUsers = 20;
const Mat emptyMat();


//callback variables
void (*onOneFingerCallback)(void);

#define MAX_USERS 10
bool g_visibleUsers[MAX_USERS] = {false};
nite::SkeletonState g_skeletonStates[MAX_USERS] = {nite::SKELETON_NONE};

//message printer
#define USER_MESSAGE(msg) \
	{printf("[%08llu] User #%d:\t%s\n",ts, user.getId(),msg);}
	
// frame buffers
Mat bgrMat(frameSize, CV_8UC3);
Mat depthMat(frameSize, CV_16UC1);
Mat depthMat8(frameSize, CV_8UC1);
Mat depthMatBgr(frameSize, CV_8UC3);

//-----------------------------------------------------------------------------
// Interface types
//-----------------------------------------------------------------------------

#ifdef MWAREINTERFACE_EXPORTS
#define MWAREINTERFACE_API extern "C" __declspec(dllexport)
#else
#define MWAREINTERFACE_API extern "C" __declspec(dllimport)
#endif

//-----------------------------------------------------------------------------
// custom struct definition for cs wrapper
//-----------------------------------------------------------------------------
//custom position point struct 
struct Point3D{
	float x; 
	float y;
	float z;
	float confidence;
};

struct Point2D{
	float x; 
	float y;
};
struct Fingers{
Point2D finger1;
Point2D finger2;
Point2D finger3;
Point2D finger4;
Point2D finger5;
};
//custom orientation struct
struct QuaternionPoint{
	float w;
	float x; 
	float y;
	float z;
};

//user joint data

struct SkeletonPoint{
int id;
Point3D Head;
Point3D RightHand;
Point3D LeftHand;
Point3D Torso;
int RightHandFingers;
int LeftHandFingers;
int RightHandGusture;
};

void ConvertPosition(SkeletonPoint * myPoint, nite::Skeleton mySkeleton);

//QuaternionPoint ConvertQuaternion(nite::Quaternion nitePt);

//-----------------------------------------------------------------------------
// MwareInterface API
//-----------------------------------------------------------------------------

//Init Update & shutdown
MWAREINTERFACE_API int Init(/*const char * DeviceUri*/);
MWAREINTERFACE_API int Update(SkeletonPoint * skeletonPoint);
MWAREINTERFACE_API void Shutdown();

//----------------------------------------------------------------------------
//callbacks
//----------------------------------------------------------------------------
typedef void (__stdcall * ProgressCallback)(int);

MWAREINTERFACE_API  void DoWork(long progressCallback);

//functions
void updateUserState(const nite::UserData& user, unsigned long long ts);
float getJointImgCoordinates(const nite::Skeleton & skeleton, nite::UserTracker & userTracker , const nite::JointType skeletonJoint, float *v);
bool getHandContour(const Mat &depthMat, const float *v, vector<Point> &handContour, vector<Point> &originalHandContour);
void detectFingerTips(const vector<Point> &handContour, vector<Point> &fingerTips, Mat *debugFrame);
void drawContour(Mat &img, const vector<Point> &contour, const Scalar &color);
double convexity(const vector<Point> &contour);

void readGustureContour(string path, vector<Point> & gusture);
