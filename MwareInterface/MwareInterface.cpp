#include "MwareInterface.h"

// Globals
nite::UserTracker userTracker;
nite::Status niteRc;
nite::UserTrackerFrameRef userTrackerFrame;
openni::VideoFrameRef depthFrame,colorFrame, userFrame;

openni::Status rc = openni::STATUS_OK;
openni::Device device;
openni::VideoStream depth, color;

vector<Point> originalHandContour;
vector<Point> g_Hi5,g_1finger,g_2finger,g_4finger,g_Horn, g_Zoom;
	

int Init()
{
	//Mat m;
	//m.setTo(5);
	//imwrite("yyyyyyyyyyyy.jpg",m);
	printf("This is from MwareInterface.dll\n");
	
	nite::NiTE::initialize();

	const char* deviceURI = openni::ANY_DEVICE;/*"D:\\myrecorder4.oni";*/
	rc = openni::OpenNI::initialize();

	printf("After initialization:\n%s\n", openni::OpenNI::getExtendedError());

	rc = device.open(deviceURI);
	if (rc != openni::STATUS_OK)
	{
		printf("SimpleViewer: Device open failed:\n%s\n", openni::OpenNI::getExtendedError());
		//openni::OpenNI::shutdown();
		return -1;
	}

	rc = depth.create(device, openni::SENSOR_DEPTH);
	if (rc == openni::STATUS_OK)
	{
		rc = depth.start();
		if (rc != openni::STATUS_OK)
		{
			printf("SimpleViewer: Couldn't start depth stream:\n%s\n", openni::OpenNI::getExtendedError());
			depth.destroy();
		}
	}
	else
	{
		printf("SimpleViewer: Couldn't find depth stream:\n%s\n", openni::OpenNI::getExtendedError());
	}

	rc = color.create(device, openni::SENSOR_COLOR);
	if (rc == openni::STATUS_OK)
	{
		rc = color.start();
		if (rc != openni::STATUS_OK)
		{
			printf("SimpleViewer: Couldn't start color stream:\n%s\n", openni::OpenNI::getExtendedError());
			color.destroy();
		}
	}
	else
	{
		printf("SimpleViewer: Couldn't find color stream:\n%s\n", openni::OpenNI::getExtendedError());
	}

	if (!depth.isValid() || !color.isValid())
	{
		printf("SimpleViewer: No valid streams. Exiting\n");
		//openni::OpenNI::shutdown();
		return -1;
	}

	niteRc = userTracker.create();
	if (niteRc != nite::STATUS_OK)
	{
		printf("Couldn't create user tracker\n");
		return -1;
	}
	///////////////////////////////////
	
	//initialize justure templates
	readGustureContour("template62.txt",g_Hi5);
	readGustureContour("template340.txt",g_4finger);
	readGustureContour("template413.txt",g_Horn);
	readGustureContour("template185.txt",g_1finger);
	readGustureContour("template305.txt",g_2finger);
	readGustureContour("template116.txt",g_Zoom);	

	return 0;
}

void updateUserState(const nite::UserData& user, unsigned long long ts)
{
	if (user.isNew())
		USER_MESSAGE("New")
	else if (user.isVisible() && !g_visibleUsers[user.getId()])
		USER_MESSAGE("Visible")
	else if (!user.isVisible() && g_visibleUsers[user.getId()])
		USER_MESSAGE("Out of Scene")
	else if (user.isLost())
		USER_MESSAGE("Lost")

	g_visibleUsers[user.getId()] = user.isVisible();


	if(g_skeletonStates[user.getId()] != user.getSkeleton().getState())
	{
		switch(g_skeletonStates[user.getId()] = user.getSkeleton().getState())
		{
		case nite::SKELETON_NONE:
			USER_MESSAGE("Stopped tracking.")
			break;
		case nite::SKELETON_CALIBRATING:
			USER_MESSAGE("Calibrating...")
			break;
		case nite::SKELETON_TRACKED:
			USER_MESSAGE("Tracking!")
			break;
		case nite::SKELETON_CALIBRATION_ERROR_NOT_IN_POSE:
		case nite::SKELETON_CALIBRATION_ERROR_HANDS:
		case nite::SKELETON_CALIBRATION_ERROR_LEGS:
		case nite::SKELETON_CALIBRATION_ERROR_HEAD:
		case nite::SKELETON_CALIBRATION_ERROR_TORSO:
			USER_MESSAGE("Calibration Failed... :-|")
			break;
		}
	}
}

//Update function Definition
int Update(SkeletonPoint * skeleton)
{
	try{
		const float minHandExtension = 0.2f; // in meters
		const double grabConvexity = 0.8;
		char key = 0;
	
		Mat mask(frameSize, CV_8UC1);

		niteRc = userTracker.readFrame(&userTrackerFrame);
		rc = depth.readFrame(&depthFrame);
		rc = color.readFrame(&colorFrame);

		if (niteRc != nite::STATUS_OK)
		{
			printf("Get next frame failed\n");
			return 0;
		}

		// acquire bgr image
		{
			Mat mat(640,480, CV_8UC3, (unsigned char*) colorFrame.getData());
			//Mat mat(frameSize, CV_8UC3, cv::Scalar::all(0));
			cvtColor(mat, bgrMat, CV_RGB2BGR);	
		}

		// acquire depth image
		{
			Mat mat(frameSize, CV_16UC1, (unsigned char*) depthFrame.getData());
			mat.copyTo(depthMat);
			depthMat.convertTo(depthMat8, CV_8UC1, 255.0f / 3000.0f);
			cvtColor(depthMat8, depthMatBgr, CV_GRAY2BGR);
		}
		
		const nite::Array<nite::UserData>& users = userTrackerFrame.getUsers();
		float conf; // joint confidence
		float rh[3]; // right hand coordinates (x[px], y[px], z[meters])
		float lh[3]; // left hand coordinates
		float t[3]; // torso coordinates
		for (int i = 0; i < users.getSize(); ++i)
		{
			const nite::UserData& user = users[i];
			updateUserState(user,userTrackerFrame.getTimestamp());
			if (user.isNew())
			{
				userTracker.startSkeletonTracking(user.getId());
			}
			else if (user.getSkeleton().getState() == nite::SKELETON_TRACKED)
			{
				//get the skeletion
				ConvertPosition(skeleton, user.getSkeleton());
				//cout<<user.getSkeleton().getJoint(nite::JOINT_RIGHT_HAND).getPosition().x<<"=="<<user.getSkeleton().getJoint(nite::JOINT_RIGHT_HAND).getPosition().y<<endl;
				// torso
				if ( getJointImgCoordinates(user.getSkeleton(),userTracker,nite::JOINT_TORSO,t)  == 1 ) {
					unsigned char shade = 255 - (unsigned char)(t[2] *  128.0f);
					circle(depthMatBgr, Point((int)t[0], (int)t[1]), 10, Scalar(shade, 0, 0), -1);

					// right hand
					if ( 
						( getJointImgCoordinates(user.getSkeleton(), userTracker, nite::JOINT_RIGHT_HAND, rh) == 1) /* confident detection */ && 
						(rh[2] < t[2] - minHandExtension) /* user extends hand towards screen */ &&
						(rh[1] < t[1]) /* user raises his hand */
					) {
						unsigned char shade = 255 - (unsigned char)(rh[2] *  128.0f);
						Scalar color(0, 0, shade);
						vector<Point> handContour;
						getHandContour(depthMat, rh, handContour, originalHandContour);
					
						//hand gusture code
						double result = matchShapes(originalHandContour, g_Hi5 ,CV_CONTOURS_MATCH_I1,1);
					
						int data = 0;
						if(result<0.15){
							cout<<result<<" => Hi5"<<endl;
							data+=1;
						}
						result = matchShapes(originalHandContour, g_1finger ,CV_CONTOURS_MATCH_I1,1);
						if(result<0.2){
							cout<<result<<" => 1finger"<<endl;
							data+=10;
						}
						result = matchShapes(originalHandContour, g_2finger ,CV_CONTOURS_MATCH_I1,1);
						if(result<0.2){
							cout<<result<<" => 2finger"<<endl;
							data+=100;
						}
						result = matchShapes(originalHandContour, g_Horn ,CV_CONTOURS_MATCH_I1,1);
						if(result<0.1){
							cout<<result<<" => HORN"<<endl;
							data+=1000;
						}
						result = matchShapes(originalHandContour, g_Zoom ,CV_CONTOURS_MATCH_I1,1);
						if(result<0.1){
							cout<<result<<" => ZOOM"<<endl;
							data+=10000;
						}

						skeleton->RightHandGusture = data;

						bool grasp = convexity(handContour) > grabConvexity;
						int thickness = grasp ? CV_FILLED : 3;
						circle(depthMatBgr, Point(rh[0], rh[1]), 10, color, thickness);
					
						vector<Point> fingerTips;
						detectFingerTips(handContour, fingerTips, &depthMatBgr);
						//right fingers number ditected
						skeleton->RightHandFingers = fingerTips.size();

					}
					// left hand
					if ( 
						(getJointImgCoordinates(user.getSkeleton(), userTracker, nite::JOINT_LEFT_HAND, lh) == 1) &&
						(lh[2] < t[2] - minHandExtension) &&
						(lh[1] < t[1]) /* user raises his hand */
					) {
						unsigned char shade = 255 - (unsigned char)(lh[2] *  128.0f);
						Scalar color(0, shade, 0);

						vector<Point> handContour;
						getHandContour(depthMat, lh, handContour, originalHandContour);
						bool grasp = convexity(handContour) > grabConvexity;
						int thickness = grasp ? CV_FILLED : 3;
						circle(depthMatBgr, Point(lh[0], lh[1]), 10, color, thickness);

						vector<Point> fingerTips;
						detectFingerTips(handContour, fingerTips, &depthMatBgr);
						//right fingers number ditected
						skeleton->LeftHandFingers = fingerTips.size();

					}
				}
			}
		}
		return 1;
	}
	catch(exception& e){
		cout<<"ERROR:"<<e.what()<<endl;
		//Shutdown();
		return -1;
	}
	//imshow("depthMatBgr", depthMatBgr);
	
		//niteRc = userTracker.readFrame(&userTrackerFrame);
		//if (niteRc != nite::STATUS_OK)
		//{
		//	printf("Get next frame failed\n");
		//	return;
		//}
		////check for the hand
		////niteRc = handTracker.readFrame(&handTrackerFrame);
		///*if (niteRc != nite::STATUS_OK)
		//{
		//	printf("Get next frame failed\n");
		//	return;
		//}*/

		//const nite::Array<nite::UserData>& users = userTrackerFrame.getUsers();
		//for (int i = 0; i < users.getSize(); ++i)
		//{
		//	const nite::UserData& user = users[i];
		//	updateUserState(user,userTrackerFrame.getTimestamp());
		//	if (user.isNew())
		//	{
		//		userTracker.startSkeletonTracking(user.getId());
		//	}
		//	else if (user.getSkeleton().getState() == nite::SKELETON_TRACKED)
		//	{
		//		const nite::SkeletonJoint& head = user.getSkeleton().getJoint(nite::JOINT_HEAD);
		//		if (head.getPositionConfidence() > .5){
		//	
		//			ConvertPosition(skeleton, user.getSkeleton());
		//			
		//			//printf( "%5.2f ,%5.2f \n",point_3d->x,head.getPosition().x);
		//			printf("%d. (%5.2f, %5.2f, %5.2f)\n", user.getId(), head.getPosition().x, head.getPosition().y, head.getPosition().z);
		//		}
		//	}
		//}
}

void Shutdown()
{
	openni::OpenNI::shutdown();
	nite::NiTE::shutdown();
}

void ConvertPosition(SkeletonPoint * myPoint, nite::Skeleton mySkeleton){
	myPoint->Head.x = mySkeleton.getJoint(nite::JOINT_HEAD).getPosition().x;
	myPoint->Head.y = mySkeleton.getJoint(nite::JOINT_HEAD).getPosition().y;
	myPoint->Head.z = mySkeleton.getJoint(nite::JOINT_HEAD).getPosition().z;
	myPoint->Head.confidence = mySkeleton.getJoint(nite::JOINT_HEAD).getPositionConfidence();

	myPoint->RightHand.x = mySkeleton.getJoint(nite::JOINT_RIGHT_HAND).getPosition().x;
	myPoint->RightHand.y = mySkeleton.getJoint(nite::JOINT_RIGHT_HAND).getPosition().y;
	myPoint->RightHand.z = mySkeleton.getJoint(nite::JOINT_RIGHT_HAND).getPosition().z;
	myPoint->RightHand.confidence = mySkeleton.getJoint(nite::JOINT_RIGHT_HAND).getPositionConfidence();
	
	myPoint->LeftHand.x = mySkeleton.getJoint(nite::JOINT_LEFT_HAND).getPosition().x;
	myPoint->LeftHand.y = mySkeleton.getJoint(nite::JOINT_LEFT_HAND).getPosition().y;
	myPoint->LeftHand.z = mySkeleton.getJoint(nite::JOINT_LEFT_HAND).getPosition().z;
	myPoint->LeftHand.confidence = mySkeleton.getJoint(nite::JOINT_LEFT_HAND).getPositionConfidence();
	
	myPoint->Torso.x = mySkeleton.getJoint(nite::JOINT_TORSO).getPosition().x;
	myPoint->Torso.y = mySkeleton.getJoint(nite::JOINT_TORSO).getPosition().y;
	myPoint->Torso.z = mySkeleton.getJoint(nite::JOINT_TORSO).getPosition().z;
	myPoint->Torso.confidence = mySkeleton.getJoint(nite::JOINT_TORSO).getPositionConfidence();
}

/*QuaternionPoint ConvertQuaternion(nite::Quaternion nitePt){
	QuaternionPoint myPoint;
	myPoint.w = nitePt.w;
	myPoint.x = nitePt.x;
	myPoint.y = nitePt.y;
	myPoint.z = nitePt.z;
	return myPoint;
}*/

//-----------------------------------------------------------------------------
// callbacks
//-----------------------------------------------------------------------------

MWAREINTERFACE_API void DoWork(long progressCallbackAddress){
	ProgressCallback progressCallback = (ProgressCallback) progressCallbackAddress;
	
	//do the work....
	int counter = 11;
	if (progressCallback)
        {
            // send progress update
            progressCallback(counter);
        }
}


float getJointImgCoordinates(const nite::Skeleton & skeleton, nite::UserTracker & userTracker , const nite::JointType skeletonJoint, float *v) {
// torso
//if ( user.getSkeleton().getJoint(nite::JOINT_TORSO).getPositionConfidence() == 1 ) {
	const nite::Point3f jointPosition = skeleton.getJoint(skeletonJoint).getPosition();
	float temp[3], x, y;
	temp[0] = jointPosition.x;
	temp[1] = jointPosition.y;
	temp[2] = jointPosition.z;
	userTracker.convertJointCoordinatesToDepth(temp[0], temp[1], temp[2], & x, & y);
	v[0] = x;
	v[1] = y;
	v[2] = temp[2]/1000.0f;

	return skeleton.getJoint(skeletonJoint).getPositionConfidence();
}

//////////////////////////////////////////////////////////////////////////////
void drawContour(Mat &img, const vector<Point> &contour) {
	vector<vector<Point> > contours;
	contours.push_back(contour);
	drawContours(img, contours, -1, Scalar(255,255,255));
}

///////////////////////////////////////////////////////////////////////////

bool getHandContour(const Mat &depthMat, const float *v, vector<Point> &handContour, vector<Point> &originalHandContour) {
	const int maxHandRadius = 96; // in px
	const short handDepthRange = 100; // in mm
	const double epsilon = 17.5; // approximation accuracy (maximum distance between the original hand contour and its approximation)

	unsigned short depth = (unsigned short) (v[2] * 1000.0f); // hand depth
	unsigned short nearClip = (depth - 100); // near clipping plane
	unsigned short farClip = depth + 100; // far clipping plane

	static Mat mask(frameSize, CV_8UC1);
	mask.setTo(0);

	// extract hand region	
	circle(mask, Point(v[0], v[1]), maxHandRadius, 255, CV_FILLED);
	mask = mask & depthMat > nearClip & depthMat < farClip;

	// DEBUG(show mask)
	//imshow("mask1", mask);

	// assume largest contour in hand region to be the hand contour
	vector<vector<Point> > contours;
	findContours(mask, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	int n = contours.size();
	int maxI = -1;
	int maxSize = -1;
	for (int i=0; i<n; i++) {
		int size  = contours[i].size();
		if (size > maxSize) {
			maxSize = size;
			maxI = i;
		}
	}

	bool handContourFound = (maxI >= 0);

	if (handContourFound) {
		approxPolyDP( Mat(contours[maxI]), handContour, epsilon, true );
		//handContour = contours[maxI];
		originalHandContour = contours[maxI];
	}

//	// DEBUG(draw hand contour
//	mask.setTo(0);	
//	if (maxI >= 0) {
//		vector<vector<Point> > contours2;
//		contours2.push_back(contours[maxI]);
//		drawContours(mask, contours2, -1, 255);
//		imshow("mask2", mask);
//	}

	return maxI >= 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

void detectFingerTips(const vector<Point> &handContour, vector<Point> &fingerTips, Mat *debugFrame = NULL) {
	Mat handContourMat(handContour);
	double area = cv::contourArea(handContourMat);
	const Scalar debugFingerTipColor(255,0,0);

	vector<int> hull;
	cv::convexHull(handContourMat, hull);

	// find upper and lower bounds of the hand and define cutoff threshold (don't consider lower vertices as fingers)
	int upper = 640, lower = 0;
	for (int j=0; j<hull.size(); j++) {
		int idx = hull[j]; // corner index
		if (handContour[idx].y < upper) upper = handContour[idx].y;
		if (handContour[idx].y > lower) lower = handContour[idx].y;
	}
	float cutoff = lower - (lower - upper) * 0.1f;

	// find interior angles of hull corners
	for (int j=0; j<hull.size(); j++) {
		int idx = hull[j]; // corner index
		int pdx = idx == 0 ? handContour.size() - 1 : idx - 1; //  predecessor of idx
		int sdx = idx == handContour.size() - 1 ? 0 : idx + 1; // successor of idx

		Point v1 = handContour[sdx] - handContour[idx];
		Point v2 = handContour[pdx] - handContour[idx];

		float angle = acos( (v1.x*v2.x + v1.y*v2.y) / (norm(v1) * norm(v2)) );

		// low interior angle + within upper 90% of region -> we got a finger
		if (angle < 1 && handContour[idx].y < cutoff) {
			int u = handContour[idx].x;
			int v = handContour[idx].y;

			fingerTips.push_back(Point2i(u,v));
			
			if (debugFrame) { // draw fingertips
				cv::circle(*debugFrame, handContour[idx], 10, debugFingerTipColor, -1);
			}
		}
	}

	if (debugFrame) {
		// draw cutoff threshold
		cv::line(*debugFrame, Point(0, cutoff), Point(640, cutoff), debugFingerTipColor);

		// draw approxCurve
		for (int j=0; j<handContour.size(); j++) {
			cv::circle(*debugFrame, handContour[j], 10, debugFingerTipColor);
			if (j != 0) {
				cv::line(*debugFrame, handContour[j], handContour[j-1], debugFingerTipColor);
			} else {
				cv::line(*debugFrame, handContour[0], handContour[handContour.size()-1], debugFingerTipColor);
			}
		}

		// draw approxCurve hull
		for (int j=0; j<hull.size(); j++) {
			cv::circle(*debugFrame, handContour[hull[j]], 10, debugFingerTipColor, 3);
			if(j == 0) {
				cv::line(*debugFrame, handContour[hull[j]], handContour[hull[hull.size()-1]], debugFingerTipColor);
			} else {
				cv::line(*debugFrame, handContour[hull[j]], handContour[hull[j-1]], debugFingerTipColor);
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////

double convexity(const vector<Point> &contour) {
	Mat contourMat(contour);

	vector<int> hull;
	convexHull(contourMat, hull);

	int n = hull.size();
	vector<Point> hullContour;

	for (int i=0; i<n; i++) {
		hullContour.push_back(contour[hull[i]]);
	}

	Mat hullContourMat(hullContour);

	return (contourArea(contourMat) / contourArea(hullContourMat));
}


bool findContour(Mat &mask, vector<Point> &contr){

	vector<vector<Point> > contours;
	findContours(mask, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	int n = contours.size();
	int maxI = -1;
	int maxSize = -1;
	for (int i=0; i<n; i++) {
		int size  = contours[i].size();
		if (size > maxSize) {
			maxSize = size;
			maxI = i;
		}
	}
	
	if(maxI >= 0){
		contr = contours[maxI];
	}
	return maxI >= 0;
}

//read contour from file
void readGustureContour(string path, vector<Point> & gusture){
	char readString[10000] = "";
	vector<int> readint;
	vector<Point> gustureContour;

	ifstream myReadFile;
	myReadFile.open(path);
	char outf[100];
	if(myReadFile.is_open()){
		while(!myReadFile.eof()){
			myReadFile >>outf;
			strcat(readString,outf);
		}
		//cout<<readString<<endl;	
	}
	
	char * pch;
	pch = strtok (readString,"\[\],;");
	while (pch != NULL)
	{
		//printf ("%s\n",pch);
		int temp;
		istringstream ( pch ) >> temp; 
		readint.push_back(temp);
		pch = strtok (NULL, "\[\],;");
	}
	
	for(int i = 0; i < readint.size();i+=2){
		gustureContour.push_back(Point(readint[i],readint[i+1]));
	}
	gusture = gustureContour; 
}

