#ifndef IMGPROCESS_H
#define IMGPROCESS_H

#include "imgprocess_msg.h"
//#include "../_Modules/Algo/localMinimum.h"
#include "../_Modules/Algo/datatypes.h"

#include <QImage>
#include <QColor>

#define _USE_MATH_DEFINES

#define PI 3.14159265
#define R2D PI/180          // convert radyant to degree

const int sobelX[3][3] = { {-1,0,1}, {-2,0,2}, {-1,0,1} };
const int sobelY[3][3] = { {1,2,1}, {0,0,0}, {-1,-2,-1} };
const int gaussianMask[5][5] = { {2,4,5,4,2},
                                 {4,9,12,9,4},
                                 {5,12,15,12,5},
                                 {4,9,12,9,4},
                                 {2,4,5,4,2} };
const int gaussianDivider = 159;

// class for the void(empty) line data
class voidLine{

public:
        QPoint start;
        QPoint end;
        int length;

        voidLine(){}
        ~voidLine(){}
};


// class for the solid(full) line data
class solidLine{

public:
        QPoint start;
        QPoint end;
        int length;
        float distance;
        float angle;

        solidLine(){}
        ~solidLine(){}
};


// class for major area indexes
class majorArea{

public:
    int startIndex;
    int endIndex;

    majorArea(){}
    ~majorArea(){}
};

struct minCostedLines {
    int c;
    int cost;
};

struct houghData {
    float distance;
    float angle;
    float voteValue;
};

// image processing class
class imgProcess{

    public:
        bool _DEBUG = false;
        QImage imgOrginal;      // orginal image
        QImage imgMono;         // mono image
        QImage imgCorner;       // orginal image with detected corners shown
        QImage imgCornerAndPrimaryLines;    // orginal image with detected corners and primary lines shown
        QImage imgSolidLines;

        const int imageWidth, imageHeight;  // org. image
        int edgeWidth, edgeHeight;          // edge detected image

        // hough transform vars
        int houghDistanceMax;               // max. hough distance, depends edge image size and center point
        int houghThetaSize;                 // hough search angle size, depends min/max/resolution of angle
        int centerX, centerY;               // search origin
        int thetaMin, thetaMax;             // search angles in between
        float thetaStep;                    // angle resolution

        // detection parameters
        int houghLineNo;                    // no. of max voted lines concerned
        int houghLinesSorted_size;
        int houghVoteAvg;                   // ave vote value of max. voted lines
        int voteThreshold;                  // vote threshold to accept primary line as concerned
        int voidThreshold;                  // void line length threhold in pixels to accept primary void line as concerned
        float distanceAvg, thetaAvg;
        float distanceAvgPrimary, thetaAvgPrimary;
        float distanceAvgSecondary, thetaAvgSecondary;
        bool primaryLineDetected;           // true: primary line detected
        bool detected;                      // false for any known un-matched criteria
        int voidIndex;                      // index no of <void space list> holding primary void line data
        int secondLineIndex;
        int trackCenterX = -1, trackCenterY = -1;     // coor. of center beteen corners
        int leftCornerX = -1, leftCornerY = -1;
        int rightCornerX = -1, rightCornerY = -1;
        int leftMostCornerX, leftMostCornerY;
        int rightMostCornerX, rightMostCornerY;
        QList<voidLine *> voidSpace;        // list to hold found void lines
        QList<int> lowLinesList, highLinesList;

        QList<solidLine *> solidSpace;      // list to hold found solid lines in single line
        QList<solidLine> solidSpaceMain;    // list to hold found solid lines in all hough space
        QList<solidLine> solidSpaceMainTrimmed; // solidSpaceMain: lines with length above threshold
        QList<solidLine> solidSpaceMainMaximums;// solidSpaceTrimmed: maximums of each distance value   NOT USED
        QList<solidLine> solidSpaceMainOrdered; // line length ordered list of solidSpaceMain   NOT USED
        QList<majorArea *> majorList;   // NOT USED
        float majorThresholdPercent;    // NOT USED
        int maxSolidLineLength;
        QList<solidLine> majorLines;    // list that hold major lines data FOR DEBUGGING
        QList<solidLine> major2Lines;   // list that hold 2 major lines
        bool majorLinesFound;
        solidLine primaryLine;
        QList<solidLine> primaryGroup;
        QList<solidLine> secondaryGroup;
        QList<solidLine> primaryGroupMaxs;
        QList<solidLine> secondaryGroupMaxs;
        bool primaryLineFound;
        bool secondaryLineFound;
        bool centerDetermined;

        // THIN JOINT ALGO
        int anglePrecision;
        bool thinJointInitSwitch;
        float *slope;
        QList<minCostedLines> lineList;
        minCostedLines *bestLines;
        float slopeBest;
        int slopeBestIndex = 0;
        QList<minCostedLines> deepLines;
        int centerC;
        QList<int> peakPoints;

        // CONTRAST ALGO
        int **contrastMatrix;               // contrast transition points
        bool contrastInitSwitch;            // to delete in destructor

        // EDGE DETETION ALGO
        int tHi, tLo;
        int hiValue, loValue, medianValue;
        int thinCornerNum;
        bool wideJoint = false;
        bool naturalBreaks = false;

        int localMaximaSize;
        int **rangeArray;
        bool rangeArrayInitSwitch;
        int listHoughDataSize;
        int **listHoughDataArray;
        bool listHoughDataArrayInitSwitch;

        int localMaxima2ndSize;
        int **rangeArray2nd;
        bool rangeArray2ndInitSwitch;
        int listHoughData2ndSize;
        int **listHoughData2ndArray;
        bool listHoughData2ndArrayInitSwitch;
        QList<houghData> listHoughData2nd;

        int localMaxima3rdSize;
        int **rangeArray3rd;
        bool rangeArray3rdInitSwitch;
        int listHoughData3rdSize;
        int **listHoughData3rdArray;
        bool listHoughData3rdArrayInitSwitch;

        int listHoughData3rdFilteredSize;
        int **listHoughData3rdFilteredArray;
        bool listHoughData3rdFilteredArrayInitSwitch;

        int neighbourhood;


        houghData centerLine;
        QList<houghData> mainEdgesList;
        int mainEdgeScore;
        float mainEdgeScorePercent;

        int solidLineLength = 0;

        int errorEdgeLimit;
        int errorAngleLimit;

        int **valueMatrix;                  // image data matrix
        int **valueMatrixOrg;                  // image data matrix
        int **edgeMatrix;                   // edge image data matrix
        int **edgeGradientMatrix;           // edge gradient data matrix
        bool edgeGradientMatrixInitSwitch = false;
        int **edgeSuppressedMatrix;
        bool edgeSuppressedMatrixInitSwitch = false;
        int **edgeStrongMatrix;
        bool edgeStrongMatrixInitSwitch = false;
        int **edgeWeakMatrix;
        bool edgeWeakMatrixInitSwitch = false;
        bool **edgeMapMatrix;
        bool edgeMapMatrixInitSwitch = false;
        bool **edgeVisitMatrix;
        bool edgeVisitMatrixInitSwitch = false;
        bool **edgeW2SMapMatrix;
        bool edgeW2SMapMatrixInitSwitch = false;

        int valueSelection;
        bool **edgeMapValueMatrix;
        bool edgeMapValueMatrixInitSwitch = false;
        bool **edgeMapRedMatrix;
        bool edgeMapRedMatrixInitSwitch = false;
        bool **edgeMapGreenMatrix;
        bool edgeMapGreenMatrixInitSwitch = false;
        bool **edgeMapBlueMatrix;
        bool edgeMapBlueMatrixInitSwitch = false;

        int **edgeThickenedMatrix;          // thickened edge image data matrix
        int **houghMatrix;                  // hough image data matrix with max. voted lines coded, edge image size
        int **houghExtendedMatrix;          // hough image data matrix with max. voted lines coded, org. image size
        int **houghSpace;                   // line votes: line search matrix, depends max. distance & angle scale
        bool houghSpaceInitSwitch = false;          // to delete in destructor

        float **houghLines;                 // line data of max. voted lines; distance/angle/vote value
        bool houghLinesInitSwitch = false;          // to delete in destructor

        float **houghLinesSorted;           // sort houghLines wrt distance
        bool houghLinesSortedInitSwitch = false;    // to delete in destructor

        float **horLineVotes;               // max vote values on hor line
        bool horLineVotesInitSwitch = false;// to delete in destructor

        int *histogram;                     // histogram array
        int *histogramFiltered;             // histogram array after noise reduction
        int *histogramD;                    // histogram derivative
        int histogramSize = 0;              // histogram array size
        bool histogramInitSwitch = false;           // to delete in destructor
        bool histogramFilteredInitSwitch = false;           // to delete in destructor
        double histogramAvg;
        int histogramMin, histogramMax;
        QList<range> histogramPeaks;
        QList<range> histogramMins;
        QList<range> histogramExtremes;
        QList<range> histogramExtremesFiltered;
        QList<int> histogramDerivative;
        double histogramMaxThreshold = 0.75;
        double histogramAngleThreshold = 11;
        QList<int> histogramMaxPeaksList;
        QList<QPoint> histogramMaxPoint;
        QList<QPoint> histogramMaxPointPair;
        QList<double> histogramMaxPointLen;
        QList<double> histogramMaxPointAng;
        int *histogramFilteredX;             // histogram array after noise reduction
        double lenRateThr = 0.50;
        int maFilterKernelSize = 11;
        int bandCheck_errorState;
        int bandWidth = 0;
        double bandShape = 0; // 1: rectengular
        int bandCenter = 0;
        double bandWidthMin = 0.25;
        double bandShapeMin = 0.5;
        double bandCenterMax = 0.1;

        double **valueMatrixNorm;           // normalised value matrix
        bool valueMatrixNormInitSwitch = false;     // to delete in destructor

        double **fuzzyEntropyMatrix;
        bool fuzzyEntropyMatrixInitSwitch = false;     // to delete in destructor
        int FEM_width = 1;
        int FEM_height = 1;

        // user information
        QString statusMessage;              // general message var about processing
        int angleAvg;                       // average angle degree wrt center point (-90)
        bool angleInLimit;                  // true if <avgAngle> is within +/- 3 degree

        int gaussianMatrixSize = 7;
        float stdDev = 0.84089642;
        float **gaussianMatrix;
        float gaussianMatrixSum = 1;
        bool gaussianMatrixInitSwitch = false;


        // constructor
        imgProcess(QImage &image, const int width, const int height) : imageWidth(width), imageHeight(height) {

            imgOrginal = image;     // passes image to class not copy of it
            valueMatrix = new int*[height];
            for (int i = 0; i < height; i++) valueMatrix[i] = new int[width];

            valueMatrixOrg = new int*[height];
            for (int i = 0; i < height; i++) valueMatrixOrg[i] = new int[width];

            edgeWidth = imageWidth - 2;
            edgeHeight = imageHeight - 2;
            centerX = edgeWidth / 2;    // HT origin coor. X
            centerY = edgeHeight - 1;   // HT origin coor. Y

            edgeMatrix = new int*[edgeHeight];
            for (int i = 0; i < edgeHeight; i++)   edgeMatrix[i] = new int[edgeWidth];

            // edge extended to orginal matrix
            edgeThickenedMatrix = new int*[height];
            for (int i = 0; i < height; i++)   edgeThickenedMatrix[i] = new int[width];

            houghMatrix = new int*[edgeHeight];
            for (int i = 0; i < edgeHeight; i++) houghMatrix[i] = new int[edgeWidth];

            houghExtendedMatrix = new int*[height];
            for (int i = 0; i < height; i++) houghExtendedMatrix[i] = new int[width];

            thetaMin = 80;
            thetaMax = 100;
            thetaStep = 1.0;
            houghLineNo = 5;
            houghVoteAvg = 0;
            voteThreshold = 200;
            voidThreshold = 30;
            distanceAvg = 0;
            thetaAvg = 0;

            primaryLineDetected = false;
            voidIndex = 0;
            trackCenterX = -1;
            trackCenterY = -1;

            errorEdgeLimit = 5;
            errorAngleLimit = 5;

            statusMessage = message0;
            angleAvg = 0;
            angleInLimit = false;

            houghSpaceInitSwitch = false;
            houghLinesInitSwitch = false;
            histogramInitSwitch = false;
            thinJointInitSwitch = false;
            contrastInitSwitch = false;
            houghLinesSortedInitSwitch = false;
            rangeArrayInitSwitch = false;
            listHoughDataArrayInitSwitch = false;
            rangeArray2ndInitSwitch = false;
            listHoughData2ndArrayInitSwitch = false;
            rangeArray3rdInitSwitch = false;
            listHoughData3rdArrayInitSwitch = false;
            listHoughData3rdFilteredArrayInitSwitch = false;
            edgeGradientMatrixInitSwitch = false;
            edgeStrongMatrixInitSwitch = false;
            edgeWeakMatrixInitSwitch = false;
            edgeVisitMatrixInitSwitch = false;
            edgeMapMatrixInitSwitch = false;
            edgeSuppressedMatrixInitSwitch = false;
            edgeW2SMapMatrixInitSwitch = false;
            edgeMapValueMatrixInitSwitch = false;
            edgeMapRedMatrixInitSwitch = false;
            edgeMapGreenMatrixInitSwitch = false;
            edgeMapBlueMatrixInitSwitch = false;
            gaussianMatrixInitSwitch = false;

            // no solid line
            primaryLine.start.setX( -1 );
            primaryLine.start.setY( -1 );
            primaryLine.end.setX( -1 );
            primaryLine.end.setY( -1 );
            primaryLine.length = -1;
            primaryLine.distance = -1;
            primaryLine.angle = -1;

            tHi = 50;
            tLo = 20;
            neighbourhood = 5;
        }

        void toMono();      // produce mono image
        void constructValueMatrix(QImage image);    // construct pixel value matrix of an image according to single color value
        void constructValueMatrix(QImage image, int selection);    // construct pixel value matrix of an image according to single color value
        void constructValueHueMatrix(QImage image, bool scale = false);
        void constructValueMaxMatrix(QImage image);    // construct pixel value matrix of an image according to single color value
        int getMatrixPoint(int *matrix, int width, int x, int y);   // returns value of a matrix

        void normalizeValueMatrix(double factor);
        float gaussianFn(int x,int y, float stddev);
        // saves a int matrix with given filename
        // saving with X and _ pointers (not used)
        bool saveMatrix(int **matrix, int width, int height, QString fname, int threshold = 255, bool xSwitch = false);

        bool saveMatrix(float **matrix, int width, int height, QString fname);  // saves a float matrix with given filename
        bool saveMatrix(double **matrix, int width, int height, QString fname);  // saves a float matrix with given filename
        bool saveMatrix(bool **matrix, int width, int height, QString fname);  // saves a boolean matrix with given filename
        bool saveArray(int *array, int length, QString fname);  // saves a int array with given filename
        bool saveList(QList<int> array, QString fname);
        bool saveList(QList<solidLine *> array, QString fname);
        bool saveList(QList<solidLine> array, QString fname);

        bool saveMinCostedLinesArray(minCostedLines *array, int length, QString fname);
        bool saveMinCostedLinesList(QList<minCostedLines> list, QString fname);

        void detectEdgeSobel();                 // detect edges & construct edge matrix
        void prepareCannyArrays();
        void constructGaussianMatrix(int size, float _stddev);
        void gaussianBlurFixed();    // to reduce noise
        void gaussianBlur();    // to reduce noise
        void detectEdgeSobelwDirections();      // detect edges and edge directios & construct edge matrix and edge dir. matrix
        void nonMaximumSuppression(bool suppress = true);
        void cannyThresholding(bool autoThresh, int loPercent = 20, int hiPercent = 50);
        void edgeTracing();
        void checkContinuity(int inX, int inY, int inDir);
        void assignEdgeMap();
        void mergeEdgeMaps();
        void thickenEdgeMap(int diameter);
        void scoreLineCrossing(bool orientation);   // line detection, in edge map

        void scaleEdgeData(int threshold);      // scales edge data acording to line maximum
        void makeBinaryEdgeMatrix(int threshold);   // converts edge data to binary
        void thickenEdges();                    // thicken edges

        void houghTransformFn(int **matrix, int width, int height);                  // conduct hough transform & construct hough space matrix for edge image size
        void houghTransform();                  // conduct hough transform & construct hough space matrix for edge image size
        void houghTransformEdgeMap();           // conduct hough transform & construct hough space matrix for edge image size
        void houghTransformExtended();          // conduct hough transform & construct hough space matrix for org img size
        void calculateHoughMaxs(int number);    // copy data of <number> of max voted lines to hough lines matrix; Finds Y values
        void codeLineData(int **matrix, int width, int height, QList<houghData> list, bool orientation); // codes line(s) to given matrix
        void constructHoughMatrix();            // construct hough matrix base on edge matrix with max voted lines coded on it
        void constructHoughMatrix2Lines();      // construct hough matrix base on edge matrix with 2 lines coded on it
        void constructHoughMatrixAvgLine();
        void constructHoughMatrixPrimaryLine(int startX, int endX);
        void constructHoughMatrixPrimaryLines(solidLine line1, solidLine line2, int line2offset);
        void constructHoughMatrixMajor2Lines( bool matrixFlag = false );
        void constructHoughExtendedMatrixMajor2Lines();
        void constructHoughMatrixFindX();       // construct hough matrix base on edge matrix with max voted lines coded on it; Finds X values - for perpendicular lines
        int calcVoteAvg();                      // calc. vote ave. of max. voted lines
        int calcAngleAvg();                     // calc. vote ave. angle max. voted lines wrt center (-90)
        void calcAvgDistAndAngle(int limit);    // calc. ave. distance anf angle of <no> hough lines ; eg houghLineNo
        void calcAvgDistAndAngleOfMajors(float multiplier);     // seperate hough lines into 2 piece; hi and lo, then find the ave. distance and angle of these majors
        void findSecondLine();                  // find second line beginning index in hough lines
        bool checkPrimaryLine();                // check if primary line found is above <voteThreshold>
        void detectVoidLines();                 // detect void lines on max. voted lines imposed on mono image
        void detectVoidLinesEdge();             // detect void lines on max. voted lines imposed on edge image

        void detectPrimaryVoid();               // detect primay void line using <voidThreshold>,
                                                // finds index of primary void in <voidSpace>, calculates center point of corners
                                                // LASER LINE MUST BE ALIGN TO WHOLE IMAGE AND VOID SPACE IN INTEREST
                                                // MUST BE BIGGEST VOID SPACE THROUGHOUT THE LINE

        solidLine detectLongestSolidLine(float distance, float angle, bool flag, int xStartOffset, int xEndOffset);  // detect longest solid(continuous) line from hough space via given distance angle&distance, flag: false->edgeThickened, true->value
        solidLine detectLongestSolidLineVert(float distance, float angle, int yStartOffset, int yEndOffset);  // detect longest solid(continuous) line (vertical) from hough space via given distance angle&distance
        void detectLongestSolidLines(bool averaging = true, bool matrixFlag = true);    // detect longest solid(continuous) lines from value(true) or edge(false) matrix

        void detectThinJointCenter(int refAngle, int precisionSize);

        // CONTRAST ALGO
        void constructContrastMatix(float multiplier);
        void houghTransformContrast();          // conduct hough transform & construct hough space matrix for org img size
        void constructContrastMatrixMajor2Lines();
        void detectContrastCenter();
        QImage getImageContrast();

        // EDGE DETECTION ALGO
        void detectMainEdges(bool thinjoint, bool debug = false);
        houghData detectMainEdgesSolidLine(float rate, bool thinjoint, bool debug = false);

        void histogramAnalysis(bool colored);

        // HORIZONTAL LINE SCAN WITH MAX VOTES
        void detectScanHorizontal(int y);

        bool sortHoughLines_toDistance(int _size);

        // produces image from matrix. if special code is included in, draws pixels with RED
        QImage* getImage(int **matrix, int width, int height, QImage::Format format = QImage::Format_RGB32);

        QImage* getImage(bool **matrix, int width, int height, QImage::Format format = QImage::Format_RGB32);

        int getLineY(int x, float distance, float theta);   // get hough line Y coor from X coor
        int getLineX(int y, float distance, float theta);   // get hough line X coor from Y coor
        int* edgeSobelHistogram();                          // produce edge matrix Y histogram accor. X values
        int* valueHistogram(bool axis = false);              // produce value matrix histogram, false: along X, true: along Y
        int* valueHistogramGray(bool axis = false);              // produce value matrix histogram, false: along X, true: along Y

        void findMedianValue();

        QImage cornerImage( bool matrixFlag = true );         // produce detected corner image based on org. image
        QImage cornerAndPrimaryLineImage(solidLine line1, solidLine line2, int line2offset, bool matrixFlag = true );    // produce detected corner and primary lines image based on org. image
        QImage drawSolidLines( QList<solidLine> lineList );
        QImage* drawSolidLines2EdgeMatrix( solidLine line, QImage::Format format );
        QImage drawLine2OrginalImage( solidLine line, QImage::Format format );
        QImage drawLines();
        QImage drawLines(minCostedLines *lineArray, int size);
        QImage drawLine(minCostedLines *line, float tangent);
        QImage* getImage_cannyThresholds(QImage::Format format);
        QImage* getImage_cannyTracedEdges(QImage::Format format);
        QImage getImageMainEdges( int number, bool matrixFlag = true );
        QImage getImageMainEdges_2ndList( bool enableCenter, bool matrixFlag = true );


        double membershipFn(double in, double k);
        double entropyFn(double in);
        double calcEntropyMatrix(int windowSize);
        void findMaxs(int *array, int array_size, QList<range> &list);
        void findMins(int *array, int array_size, QList<range> &list);

        ~imgProcess();                                      // destructor
};



#endif // IMGPROCESS_H
