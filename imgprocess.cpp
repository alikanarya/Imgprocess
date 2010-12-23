#include "imgprocess.h"
//#include <QImage>
//#include <QColor>
#include <QFile>
#include <QTextStream>
#include "math.h"

#include "imgprocess_msg.h"

void imgProcess::toMono(){
    imgMono = imgOrginal.convertToFormat(QImage::Format_Mono,Qt::ThresholdDither);
}

void imgProcess::constructValueMatrix(QImage image){
    QRgb rgbValue;
    QColor *color;
    int colorValue;

    for (int y = 0; y < image.height(); y++)
        for (int x = 0; x < image.width(); x++){
            rgbValue = image.pixel(x,y);
            color = new QColor(rgbValue);
            colorValue = color->value();

            if ( colorValue > 255) colorValue = 255;
            else if (colorValue < 0)  colorValue = 0;

            valueMatrix[y][x] = colorValue;
            delete color;
        }
}

int imgProcess::getMatrixPoint(int *matrix, int width, int x, int y){
    int (*ptr)[width] = (int (*)[width])matrix;
    return (int)ptr[y][x];
}

bool imgProcess::saveMatrix(int **matrix, int width, int height, QString fname, int threshold, bool xSwitch){
    QFile file(fname);
    bool saveStatus = true;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);

        for(int y = 0; y < height; y++){
            for(int x = 0; x < width; x++){
//                if (matrix[y][x]>threshold) out << "X";
//                else
                    if (!xSwitch)
                        out << matrix[y][x];
                    else
                        out << "_";
                if (x != (width-1)) out << ",";
            }
            out << "\n";
        }
        file.close();
    } else saveStatus = false;

    return saveStatus;
}

bool imgProcess::saveMatrix(float **matrix, int width, int height, QString fname){
    QFile file(fname);
    bool saveStatus = true;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);

        for(int y = 0; y < height; y++){
            for(int x = 0; x < width; x++){
                out << matrix[y][x];
                if (x != (width - 1)) out << ",";
            }
            out << "\n";
        }
        file.close();
    } else saveStatus = false;
    return saveStatus;
}

bool imgProcess::saveArray(int *array, int length, QString fname){
    QFile file(fname);
    bool saveStatus = true;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);

        for(int i = 0; i < length; i++) out << array[i] << "\n";
        file.close();
    } else saveStatus = false;

    return saveStatus;
}

void imgProcess::detectEdgeSobel(){

    int G, Gx, Gy;
    for (int y = 1;y < imageHeight - 1; y++)
        for (int x = 1; x < imageWidth - 1; x++){
            Gx =    sobelX[0][0]*valueMatrix[y-1][x-1] + sobelX[0][1]*valueMatrix[y-1][x] + sobelX[0][2]*valueMatrix[y-1][x+1] +
                    sobelX[1][0]*valueMatrix[y][x-1]   + sobelX[1][1]*valueMatrix[y][x]   + sobelX[1][2]*valueMatrix[y][x+1] +
                    sobelX[2][0]*valueMatrix[y+1][x-1] + sobelX[2][1]*valueMatrix[y+1][x] + sobelX[2][2]*valueMatrix[y+1][x+1];

            Gy =    sobelY[0][0]*valueMatrix[y-1][x-1] + sobelY[0][1]*valueMatrix[y-1][x] + sobelY[0][2]*valueMatrix[y-1][x+1] +
                    sobelY[1][0]*valueMatrix[y][x-1]   + sobelY[1][1]*valueMatrix[y][x]   + sobelY[1][2]*valueMatrix[y][x+1] +
                    sobelY[2][0]*valueMatrix[y+1][x-1] + sobelY[2][1]*valueMatrix[y+1][x] + sobelY[2][2]*valueMatrix[y+1][x+1];

            G = (int)(sqrt(pow(Gx, 2) + pow(Gy, 2)));

            if (G > 255) G = 255;
            else if (G < 0) G = 0;
            edgeMatrix[y-1][x-1] = G;
        }
}

void imgProcess::houghTransform(){
    houghDistanceMax = (int) (sqrt(pow(edgeWidth, 2) + pow(edgeHeight, 2)));
    centerX = edgeWidth / 2;
    centerY = edgeHeight - 1;

    houghThetaSize = (int) ((thetaMax - thetaMin) / thetaStep) + 1;

    houghSpace = new int*[houghDistanceMax];
    for (int i = 0; i < houghDistanceMax; i++)   houghSpace[i] = new int[houghThetaSize];

    for (int y = 0; y < houghDistanceMax; y++)
        for (int x = 0; x < houghThetaSize; x++) houghSpace[y][x] = 0;

    int distance, theta;
    for (int y = 0; y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            if (edgeMatrix[y][x] != 0)
                for (int i = 0; i < houghThetaSize; i++){
                    theta = thetaMin + i * thetaStep;
                    distance = (int) ((x - centerX) * cos(theta * R2D) + (centerY - y) * sin(theta * R2D));
                    if (distance >= 0) houghSpace[distance][i]++;
                }
}

void imgProcess::calculateHoughMaxs(int number){
    houghLineNo = number;

    houghLines = new float*[houghLineNo];
    for (int i = 0; i < houghLineNo; i++)   houghLines[i] = new float[3];

    int max, maxDistance, maxThetaIndex;
    for (int line = 0; line < houghLineNo; line++){
        max = 0;
        maxDistance = 0;
        maxThetaIndex = 0;

        for (int distance = 0; distance < houghDistanceMax; distance++)
            for (int i = 0; i < houghThetaSize; i++)
                if (houghSpace[distance][i] > max){
                    max = houghSpace[distance][i];
                    maxDistance = distance;
                    maxThetaIndex = i;
                }

        houghLines[line][0] = maxDistance;
        houghLines[line][1] = thetaMin + maxThetaIndex * thetaStep;
        houghLines[line][2] = max;

        houghSpace[maxDistance][maxThetaIndex] = 0;
        // re construct hough space - necessary?
    }
}

void imgProcess::constructHoughMatrix(){

    int lineY;

    for (int y = 0; y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            houghMatrix[y][x] = edgeMatrix[y][x];

    for (int i = 0; i< houghLineNo; i++)
        for (int x = 0; x < edgeWidth; x++){
            lineY = centerY - getLineY((x-centerX), houghLines[i][0], houghLines[i][1]);

            if (lineY >= 0 && lineY < edgeHeight)
                if (houghMatrix[lineY][x] == 0) houghMatrix[lineY][x] = 2555;       // 2555 special code to differeciate line data, arbitrary
        }
}

int imgProcess::calcVoteAvg(){
    houghVoteAvg = 0;
    for (int line = 0; line < houghLineNo; line++) houghVoteAvg += houghLines[line][2];

    houghVoteAvg = houghVoteAvg / houghLineNo;
    return houghVoteAvg;
}

int imgProcess::calcAngleAvg(){
    angleAvg = 0;

    if (primaryLineDetected) {
        for (int line = 0; line < houghLineNo; line++) angleAvg += houghLines[line][1];
        angleAvg = (angleAvg / houghLineNo) - 90;
    }

    return angleAvg;
}

bool imgProcess::checkPrimaryLine(){
    if (houghLines[0][2] >= voteThreshold)
        primaryLineDetected = true;
    else primaryLineDetected = false;
    return primaryLineDetected;
}

void imgProcess::detectVoidLines(){
    if (primaryLineDetected){
        float distanceAvg = 0, thetaAvg = 0;
        for (int line = 0; line < houghLineNo; line++){
            distanceAvg += houghLines[line][0];
            thetaAvg += houghLines[line][1];
        }
        distanceAvg = distanceAvg / houghLineNo;
        thetaAvg = thetaAvg / houghLineNo;

        voidSpace.clear();
        int lineY = 0, voidCount = 0, fullX = 0, fullY = 0;
        int prevValue = 255, currentValue = 0, state = 0;
        voidLine *line;

        for (int x = 0; x < imageWidth; x++){
            lineY = centerY - getLineY((x-centerX), distanceAvg, thetaAvg);

            if (lineY >= 0 && lineY < imageHeight){
                currentValue = valueMatrix[lineY][x];

                if (prevValue == 0 && currentValue == 255){
                    state = 1;  // void to full
                } else
                if (prevValue == 255 && currentValue == 0){
                    state = 0;  // full to void
                } else
                if (prevValue == 0 && currentValue == 0){
                    state = 3;  // void unchanged
                } else
                if (prevValue == 255 && currentValue == 255){
                    state = 4;  // full unchanged
                } else
                    state = 5;  // not recognized

                if (state == 0){    // if void line is started get coor. with prev. coor (last full point coor)
                    line = new voidLine();
                    line->start.setX(fullX);
                    line->start.setY(fullY);
                    voidCount++;
                }

                if (state == 3){    // calc. void line length
                    voidCount++;
                }

                if (state == 1){    // if void line is end get coor. of end point & append void line data to list
                    line->end.setX(x);
                    line->end.setY(lineY);
                    line->length = voidCount;
                    voidSpace.append(line);
                    voidCount = 0;
                }

                if (currentValue == 255){   // to get last full point coor immediately before void line start
                    fullX = x;
                    fullY = lineY;
                }

                prevValue = currentValue;
            }
        }
    }
}

// DETECTION FUNCTION
void imgProcess::detectPrimaryVoid(){
    cornersDetected = true;
    detected = true;

    if (!primaryLineDetected){
        cornersDetected = false;
        detected = false;
        statusMessage = alarm1;
    }
    else if (voidSpace.size() == 0) {
        cornersDetected = false;
        detected = false;
        statusMessage = alarm2;
    }
    else {
        int max = 0;
        voidIndex = 0;
        for (int i=0; i<voidSpace.size(); i++)
            if (voidSpace[i]->length > max){
                max = voidSpace[i]->length;
                voidIndex = i;
            }

        trackCenterX = abs((voidSpace[voidIndex]->end.x() + voidSpace[voidIndex]->start.x())) / 2;
        trackCenterY = abs((voidSpace[voidIndex]->end.y() + voidSpace[voidIndex]->start.y())) / 2;

        if (max < voidThreshold){
            cornersDetected = false;
            detected = false;
            statusMessage = alarm3;
        }
        else if (voidSpace[voidIndex]->start.x() < errorEdgeLimit || voidSpace[voidIndex]->start.x() > (imageWidth - errorEdgeLimit)){
            cornersDetected = false;
            detected = false;
            statusMessage = alarm4;
        }
        else {
            if (abs(angleAvg) <= errorAngleLimit)
                angleInLimit = true;
            else {
                angleInLimit = false;
                detected = false;       // do not accecpt this cam setup
                statusMessage = alarm5;
            }
        }
    }
}

QImage* imgProcess::getImage(int **matrix, int width, int height, QImage::Format format){
    QImage *image = new QImage(width, height, format);
    QRgb value;

    for(int y = 0; y < height; y++)
        for(int x = 0; x < width; x++){
            if (matrix[y][x] == 2555)
                value = qRgb(255, 0, 0);    // red for hough lines
            else
                value = qRgb(matrix[y][x], matrix[y][x], matrix[y][x]);
            image->setPixel(x, y, value);
        }
    return image;
}

int imgProcess::getLineY(int x, float distance, float theta){
    int y = -1;
    if (theta >= 0.1)
        y = (int) ( (distance - x*cos(theta*R2D)) / sin(theta*R2D) );
    return y;
}

int* imgProcess::edgeSobelHistogram(){
    histogram = new int[edgeHeight];

    int sum;

    for(int y = 0; y < edgeHeight; y++){
        sum = 0;
        for(int x = 0; x < edgeWidth; x++) sum += edgeMatrix[y][x];
        histogram[y] = sum / edgeWidth;
    }
    return histogram;
}

int* imgProcess::valueHistogram(){
    histogram = new int[imageHeight];

    int sum;

    for(int y = 0; y < imageHeight; y++){
        sum = 0;
        for(int x = 0; x < imageWidth; x++) sum += valueMatrix[y][x];
        histogram[y] = sum / imageWidth;
    }
    return histogram;
}

QImage imgProcess::cornerImage(){
    imgCorner = imgOrginal.copy();
    if (cornersDetected){
        QRgb value;
        value = qRgb(0, 255, 0);        // green
        for (int x = -4; x <= 4; x++){
            imgCorner.setPixel(voidSpace[voidIndex]->start.x()+x, voidSpace[voidIndex]->start.y(), value);
            imgCorner.setPixel(voidSpace[voidIndex]->end.x()+x, voidSpace[voidIndex]->end.y(), value);
        }
        for (int y = -4; y <= 4; y++){
            imgCorner.setPixel(voidSpace[voidIndex]->start.x(), voidSpace[voidIndex]->start.y()+y, value);
            imgCorner.setPixel(voidSpace[voidIndex]->end.x(), voidSpace[voidIndex]->end.y()+y, value);
            imgCorner.setPixel(trackCenterX, trackCenterY+y, value);
        }

    }

    return imgCorner;
}


imgProcess::~imgProcess(){
    for (int y = 0; y < imageHeight; y++) delete []valueMatrix[y];
    delete []valueMatrix;

    for (int y = 0; y < edgeHeight; y++) delete []edgeMatrix[y];
    delete []edgeMatrix;

    for (int y = 0; y < edgeHeight; y++) delete []houghMatrix[y];
    delete []houghMatrix;

    for (int y = 0; y < houghDistanceMax; y++) delete []houghSpace[y];
    delete []houghSpace;

    for (int y = 0; y < houghLineNo; y++) delete []houghLines[y];
    delete []houghLines;

    delete []histogram;
}
