#include "CourtDetect.h"

#include <opencv2/highgui.hpp>

const int minImagePoints = 4;
const int maxImagePoints = 15;
const std::string courtWindow = "Court";
const std::string frameWindow = "Frame";
const cv::Scalar green(0, 255, 0);
const cv::Scalar black(0, 0, 0);

struct cbData {
	CourtDetect* p;
	cv::Mat* mat;
};

CourtDetect::CourtDetect(const std::string& winName)
	: winName(winName), calibrated(false)
{
}

CourtDetect::~CourtDetect()
{
}

CourtDetect::CourtDetect(const std::string& winName, const std::string& settingsFile)
	: winName(winName), calibrated(false)
{
	cv::FileStorage fs(settingsFile, cv::FileStorage::READ);
	if (fs.isOpened())
	{
		fs["camera_matrix"] >> intrinsics;
		fs["distortion_coefficients"] >> distortion;
	}
}

bool CourtDetect::setCourt(const std::string courtFile)
{
	court = cv::imread(courtFile);
	if (court.empty())
	{
		return false;
	}
	return true;
}

cv::Mat CourtDetect::getCourt() const
{
	return court;
}

bool CourtDetect::setSettingsFile(const std::string settingsFile)
{
	cv::FileStorage fs(settingsFile, cv::FileStorage::READ);
	if (!fs.isOpened())
	{
		return false;
	}
	fs["camera_matrix"] >> intrinsics;
	fs["distortion_coefficients"] >> distortion;
	return true;
}

void CourtDetect::projectPosition(cv::Mat& court, cv::Point2f position, cv::Scalar teamColor) const
{
	cv::Mat homography = findHomography(framePoints, courtPoints, cv::RANSAC);

	cv::Point2f courtPoint;
	std::vector<cv::Point2f> srcVecP{ position };
	std::vector<cv::Point2f> courtVecP{ courtPoint };

	cv::perspectiveTransform(srcVecP, courtVecP, homography);
	courtPoint = courtVecP[0];

	circle(court, courtPoint, 3, teamColor, 2, cv::LINE_8);

	cv::imshow(winName, court);
	cv::waitKey(10);
}

void CourtDetect::calibratePoints(const std::string& courtImage, const cv::Mat& frame)
{
	std::cout << "Click frame and court points\n";
	std::cout << "Up to 15 points can be captured. Press ESC to finish with less than 15 points.\n";

	// get court points
	court = cv::imread(courtImage);
	cv::Mat courtCopy = court.clone();
	cv::putText(courtCopy, "Click 4-15 court points", cv::Point(0, 25), 1, 2, green, 2);
	cv::imshow(courtWindow, courtCopy);
	cv::namedWindow(courtWindow);

	cbData courtCB;
	courtCB.p = this;
	courtCB.mat = &courtCopy;
	cv::setMouseCallback(courtWindow, &CourtDetect::onMouseClickCourt, &courtCB);

	// get frame points
	cv::Mat myFrame = frame.clone();
	cv::putText(myFrame, "Click 4-15 frame points", cv::Point(0, 25), 1, 2, green, 2);
	cv::imshow(frameWindow, myFrame);

	cv::namedWindow(frameWindow);

	cbData frameCB;
	frameCB.p = this;
	frameCB.mat = &myFrame;
	cv::setMouseCallback(frameWindow, &CourtDetect::onMouseClickFrame, &frameCB);

	cv::waitKey();
	cv::destroyWindow(courtWindow);
	cv::destroyWindow(frameWindow);

	if (framePoints.size() < minImagePoints || courtPoints.size() < minImagePoints)
	{
		std::cout << "ERROR must select at least 4 points for each.\n";
		std::exit(-1);
	}
	else if (framePoints.size() != courtPoints.size())
	{
		std::cout << "ERROR frame and court selected points must match.\n";
		std::exit(-1);
	}

	calibrated = true;
}

bool CourtDetect::isCalibrated() const
{
	return calibrated;
}

void CourtDetect::markClickedPoint(cv::Mat& image, const size_t points, int x, int y)
{
	cv::circle(image, cv::Point(x, y), 1, black, 5);
	cv::circle(image, cv::Point(x, y), 1, green, 2);
	cv::putText(image, std::to_string(points), cv::Point(x - 5, y - 10), 1, 1, black, 5);
	cv::putText(image, std::to_string(points), cv::Point(x - 5, y - 10), 1, 1, green, 2);
}

void CourtDetect::onMouseClickFrame(int event, int x, int y, int flags, void* param)
{
	cbData* data = (cbData*)param;

	if (data->p->framePoints.size() < maxImagePoints && event == cv::EVENT_FLAG_LBUTTON)
	{
		data->p->framePoints.push_back(cv::Point2f((float)x, (float)y));
		cv::Mat& tempFrame = *(data->mat);
		data->p->markClickedPoint(tempFrame, data->p->framePoints.size(), x, y);

		std::cout << "Frame point " << x << "," << y << " captured\n";
		cv::imshow(frameWindow, tempFrame);
	}
	else if (data->p->framePoints.size() == maxImagePoints)
	{
		std::cout << "Finished capturing frame points\n";
		cv::destroyWindow(frameWindow);
	}
}

void CourtDetect::onMouseClickCourt(int event, int x, int y, int flags, void* param)
{
	cbData* data = (cbData*)param;

	if (data->p->courtPoints.size() < maxImagePoints && event == cv::EVENT_FLAG_LBUTTON)
	{
		data->p->courtPoints.push_back(cv::Point2f((float)x, (float)y));
		cv::Mat& tempCourt = *(data->mat);
		data->p->markClickedPoint(tempCourt, data->p->courtPoints.size(), x, y);

		std::cout << "Court point " << x << "," << y << " captured\n";
		cv::imshow(courtWindow, tempCourt);
	}
	else if (data->p->courtPoints.size() == maxImagePoints)
	{
		std::cout << "Finished capturing court points\n";
		cv::destroyWindow(courtWindow);
	}
}
