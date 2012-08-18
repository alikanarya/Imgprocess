#include "imgprocess.h"
//#include <QImage>
//#include <QColor>
#include <QFile>
#include <QTextStream>
#include "math.h"

#include "imgprocess_msg.h"
#include "../_Modules/Algo/localMinimum.h"


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


void imgProcess::constructValueMatrix(QImage image, int selection){

    valueSelection = selection;

    QRgb rgbValue;
    QColor *color;
    int colorValue;

    for (int y = 0; y < image.height(); y++)
        for (int x = 0; x < image.width(); x++){
            rgbValue = image.pixel(x,y);
            color = new QColor(rgbValue);

            switch (valueSelection) {
            case 0:
                colorValue = color->value();
                break;
            case 1:
                colorValue = color->red();
                break;
            case 2:
                colorValue = color->green();
                break;
            case 3:
                colorValue = color->blue();
                break;
            default:
                colorValue = color->value();
                break;
            }

            if ( colorValue > 255) colorValue = 255;
            else if (colorValue < 0)  colorValue = 0;

            valueMatrix[y][x] = colorValue;
            delete color;
        }
}


void imgProcess::constructValueHueMatrix(QImage image, bool scale){

    QRgb rgbValue;
    QColor *color;
    int colorValue;

    for (int y = 0; y < image.height(); y++)
        for (int x = 0; x < image.width(); x++){
            rgbValue = image.pixel(x,y);
            color = new QColor(rgbValue);
            colorValue = color->hue();

            if (scale) {
                colorValue = colorValue * 255.0 / 360.0;

                if ( colorValue > 255) colorValue = 255;
                else if (colorValue < 0)  colorValue = 0;
            } else {
                if ( colorValue > 359) colorValue = 359;
                else if (colorValue < 0)  colorValue = 0;
            }

            valueMatrix[y][x] = colorValue;
            delete color;
        }
}


void imgProcess::constructValueMaxMatrix(QImage image){

    QRgb rgbValue;
    QColor *color;
    int colorValue;

    for (int y = 0; y < image.height(); y++)
        for (int x = 0; x < image.width(); x++){
            rgbValue = image.pixel(x,y);
            color = new QColor(rgbValue);

            QList<int> list;
            list.append( color->value() );
            list.append( color->red() );
            list.append( color->blue() );
            list.append( color->green() );

            colorValue = -1;
            for (int i = 0; i < 4; i++)
                if ( list[i] > colorValue)
                    colorValue = list[i];

            if ( colorValue > 255) colorValue = 255;
            else if (colorValue < 0)  colorValue = 0;

            valueMatrix[y][x] = colorValue;

            list.empty();
            delete color;
        }
}


void imgProcess::gaussianBlur(){

    int sum;

    for (int y = 2;y < imageHeight - 2; y++){

        for (int x = 2; x < imageWidth - 2; x++) {

            sum =   gaussianMask[0][0]*valueMatrix[y-2][x-2] + gaussianMask[0][1]*valueMatrix[y-2][x-1] + gaussianMask[0][2]*valueMatrix[y-2][x] + gaussianMask[0][3]*valueMatrix[y-2][x+1] + gaussianMask[0][4]*valueMatrix[y-2][x+2] +
                    gaussianMask[1][0]*valueMatrix[y-1][x-2] + gaussianMask[1][1]*valueMatrix[y-1][x-1] + gaussianMask[1][2]*valueMatrix[y-1][x] + gaussianMask[1][3]*valueMatrix[y-1][x+1] + gaussianMask[1][4]*valueMatrix[y-1][x+2] +
                    gaussianMask[2][0]*valueMatrix[y][x-2]   + gaussianMask[2][1]*valueMatrix[y][x-1]   + gaussianMask[2][2]*valueMatrix[y][x]   + gaussianMask[2][3]*valueMatrix[y][x+1]   + gaussianMask[2][4]*valueMatrix[y][x+2]   +
                    gaussianMask[3][0]*valueMatrix[y+1][x-2] + gaussianMask[3][1]*valueMatrix[y+1][x-1] + gaussianMask[3][2]*valueMatrix[y+1][x] + gaussianMask[3][3]*valueMatrix[y+1][x+1] + gaussianMask[3][4]*valueMatrix[y+1][x+2] +
                    gaussianMask[4][0]*valueMatrix[y+2][x-2] + gaussianMask[4][1]*valueMatrix[y+2][x-1] + gaussianMask[4][2]*valueMatrix[y+2][x] + gaussianMask[4][3]*valueMatrix[y+2][x+1] + gaussianMask[4][4]*valueMatrix[y+2][x+2];

            valueMatrix[y][x] = sum / gaussianDivider;

            //if (valueMatrix[y][x] > 255)        valueMatrix[y][x] = 255;
            //else if (valueMatrix[y][x] < 0)     valueMatrix[y][x] = 0;
        }
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


bool imgProcess::saveMatrix(bool **matrix, int width, int height, QString fname){

    QFile file(fname);
    bool saveStatus = true;
    int val;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);

        for(int y = 0; y < height; y++){
            for(int x = 0; x < width; x++){
                if ( matrix[y][x] )
                    val = 1;
                else val = 0;

                out << val;
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
            out << array[i].distance << " " << QString::number(array[i].angle, 'f', 1) << " " << array[i].start.x() << " " << array[i].start.y() << " " << array[i].end.x() << " " << array[i].end.y() << " " << array[i].length << "\n";
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

            if (G > 255)
                G = 255;
            else if (G < 0)
                G = 0;
            edgeMatrix[y-1][x-1] = G;
        }
}


void imgProcess::prepareCannyArrays(){

    edgeGradientMatrix = new int*[edgeHeight];
    for (int i = 0; i < edgeHeight; i++)   edgeGradientMatrix[i] = new int[edgeWidth];
    edgeGradientMatrixInitSwitch = true;

    edgeSuppressedMatrix = new int*[edgeHeight];
    for (int i = 0; i < edgeHeight; i++)   edgeSuppressedMatrix[i] = new int[edgeWidth];
    edgeSuppressedMatrixInitSwitch = true;

    edgeStrongMatrix = new int*[edgeHeight];
    for (int i = 0; i < edgeHeight; i++)   edgeStrongMatrix[i] = new int[edgeWidth];
    edgeStrongMatrixInitSwitch = true;

    edgeWeakMatrix = new int*[edgeHeight];
    for (int i = 0; i < edgeHeight; i++)   edgeWeakMatrix[i] = new int[edgeWidth];
    edgeWeakMatrixInitSwitch = true;

    edgeMapMatrix = new bool*[edgeHeight];
    for (int i = 0; i < edgeHeight; i++)   edgeMapMatrix[i] = new bool[edgeWidth];
    edgeMapMatrixInitSwitch = true;

    edgeVisitMatrix = new bool*[edgeHeight];
    for (int i = 0; i < edgeHeight; i++)   edgeVisitMatrix[i] = new bool[edgeWidth];
    edgeVisitMatrixInitSwitch = true;

    // map coor.s of weaks connected to strength, for debugging
    edgeW2SMapMatrix = new bool*[edgeHeight];
    for (int i = 0; i < edgeHeight; i++)   edgeW2SMapMatrix[i] = new bool[edgeWidth];
    edgeW2SMapMatrixInitSwitch = true;

}


void imgProcess::detectEdgeSobelwDirections(){

    int G, Gx, Gy;
    float angle, angleApprox = 0;

    for (int y = 1;y < imageHeight - 1; y++)
        for (int x = 1; x < imageWidth - 1; x++){

            Gx =    sobelX[0][0]*valueMatrix[y-1][x-1] + sobelX[0][1]*valueMatrix[y-1][x] + sobelX[0][2]*valueMatrix[y-1][x+1] +
                    sobelX[1][0]*valueMatrix[y][x-1]   + sobelX[1][1]*valueMatrix[y][x]   + sobelX[1][2]*valueMatrix[y][x+1] +
                    sobelX[2][0]*valueMatrix[y+1][x-1] + sobelX[2][1]*valueMatrix[y+1][x] + sobelX[2][2]*valueMatrix[y+1][x+1];

            Gy =    sobelY[0][0]*valueMatrix[y-1][x-1] + sobelY[0][1]*valueMatrix[y-1][x] + sobelY[0][2]*valueMatrix[y-1][x+1] +
                    sobelY[1][0]*valueMatrix[y][x-1]   + sobelY[1][1]*valueMatrix[y][x]   + sobelY[1][2]*valueMatrix[y][x+1] +
                    sobelY[2][0]*valueMatrix[y+1][x-1] + sobelY[2][1]*valueMatrix[y+1][x] + sobelY[2][2]*valueMatrix[y+1][x+1];

            G = (int)(sqrt(pow(Gx, 2) + pow(Gy, 2)));

            //if (G > 255)    G = 255;
            //else if (G < 0) G = 0;
            edgeMatrix[y-1][x-1] = G;

            angle = atan2(Gy, Gx) * 180 / PI;

            /* Convert actual edge direction to approximate value */
            if ( ( (angle <= 22.5) && (angle > -22.5) ) || (angle > 157.5) || (angle < -157.5) )
                    angleApprox = 0;
            if ( ( (angle > 22.5) && (angle <= 67.5) ) || ( (angle < -112.5) && (angle >= -157.5) ) )
                    angleApprox = 45;
            if ( ( (angle > 67.5) && (angle <= 112.5) ) || ( (angle <= -67.5) && (angle > -112.5) ) )
                    angleApprox = 90;
            if ( ( (angle > 112.5) && (angle <= 157.5) ) || ( (angle <= -22.5) && (angle > -67.5) ) )
                    angleApprox = 135;

            edgeGradientMatrix[y-1][x-1] = angleApprox;
        }
}


void imgProcess::nonMaximumSuppression(){

    for (int y = 0;y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            edgeSuppressedMatrix[y][x] = edgeMatrix[y][x];

    int hotPixel, prevPixel, nextPixel;
    int prevPixelX, prevPixelY, nextPixelX, nextPixelY;

    for (int y = 0;y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++){

            hotPixel = edgeMatrix[y][x];

            switch (edgeGradientMatrix[y][x]) {

                case 0  :
                    prevPixelX = x - 1;
                    prevPixelY = y;
                    nextPixelX = x + 1;
                    nextPixelY = y;
                    break;

                case 45 :
                    prevPixelX = x - 1;
                    prevPixelY = y + 1;
                    nextPixelX = x + 1;
                    nextPixelY = y - 1;
                    break;

                case 90 :
                    prevPixelX = x;
                    prevPixelY = y + 1;
                    nextPixelX = x;
                    nextPixelY = y - 1;
                    break;

                case 135 :
                    prevPixelX = x + 1;
                    prevPixelY = y + 1;
                    nextPixelX = x - 1;
                    nextPixelY = y - 1;
                    break;

                default  :
                    prevPixelX = nextPixelX = x;
                    prevPixelY = nextPixelY = y;
                    break;
            }

            if ( prevPixelX < 0 || prevPixelY < 0 || prevPixelX >= edgeWidth ||  prevPixelY >= edgeHeight )
                prevPixel = 0;
            else
                prevPixel = edgeMatrix[prevPixelY][prevPixelX];

            if ( nextPixelX < 0 || nextPixelY < 0 || nextPixelX >= edgeWidth ||  nextPixelY >= edgeHeight )
                nextPixel = 0;
            else
                nextPixel = edgeMatrix[nextPixelY][nextPixelX];

            if ( (hotPixel < prevPixel) || (hotPixel < nextPixel) )
                edgeSuppressedMatrix[y][x] = 0;

        }
}


void imgProcess::cannyThresholding(bool autoThresh, int loPercent, int hiPercent){

    if (autoThresh){

        findMedianValue();

    } else {
        tLo = 255.0 * loPercent / 100.0;
        tHi = 255.0 * hiPercent / 100.0;

    }


    for (int y = 0;y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++){
            edgeStrongMatrix[y][x] = edgeWeakMatrix[y][x] = 0;
            edgeMapMatrix[y][x] = false;
            edgeVisitMatrix[y][x] = false;
            edgeW2SMapMatrix[y][x] = false;
        }

    for (int y = 0;y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            if ( edgeSuppressedMatrix[y][x] >= tHi ) {
                edgeStrongMatrix[y][x] = edgeSuppressedMatrix[y][x];
                edgeMapMatrix[y][x] = true;
            }
            else if ( edgeSuppressedMatrix[y][x] < tHi && edgeSuppressedMatrix[y][x] >= tLo )
                edgeWeakMatrix[y][x] = edgeSuppressedMatrix[y][x];
}


void imgProcess::edgeTracing(){

    for (int y = 0;y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            if (edgeStrongMatrix[y][x] > 0 && !edgeVisitMatrix[y][x]){

                edgeVisitMatrix[y][x] = true;
                checkContinuity(x, y, edgeGradientMatrix[y][x]);
            }
}


void imgProcess::checkContinuity(int inX, int inY, int inDir){

    int nextPixelX, nextPixelY;
    bool coorValid;

    for (int cy = -1; cy <= 1; cy++){
        for (int cx = -1; cx <= 1; cx++){

            if ( !(cy == 0 && cx == 0) ){
                nextPixelX = inX + cx;
                nextPixelY = inY + cy;

                coorValid =  !( (nextPixelX < 0) || (nextPixelY < 0) || (nextPixelX >= edgeWidth) || (nextPixelY >= edgeHeight) );

                if ( coorValid ) {

                    if ( !edgeVisitMatrix[nextPixelY][nextPixelX] && (edgeWeakMatrix[nextPixelY][nextPixelX] > 0) && (edgeGradientMatrix[nextPixelY][nextPixelX] == inDir) ) {
                        edgeStrongMatrix[nextPixelY][nextPixelX] = edgeWeakMatrix[nextPixelY][nextPixelX];
                        edgeMapMatrix[nextPixelY][nextPixelX] = true;
                        edgeVisitMatrix[nextPixelY][nextPixelX] = true;
                        edgeW2SMapMatrix[nextPixelY][nextPixelX] = true;

                        checkContinuity(nextPixelX, nextPixelY, inDir);
                    }
                }
            }
        }
    }
}


void imgProcess::assignEdgeMap(){

    switch (valueSelection) {
    case 0:

        edgeMapValueMatrix = new bool*[edgeHeight];
        for (int i = 0; i < edgeHeight; i++)   edgeMapValueMatrix[i] = new bool[edgeWidth];
        edgeMapValueMatrixInitSwitch = true;

        for (int y = 0; y < edgeHeight; y++)
            for (int x = 0; x < edgeWidth; x++)
                edgeMapValueMatrix[y][x] = edgeMapMatrix[y][x];

        break;

    case 1:

        edgeMapRedMatrix = new bool*[edgeHeight];
        for (int i = 0; i < edgeHeight; i++)   edgeMapRedMatrix[i] = new bool[edgeWidth];
        edgeMapRedMatrixInitSwitch = true;

        for (int y = 0; y < edgeHeight; y++)
            for (int x = 0; x < edgeWidth; x++)
                edgeMapRedMatrix[y][x] = edgeMapMatrix[y][x];

        break;

    case 2:

        edgeMapGreenMatrix = new bool*[edgeHeight];
        for (int i = 0; i < edgeHeight; i++)   edgeMapGreenMatrix[i] = new bool[edgeWidth];
        edgeMapGreenMatrixInitSwitch = true;

        for (int y = 0; y < edgeHeight; y++)
            for (int x = 0; x < edgeWidth; x++)
                edgeMapGreenMatrix[y][x] = edgeMapMatrix[y][x];

        break;

    case 3:

        edgeMapBlueMatrix = new bool*[edgeHeight];
        for (int i = 0; i < edgeHeight; i++)   edgeMapBlueMatrix[i] = new bool[edgeWidth];
        edgeMapBlueMatrixInitSwitch = true;

        for (int y = 0; y < edgeHeight; y++)
            for (int x = 0; x < edgeWidth; x++)
                edgeMapBlueMatrix[y][x] = edgeMapMatrix[y][x];

        break;

    default:
        break;
    }

}


void imgProcess::mergeEdgeMaps(){

    for (int y = 0; y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)

            edgeMapMatrix[y][x] = edgeMapValueMatrix[y][x] &&
                                  edgeMapRedMatrix[y][x] &&
                                  edgeMapGreenMatrix[y][x] &&
                                  edgeMapBlueMatrix[y][x];

/*
    edgeMapMatrix[y][x] = edgeMapValueMatrix[y][x] ||
                          edgeMapRedMatrix[y][x] ||
                          edgeMapGreenMatrix[y][x] ||
                          edgeMapBlueMatrix[y][x];
*/
}


void imgProcess::scaleEdgeData(int threshold){

    int max, index;
    float multiplier;
    for (int y = 0; y < edgeHeight; y++){

        max = index = 0;
        for (int x = 0; x < edgeWidth; x++)
            if (edgeMatrix[y][x] > max){
                max = edgeMatrix[y][x];
                index = x;
            }

        if (max < 255 && max != 0 ){
            multiplier = 255.0 / max;
            for (int x = 0; x < edgeWidth; x++)
                if ( edgeMatrix[y][x] > threshold )
                    edgeMatrix[y][x] = edgeMatrix[y][x] * multiplier;
        }
    }
}


void imgProcess::makeBinaryEdgeMatrix(int threshold){

    for (int y = 0; y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            if (edgeMatrix[y][x] >= threshold)
                edgeMatrix[y][x] = 1;
            else
                edgeMatrix[y][x] = 0;
}


void imgProcess::thickenEdges(){

    int xnStart = -1, xnEnd = 1, ynStart = -1, ynEnd = 1;

    for (int y = 0; y < imageHeight; y++)
        for (int x = 0; x < imageWidth; x++)
            edgeThickenedMatrix[y][x] = 0;

    for (int y = 1; y < (imageHeight - 1); y++)
        for (int x = 1; x < (imageWidth - 1); x++){
            /* boundaries for edge -> edge matrix
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
            */
            if ( edgeMatrix[y - 1][x - 1] == 255 )
                for (int xn = xnStart; xn <= xnEnd; xn++)
                    for (int yn = ynStart; yn <= ynEnd; yn++)
                       edgeThickenedMatrix[y + yn][x + xn] = 255;
            //else edgeThickenedMatrix[y][x] = 0;
        }
}


void imgProcess::houghTransform(){

    houghDistanceMax = (int) (sqrt(pow(edgeWidth, 2) + pow(edgeHeight, 2)));
    //centerX = edgeWidth / 2;
    //centerY = edgeHeight - 1;
    centerX = 0;
    centerY = 0;

    houghThetaSize = (int) ((thetaMax - thetaMin) / thetaStep) + 1;

    houghSpace = new int*[houghDistanceMax];
    for (int i = 0; i < houghDistanceMax; i++)   houghSpace[i] = new int[houghThetaSize];
    houghSpaceInitSwitch = true;

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


void imgProcess::houghTransformEdgeMap(){

    houghDistanceMax = (int) (sqrt(pow(edgeWidth, 2) + pow(edgeHeight, 2)));
    //centerX = edgeWidth / 2;
    //centerY = edgeHeight - 1;
    centerX = 0;
    centerY = 0;

    houghThetaSize = (int) ((thetaMax - thetaMin) / thetaStep) + 1;

    houghSpace = new int*[houghDistanceMax];
    for (int i = 0; i < houghDistanceMax; i++)   houghSpace[i] = new int[houghThetaSize];
    houghSpaceInitSwitch = true;

    for (int y = 0; y < houghDistanceMax; y++)
        for (int x = 0; x < houghThetaSize; x++) houghSpace[y][x] = 0;

    int distance, theta;
    for (int y = 0; y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            if (edgeMapMatrix[y][x])
                for (int i = 0; i < houghThetaSize; i++){
                    theta = thetaMin + i * thetaStep;
                    distance = (int) ((x - centerX) * cos(theta * R2D) + (centerY - y) * sin(theta * R2D));
                    if (distance >= 0) houghSpace[distance][i]++;
                }
}


void imgProcess::houghTransformExtended(){

    houghDistanceMax = (int) (sqrt(pow(imageWidth, 2) + pow(imageHeight, 2)));

    centerX = imageWidth / 2;
    centerY = imageHeight - 1;
    houghThetaSize = (int) ((thetaMax - thetaMin) / thetaStep) + 1;

    houghSpace = new int*[houghDistanceMax];
    for (int i = 0; i < houghDistanceMax; i++)   houghSpace[i] = new int[houghThetaSize];
    houghSpaceInitSwitch = true;

    for (int y = 0; y < houghDistanceMax; y++)
        for (int x = 0; x < houghThetaSize; x++) houghSpace[y][x] = 0;

    int distance, theta;
    for (int y = 0; y < imageHeight; y++)
        for (int x = 0; x < imageWidth; x++)
            if (edgeThickenedMatrix[y][x] != 0)
                for (int i = 0; i < houghThetaSize; i++){
                    theta = thetaMin + i * thetaStep;
                    distance = (int) ((x - centerX) * cos(theta * R2D) + (centerY - y) * sin(theta * R2D));
                    if (distance >= 0) houghSpace[distance][i]++;
                }
}


void imgProcess::houghTransformContrast(){

    houghDistanceMax = (int) (sqrt(pow(imageWidth, 2) + pow(imageHeight, 2)));
    centerX = 0;
    centerY = 0;

    houghThetaSize = (int) ((thetaMax - thetaMin) / thetaStep) + 1;

    houghSpace = new int*[houghDistanceMax];
    for (int i = 0; i < houghDistanceMax; i++)   houghSpace[i] = new int[houghThetaSize];
    houghSpaceInitSwitch = true;

    for (int y = 0; y < houghDistanceMax; y++)
        for (int x = 0; x < houghThetaSize; x++) houghSpace[y][x] = 0;

    int distance, theta;
    for (int y = 0; y < imageHeight; y++)
        for (int x = 0; x < imageWidth; x++)
            if (contrastMatrix[y][x] != 0)
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


bool imgProcess::sortHoughLines_toDistance(int _size){

    houghLinesSorted_size = _size;

    if (houghLineNo != 0) {

        float **houghLinesCopy = new float*[houghLinesSorted_size];
        for (int i = 0; i < houghLinesSorted_size; i++)   houghLinesCopy[i] = new float[3];

        for (int line = 0; line < houghLinesSorted_size; line++)
            for (int i = 0; i < 3; i++)
                houghLinesCopy[line][i] = houghLines[line][i];

        houghLinesSorted = new float*[houghLinesSorted_size];
        for (int i = 0; i < houghLinesSorted_size; i++)   houghLinesSorted[i] = new float[3];
        houghLinesSortedInitSwitch = true;

        int maxDistance, index;

        for (int x = (houghLinesSorted_size - 1); x >= 0; x--){

            maxDistance = -1;
            index = 0;

            for (int y = 0; y < houghLinesSorted_size; y++){
                if (houghLinesCopy[y][0] >= maxDistance){
                    maxDistance = houghLinesCopy[y][0];
                    index = y;
                }
            }

            houghLinesSorted[x][0] = houghLinesCopy[index][0];      // distance
            houghLinesSorted[x][1] = houghLinesCopy[index][1];      // angle
            houghLinesSorted[x][2] = houghLinesCopy[index][2];      // vote value

            houghLinesCopy[index][0] = -2;
        }

        for (int y = 0; y < houghLinesSorted_size; y++) delete []houghLinesCopy[y];
        delete []houghLinesCopy;

        return true;
    } else
        return false;

}


void imgProcess::codeLineData(int **matrix, int width, int height, QList<houghData> list, bool orientation){

    // orientation
    // true: vertical line
    // false: horizontal line

    if (list.size() != 0) {


        for (int c = 0; c < list.size(); c++){

            // +1 additions to centers for edge to original matrix transformations

            if (orientation) {

                int lineX;

                for (int y = 0; y < height; y++){
                    lineX = centerX + 1 + getLineX((centerY + 1 - y), list[c].distance, list[c].angle);

                    if (lineX >= 0 && lineX < width) matrix[y][lineX] = 2555;       // 2555 special code to differenciate line data, arbitrary
                }

            } else {

                int lineY;

                for (int x = 0; x < width; x++){
                    lineY = centerY + 1 - getLineY((x - centerX - 1), list[0].distance, list[0].angle);

                    if (lineY >= 0 && lineY < height) matrix[lineY][x] = 2555;       // 2555 special code to differenciate line data, arbitrary
                }
            }
        }
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


void imgProcess::constructHoughMatrixPrimaryLines(solidLine line1, solidLine line2, int line2offset){

    int lineY;

    for (int y = 0; y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            houghMatrix[y][x] = edgeMatrix[y][x];

    for (int x = line1.start.x(); x <= line1.end.x(); x++){
        lineY = centerY - getLineY((x - centerX), line1.distance, line1.angle);

        if (lineY >= 0 && lineY < edgeHeight)
            if (houghMatrix[lineY][x] == 0) houghMatrix[lineY][x] = 2555;       // 2555 special code to differeciate line data, arbitrary
    }

    int startX = line2offset + line2.start.x();
    int endX = line2offset + line2.end.x();

    for (int x = startX; x <= endX; x++){
        lineY = centerY - getLineY((x - centerX), line2.distance, line2.angle);

        if (lineY >= 0 && lineY < edgeHeight)
            if (houghMatrix[lineY][x] == 0) houghMatrix[lineY][x] = 2555;       // 2555 special code to differeciate line data, arbitrary
    }
}


void imgProcess::constructHoughMatrixMajor2Lines(){

    int lineY;

    for (int y = 0; y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            houghMatrix[y][x] = edgeMatrix[y][x];

    if (majorLines.size() == 2){

        for (int x = majorLines[0].start.x(); x <= majorLines[0].end.x(); x++){
            lineY = centerY - getLineY((x - centerX), majorLines[0].distance, majorLines[0].angle);

            if (lineY >= 0 && lineY < edgeHeight)
                if (houghMatrix[lineY][x] == 0) houghMatrix[lineY][x] = 2555;       // 2555 special code to differeciate line data, arbitrary
        }

        for (int x = majorLines[1].start.x(); x <= majorLines[1].end.x(); x++){
            lineY = centerY - getLineY((x - centerX), majorLines[1].distance, majorLines[1].angle);

            if (lineY >= 0 && lineY < edgeHeight)
                if (houghMatrix[lineY][x] == 0) houghMatrix[lineY][x] = 2555;       // 2555 special code to differeciate line data, arbitrary
        }
    }
}


void imgProcess::constructHoughExtendedMatrixMajor2Lines(){

    for (int y = 0; y < imageHeight; y++)
        for (int x = 0; x < imageWidth; x++)
            houghExtendedMatrix[y][x] = edgeThickenedMatrix[y][x];

    int lineY;

    if (major2Lines.size() == 2){

        for (int x = major2Lines[0].start.x(); x <= major2Lines[0].end.x(); x++){
            lineY = centerY - getLineY((x - centerX), major2Lines[0].distance, major2Lines[0].angle);

            if (lineY >= 0 && lineY < imageHeight) houghExtendedMatrix[lineY][x] = 2555;       // 2555 special code to differenciate line data, arbitrary
        }

        for (int x = major2Lines[1].start.x(); x <= major2Lines[1].end.x(); x++){
            lineY = centerY - getLineY((x - centerX), major2Lines[1].distance, major2Lines[1].angle);

            if (lineY >= 0 && lineY < imageHeight) houghExtendedMatrix[lineY][x] = 2555;       // 2555 special code to differenciate line data, arbitrary
        }
    }
}


void imgProcess::constructContrastMatix(float multiplier){

    contrastMatrix = new int*[imageHeight];
    for (int i = 0; i < imageHeight; i++)    contrastMatrix[i] = new int[imageWidth];

    contrastInitSwitch = true;

    int *lineArray = new int[imageWidth];

    int sum, threshold;

    for(int y = 0; y < imageHeight; y++){
        lineArray[0] = 0;

        sum = 0;
        for(int x = 1; x < imageWidth; x++){

            lineArray[x] = valueMatrix[y][x] - valueMatrix[y][x-1];
            sum += abs(lineArray[x]);
        }
        threshold = multiplier * sum / imageWidth;

        for(int x = 0; x < imageWidth; x++)
            if (abs(lineArray[x]) > threshold)
                contrastMatrix[y][x] = 1;
            else
                contrastMatrix[y][x] = 0;
    }

    delete []lineArray;
}


void imgProcess::constructContrastMatrixMajor2Lines(){

    for (int y = 0; y < imageHeight; y++)
        for (int x = 0; x < imageWidth; x++)
            contrastMatrix[y][x] = valueMatrix[y][x];
            //contrastMatrix[y][x] = contrastMatrix[y][x] * 255;   // to draw contrast image

    int lineX;

    if (major2Lines.size() == 2){

        for (int y = major2Lines[0].start.y(); y <= major2Lines[0].end.y(); y++){
            lineX = centerX + getLineX((centerY - y), major2Lines[0].distance, major2Lines[0].angle);

            if (lineX >= 0 && lineX < imageWidth) contrastMatrix[y][lineX] = 2555;       // 2555 special code to differenciate line data, arbitrary
        }

        for (int y = major2Lines[1].start.y(); y <= major2Lines[1].end.y(); y++){
            lineX = centerX + getLineX((centerY - y), major2Lines[1].distance, major2Lines[1].angle);

            if (lineX >= 0 && lineX < imageWidth) contrastMatrix[y][lineX] = 2555;       // 2555 special code to differenciate line data, arbitrary
        }
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

        for (int line = 0; line < houghLineNo; line++)
            angleAvg += houghLines[line][1];

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
void imgProcess::calcAvgDistAndAngleOfMajors(float multiplier){

    int _voteThreshold = (int) (houghLines[0][2] * multiplier);    // %x of first line (most voted)
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


/* Avreage Method: global
    for (int line = 0; line <= _voteThresholdIndex; line++)
        _distanceAvg += houghLines[line][0];
    _distanceAvg = (int) (_distanceAvg / (_voteThresholdIndex + 1));
*/


/* Avreage Method: (max dist + min dist) / 2
    float distanceMax = 0;
    for (int line = 0; line <= _voteThresholdIndex; line++)
        if (houghLines[line][0] > distanceMax){
            distanceMax = houghLines[line][0];
        }

    float distanceMin = distanceMax;
    for (int line = 0; line <= _voteThresholdIndex; line++)
        if (houghLines[line][0] < distanceMin){
            distanceMin = houghLines[line][0];
        }

    _distanceAvg = ( distanceMax + distanceMin )/2;

*/

    // Avreage Method: distance derivative

    int size = _voteThresholdIndex + 1;

    sortHoughLines_toDistance(size);

    int *distanceVector = new int[size];

    distanceVector[0] = 0;
    for (int line = 1; line < size; line++)
        distanceVector[line] = abs(houghLinesSorted[line][0] - houghLinesSorted[line-1][0]);

    int derivativeMax = 0, index = 0;
    for (int line = 1; line < size; line++)
        if (distanceVector[line] > derivativeMax){
            derivativeMax = distanceVector[line];
            index = line;
        }

    _distanceAvg = houghLinesSorted[index][0];
    delete []distanceVector;
    // ---------




    // group lines in 2: higher and lower than ave. distance
    lowLinesList.clear();
    highLinesList.clear();

    for (int line = 0; line <= _voteThresholdIndex; line++){
        if (houghLines[line][0] >= _distanceAvg)
            highLinesList.append(line);
        else
            lowLinesList.append(line);
    }


    /* calc. ave. distance and angle of higher lines which have same max vote value
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
    */

    // calc. ave. distance and angle of higher lines which are voted above some threshold
    int distanceAvgHigh = 0;
    int thetaAvgHigh = 0;
    int countHigh = 0;
    int thresholdHigh = houghLines[ highLinesList[0] ][2] * 0.80;

    for (int i = 0; i < highLinesList.size(); i++)
        if (houghLines[ highLinesList[i] ][2] >= thresholdHigh) {
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

    /* calc. ave. distance and angle of lower lines which have same max vote value
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
    */

    // calc. ave. distance and angle of lower lines which are voted above some threshold
    int distanceAvgLo = 0;
    int thetaAvgLo = 0;
    int countLo = 0;
    int thresholdLo = 0;

    if (lowLinesList.size() != 0)
        thresholdLo = houghLines[ lowLinesList[0] ][2] * 0.80;

    for (int i = 0; i < lowLinesList.size(); i++)
        if (houghLines[ lowLinesList[i] ][2] >= thresholdLo) {
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

    // implemented for contrast detection
    major2Lines.clear();

    solidLine firstLine;
    firstLine.start.setX( 0 );
    firstLine.start.setY( 0 );
    firstLine.end.setX( imageWidth - 1);
    firstLine.end.setY( imageHeight - 1);
    firstLine.length = 1;
    firstLine.distance = distanceAvgHigh;
    firstLine.angle = thetaAvgHigh;
    major2Lines.append( firstLine);

    solidLine secondLine;
    secondLine.start.setX( 0 );
    secondLine.start.setY( 0 );
    secondLine.end.setX( imageWidth - 1);
    secondLine.end.setY( imageHeight - 1);
    secondLine.length = 1;
    secondLine.distance = distanceAvgLo;
    secondLine.angle = thetaAvgLo;
    major2Lines.append( secondLine);


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
                    if (x == imageWidth)
                        line->end.setX( x - 1 );  // last void is at rightmost border
                    else
                        line->end.setX( x );

                    line->end.setY( lineY );
                    line->length = voidCount;
                    voidSpace.append( line );
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
                    if (x == edgeWidth)
                        line->end.setX( x - 1 );  // last void is at rightmost border
                    else
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


// DETECTION FUNCTION BASED ON VOID SPACES
void imgProcess::detectPrimaryVoid(){

    detected = true;

    if (!primaryLineDetected){  // no primary line found
        detected = false;
        statusMessage = alarm1;
    }
    else if (voidSpace.size() == 0) {   // no void space found
        detected = false;
        statusMessage = alarm2;
    }
    else {

        // find maximum void area
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

        if (max < voidThreshold){   // not acceptable void area length
            detected = false;
            statusMessage = alarm3;
        }
        /*
        else if (voidSpace[voidIndex]->end.x() < errorEdgeLimit || voidSpace[voidIndex]->start.x() > (imageWidth - errorEdgeLimit)){
            detected = false;
            statusMessage = alarm4;
        }
        */
        else {
            if (abs(angleAvg) <= errorAngleLimit)   // calcAngleAvg() or bypass by default
                angleInLimit = true;
            else {
                angleInLimit = false;
                detected = false;       // do not accecpt this cam setup, line angle is not in tolerance
                statusMessage = alarm5;
            }
        }
    }
}


solidLine imgProcess::detectLongestSolidLine(float distance, float angle, bool flag, int xOffset, int xEndOffset) {

    solidSpace.clear();

    //int width = imageWidth;
    int height = imageHeight;
/*
    if (flag){
        width = imageWidth;
        height = imageHeight;
    } else {
        width = edgeWidth;
        height = edgeHeight;
    }
*/
    int lineY = 0, lineLength = 0;
    int prevValue = 0,    // begin with empty
        prevX = 0, prevY = 0,
        currentValue = 0, state = 0;
    solidLine *line;

    for (int x = xOffset; x <= (xEndOffset + 1); x++){   // +1 for last coordinate to catch last line shape

        lineY = centerY - getLineY((x - centerX), distance, angle);

        if (lineY >= 0 && lineY < height){

            if (x == (xEndOffset+1) )
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

    for (int i = 0; i < solidSpace.size(); i++) delete solidSpace[i];

    return longestLine;
}


// DETECTION FUNCTION BASED ON SOLID LINES
void imgProcess::detectLongestSolidLines(){

    primaryLineFound = false;
    secondaryLineFound = false;

    //----- find all solid lines in value matrix
    solidSpaceMain.clear();
    float angle = 0;
    float angleSize = (int) ((thetaMax - thetaMin) / thetaStep) + 1;

    for (int distance = 0; distance < imageHeight; distance++)
        for (int angleIndex = 0; angleIndex < angleSize; angleIndex++){
            angle = thetaMin + angleIndex * thetaStep;
            solidSpaceMain.append( detectLongestSolidLine( distance, angle, true, 0, (imageWidth-1) ) );  // in value matrix
        }
    //------------------------------------------------------------------------------------


    //----- find max length
    float globalMaxLength = 0;
    for (int x = 0; x < solidSpaceMain.size(); x++)
        if ( solidSpaceMain[x].length > globalMaxLength )
            globalMaxLength = solidSpaceMain[x].length;
    //------------------------------------------------------------------------------------


    if ( globalMaxLength != 0 ) {

        //----- remove lines w/ length behind threshold
        solidSpaceMainTrimmed.clear();
        float thresholdLength = globalMaxLength * 0.20;  // % 20 of global maximum

        for (int i = 0; i < solidSpaceMain.size(); i++)
            if ( solidSpaceMain[i].length > thresholdLength )
                solidSpaceMainTrimmed.append( solidSpaceMain[i] );
        //------------------------------------------------------------------------------------


        int size = solidSpaceMainTrimmed.size();

        if ( size != 0 ){

            //----- detect maximum length in solidSpaceMainTrimmed
            maxSolidLineLength = 0;
            int index = 0;

            for (int i = 0; i < size; i++)
                if ( solidSpaceMainTrimmed[i].length > maxSolidLineLength ) {
                    maxSolidLineLength = solidSpaceMainTrimmed[i].length;
                    index = i;
                }
            //------------------------------------------------------------------------------------


            //----- devide found lines into 2 main group,
            //----- each group should intersect reference line's coordinates
            primaryGroup.clear();
            secondaryGroup.clear();

            primaryGroup.append( solidSpaceMainTrimmed[index] );    // reference line

            for (int i = 0; i < size; i++)
                if ( i != index ) {

                    if ( solidSpaceMainTrimmed[index].start.x() > solidSpaceMainTrimmed[i].end.x() ||
                         solidSpaceMainTrimmed[index].end.x() < solidSpaceMainTrimmed[i].start.x()
                        )
                        secondaryGroup.append( solidSpaceMainTrimmed[i] );
                    else
                        primaryGroup.append( solidSpaceMainTrimmed[i] );
                }
            //------------------------------------------------------------------------------------


            //----- detect maximum length in primaryGroup (to recheck, can be removed)
            maxSolidLineLength = 0;

            for (int i = 0; i < primaryGroup.size(); i++)
                if ( primaryGroup[i].length > maxSolidLineLength )
                    maxSolidLineLength = primaryGroup[i].length;
            float primaryLengthThreshold = maxSolidLineLength * 0.75;
            //------------------------------------------------------------------------------------

            int count = 0;

            /* for angle band program
            //----- primaryGroup: calculate average angle of max lines if more than 1 max
            count = 0;
            thetaAvgPrimary = 0;

            for (int i = 0; i < primaryGroup.size(); i++)
                if ( primaryGroup[i].length == maxSolidLineLength ) {
                    thetaAvgPrimary += primaryGroup[i].angle;
                    count++;
                }

            if ( count != 0 ) {
                thetaAvgPrimary /= count;
            }
            //------------------------------------------------------------------------------------
            */

            //----- detect maximum length in secondaryGroup
            maxSolidLineLength = 0;

            for (int i = 0; i < secondaryGroup.size(); i++)
                if ( secondaryGroup[i].length > maxSolidLineLength )
                    maxSolidLineLength = secondaryGroup[i].length;
            float secondaryLengthThreshold = maxSolidLineLength * 0.75;
            //------------------------------------------------------------------------------------


            /* for angle band program
            //----- secondaryGroup: calculate average angle of max lines if more than 1 max
            count = 0;
            thetaAvgSecondary = 0;

            for (int i = 0; i < secondaryGroup.size(); i++)
                if ( secondaryGroup[i].length == maxSolidLineLength ) {
                    thetaAvgSecondary += secondaryGroup[i].angle;
                    count++;
                }

            if ( count != 0 ) {
                thetaAvgSecondary /= count;
            }
            //------------------------------------------------------------------------------------
            */

            //----- primaryGroup:
            //----- average above lenght threshold and obtain major line
            //      //----- average angle and distance within the same angle band of max. lenght and obtain major line
            //float avgAngleUp = thetaAvgPrimary + 0.5;      // +0.5C for deadband
            //float avgAngleDown = thetaAvgPrimary - 0.5;    // -0.5C for deadband

            // * for weighted average
            QList<solidLine> buffer;
            buffer.clear();

            //distanceAvgPrimary = 0;
            //thetaAvgPrimary = 0;
            count = 0;
            int sum = 0;
            for (int i = 0; i < primaryGroup.size(); i++)
                if ( primaryGroup[i].length > primaryLengthThreshold ) {
                //if ( primaryGroup[i].angle >= avgAngleDown && primaryGroup[i].angle <= avgAngleUp ) {
                    count++;

                    //distanceAvgPrimary += primaryGroup[i].distance;
                    //thetaAvgPrimary += primaryGroup[i].angle;

                    // * for weighted average
                    buffer.append( primaryGroup[i] );
                    sum += primaryGroup[i].length;
                }

            major2Lines.clear();

            if ( count != 0 ) {
                // * weighted average
                distanceAvgPrimary = 0;
                thetaAvgPrimary = 0;
                if ( sum != 0 ){
                    for (int i = 0; i < count; i++ ) {
                        distanceAvgPrimary += buffer[i].length * buffer[i].distance;
                        thetaAvgPrimary += buffer[i].length * buffer[i].angle;
                    }

                    distanceAvgPrimary /= sum;
                    thetaAvgPrimary /= sum;
                } else {
                    distanceAvgPrimary  = imageHeight/2;
                    thetaAvgPrimary = 90;
                }

                //distanceAvgPrimary /= count;
                //thetaAvgPrimary /= count;

                int priXStartOffset = imageWidth - 1;
                for (int i = 0; i < buffer.size(); i++ ) {

                    if ( buffer[i].start.x() < priXStartOffset )
                        priXStartOffset = buffer[i].start.x();
                }

                int priXEndOffset = 0;
                for (int i = 0; i < buffer.size(); i++ ) {

                    if ( buffer[i].end.x() > priXEndOffset )
                        priXEndOffset = buffer[i].end.x();
                }

                primaryLineFound = true;
                major2Lines.append( detectLongestSolidLine( distanceAvgPrimary, thetaAvgPrimary , true, priXStartOffset, priXEndOffset ) );  // in value matrix

            }
            //------------------------------------------------------------------------------------


            //----- secondaryGroup:
            //----- average above lenght threshold and obtain major line
            //      //----- average angle and distance within the same angle band of max. lenght and obtain major line
            //avgAngleUp = thetaAvgSecondary + 0.5;      // +0.5C for deadband
            //avgAngleDown = thetaAvgSecondary - 0.5;    // -0.5C for deadband

            // * for weighted average
            buffer.clear();

            //distanceAvgSecondary = 0;
            //thetaAvgSecondary = 0;
            count = 0;
            sum = 0;
            for (int i = 0; i < secondaryGroup.size(); i++)
                if ( secondaryGroup[i].length > secondaryLengthThreshold ) {
                //if ( secondaryGroup[i].angle >= avgAngleDown && secondaryGroup[i].angle <= avgAngleUp ) {
                    count++;

                    //distanceAvgSecondary += secondaryGroup[i].distance;
                    //thetaAvgSecondary += secondaryGroup[i].angle;

                    // * for weighted average
                    buffer.append( secondaryGroup[i] );
                    sum += secondaryGroup[i].length;
                }

            if ( count != 0 ) {

                // * weighted average
                distanceAvgSecondary = 0;
                thetaAvgSecondary = 0;
                if ( sum != 0 ){
                    for (int i = 0; i < count; i++ ) {
                        distanceAvgSecondary += buffer[i].length * buffer[i].distance;
                        thetaAvgSecondary += buffer[i].length * buffer[i].angle;
                    }

                    distanceAvgSecondary /= sum;
                    thetaAvgSecondary /= sum;
                } else {
                    distanceAvgSecondary  = imageHeight/2;
                    thetaAvgSecondary = 90;
                }

                //distanceAvgSecondary /= count;
                //thetaAvgSecondary /= count;

                int secXStartOffset = imageWidth - 1;
                for (int i = 0; i < buffer.size(); i++ ) {

                    if ( buffer[i].start.x() < secXStartOffset )
                        secXStartOffset = buffer[i].start.x();
                }

                int secXEndOffset = 0;
                for (int i = 0; i < buffer.size(); i++ ) {

                    if ( buffer[i].end.x() > secXEndOffset )
                        secXEndOffset = buffer[i].end.x();
                }

                secondaryLineFound = true;
                major2Lines.append( detectLongestSolidLine( distanceAvgSecondary, thetaAvgSecondary , true, secXStartOffset, secXEndOffset ) );  // in value matrix

                if ( major2Lines.last().length == -1 )  // in case of produced line dont touch the value matrix
                    secondaryLineFound = false;
            }
            //------------------------------------------------------------------------------------
        }
    }


    //----- calculate corners & center
    leftCornerX = 0;
    leftCornerY = imageHeight/2;
    rightCornerX = imageWidth - 1;
    rightCornerY = imageHeight/2;

    centerDetermined = false;
    angleAvg = 0;

    if ( primaryLineFound && secondaryLineFound ) {


        if ( major2Lines[0].start.x() > major2Lines[1].start.x() ) {    // primary = RIGHT
            rightCornerX = major2Lines[0].start.x();
            rightCornerY = major2Lines[0].start.y();

            leftCornerX = major2Lines[1].end.x();
            leftCornerY = major2Lines[1].end.y();

            centerDetermined = true;
        }
        else
        if ( major2Lines[0].start.x() < major2Lines[1].start.x() ) {    // primary = LEFT
            leftCornerX = major2Lines[0].end.x();
            leftCornerY = major2Lines[0].end.y();

            rightCornerX = major2Lines[1].start.x();
            rightCornerY = major2Lines[1].start.y();

            centerDetermined = true;
        }

        angleAvg = ( major2Lines[0].angle + major2Lines[1].angle ) / 2 - 90;  // to check laser setup
    }
    else
        if ( primaryLineFound ) {

            int distance2Left = major2Lines[0].start.x();
            int distance2Right = (imageWidth - 1) - major2Lines[0].end.x();

            if ( distance2Left > distance2Right ) {                     // primary = RIGHT
                rightCornerX = major2Lines[0].start.x();
                rightCornerY = major2Lines[0].start.y();

                centerDetermined = true;
            } else
                if ( distance2Left < distance2Right ) {                 // primary = LEFT
                    leftCornerX = major2Lines[0].end.x();
                    leftCornerY = major2Lines[0].end.y();

                    centerDetermined = true;
                }
            angleAvg = major2Lines[0].angle - 90;     // to check laser setup
        }


    trackCenterX = ( leftCornerX + rightCornerX ) / 2;
    trackCenterY = ( leftCornerY + rightCornerY ) / 2;
    //------------------------------------------------------------------------------------


    //----- alarms
    detected = true;

    if ( !primaryLineFound ) {  // no primary line found
        detected = false;
        statusMessage = alarm1;
    }

    if ( abs(angleAvg) <= errorAngleLimit )
        angleInLimit = true;
    else {
        angleInLimit = false;
        detected = false;       // do not accecpt this cam setup, line angle is not in tolerance
        statusMessage = alarm5;
    }

    if ( !centerDetermined )
        detected = false;
    //------------------------------------------------------------------------------------
}


void imgProcess::detectThinJointCenter(int refAngle, int precisionSize){

    thinJointInitSwitch = true;

    //----- prepare slope array
    slope = new float[precisionSize];
    float tangent;
    float minusAngle = -1 * refAngle;
    float plusAngle = refAngle;
    float step = 2 * plusAngle / (precisionSize - 1);

    for ( int i = 0; i < precisionSize; i++ ) {

        tangent = tan ( R2D * ( 90 + minusAngle + i * step ) );

        if (tangent < 573.0 && tangent > -573.0)     //89.9 - 90.1 degree
            slope[i] = tangent;
        else if ( tangent <= -573)
            slope[i] = -573;
        else
            slope[i] = 573.0;
    }
    //------------------------------------------------------------------------------------



    // scan image for all angles with c parameters,
    // find the c parameters of each angle according to minimum cost: bestLines array
    int sum, x, min, index=0;
    bool accept;
    bestLines = new minCostedLines[precisionSize];

    for (int m = 0; m < precisionSize; m++) {

        lineList.clear();

        for (int c = 0; c < imageWidth; c++){
            sum = 0;

            accept = true;
            for (int y = 0; y < imageHeight; y++){

                //y = slope[m] * x + c;
                if (slope[m] > -573 && slope[m] < 573)
                    x = round ( (float)( y + slope[m] * c ) / slope[m] );
                else
                    x = c;

                if (x < 0 || x >= imageWidth) {
                    accept = false;
                } else {
                    sum += valueMatrix[y][x];
                }

            }

            if (accept) {

                minCostedLines z;
                z.c = c;
                z.cost = sum;
                lineList.append(z);
            }

        }


        // find minimum brightness (max darkness) of the angle in interest
        min = 255 * imageHeight;

        for (int i = 0; i < lineList.size(); i++) {

            if (lineList[i].cost < min){
                index = i;
                min = lineList[i].cost;
            }
        }

        if (lineList.size() != 0){
            bestLines[m].c = lineList[index].c;
            bestLines[m].cost = lineList[index].cost;
        } else {
            bestLines[m].c = 0;
            bestLines[m].cost = min;
        }
    }
    //------------------------------------------------------------------------------------


    // find global minimum with angle & c among bestLines
    min = 255 * imageHeight;
    index = 0;
    for (int m = 0; m < precisionSize; m++){
        if (bestLines[m].cost < min){
            index = m;
            min = bestLines[m].cost;
        }
    }

    slopeBest = slope[index];
    //------------------------------------------------------------------------------------


    // re-scan image with best angle & c combination,
    // get lineList with c && cost information
    lineList.clear();

    for (int c = 0; c < imageWidth; c++){
        sum = 0;

        accept = true;
        for (int y = 0; y < imageHeight; y++){

            //y = slope[m] * x + c;
            if (slopeBest > -573 && slopeBest < 573)
                x = round ( (float)( y + slopeBest * c ) / slopeBest );
            else
                x = c;

            if (x < 0 || x >= imageWidth) {
                accept = false;
            } else {
                sum += valueMatrix[y][x];
            }
        }

        if (accept) {
            minCostedLines z;
            z.c = c;
            z.cost = sum;
            lineList.append(z);
        }
    }
    //------------------------------------------------------------------------------------


    //----- alarm preparetion
    primaryLineFound = detected = centerDetermined = true;


    // ----- find the center of the valley
    int minX = imageWidth;
    int maxX = 0;

    if (lineList.size() >= 2) {
        peakPoints.clear();

        int derivative = 0;
        int startIndex = -1;

        // search for first signed derivative
        for (int i = 0; i < lineList.size() - 1; i++){

            derivative = lineList[i+1].cost - lineList[i].cost;

            if (derivative != 0){
                startIndex = i;
                break;
            }
        }

        // if signed derivative found, search for peak points
        if (startIndex != -1){

            peakPoints.append(startIndex);

            int sign;

            if (derivative < 0)
                sign = -1;
            else
                sign = 1;

            for (int i = startIndex + 1; i < lineList.size() - 1; i++){

                derivative = lineList[i+1].cost - lineList[i].cost;

                if (sign < 0){
                    if (derivative > 0){
                        peakPoints.append(i);
                        sign = 1;
                    }
                } else {
                    if (derivative < 0){
                        peakPoints.append(i);
                        sign = -1;
                    }
                }
            }
            peakPoints.append(lineList.size() - 1);

            // find derivatives of peak points
            int *derivativeArray = new int [ peakPoints.size() ];
            derivativeArray[0] = lineList[ peakPoints[0] ].cost - lineList[ peakPoints[1] ].cost;

            for (int i = 1; i < peakPoints.size(); i++)
                derivativeArray[i] = lineList[ peakPoints[i] ].cost - lineList[ peakPoints[i-1] ].cost;

            int minDerivative = 0, minCostIndex = 0;

            for (int i = 1; i < peakPoints.size(); i++)
                if ( derivativeArray[i] < minDerivative ){
                    minDerivative = derivativeArray[i];
                    minCostIndex = i;
                }

            // calc average brigthness cost and assign valley threshold
            int avgCost = 0;
            for (int i = 0; i < peakPoints.size(); i++)
                avgCost += lineList[ peakPoints[i] ].cost;

            avgCost /= peakPoints.size();

            int threshold = abs(avgCost - lineList[ peakPoints[minCostIndex] ].cost)/2 + lineList[ peakPoints[minCostIndex] ].cost;


            if ( peakPoints.size() >= 3 && minCostIndex > 0 && minCostIndex < (peakPoints.size()-1) ){
                //minX = lineList[ peakPoints[minCostIndex - 1] ].c;
                //maxX = lineList[ peakPoints[minCostIndex + 1] ].c;

                for (int i = peakPoints[minCostIndex]; i < lineList.size(); i++)
                    if ( lineList[i].cost > threshold){
                        maxX = lineList[i].c;
                        break;
                    }

                for (int i = peakPoints[minCostIndex]; i >= 0 ; i--)
                    if ( lineList[i].cost > threshold){
                        minX = lineList[i].c;
                        break;
                    }

                centerC = ( minX + maxX ) / 2;

            } else
                centerC = minX = maxX = lineList[ peakPoints[minCostIndex] ].c;


            //----- calculate track center and corner points
            leftCornerY = rightCornerY = trackCenterY = imageHeight / 2;

            if ( slopeBest > -573 && slopeBest < 573 && slopeBest != 0 ) {
                trackCenterX = round ( (float)( trackCenterY + slopeBest * centerC ) / slopeBest );
                leftCornerX  = round ( (float)( leftCornerY + slopeBest * minX ) / slopeBest );
                rightCornerX  = round ( (float)( rightCornerY + slopeBest * maxX ) / slopeBest );
            } else {
                trackCenterX = centerC;
                leftCornerX = minX;
                rightCornerX = maxX;
            }
            //------------------------------------------------------------------------------------

            delete []derivativeArray;

        } else {
            primaryLineFound = detected = centerDetermined = false;
        }
    } else {
        primaryLineFound = detected = centerDetermined = false;
    }

    /* / find the valley among lineList, cost below some threshold
    deepLines.clear();
    int threshold = bestLines[index].cost * 1.5;
    for(int i = 0; i < lineList.size(); i++)
        if (lineList[i].cost <= threshold)
            deepLines.append(lineList[i]);

    for(int i = 0; i < deepLines.size(); i++){
        if (deepLines[i].c < minX)
            minX = deepLines[i].c;

        if (deepLines[i].c > maxX)
            maxX = deepLines[i].c;
    }
    centerC = (minX + maxX)/2;
    */ // ------------------------------------------------------------------------------------


    //----- alarms
    angleAvg = 90 + minusAngle + index * step;
    angleAvg = 180 - angleAvg;  // since image is mirrored
    if ( abs(angleAvg) <= (90 + errorAngleLimit) )
        angleInLimit = true;
    else {
        angleInLimit = false;
        detected = false;       // do not accecpt this cam setup, line angle is not in tolerance
        statusMessage = alarm5;
    }
    //------------------------------------------------------------------------------------
}


void imgProcess::detectContrastCenter(){

    detected = true;

    int y = imageHeight/2;

    trackCenterX = centerX + getLineX((centerY - y), distanceAvg, thetaAvg);
    rightCornerX = centerX + getLineX((centerY - y), major2Lines[0].distance, major2Lines[0].angle);
    leftCornerX = centerX + getLineX((centerY - y), major2Lines[1].distance, major2Lines[1].angle);

    trackCenterY = rightCornerY = leftCornerY = y;

    //----- alarms
    angleAvg = 90 + thetaAvg;

    if ( abs(angleAvg) <= (90 + errorAngleLimit) )
        angleInLimit = true;
    else {
        angleInLimit = false;
        detected = false;       // do not accecpt this cam setup, line angle is not in tolerance
        statusMessage = alarm5;
    }
    //------------------------------------------------------------------------------------

}


void imgProcess::detectMainEdges(bool thinjoint, bool DEBUG){

    // sort wrt distance low > high
    sortHoughLines_toDistance( houghLineNo );
        // ** HOUGH DATA > SORTED HOUGH DATA WRT DISTANCE

    // 1ST ITERATION
    int *valArray = new int[houghLineNo];
    for (int i = 0; i < houghLineNo; i++) valArray[i] = houghLinesSorted[i][2];     // votes

    QList<range> localMaximalist;

    findLocalMinimum(valArray, houghLineNo, localMaximalist);
        // ** SORTED HOUGH DATA > localMaximalist
    localMaximaSize = localMaximalist.size();


    if ( !localMaximalist.isEmpty() ){

        if (DEBUG) {
            rangeArray = new int*[localMaximaSize];
            for (int i = 0; i < localMaximaSize; i++)    rangeArray[i] = new int[2];
            rangeArrayInitSwitch = true;
            for (int i = 0; i < localMaximaSize; i++){
                rangeArray[i][0] = localMaximalist[i].start;
                rangeArray[i][1] = localMaximalist[i].end;
            }
        }

        // get hough data of local maximas
        // if there are same distances in same range, obtain one hough data with averaged angle
        QList<houghData> listHoughData;

        int *distArray = new int[houghLineNo];

        for (int i = 0; i < houghLineNo; i++)   distArray[i] = houghLinesSorted[i][0];

        int refValue, compValue, c;
        int flag = -1;
        float angleAvg;

        for (int i = 0; i < localMaximalist.size(); i++) {

            c = localMaximalist[i].start;

            do {

                QList<int> indexList;
                indexList.empty();
                refValue = distArray[c];
                indexList.append(c);
                distArray[c] = flag;

                for (int j = c+1; j <= localMaximalist[i].end; j++){
                    compValue = distArray[j];
                    if (refValue == compValue && compValue != flag) {
                        indexList.append(j);
                        distArray[j] = flag;
                    }
                }

                angleAvg = 0;
                if (indexList.size() == 1){     // if there is not equality
                    angleAvg = houghLinesSorted[ indexList.first() ][1];
                    c++;
                } else {
                    for (int k = 0; k < indexList.size(); k++)
                        angleAvg += houghLinesSorted[ indexList[k] ][1];

                    angleAvg = angleAvg / indexList.size();
                    c = indexList.last()+1;
                }

                houghData hd;
                hd.distance = houghLinesSorted[ indexList.first() ][0];
                hd.angle = angleAvg;
                hd.voteValue = houghLinesSorted[ indexList.first() ][2];
                listHoughData.append(hd);

                indexList.empty();

            } while (c <= localMaximalist[i].end);
                // ** localMaximalist > listHoughData

            listHoughDataSize = listHoughData.size();

            if (DEBUG) {
                listHoughDataArray = new int*[listHoughDataSize];
                for (int i = 0; i < listHoughDataSize; i++)    listHoughDataArray[i] = new int[3];
                listHoughDataArrayInitSwitch = true;
                for (int i = 0; i < listHoughDataSize; i++){
                    listHoughDataArray[i][0] = listHoughData[i].distance;
                    listHoughDataArray[i][1] = listHoughData[i].angle;
                    listHoughDataArray[i][2] = listHoughData[i].voteValue;
                }
            }
        }
        //------------------------------------------------------------------


        // 2ND ITERATION

        // get votes from hough data list
        int *houghDataVotes = new int[listHoughData.size()];
        for (int z = 0; z < listHoughData.size(); z++)
            houghDataVotes[z] = listHoughData[z].voteValue;

        QList<range> localMaximalist2nd;

        findLocalMinimum(houghDataVotes, listHoughData.size(), localMaximalist2nd);
            // ** listHoughData > localMaximalist2nd
        localMaxima2ndSize = localMaximalist2nd.size();

        if (DEBUG) {
            rangeArray2nd = new int*[localMaxima2ndSize];
            for (int i = 0; i < localMaxima2ndSize; i++)    rangeArray2nd[i] = new int[2];
            rangeArray2ndInitSwitch = true;
            for (int i = 0; i < localMaxima2ndSize; i++){
                rangeArray2nd[i][0] = localMaximalist2nd[i].start;
                rangeArray2nd[i][1] = localMaximalist2nd[i].end;
            }
        }

        // get hough datas of 2nd iter. local maximas
        QList<houghData> listHoughData2nd;

        for (int t = 0; t < localMaximalist2nd.size(); t++)
            for (int ts = localMaximalist2nd[t].start; ts <= localMaximalist2nd[t].end; ts++){
                houghData hd;
                hd.distance = listHoughData[ ts ].distance;
                hd.angle = listHoughData[ ts ].angle;
                hd.voteValue = listHoughData[ ts ].voteValue;
                listHoughData2nd.append(hd);
            }
        // ** localMaximalist2nd > listHoughData2nd

        listHoughData2ndSize = listHoughData2nd.size();

        if (DEBUG) {
            listHoughData2ndArray = new int*[listHoughData2ndSize];
            for (int d = 0; d < listHoughData2ndSize; d++)    listHoughData2ndArray[d] = new int[3];
            listHoughData2ndArrayInitSwitch = true;

            for (int d = 0; d < listHoughData2ndSize; d++){
                listHoughData2ndArray[d][0] = listHoughData2nd[d].distance;
                listHoughData2ndArray[d][1] = listHoughData2nd[d].angle;
                listHoughData2ndArray[d][2] = listHoughData2nd[d].voteValue;
            }
        }


        if (!listHoughData2nd.isEmpty()){

            // SELECT MAIN EDGES

            QList<houghData> mainEdgesList;

            if ( !thinjoint ){
                //
                // WIDE JOINT
                //
                // APPLY MEAN VALUE THRESHOLD

                int sum = 0;

                for (int i = 0; i < listHoughData2nd.size(); i++)
                    sum += listHoughData2nd[i].voteValue;

                int meanValue = sum / listHoughData2nd.size();

                for (int i = 0; i < listHoughData2nd.size(); i++)
                    if ( listHoughData2nd[i].voteValue > meanValue ){
                        houghData hd;
                        hd.distance =  listHoughData2nd[i].distance;
                        hd.angle =  listHoughData2nd[i].angle;
                        hd.voteValue =  listHoughData2nd[i].voteValue;
                        mainEdgesList.append(hd);

                }

            } else {
                //
                // THIN JOINT
                //
                // SELECT 2 MAXS
                //

                int size = 2;

                if ( listHoughData2nd.size() < 2 )
                    size = listHoughData2nd.size();

                int max, index;

                for (int i = 0; i < size; i++){
                    max = -1, index = 0;
                    for (int j = 0; j < listHoughData2nd.size(); j++)
                        if (listHoughData2nd[j].voteValue > max){
                            max = listHoughData2nd[j].voteValue;
                            index = j;
                        }

                    houghData hd;
                    hd.distance =  listHoughData2nd[index].distance;
                    hd.angle =  listHoughData2nd[index].angle;
                    hd.voteValue =  listHoughData2nd[index].voteValue;
                    mainEdgesList.append(hd);
                    listHoughData2nd[index].voteValue = -1;
                }

            }



            detected = true;

            int yCoor = imageHeight/2;

            if ( !mainEdgesList.isEmpty() ){

                QList<int> xCoors;

                for (int i = 0; i < mainEdgesList.size(); i++){
                    int xCoor = centerX + 1 + getLineX((centerY + 1 - yCoor), mainEdgesList[i].distance, mainEdgesList[i].angle);
                    if (xCoor >= 0 && xCoor < imageWidth)
                        xCoors.append( xCoor );
                }

                int max = -1, min = 1000;

                if ( !xCoors.isEmpty() ){

                    for (int i = 0; i < xCoors.size(); i++){
                        if ( xCoors[i] > max)
                            max = xCoors[i];
                        if ( xCoors[i] < min)
                            min = xCoors[i];
                    }

                    trackCenterX = (max + min) / 2;
                    rightCornerX = max;
                    leftCornerX = min;

                } else {

                    trackCenterX = rightCornerX = leftCornerX = imageWidth / 2;     // DETECTION ERROR

                }

                xCoors.empty();

            } else {

                trackCenterX = rightCornerX = leftCornerX = imageWidth / 2;     // DETECTION ERROR

            }   // if ( !mainEdgesList.isEmpty() )


            trackCenterY = rightCornerY = leftCornerY = yCoor;


            if (DEBUG) {
                codeLineData(valueMatrix, imageWidth, imageHeight, mainEdgesList, true);
            }

            //----- alarms
            //------------------------------------------------------------------------------------

            // clear vars
            mainEdgesList.empty();

        } else {

            trackCenterX = rightCornerX = leftCornerX = imageWidth / 2;     // DETECTION ERROR

        }   // if (!localMaximalist2nd.isEmpty())


        // clear vars
        delete distArray;
        delete houghDataVotes;
        listHoughData.empty();
        localMaximalist2nd.empty();
        listHoughData2nd.empty();

    } else {

        trackCenterX = rightCornerX = leftCornerX = imageWidth / 2;     // DETECTION ERROR

    }   // if ( !localMaximalist.isEmpty() )

    // clear vars
    delete valArray;
    localMaximalist.empty();
}


QImage* imgProcess::getImage(int **matrix, int width, int height, QImage::Format format){

    QImage *image = new QImage(width, height, format);
    QRgb value;

    for(int y = 0; y < height; y++)
        for(int x = 0; x < width; x++){
            if (matrix[y][x] == 2555)
                value = qRgb(255, 0, 0);    // red for special data
            else
                value = qRgb(matrix[y][x], matrix[y][x], matrix[y][x]);
            image->setPixel(x, y, value);
        }
    return image;
}


QImage* imgProcess::getImage(bool **matrix, int width, int height, QImage::Format format){

    QImage *image = new QImage(width, height, format);

    for(int y = 0; y < height; y++)
        for(int x = 0; x < width; x++){
            if (matrix[y][x])
                image->setPixel(x, y, qRgb(255, 255, 255));
        }

    return image;
}


int imgProcess::getLineY(int x, float distance, float theta){

    int y = -1;
    float sine = sin(theta*R2D);
    if (sine >= 0.1)
        y = (int) ( (distance - x*cos(theta*R2D)) / sine );
    return y;
}


int imgProcess::getLineX(int y, float distance, float theta){

    int x = -1;
    float cosine = cos(theta*R2D);
    if (cosine != 0)
        x = (int) ( (distance - y*sin(theta*R2D)) / cosine );
    return x;
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


void imgProcess::findMedianValue(){

    loValue = 400;
    hiValue = -1;

    for(int y = 0; y < imageHeight; y++)
        for(int x = 0; x < imageWidth; x++) {
            if ( valueMatrix[y][x] < loValue )
                loValue = valueMatrix[y][x];
            if ( valueMatrix[y][x] > hiValue )
                hiValue = valueMatrix[y][x];
        }

    //medianValue = (min + max) / 2;
    int delta = hiValue - loValue + 1;
    medianValue = loValue + delta / 2;

    tLo = ( loValue + 0.66 * delta / 2 ) * 100  / 255.0;
    tHi = ( loValue + 1.33 * delta / 2 ) * 100  / 255.0;
}


QImage imgProcess::cornerImage(){

    imgCorner = imgOrginal.copy();

    if (detected){
        QRgb value;
        value = qRgb(0, 255, 0);        // green

        int X, Y;

        for (int x = -4; x <= 4; x++){

            X = leftCornerX + x;
            Y = leftCornerY;
            if ( X >= 0 && X < imgCorner.width() && Y >= 0 && Y < imgCorner.height())
                imgCorner.setPixel( X, Y, value );

            X = rightCornerX + x;
            Y = rightCornerY;
            if ( X >= 0 && X < imgCorner.width() && Y >= 0 && Y < imgCorner.height())
                imgCorner.setPixel( X, Y, value );
        }

        for (int y = -4; y <= 4; y++){

            X = leftCornerX;
            Y = leftCornerY + y;
            if ( X >= 0 && X < imgCorner.width() && Y >= 0 && Y < imgCorner.height())
                imgCorner.setPixel( X, Y, value);

            X = rightCornerX;
            Y = rightCornerY + y;
            if ( X >= 0 && X < imgCorner.width() && Y >= 0 && Y < imgCorner.height())
                imgCorner.setPixel( X, Y, value);

            X = trackCenterX;
            Y = trackCenterY + y;
            if ( X >= 0 && X < imgCorner.width() && Y >= 0 && Y < imgCorner.height())
                imgCorner.setPixel( X, Y, value);
        }
    }
    return imgCorner;
}


QImage imgProcess::cornerAndPrimaryLineImage( solidLine line1, solidLine line2, int line2offset ){

    imgCornerAndPrimaryLines = imgOrginal.copy();

//    if (detected){
        QRgb valueCorner, valuePrimary;

        // draw primary lines
        valuePrimary = qRgb(255, 0, 0);     // red
        int lineY;

        if ( line1.distance > 0 ) {

            for (int x = line1.start.x(); x <= line1.end.x(); x++){
                lineY = centerY - getLineY((x - centerX), line1.distance, line1.angle);

                if ( x >= 0 && x < imgCornerAndPrimaryLines.width() && lineY >= 0 && lineY < imgCornerAndPrimaryLines.height())
                    imgCornerAndPrimaryLines.setPixel( x, lineY, valuePrimary );
            }
        }

        if ( line2.distance > 0 ) {

            int startX = line2offset + line2.start.x();
            int endX = line2offset + line2.end.x();

            for (int x = startX; x <= endX; x++){
                lineY = centerY - getLineY((x - centerX), line2.distance, line2.angle);

                if ( x >= 0 && x < imgCornerAndPrimaryLines.width() && lineY >= 0 && lineY < imgCornerAndPrimaryLines.height())
                    imgCornerAndPrimaryLines.setPixel( x, lineY, valuePrimary );
            }
        }


        // draw corners and center
        valueCorner = qRgb(0, 255, 0);        // green
        int X, Y;

        for (int x = -4; x <= 4; x++){

            X = leftCornerX + x;
            Y = leftCornerY;
            if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height())
                imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );

            X = rightCornerX + x;
            Y = rightCornerY;
            if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height())
                imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );
        }

        for (int y = -4; y <= 4; y++){

            X = leftCornerX;
            Y = leftCornerY + y;
            if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height())
                imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );

            X = rightCornerX;
            Y = rightCornerY + y;
            if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height())
                imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );

            X = trackCenterX;
            Y = trackCenterY + y;
            if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height())
                imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );
        }
//    }
    return imgCornerAndPrimaryLines;
}


QImage imgProcess::drawLines(){

    QImage image = imgOrginal.copy();

    QRgb rgbRed;

    rgbRed = qRgb(255, 0, 0);     // red

    for(int y = 0; y < image.height(); y++)
        for(int x = 0; x < image.width(); x++){
            if (valueMatrix[y][x] == 2555)
                image.setPixel(x, y, rgbRed);
        }

    return image;
}


QImage imgProcess::drawLines(minCostedLines *lineArray, int size){

    QImage image = imgOrginal.copy();

    QRgb rgbRed;

    rgbRed = qRgb(255, 0, 0);     // red
    int x;

    for (int m = 0; m < size; m++){

        for (int y = 0; y < imageHeight; y++){
            if (m != 10)
                x = lineArray[m].c - round ( (y - lineArray[m].c) / tan ( R2D * (90+(m-10)/4.0) ) );
            else
                x = lineArray[m].c;

            if (x >= 0 && x < imageWidth)
                image.setPixel(x, y, rgbRed);
        }

    }

    return image;
}


QImage imgProcess::drawLine(minCostedLines *line, float tangent){

    QImage image = imgOrginal.copy();

    QRgb rgbRed;

    rgbRed = qRgb(255, 0, 0);     // red
    int x;


    for (int y = 0; y < imageHeight; y++){
        if (tangent != 0)
            x = round ( (float)( y + tangent * line->c) / tangent );
        else
            x = line->c;

        if (x >= 0 && x < imageWidth)
            image.setPixel(x, y, rgbRed);
    }

    return image;
}


QImage* imgProcess::getImage_cannyThresholds( QImage::Format format ){

    QImage *image = new QImage(edgeWidth, edgeHeight, format);
    QRgb red = qRgb(255, 0, 0);
    QRgb blue = qRgb(0, 0, 255);

    for(int y = 0; y < edgeHeight; y++)
        for(int x = 0; x < edgeWidth; x++)
            if ( edgeStrongMatrix[y][x] > 0 )
                image->setPixel(x, y, red);
            else if ( edgeWeakMatrix[y][x] > 0 )
                image->setPixel(x, y, blue);

    return image;
}


QImage* imgProcess::getImage_cannyTracedEdges( QImage::Format format ){

    QImage *image = new QImage(edgeWidth, edgeHeight, format);
    QRgb red = qRgb(255, 0, 0);
    QRgb blue = qRgb(0, 0, 255);
    QRgb black = qRgb(0, 0, 0);

    for(int y = 0; y < edgeHeight; y++)
        for(int x = 0; x < edgeWidth; x++)
            if ( edgeStrongMatrix[y][x] > 0 ){
                if ( !edgeW2SMapMatrix[y][x] )
                    image->setPixel(x, y, red);
                else
                    image->setPixel(x, y, blue);
            } else
                image->setPixel(x, y, black);

    return image;
}


imgProcess::~imgProcess(){

    for (int y = 0; y < imageHeight; y++) delete []valueMatrix[y];
    delete []valueMatrix;

    for (int y = 0; y < edgeHeight; y++) delete []edgeMatrix[y];
    delete []edgeMatrix;

    for (int y = 0; y < imageHeight; y++) delete []edgeThickenedMatrix[y];
    delete []edgeThickenedMatrix;

    for (int y = 0; y < edgeHeight; y++) delete []houghMatrix[y];
    delete []houghMatrix;

    for (int y = 0; y < imageHeight; y++) delete []houghExtendedMatrix[y];
    delete []houghExtendedMatrix;

    if ( houghSpaceInitSwitch ) {
        for (int y = 0; y < houghDistanceMax; y++) delete []houghSpace[y];
        delete []houghSpace;
    }

    if ( houghLinesInitSwitch ) {
        for (int y = 0; y < houghLineNo; y++) delete []houghLines[y];
        delete []houghLines;
    }

    if ( histogramInitSwitch ) {
        delete []histogram;
    }

    if ( voidSpace.size() > 0 ) {
        for (int i = 0; i < voidSpace.size(); i++) delete voidSpace[i];
        voidSpace.clear();
    }

    if ( majorList.size() > 0 ) {
        for (int i = 0; i < majorList.size(); i++) delete majorList[i];
        majorList.clear();
    }

    if ( solidSpaceMain.size() > 0 ) solidSpaceMain.clear();
    if ( solidSpaceMainTrimmed.size() > 0 ) solidSpaceMainTrimmed.clear();
    if ( solidSpaceMainMaximums.size() > 0 ) solidSpaceMainMaximums.clear();
    if ( solidSpaceMainOrdered.size() > 0 ) solidSpaceMainOrdered.clear();
    if ( majorLines.size() > 0 ) majorLines.clear();
    if ( major2Lines.size() > 0 ) major2Lines.clear();

    if ( thinJointInitSwitch ) {
        delete []slope;
        delete []bestLines;
        lineList.clear();
        deepLines.clear();
        peakPoints.clear();
    }

    if ( contrastInitSwitch ) {
        for (int y = 0; y < imageHeight; y++) delete []contrastMatrix[y];
        delete []contrastMatrix;
    }

    if ( houghLinesSortedInitSwitch ) {
        for (int y = 0; y < houghLinesSorted_size; y++) delete []houghLinesSorted[y];
        delete []houghLinesSorted;
    }

    if ( rangeArrayInitSwitch ) {
        for (int y = 0; y < localMaximaSize; y++) delete []rangeArray[y];
        delete []rangeArray;
    }

    if ( listHoughDataArrayInitSwitch ) {
        for (int y = 0; y < listHoughDataSize; y++) delete []listHoughDataArray[y];
        delete []listHoughDataArray;
    }

    if ( rangeArray2ndInitSwitch ) {
        for (int y = 0; y < localMaxima2ndSize; y++) delete []rangeArray2nd[y];
        delete []rangeArray2nd;
    }

    if ( listHoughData2ndArrayInitSwitch ) {
        for (int y = 0; y < listHoughData2ndSize; y++) delete []listHoughData2ndArray[y];
        delete []listHoughData2ndArray;
    }

    if ( rangeArray3rdInitSwitch ) {
        for (int y = 0; y < localMaxima3rdSize; y++) delete []rangeArray3rd[y];
        delete []rangeArray3rd;
    }

    if ( listHoughData3rdArrayInitSwitch ) {
        for (int y = 0; y < listHoughData3rdSize; y++) delete []listHoughData3rdArray[y];
        delete []listHoughData3rdArray;
    }

    if ( listHoughData3rdFilteredArrayInitSwitch ) {
        for (int y = 0; y < listHoughData3rdFilteredSize; y++) delete []listHoughData3rdFilteredArray[y];
        delete []listHoughData3rdFilteredArray;
    }

    if ( edgeGradientMatrixInitSwitch ) {
        for (int y = 0; y < edgeHeight; y++) delete []edgeGradientMatrix[y];
        delete []edgeGradientMatrix;
    }

    if ( edgeStrongMatrixInitSwitch ) {
        for (int y = 0; y < edgeHeight; y++) delete []edgeStrongMatrix[y];
        delete []edgeStrongMatrix;
    }

    if ( edgeWeakMatrixInitSwitch ) {
        for (int y = 0; y < edgeHeight; y++) delete []edgeWeakMatrix[y];
        delete []edgeWeakMatrix;
    }

    if ( edgeVisitMatrixInitSwitch ) {
        for (int y = 0; y < edgeHeight; y++) delete []edgeVisitMatrix[y];
        delete []edgeVisitMatrix;
    }

    if ( edgeMapMatrixInitSwitch ) {
        for (int y = 0; y < edgeHeight; y++) delete []edgeMapMatrix[y];
        delete []edgeMapMatrix;
    }

    if ( edgeSuppressedMatrixInitSwitch ) {
        for (int y = 0; y < edgeHeight; y++) delete []edgeSuppressedMatrix[y];
        delete []edgeSuppressedMatrix;
    }

    if ( edgeW2SMapMatrixInitSwitch ) {
        for (int y = 0; y < edgeHeight; y++) delete []edgeW2SMapMatrix[y];
        delete []edgeW2SMapMatrix;
    }


    if ( edgeMapValueMatrixInitSwitch ) {
        for (int y = 0; y < edgeHeight; y++) delete []edgeMapValueMatrix[y];
        delete []edgeMapValueMatrix;
    }

    if ( edgeMapRedMatrixInitSwitch ) {
        for (int y = 0; y < edgeHeight; y++) delete []edgeMapRedMatrix[y];
        delete []edgeMapRedMatrix;
    }

    if ( edgeMapGreenMatrixInitSwitch ) {
        for (int y = 0; y < edgeHeight; y++) delete []edgeMapGreenMatrix[y];
        delete []edgeMapGreenMatrix;
    }

    if ( edgeMapBlueMatrixInitSwitch ) {
        for (int y = 0; y < edgeHeight; y++) delete []edgeMapBlueMatrix[y];
        delete []edgeMapBlueMatrix;
    }

}






/*
// DETECTION FUNCTION BASED ON SOLID LINES
void imgProcess::detectLongestSolidLines(){

    solidSpaceMain.clear();

    float angle = 0;

//    houghDistanceMax = (int) (sqrt(pow(edgeWidth, 2) + pow(edgeHeight, 2)));
    float angleSize = (int) ((thetaMax - thetaMin) / thetaStep) + 1;
//------------------------------------------------------------------------------------
    for (int distance = 0; distance < edgeHeight; distance++)
        for (int angleIndex = 0; angleIndex < angleSize; angleIndex++){
            angle = thetaMin + angleIndex * thetaStep;
            solidSpaceMain.append( detectLongestSolidLine( distance, angle, false ) );  // in edge thickened matrix
        }
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
    // find max length

    float globalMaxLength = 0;
    for (int x = 0; x < solidSpaceMain.size(); x++)
        if ( solidSpaceMain[x].length > globalMaxLength )
            globalMaxLength = solidSpaceMain[x].length;
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
    // remove lines w/ length behind threshold      //remove no lines

    solidSpaceMainTrimmed.clear();
    float thresholdLength = globalMaxLength * 0.5;  // % 50 of global maximum

    for (int i = 0; i < solidSpaceMain.size(); i++)
        //if ( solidSpaceMain[i].length != -1 )
        if ( solidSpaceMain[i].length > thresholdLength )
            solidSpaceMainTrimmed.append( solidSpaceMain[i] );
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
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

            // group equal distances in same group (buffer)
            for (index = currentStart; index < size; index++){
                if (solidSpaceMainTrimmed[index].distance == currentDistance)
                    buffer.append( solidSpaceMainTrimmed[index] );
                else
                    break;
            }

            // loop end or next iteration assignments
            // solidSpaceMain is ordered by ascending distance values, so the solidSpaceMainTrimmed
            // therefore, currentDistance change will be executed in next iteration
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

                // detect indexes of equal maximum points, in case of more than 1 max. point
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
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
    // find major areas
    majorThresholdPercent = 0.8;   // %50

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

        if ( solidSpaceMainMaximums[i].length > majorThreshold && !areaStartFlag ) {
            areaStartFlag = true;
            area = new majorArea();
            area->startIndex = i;
        }

        if ( (solidSpaceMainMaximums[i].length < majorThreshold || (i == solidSpaceMainMaximums.size() - 1 )) && areaStartFlag ) {
            areaStartFlag = false;
            area->endIndex = i - 1;
            majorList.append( area );
        }
    }
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
    // find major lines (max length, avg if more than 1 equal max) of areas

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
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
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
        // detect max 2 of more

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
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
    // calculate average distance and angle value of 2 major line = PRIMARY LINE
    // construct primary line object to find coordinates

    if (majorLinesFound) {

        // *
        // weighted average
        int sum = major2Lines[0].length + major2Lines[1].length;

        if ( sum != 0 ){
            distanceAvg = (major2Lines[0].distance * major2Lines[0].length + major2Lines[1].distance * major2Lines[1].length) / sum;
            thetaAvg = (major2Lines[0].angle * major2Lines[0].length + major2Lines[1].angle * major2Lines[1].length ) / sum;
        } else {
            distanceAvg  = 0;
            thetaAvg = 0;
        }
        * /

        // classical average
        distanceAvg = (major2Lines[0].distance + major2Lines[1].distance ) / 2;
        thetaAvg = (major2Lines[0].angle + major2Lines[1].angle ) / 2;

        // MAIN DETECTION OF JUNCTION; true; value, false: edge thickened
        primaryLine = detectLongestSolidLine( distanceAvg, thetaAvg , true);    // in value matrix
    } else {
        // no solid line
        primaryLine.start.setX( -1 );
        primaryLine.start.setY( -1 );
        primaryLine.end.setX( -1 );
        primaryLine.end.setY( -1 );
        primaryLine.length = -1;
        primaryLine.distance = -1;
        primaryLine.angle = -1;

        distanceAvg = -1;
        thetaAvg = -1;
    }
//------------------------------------------------------------------------------------


}
*/

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
