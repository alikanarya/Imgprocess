#ifndef IMGPROCESS_H
#define IMGPROCESS_H

#include "imgprocess_msg.h"
#include <QImage>
#include <QColor>

#define PI 3.14159265
#define R2D PI/180          // convert radyant to degree

const int sobelX[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
const int sobelY[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};

// class for the void(empty) line data
class voidLine{

public:
        QPoint start;
        QPoint end;
        int length;

        voidLine(){}
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
};


// class for major area indexes
class majorArea{

public:
    int startIndex;
    int endIndex;

    majorArea(){}
};


// image processing class
class imgProcess{

    public:
        QImage imgOrginal;      // orginal image
        QImage imgMono;         // mono image
        QImage imgCorner;       // otginal image with detected corners shown

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
        int houghVoteAvg;                   // ave vote value of max. voted lines
        int voteThreshold;                  // vote threshold to accept primary line as concerned
        int voidThreshold;                  // void line length threhold in pixels to accept primary void line as concerned
        float distanceAvg, thetaAvg;
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
        QList<solidLine> solidSpaceMainTrimmed; // solidSpaceMain: no lines (length -1) excluded
        QList<solidLine> solidSpaceMainMaximums;// solidSpaceTrimmed: maximums of each distance value
        QList<solidLine> solidSpaceMainOrdered; // line length ordered list of solidSpaceMain
        QList<majorArea *> majorList;
        float majorThresholdPercent;
        int maxSolidLineLength;
        QList<solidLine> majorLines;    // list that hold major lines data from major areas
        QList<solidLine> major2Lines;   // list that hold 2 major lines
        bool majorLinesFound;
        solidLine primaryLine;

        int errorEdgeLimit;
        int errorAngleLimit;

        int **valueMatrix;                  // image data matrix
        int **edgeMatrix;                   // edge image data matrix
        int **edgeThickenedMatrix;          // thickened edge image data matrix
        int **houghMatrix;                  // hough image data matrix with max. voted lines coded

        int **houghSpace;                   // line votes: line search matrix, depends max. distance & angle scale
        float **houghLines;                 // line data of max. voted lines; distance/angle/vote value
        bool houghLinesInitSwitch;          // to delete in destructor

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

            edgeThickenedMatrix = new int*[edgeHeight];
            for (int i = 0; i < edgeHeight; i++)   edgeThickenedMatrix[i] = new int[edgeWidth];

            houghMatrix = new int*[edgeHeight];
            for (int i = 0; i < edgeHeight; i++) houghMatrix[i] = new int[edgeWidth];

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

            houghLinesInitSwitch = false;
            histogramInitSwitch = false;
        }

        void toMono();      // produce mono image
        void constructValueMatrix(QImage image);    // construct pixel value matrix of an image according to single color value
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
        void thickenEdges();                    // thicken edges
        void houghTransform();                  // conduct hough transform & construct hough space matrix
        void calculateHoughMaxs(int number);    // copy data of <number> of max voted lines to hough lines matrix
        void constructHoughMatrix();            // construct hough matrix base on edge matrix with max voted lines coded on it
        void constructHoughMatrix2Lines();      // construct hough matrix base on edge matrix with 2 lines coded on it
        void constructHoughMatrixAvgLine();
        void constructHoughMatrixPrimaryLine(int startX, int endX);
        void constructHoughMatrixPrimaryLines(solidLine line1, solidLine line2, int line2offset);
        void constructHoughMatrixMajor2Lines();
        int calcVoteAvg();                      // calc. vote ave. of max. voted lines
        int calcAngleAvg();                     // calc. vote ave. angle max. voted lines wrt center (-90)
        void calcAvgDistAndAngle(int limit);    // calc. ave. distance anf angle of <no> hough lines ; eg houghLineNo
        void calcAvgDistAndAngleOfMajors();     // seperate hough lines into 2 piece; hi and lo, then find the ave. distance and angle of these majors
        void findSecondLine();                  // find second line beginning index in hough lines
        bool checkPrimaryLine();                // check if primary line found is above <voteThreshold>
        void detectVoidLines();                 // detect void lines on max. voted lines imposed on mono image
        void detectVoidLinesEdge();             // detect void lines on max. voted lines imposed on edge image

        void detectPrimaryVoid();               // detect primay void line using <voidThreshold>,
                                                // finds index of primary void in <voidSpace>, calculates center point of corners
                                                // LASER LINE MUST BE ALIGN TO WHOLE IMAGE AND VOID SPACE IN INTEREST
                                                // MUST BE BIGGEST VOID SPACE THROUGHOUT THE LINE

        solidLine detectLongestSolidLine(float distance, float angle, bool flag);  // detect longest solid(continuous) line from hough space via given distance angle&distance, flag: false->edgeThickened, true->value
        void detectLongestSolidLines();         // detect longest solid(continuous) lines from hough-lines array

        // produces image from matrix. if hough line code is included in, dras lines with RED
        QImage* getImage(int **matrix, int width, int height, QImage::Format format = QImage::Format_RGB32);

        int getLineY(int x, float distance, float theta);   // get hough line Y coor from X coor
        int* edgeSobelHistogram();                          // produce edge matrix Y histogram accor. X values
        int* valueHistogram();                              // produce value matrix Y histogram accor. X values
        QImage cornerImage();                               // produce detected corner image based on org. image

        ~imgProcess();                                      // destructor
};
#endif // IMGPROCESS_H
