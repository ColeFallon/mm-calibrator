/*! \file	calibration.hpp
 *  \brief	Header file for generic calibration functions.
 *
 * This should contain functions useful for both intrinsic and extrinsic calibration.
 */

#ifndef CALIBRATION_HPP
#define CALIBRATION_HPP

//#include "cv_utils.hpp"
#include "improc.h"

// =============================================
// INCLUDES
// =============================================

//#include "cv_utils.hpp"
//#include "im_proc.hpp"

#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <sys/stat.h>
#include <stdio.h>

#ifdef _WIN32
#include <ctime>
#endif

#define CHESSBOARD_FINDER_CODE          0
#define MASK_FINDER_CODE                1
#define HEATED_CHESSBOARD_FINDER_CODE   2

#define ALL_PATTERNS_OPTIMIZATION_CODE              0
#define RANDOM_SET_OPTIMIZATION_CODE                1
#define FIRST_N_PATTERNS_OPTIMIZATION_CODE          2
#define ENHANCED_MCM_OPTIMIZATION_CODE              3
#define BEST_OF_RANDOM_PATTERNS_OPTIMIZATION_CODE   4
#define EXHAUSTIVE_SEARCH_OPTIMIZATION_CODE         5
#define RANDOM_SEED_OPTIMIZATION_CODE               6
#define SCORE_BASED_OPTIMIZATION_CODE               7

#define DEBUG_MODE 0

#define PATTERN_FINDER_CV_CORNER_SUBPIX_FLAG 4

#define TRACKING 0
#define RADIAL_LENGTH 1000
#define FOLD_COUNT 1

#define PI 3.14159265

#define MAX_CAMS 3
#define MAX_SEARCH_DIST 3

#define REGULAR_OPENCV_CHESSBOARD_FINDER    0
#define MASK_FINDER                         3
#define MASK_FINDER_INNERS                  8
#define EXTENDED_CHESSBOARD_FINDER          5
#define INVERTED_OPENCV_CHESSBOARD_FINDER   10

#define MIN_DISTANCE_FROM_EDGE 2

#define MAX_FRAMES_PER_INPUT    5000
#define MAX_FRAMES_TO_LOAD      1000
#define MAX_PATTERNS_TO_KEEP    500
#define MAX_CANDIDATE_PATTERNS  100
#define MAX_PATTERNS_PER_SET    10

#define DEFAULT_CORRECTION_FACTOR 0.5

// DEFAULT MSER SETTINGS
#define MSER_delta				7.5
#define MSER_max_variation		0.25
#define MSER_min_diversity		0.20
#define MSER_max_evolution		200
#define MSER_area_threshold		1.01
#define MSER_min_margin			0.003
#define MSER_edge_blur_size		5

#define PATCH_CORRECTION_INTRINSICS_FLAGS CV_CALIB_RATIONAL_MODEL

using namespace std;
using namespace cv;

struct mserParameterGroup {
	double delta;
	double max_variation;
	double min_diversity;
	int max_evolution;
	double area_threshold;
	double min_margin;
	int edge_blur_size;
	
	mserParameterGroup();
	mserParameterGroup(double delta_, double max_variation_, double min_diversity_, int max_evolution_, double area_threshold_, double min_margin_, int edge_blur_size_);
};

/// \brief		Class enables for storing an MSER feature beyond simply its bounding points
class mserPatch
{
public:
    /// \brief		Point vector representing the bounding points of the patch
    vector<Point> hull;
    /// \brief		Integer pixel location of the patches centroid
    Point centroid;
    /// \brief		Integer pixel location of the patches centroid
    Point2f centroid2f;
    /// \brief		A vector storing all of the moments associated with the patch
    Moments momentSet;
    /// \brief		The total area in pixels of the patch
    double area;
    /// \brief		An estimate of the mean intensity of pixels within the patch
    double meanIntensity;
    /// \brief		An estimate of the variance of intensity of pixels within the patch
    double varIntensity;

    /// \brief 		Default Constructor.
    mserPatch();

    /// \brief 		Constructor from a point vector and a pointer to the matrix (for intensities).
    mserPatch(vector<Point>& inputHull, const Mat& image);
};

/// \brief      Generates a random set of indices from a valid range
void generateRandomIndexArray(int * randomArray, int maxElements, int maxVal);

/// \brief      Blurs and enhances a distribution map to make it more displayable
void prepForDisplay(const Mat& distributionMap, Mat& distributionDisplay);

/// \brief      Adds a pointset to a radial distribution array
void addToRadialDistribution(double *radialDistribution, cv::vector<Point2f>& cornerSet, Size imSize);

/// \brief      Add a cornerset to the tally matrix
void addToBinMap(Mat& binMap, cv::vector<Point2f>& cornerSet, Size imSize);

/// \brief      Calculate the score for a pointset in terms of its contribution to calibration
double obtainSetScore(Mat& distributionMap,
                      Mat& binMap,
                      Mat& gaussianMat,
                      cv::vector<Point2f>& cornerSet,
                      double *radialDistribution);

/// \brief 		Verifies that the final patches do actually represent a grid pattern
bool verifyCorners(Size imSize, Size patternSize, vector<Point2f>& patternPoints, double minDist, double maxDist);

/// \brief 		Move a vector of points from one vector of vectors to another
void transferMserElement(vector<vector<Point> >& dst, vector<vector<Point> >& src, int index);
/// \brief 		Move an MSER-patch from one vector to another, resizing and shifting the old vector
void transferPatchElement(vector<mserPatch>& dst, vector<mserPatch>& src, int index);

/// \brief      Filters out MSERs that cannot possibly be part of the geometric pattern
void clusterFilter(vector<mserPatch>& patches, vector<vector<Point> >& msers, int totalPatches);
/// \brief      Filters out most deviant patches to retain only the correct number for the pattern
void reduceCluster(vector<mserPatch>& patches, vector<vector<Point> >& msers, int totalPatches);

/// \brief      Filters out MSERs that are enclosed by or enclose true patches
void enclosureFilter(vector<mserPatch>& patches, vector<vector<Point> >& msers);

/// \brief      Filters out MSERs that have a large amount of internal pixel intensity variance
void varianceFilter(vector<mserPatch>& patches, vector<vector<Point> >& msers);

/// \brief      Filters out MSERs that do not have a realistic shape
void shapeFilter(vector<mserPatch>& patches, vector<vector<Point> >& msers);

/// \brief      Measures the coverage of a cameras field of view by analysing the distribution tally
void addToDistributionMap(Mat& distributionMap, vector<Point2f>& corners);

///// \brief      Redistorts points using an established distortion model
//void redistortPoints(const vector<Point2f>& src, vector<Point2f>& dst, const Mat& cameraMatrix, const Mat& distCoeffs, const Mat& newCamMat=Mat::eye(3,3,CV_64FC1));

/// \brief 		Generic grid verification function
bool verifyPattern(Size imSize, Size patternSize, vector<Point2f>& patternPoints, double minDist, double maxDist);

/// \brief      Finds just the MSER centres from an image
bool findPatternCentres(const Mat& image, Size patternSize, vector<Point2f>& centres, int mode);

/// \brief 		Sorts the patches into a standard order to prepare for homography
void sortPatches(Size imageSize, Size patternSize, vector<Point2f>& patchCentres, int mode);

/// \brief 		Re-orders patches into row by row, left to right
void reorderPatches(Size patternSize, int mode, int *XVec, int *YVec, vector<Point2f>& patchCentres);

/// \brief      Estimate the locations of patch corners based on patch centroids
void interpolateCornerLocations2(const Mat& image, int mode, Size patternSize, vector<Point2f>& vCentres, vector<Point2f>& vCorners);

/// \brief      Converts points ordering from row-by-row to quad-clustered
void groupPointsInQuads(Size patternSize, vector<Point2f>& corners);

/// \brief      Refines positions of corners through iterative local homography mappings
void refineCornerPositions(const Mat& image, Size patternSize, vector<Point2f>& vCorners, double correctionFactor);

/// \brief      Initial attempt to correct locations of all corners based on estimates from MSER centroids
void initialRefinementOfCorners(const Mat& imGrey, vector<Point2f>& src, Size patternSize);

/// \brief 		Sorts the corners into a standard order to prepare for calibration
void sortCorners(Size imageSize, Size patternSize, vector<Point2f>& corners);

/// \brief      Returns the subpixel location of the corner given a decent estimate
int findBestCorners(const Mat& image, vector<Point2f>& src, vector<Point2f>& dst, Size patternSize, int detector, int searchDist = MAX_SEARCH_DIST);

/// \brief      Visually pattern on image
void debugDisplayPattern(const Mat& image, Size patternSize, Mat& corners, bool mode = true);

/// \brief 		Verifies that the final patches do actually represent a grid pattern
bool verifyPatches(Size imSize, Size patternSize, vector<Point2f>& patchCentres, int mode, double minDist, double maxDist);

/// \brief      Uses contrast enhancement etc to improve positional estimates of patch centres
bool correctPatchCentres(const Mat& image, Size patternSize, vector<Point2f>& patchCentres, int mode);

/// \brief 		Estimates the co-ordinates of the corners of the patches
bool findPatchCorners(const Mat& image, Size patternSize, Mat& homography, vector<Point2f>& corners, vector<Point2f>& patchCentres2f, double correctionFactor, int mode, int detector = 0);

/// \brief 		MSER-clustering mask corner locater
bool findMaskCorners_1(const Mat& image, Size patternSize, vector<Point2f>& corners, mserParameterGroup mserParams, double correctionFactor, int detector = 0);

/// \brief 		Core pattern-finding function
bool findPatternCorners(const Mat& image, Size patternSize, vector<Point2f>& corners, int mode, mserParameterGroup mserParams, double correctionFactor, int detector = 0);

/// \brief 		Find all patches (MSERS - using default settings) in an image
void findAllPatches(const Mat& image, Size patternSize, vector<vector<Point> >& msers, mserParameterGroup mserParams);

/// \brief 		Finds (or simulates) the edge patches in a pattern
void findEdgePatches(Size patternSize, int mode, int *XVec, int *YVec, vector<Point2f>& patchCentres, vector<Point2f>& remainingPatches);

/// \brief 		Finds (or simulates) the interior patches in a pattern
void findInteriorPatches(Size patternSize, int mode, int *XVec, int *YVec, vector<Point2f>& patchCentres, vector<Point2f>& remainingPatches);

/// \brief      Randomly cuts down a list of input files
void randomCulling(vector<std::string>& inputList, int maxSearch);

/// \brief      Culls some patterns from a vector, and from the corresponding names list
void randomCulling(vector<std::string>& inputList, int maxSearch, vector<vector<Point2f> >& patterns);

/// \brief      Culls some sets of patterns from a vector vector, and from the corresponding names list
void randomCulling(vector<std::string>& inputList, int maxSearch, vector<vector<vector<Point2f> > >& patterns);

/// \brief      Checks validity of image for calibration
bool checkAcutance();

/// \brief 		Determines how many patches can be found in each row and extreme columns
void determineFindablePatches(Size patternSize, int mode, int *XVec, int *YVec);

/// \brief 		Finds (or simulates) the four corners patches in a pattern
void findCornerPatches(Size imageSize, Size patternSize, int mode, int *XVec, int *YVec, vector<Point2f>& patchCentres, vector<Point2f>& remainingPatches);

/// \brief      Determines the distribution of patches based on pattern dimensions and pattern type
void determinePatchDistribution(Size patternSize, int mode, int& rows, int& cols, int& quant);

/// \brief      Visually display patches on image
void debugDisplayPatches(const Mat& image, vector<vector<Point> >& msers);

/// \brief 		Applies various area, colour and positional filters to reduce no. of patches to desired amount
bool refinePatches(const Mat& image, Size patternSize, vector<vector<Point> >& msers, vector<Point2f>& patchCentres, int mode);

/// \brief          Checks that the entire pattern is inside the frame by at least a specified margin.
bool patternInFrame(Size imSize, vector<Point2f>& patternPoints, int minBorder = 2);

#endif
