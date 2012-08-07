#ifndef IMGPROCESS_H
#define IMGPROCESS_H

#include "imgprocess_msg.h"
//#include "../_Modules/Algo/localMinimum.h"

#include <QImage>
#include <QColor>

#define PI 3.14159265
#define R2D PI/180          // convert radyant to degree

const int sobelX[3][3] = { {-1,0,1}, {-2,0,2}, {-1,0,1} };
const int sobelY[3][3] = { {1,2,1}, {0,0,0}, {-1,-2,-1} };
const int gaussianMask[5][5] = { {2,4,5,4,2}, {4,9,12,9,4}, {5,12,15,12,5}, {4,9,12,9,4}, {2,4,5,4,2} };
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
        QImage imgOrginal;      // orginal image
        QImage imgMono;         // mono image
        QImage imgCorner;       // orginal image with detected corners shown
        QImage imgCornerAndPrimaryLines;    // orginal image with detected corners and primary lines shown

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
        int trackCenterX, trackCenterY;     // coor. of center beteen corners
        int leftCornerX, leftCornerY;
        int rightCornerX, rightCornerY;
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
        QList<solidLine> majorLines;    // list that hold major lines data from major areas NOT USED
        QList<solidLine> major2Lines;   // list that hold 2 major lines
        bool majorLinesFound;
        solidLine primaryLine;
        QList<solidLine> primaryGroup;
        QList<solidLine> secondaryGroup;
        bool primaryLineFound;
        bool secondaryLineFound;
        bool centerDetermined;

        // THIN JOINT ALGO
        bool thinJointInitSwitch;
        float *slope;
        QList<minCostedLines> lineList;
        minCostedLines *bestLines;
        float slopeBest;
        QList<minCostedLines> deepLines;
        int centerC;
        QList<int> peakPoints;

        // CONTRAST ALGO
        int **contrastMatrix;               // contrast transition points
        bool contrastInitSwitch;            // to delete in destructor

        // EDGE DETETION ALGO
        int tHi, tLo;
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

        int listHoughData2ndFilteredSize;
        int **listHoughData2ndFilteredArray;
        bool listHoughData2ndFilteredArrayInitSwitch;

        int neighbourhood;


        int errorEdgeLimit;
        int errorAngleLimit;

        int **valueMatrix;                  // image data matrix
        int **edgeMatrix;                   // edge image data matrix
        int **edgeGradientMatrix;           // edge gradient data matrix
        bool edgeGradientMatrixInitSwitch;
        int **edgeSuppressedMatrix;
        bool edgeSuppressedMatrixInitSwitch;
        int **edgeStrongMatrix;
        bool edgeStrongMatrixInitSwitch;
        int **edgeWeakMatrix;
        bool edgeWeakMatrixInitSwitch;
        bool **edgeMapMatrix;
        bool edgeMapMatrixInitSwitch;
        bool **edgeVisitMatrix;
        bool edgeVisitMatrixInitSwitch;
        bool **edgeW2SMapMatrix;
        bool edgeW2SMapMatrixInitSwitch;

        int **edgeThickenedMatrix;          // thickened edge image data matrix
        int **houghMatrix;                  // hough image data matrix with max. voted lines coded, edge image size
        int **houghExtendedMatrix;          // hough image data matrix with max. voted lines coded, org. image size
        int **houghSpace;                   // line votes: line search matrix, depends max. distance & angle scale
        bool houghSpaceInitSwitch;          // to delete in destructor

        float **houghLines;                 // line data of max. voted lines; distance/angle/vote value
        bool houghLinesInitSwitch;          // to delete in destructor

        float **houghLinesSorted;           // sort houghLines wrt distance
        bool houghLinesSortedInitSwitch;    // to delete in destructor

        int *histogram;                     // histogram array
        bool histogramInitSwitch;           // to delete in destructor

        // user information
        QString statusMessage;              // general message var about processing
        int angleAvg;                       // average angle degree wrt center point (-90)
        bool angleInLimit;                  // true if <avgAngle> is within +/- 3 degree


        // constructor
        imgProcess(QImage &image, const int width, const int height) : imageWidth(width), imageHeight(height) {

            imgOrginal = image;     // passes image to class not copy of it
            valueMatrix = new int*[height];
            for (int i = 0; i < height; i++) valueMatrix[i] = new int[width];

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
            listHoughData2ndFilteredArrayInitSwitch = false;
            edgeStrongMatrixInitSwitch = false;
            edgeWeakMatrixInitSwitch = false;
            edgeVisitMatrixInitSwitch = false;

            // no solid line
            primaryLine.start.setX( -1 );
            primaryLine.start.setY( -1 );
            primaryLine.end.setX( -1 );
            primaryLine.end.setY( -1 );
            primaryLine.length = -1;
            primaryLine.distance = -1;
            primaryLine.angle = -1;

            tHi = 80;
            tLo = 20;
            neighbourhood = 5;
        }

        void toMono();      // produce mono image
        void constructValueMatrix(QImage image);    // construct pixel value matrix of an image according to single color value
        void gaussianBlur();    // to reduce noise
        int getMatrixPoint(int *matrix, int width, int x, int y);   // returns value of a matrix

        // saves a int matrix with given filename
        // saving with X and _ pointers (not used)
        bool saveMatrix(int **matrix, int width, int height, QString fname, int threshold = 255, bool xSwitch = false);

        bool saveMatrix(float **matrix, int width, int height, QString fname);  // saves a float matrix with given filename
        bool saveArray(int *array, int length, QString fname);  // saves a int array with given filename
        bool saveList(QList<int> array, QString fname);
        bool saveList(QList<solidLine *> array, QString fname);
        bool saveList(QList<solidLine> array, QString fname);

        void detectEdgeSobel();                 // detect edges & construct edge matrix
        void detectEdgeSobelwDirections();      // detect edges and edge directios & construct edge matrix and edge dir. matrix
        void nonMaximumSuppression();
        void cannyThresholding(int lo, int hi);
        void edgeTracing();
        void checkContinuity(int inX, int inY, int inDir);

        void scaleEdgeData(int threshold);      // scales edge data acording to line maximum
        void makeBinaryEdgeMatrix(int threshold);   // converts edge data to binary
        void thickenEdges();                    // thicken edges

        void houghTransform();                  // conduct hough transform & construct hough space matrix for edge image size
        void houghTransformEdgeMap();                  // conduct hough transform & construct hough space matrix for edge image size
        void houghTransformExtended();          // conduct hough transform & construct hough space matrix for org img size
        void calculateHoughMaxs(int number);    // copy data of <number> of max voted lines to hough lines matrix
        void codeLineData(int **matrix, int width, int height, QList<houghData> list, bool orientation); // codes line(s) to given matrix
        void constructHoughMatrix();            // construct hough matrix base on edge matrix with max voted lines coded on it
        void constructHoughMatrix2Lines();      // construct hough matrix base on edge matrix with 2 lines coded on it
        void constructHoughMatrixAvgLine();
        void constructHoughMatrixPrimaryLine(int startX, int endX);
        void constructHoughMatrixPrimaryLines(solidLine line1, solidLine line2, int line2offset);
        void constructHoughMatrixMajor2Lines();
        void constructHoughExtendedMatrixMajor2Lines();
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
        void detectLongestSolidLines();         // detect longest solid(continuous) lines from value matrix

        void detectThinJointCenter(int refAngle, int precisionSize);

        // CONTRAST ALGO
        void constructContrastMatix(float multiplier);
        void houghTransformContrast();          // conduct hough transform & construct hough space matrix for org img size
        void constructContrastMatrixMajor2Lines();
        void detectContrastCenter();

        // EDGE DETECTION ALGO
        void detectMainEdges();

        bool sortHoughLines_toDistance(int _size);

        // produces image from matrix. if special code is included in, draws pixels with RED
        QImage* getImage(int **matrix, int width, int height, QImage::Format format = QImage::Format_RGB32);

        QImage* getImage(bool **matrix, int width, int height, QImage::Format format = QImage::Format_RGB32);

        int getLineY(int x, float distance, float theta);   // get hough line Y coor from X coor
        int getLineX(int y, float distance, float theta);   // get hough line X coor from Y coor
        int* edgeSobelHistogram();                          // produce edge matrix Y histogram accor. X values
        int* valueHistogram();                              // produce value matrix Y histogram accor. X values
        QImage cornerImage();                               // produce detected corner image based on org. image
        QImage cornerAndPrimaryLineImage(solidLine line1, solidLine line2, int line2offset);    // produce detected corner and primary lines image based on org. image
        QImage drawLines(minCostedLines *lineArray, int size);
        QImage drawLine(minCostedLines *line, float tangent);
        QImage* getImage_cannyThresholds(QImage::Format format);
        QImage* getImage_cannyTracedEdges(QImage::Format format);

        ~imgProcess();                                      // destructor
};
#endif // IMGPROCESS_H
