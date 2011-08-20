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

bool imgProcess::saveList(QList<int> array, QString fname){

    QFile file(fname);
    bool saveStatus = true;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);

        for(int i = 0; i < array.size(); i++) out << array[i] << "\n";
        file.close();
    } else saveStatus = false;

    return saveStatus;
}

bool imgProcess::saveList(QList<solidLine *> array, QString fname){

    QFile file(fname);
    bool saveStatus = true;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);

        out << "startX startY endX endY length\n";

        for(int i = 0; i < array.size(); i++)
            out << array[i]->start.x() << " " << array[i]->start.y() << " " << array[i]->end.x() << " " << array[i]->end.y() << " " << array[i]->length << "\n";
        file.close();
    } else saveStatus = false;

    return saveStatus;
}

bool imgProcess::saveList(QList<solidLine> array, QString fname){

    QFile file(fname);
    bool saveStatus = true;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);

        out << "distance angle startX startY endX endY length\n";

        for(int i = 0; i < array.size(); i++)
            out << array[i].distance << " " << array[i].angle << " " << array[i].start.x() << " " << array[i].start.y() << " " << array[i].end.x() << " " << array[i].end.y() << " " << array[i].length << "\n";
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

void imgProcess::thickenEdges(){

    int xnStart = -1, xnEnd = 1, ynStart = -1, ynEnd = 1;

    for (int y = 0; y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++){

            if (y == 0) {
                ynStart = 0; ynEnd = 1;
            } else
            if (y == (edgeHeight - 1)) {
                ynStart = -1; ynEnd = 0;
            } else {
                ynStart = -1; ynEnd = 1;
            }

            if (x == 0) {
                xnStart = 0; xnEnd = 1;
            } else
            if (x == (edgeWidth - 1)) {
                xnStart = -1; xnEnd = 0;
            } else {
                xnStart = -1; xnEnd = 1;
            }

            if ( edgeMatrix[y][x] == 255 )
                for (int xn = xnStart; xn <= xnEnd; xn++)
                    for (int yn = ynStart; yn <= ynEnd; yn++)
                       edgeThickenedMatrix[y + yn][x + xn] = 255;
            else
                edgeThickenedMatrix[y][x] = 0;
        }
}

void imgProcess::houghTransform(){
    houghDistanceMax = (int) (sqrt(pow(edgeWidth, 2) + pow(edgeHeight, 2)));
    //centerX = edgeWidth / 2;
    //centerY = edgeHeight - 1;

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
    houghLinesInitSwitch = true;

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

        houghLines[line][0] = maxDistance;                              // distance
        houghLines[line][1] = thetaMin + maxThetaIndex * thetaStep;     // angle
        houghLines[line][2] = max;                                      // vote value
/*
        if (line == 0)                                                  // derivative
            houghLines[line][3] = 0;
        else
            houghLines[line][3] = houghLines[line][0] - houghLines[line - 1][0];
*/
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

void imgProcess::constructHoughMatrix2Lines(){

    int lineY;

    for (int y = 0; y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            houghMatrix[y][x] = edgeMatrix[y][x];

    for (int x = 0; x < edgeWidth; x++){
        lineY = centerY - getLineY((x-centerX), houghLines[0][0], houghLines[0][1]);

        if (lineY >= 0 && lineY < edgeHeight)
            if (houghMatrix[lineY][x] == 0) houghMatrix[lineY][x] = 2555;       // 2555 special code to differeciate line data, arbitrary

        lineY = centerY - getLineY((x-centerX),houghLines[secondLineIndex][0], houghLines[secondLineIndex][1]);

        if (lineY >= 0 && lineY < edgeHeight)
            if (houghMatrix[lineY][x] == 0) houghMatrix[lineY][x] = 2555;       // 2555 special code to differeciate line data, arbitrary
    }
}

void imgProcess::constructHoughMatrixAvgLine(){

    int lineY;

    for (int y = 0; y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            houghMatrix[y][x] = edgeMatrix[y][x];

    for (int x = 0; x < edgeWidth; x++){
        lineY = centerY - getLineY((x-centerX), distanceAvg, thetaAvg);

        if (lineY >= 0 && lineY < edgeHeight)
            if (houghMatrix[lineY][x] == 0) houghMatrix[lineY][x] = 2555;       // 2555 special code to differeciate line data, arbitrary
    }
}

void imgProcess::constructHoughMatrixPrimaryLine(int startX, int endX){

    int lineY;

    for (int y = 0; y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            houghMatrix[y][x] = edgeMatrix[y][x];

    for (int x = startX; x <= endX; x++){
        lineY = centerY - getLineY((x - centerX), distanceAvg, thetaAvg);

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

void imgProcess::calcAvgDistAndAngle(int limit){
    distanceAvg = 0;
    thetaAvg = 0;

    for (int line = 0; line < limit; line++){
        distanceAvg += houghLines[line][0];
        thetaAvg += houghLines[line][1];
    }
    distanceAvg = distanceAvg / limit;
    thetaAvg = thetaAvg / limit;
}

// seperate hough lines into 2 piece; higher and lower than ave. distance
// then find the ave. distance and angle of these majors
void imgProcess::calcAvgDistAndAngleOfMajors(){

    int _voteThreshold = (int) (houghLines[0][2] * 0.80);    // %x of first line (most voted)
    int _voteThresholdIndex = houghLineNo - 1;               // last index

    // find the index of less than vote threshold
    for (int line = 1; line < houghLineNo; line++){
        if ( houghLines[line][2] < _voteThreshold){
            _voteThresholdIndex = line - 1;     // last index of higher than threshold
            break;
        }
    }


    // calc. ave. distance of lines in between max voted and higher than threshold
    float _distanceAvg = 0;

    for (int line = 0; line <= _voteThresholdIndex; line++)
        _distanceAvg += houghLines[line][0];

    _distanceAvg = (int) (_distanceAvg / (_voteThresholdIndex + 1));


    // group lines in 2: higher and lower than ave. distance
    lowLinesList.clear();
    highLinesList.clear();

    for (int line = 0; line <= _voteThresholdIndex; line++){
        if (houghLines[line][0] >= _distanceAvg)
            highLinesList.append(line);
        else
            lowLinesList.append(line);
    }


    // calc. ave. distance and angle of higher lines which have same max vote value
    int distanceAvgHigh = 0;
    int thetaAvgHigh = 0;
    int countHigh = 0;

    for (int i = 0; i < highLinesList.size(); i++)
        if (houghLines[ highLinesList[i] ][2] == houghLines[ highLinesList[0] ][2]) {
            countHigh++;
            distanceAvgHigh += houghLines[ highLinesList[i] ][0];
            thetaAvgHigh += houghLines[ highLinesList[i] ][1];
        }

    if (countHigh != 0) {
        distanceAvgHigh = distanceAvgHigh / countHigh;
        thetaAvgHigh = thetaAvgHigh / countHigh;
    }
    /*
    distanceAvg = distanceAvgHigh;
    thetaAvg = thetaAvgHigh;
    constructHoughMatrixAvgLine();
    getImage(houghMatrix, edgeWidth, edgeHeight)->save("_hi.jpg");
    */

    // calc. ave. distance and angle of lower lines which have same max vote value
    int distanceAvgLo = 0;
    int thetaAvgLo = 0;
    int countLo = 0;

    for (int i = 0; i < lowLinesList.size(); i++)
        if (houghLines[ lowLinesList[i] ][2] == houghLines[ lowLinesList[0] ][2]) {
            countLo++;
            distanceAvgLo += houghLines[ lowLinesList[i] ][0];
            thetaAvgLo += houghLines[ lowLinesList[i] ][1];
        }

    if (countLo != 0) {
        distanceAvgLo = distanceAvgLo / countLo;
        thetaAvgLo = thetaAvgLo / countLo;
    } else {    // in case of finding only 1 major (it will be classified in higher profile)
        distanceAvgLo = distanceAvgHigh;
        thetaAvgLo = thetaAvgHigh;
    }

    /*
    distanceAvg = distanceAvgLo;
    thetaAvg = thetaAvgLo;
    constructHoughMatrixAvgLine();
    getImage(houghMatrix, edgeWidth, edgeHeight)->save("_lo.jpg");
    */

    // calc. ave distance and angle of higher and lower majors
    distanceAvg = (distanceAvgHigh + distanceAvgLo ) / 2;
    thetaAvg = (thetaAvgHigh + thetaAvgLo ) / 2;
}


// find second line beginning index in hough lines
void imgProcess::findSecondLine(){

    int _voteThreshold = (int) (houghLines[0][2] * 0.50);   // %x of first line
    int _voteThresholdIndex = houghLineNo - 1;

    // find the index of less than vote threshold
    for (int line = 1; line < houghLineNo; line++){
        if ( houghLines[line][2] < _voteThreshold){
            _voteThresholdIndex = line - 1;
            break;
        }
    }


    // calc. ave. distance of lines in between max voted and higher than threshold
    float _distanceAvg = 0;

    for (int line = 0; line <= _voteThresholdIndex; line++)
        _distanceAvg += houghLines[line][0];

    _distanceAvg = (int) (_distanceAvg / (_voteThresholdIndex + 1));


    // determine (1st major) first line (max voted) is higher or lower than ave. distance
    bool isPrimaryLowLine = true;

    if (houghLines[0][0] < _distanceAvg)
        isPrimaryLowLine = true;
    else
        isPrimaryLowLine = false;


    // find the 2nd major beginning index.
    // it should has opposite profile
    secondLineIndex = 0;

    for (int line = 1; line <= _voteThresholdIndex; line++){
        if (isPrimaryLowLine) {
            if (houghLines[line][0] > _distanceAvg) {
                secondLineIndex = line;
                break;
            }
        } else {
            if (houghLines[line][0] < _distanceAvg) {
                secondLineIndex = line;
                break;
            }
        }
    }
}

bool imgProcess::checkPrimaryLine(){
    if (houghLines[0][2] >= voteThreshold)
        primaryLineDetected = true;
    else
        primaryLineDetected = false;
    return primaryLineDetected;
}

void imgProcess::detectVoidLines(){
    if (primaryLineDetected){

        voidSpace.clear();
        int lineY = 0, voidCount = 0;
        int fullX = 0;
        int fullY = centerY - getLineY((fullX - centerX), distanceAvg, thetaAvg);;
        int prevValue = 255, currentValue = 0, state = 0;
        voidLine *line;

        for (int x = 0; x < imageWidth + 1; x++){   // +1 for last coordinate
            lineY = centerY - getLineY((x-centerX), distanceAvg, thetaAvg);

            if (lineY >= 0 && lineY < imageHeight){
                if (x == imageWidth)
                    currentValue = 255;
                else
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

void imgProcess::detectVoidLinesEdge(){
    if (primaryLineDetected){

        voidSpace.clear();
        int lineY = 0, voidCount = 0;
        int fullX = 0;
        int fullY = centerY - getLineY((fullX - centerX), distanceAvg, thetaAvg);;
        int prevValue = 255, currentValue = 0, state = 0;
        voidLine *line;

        for (int x = 0; x < edgeWidth + 1; x++){   // +1 for last coordinate
            lineY = centerY - getLineY((x-centerX), distanceAvg, thetaAvg);

            if (lineY >= 0 && lineY < edgeHeight){
                if (x == edgeWidth)
                    currentValue = 255;
                else
                    currentValue = edgeMatrix[lineY][x];

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
    detected = true;

    if (!primaryLineDetected){
        detected = false;
        statusMessage = alarm1;
    }
    else if (voidSpace.size() == 0) {
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
        leftMostCornerX = voidSpace[0]->end.x();
        leftMostCornerY = voidSpace[0]->end.y();

        rightMostCornerX = voidSpace[voidSpace.size()-1]->start.x();
        rightMostCornerY = voidSpace[voidSpace.size()-1]->start.y();

        leftCornerX = voidSpace[voidIndex]->start.x();
        leftCornerY = voidSpace[voidIndex]->start.y();

        rightCornerX = voidSpace[voidIndex]->end.x();
        rightCornerY = voidSpace[voidIndex]->end.y();

        trackCenterX = (leftCornerX + rightCornerX) / 2;
        trackCenterY = (leftCornerY + rightCornerY) / 2;

        if (max < voidThreshold){
            detected = false;
            statusMessage = alarm3;
        }
        else if (voidSpace[voidIndex]->end.x() < errorEdgeLimit || voidSpace[voidIndex]->start.x() > (imageWidth - errorEdgeLimit)){
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

solidLine imgProcess::detectLongestSolidLine(float distance, float angle, bool flag) {

    solidSpace.clear();

    int width, height;

    if (flag){
        width = imageWidth;
        height = imageHeight;
    } else {
        width = edgeWidth;
        height = edgeHeight;
    }

    int lineY = 0, lineLength = 0;
    int prevValue = 0,    // begin with empty
        prevX = 0, prevY = 0,
        currentValue = 0, state = 0;
    solidLine *line;

    for (int x = 0; x < width + 1; x++){   // +1 for last coordinate to catch last line shape

        lineY = centerY - getLineY((x - centerX), distance, angle);

        if (lineY >= 0 && lineY < height){

            if (x == width)
                currentValue = 0;   // end with empty
            else {
                if (flag)
                    currentValue = valueMatrix[lineY][x];
                else
                    currentValue = edgeThickenedMatrix[lineY][x];
            }

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

            if (state == 1){    // if solid line is STARTED get coor. with prev. coor (last full point coor)
                line = new solidLine();
                line->start.setX(x);
                line->start.setY(lineY);
                lineLength++;
            }

            if (state == 4){    // calc. solid line length - SOLID CONTINUES
                lineLength++;
            }

            if (state == 0){    // if solid line is END get coor. of end point & append solid line data to list
                line->end.setX(prevX);
                line->end.setY(prevY);
                line->length = lineLength;
                line->angle = angle;
                line->distance = distance;
                solidSpace.append(line);
                lineLength = 0;
            }

            prevValue = currentValue;
            prevX = x;
            prevY = lineY;
        }
    }


    // find longest line in solid space
    solidLine longestLine;

    // no solid line
    longestLine.start.setX( -1 );
    longestLine.start.setY( -1 );
    longestLine.end.setX( -1 );
    longestLine.end.setY( -1 );
    longestLine.length = -1;
    longestLine.distance = distance;
    longestLine.angle = angle;

    int index = 0, maxLength = 0;

    if (solidSpace.size() != 0){

        for (int i = 0; i < solidSpace.size(); i++){
            if (solidSpace[i]->length > maxLength){
                index = i;
                maxLength = solidSpace[i]->length;
            }
        }

        if (maxLength != 0){
            longestLine.start.setX( solidSpace[index]->start.x() );
            longestLine.start.setY( solidSpace[index]->start.y() );
            longestLine.end.setX( solidSpace[index]->end.x() );
            longestLine.end.setY( solidSpace[index]->end.y() );
            longestLine.length = solidSpace[index]->length;
        }
    }

    return longestLine;
}

void imgProcess::detectLongestSolidLines(){

    solidSpaceMain.clear();

    float angle = 0;

    for (int distance = 0; distance < houghDistanceMax; distance++)
        for (int angleIndex = 0; angleIndex < houghThetaSize; angleIndex++){
            angle = thetaMin + angleIndex * thetaStep;
            solidSpaceMain.append( detectLongestSolidLine( distance, angle, false ) );
        }


    // remove no lines

    solidSpaceMainTrimmed.clear();

    for (int i = 0; i < solidSpaceMain.size(); i++)
        if ( solidSpaceMain[i].length != -1 )
            solidSpaceMainTrimmed.append( solidSpaceMain[i] );



    // take maximums of each distance value

    int size = solidSpaceMainTrimmed.size();

    if ( size != 0){

        solidSpaceMainMaximums.clear();

        int currentDistance = solidSpaceMainTrimmed[0].distance;
        int currentStart = 0, index;
        bool loopEnd = true;
        QList<solidLine> buffer;    // equal distance list
        QList<int> bufferEquals;    // eqaul maximums indexes

        while (loopEnd){

            buffer.clear();

            // get same distances
            for (index = currentStart; index < size; index++){
                if (solidSpaceMainTrimmed[index].distance == currentDistance)
                    buffer.append( solidSpaceMainTrimmed[index] );
                else
                    break;
            }

            // loop end or next iteration assignments
            if ( index == size )
                loopEnd = false;
            else {
                currentStart = index;
                currentDistance = solidSpaceMainTrimmed[currentStart].distance;
            }


            if ( buffer.size() != 0 ){

                int maxLength = 0, maxIndex = 0;

                // detect maximum length in buffer
                for (int i = 0; i < buffer.size(); i++)
                    if (buffer[i].length > maxLength) {
                        maxIndex = i;
                        maxLength = buffer[i].length;
                    }

                bufferEquals.clear();

                // detect indexes of equal maximum points
                for (int i = 0; i < buffer.size(); i++)
                    if (buffer[i].length == maxLength)
                        bufferEquals.append( i );

                // calculate avg distance & angle of maximum length points
                solidLine avgLine;
                float avgDistance = 0, avgAngle = 0;

                for (int i = 0; i < bufferEquals.size(); i++){
                    avgDistance += buffer[ bufferEquals[i] ].distance;
                    avgAngle += buffer[ bufferEquals[i] ].angle;
                }

                if ( bufferEquals.size() != 0) {
                    avgDistance /= bufferEquals.size();
                    avgAngle /= bufferEquals.size();
                }

                // x & y assignments are just for occupation, first occurance of maximum in buffer list
                avgLine.start.setX( buffer[maxIndex].start.x() );
                avgLine.start.setY( buffer[maxIndex].start.y() );
                avgLine.end.setX( buffer[maxIndex].end.x() );
                avgLine.end.setY( buffer[maxIndex].end.y() );
                avgLine.length = maxLength;
                avgLine.distance = avgDistance;
                avgLine.angle = avgAngle;

                solidSpaceMainMaximums.append( avgLine );
            }
        } // while
    }



    // find major areas
    majorThresholdPercent = 0.8;   // %80

    int maxIndex = 0;
    maxSolidLineLength = 0;

    // detect maximum length in solidSpaceMainMaximums
    for (int i = 0; i < solidSpaceMainMaximums.size(); i++)
        if (solidSpaceMainMaximums[i].length > maxSolidLineLength) {
            maxIndex = i;
            maxSolidLineLength = solidSpaceMainMaximums[i].length;
        }

    float majorThreshold = maxSolidLineLength * majorThresholdPercent;
    majorArea *area;
    bool areaStartFlag = false;
    majorList.clear();

    for (int i = 0; i < solidSpaceMainMaximums.size(); i++) {

        if (solidSpaceMainMaximums[i].length > majorThreshold && !areaStartFlag) {
            areaStartFlag = true;
            area = new majorArea();
            area->startIndex = i;
        }

        if (solidSpaceMainMaximums[i].length < majorThreshold && areaStartFlag) {
            areaStartFlag = false;
            area->endIndex = i - 1;
            majorList.append( area );
        }
    }



    // find major lines from areas

    int _max = 0, _index = 0;
    majorLines.clear();
    solidLine avgLine;
    float avgDistance, avgAngle, equalCount;

    for (int i = 0; i < majorList.size(); i++){
        _max = 0;
        _index = 0;

        // detect max of area
        for (int j = majorList[i]->startIndex; j <= majorList[i]->endIndex; j++)
            if (solidSpaceMainMaximums[j].length > _max) {
                _index = j;
                _max = solidSpaceMainMaximums[j].length;
            }


        // average equal maxs
        avgDistance = 0, avgAngle = 0, equalCount = 0;

        for (int j = majorList[i]->startIndex; j <= majorList[i]->endIndex; j++)
            if (solidSpaceMainMaximums[j].length == _max) {
                equalCount++;
                avgDistance += solidSpaceMainMaximums[j].distance;
                avgAngle += solidSpaceMainMaximums[j].angle;
            }

        if (equalCount != 0){
            avgDistance /= equalCount;
            avgAngle /= equalCount;

            // x & y assignments are just for occupation, first occurance of maximum in area list
            avgLine.start.setX( solidSpaceMainMaximums[_index].start.x() );
            avgLine.start.setY( solidSpaceMainMaximums[_index].start.y() );
            avgLine.end.setX( solidSpaceMainMaximums[_index].end.x() );
            avgLine.end.setY( solidSpaceMainMaximums[_index].end.y() );
            avgLine.length = _max;
            avgLine.distance = avgDistance;
            avgLine.angle = avgAngle;

            majorLines.append( avgLine );
        }
    }



    // obtain 2 major lines
    major2Lines.clear();
    majorLinesFound = true;

    if (majorLines.size() == 2){
        major2Lines.append(majorLines[0]);
        major2Lines.append(majorLines[1]);
    } else
    if (majorLines.size() == 1){
        major2Lines.append(majorLines[0]);
        major2Lines.append(majorLines[0]);
    } else
    if (majorLines.size() == 0){
        majorLinesFound = false;
    } else {
        // detect max 2

        int maxValue = 0, indexValue = 0;

        for (int j = 0; j < majorLines.size(); j++)
            if (majorLines[j].length > maxValue) {
                indexValue = j;
                maxValue = majorLines[j].length;
            }
        major2Lines.append( majorLines[indexValue] );

        majorLines[indexValue].length = 0;
        maxValue = 0, indexValue = 0;

        for (int j = 0; j < majorLines.size(); j++)
            if (majorLines[j].length > maxValue) {
                indexValue = j;
                maxValue = majorLines[j].length;
            }
        major2Lines.append( majorLines[indexValue] );
    }


    // calculate average distance and angle value of 2 major line
    if (majorLinesFound) {
        distanceAvg = (major2Lines[0].distance + major2Lines[1].distance ) / 2;
        thetaAvg = (major2Lines[0].angle + major2Lines[1].angle ) / 2;
        primaryLine = detectLongestSolidLine( distanceAvg, thetaAvg , true);
    } else {
        distanceAvg = -1;
        thetaAvg = -1;
    }



    /*
    // order according to line length descending
    solidSpaceMainOrdered.clear();

    int maxLength = 0, maxIndex = 0;

    for (int i = 0; i < solidSpaceMain.size(); i++){
        for (int j = 0; j < solidSpaceMain.size(); j++){
            if ( solidSpaceMain[j].length > maxLength ){
                maxIndex = j;
                maxLength = solidSpaceMain[j].length;
            }
        }
        solidSpaceMainOrdered.append( solidSpaceMain[maxIndex] );
        solidSpaceMain[maxIndex].length = 0;
        maxLength = 0;

    }
*/
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
    histogramInitSwitch = true;

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
    histogramInitSwitch = true;

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

    if (detected){
        QRgb value;
        value = qRgb(0, 255, 0);        // green

        for (int x = -4; x <= 4; x++){
            imgCorner.setPixel(leftCornerX + x, leftCornerY, value);
            imgCorner.setPixel(rightCornerX + x, rightCornerY, value);
        }

        for (int y = -4; y <= 4; y++){
            imgCorner.setPixel(leftCornerX, leftCornerY + y, value);
            imgCorner.setPixel(rightCornerX, rightCornerY + y, value);
            imgCorner.setPixel(trackCenterX, trackCenterY + y, value);
        }
    }
    return imgCorner;
}


imgProcess::~imgProcess(){
    for (int y = 0; y < imageHeight; y++) delete []valueMatrix[y];
    delete []valueMatrix;

    for (int y = 0; y < edgeHeight; y++) delete []edgeMatrix[y];
    delete []edgeMatrix;

    for (int y = 0; y < edgeHeight; y++) delete []edgeThickenedMatrix[y];
    delete []edgeThickenedMatrix;

    for (int y = 0; y < edgeHeight; y++) delete []houghMatrix[y];
    delete []houghMatrix;

    for (int y = 0; y < houghDistanceMax; y++) delete []houghSpace[y];
    delete []houghSpace;

    if (houghLinesInitSwitch) {
        for (int y = 0; y < houghLineNo; y++) delete []houghLines[y];
        delete []houghLines;
    }

    if (histogramInitSwitch) delete []histogram;

    if (voidSpace.size() > 0) {
        for (int i = 0; i < voidSpace.size(); i++) delete voidSpace[i];
        voidSpace.clear();
    }

    if (solidSpace.size() > 0) {
        for (int i = 0; i < solidSpace.size(); i++) delete solidSpace[i];
        solidSpace.clear();
    }

    if (majorList.size() > 0) {
        for (int i = 0; i < majorList.size(); i++) delete majorList[i];
        majorList.clear();
    }

    if ( solidSpaceMain.size() > 0 ) solidSpaceMain.clear();
    if ( solidSpaceMainTrimmed.size() > 0 ) solidSpaceMainTrimmed.clear();
    if ( solidSpaceMainMaximums.size() > 0 ) solidSpaceMainMaximums.clear();
    if ( solidSpaceMainOrdered.size() > 0 ) solidSpaceMainOrdered.clear();
    if ( majorLines.size() > 0 ) majorLines.clear();
    if ( major2Lines.size() > 0 ) major2Lines.clear();
}
