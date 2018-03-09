#include "imgprocess.h"
//#include <QImage>
//#include <QColor>
#include <QFile>
#include <QTextStream>
#include "math.h"
#include <cmath>
#include <QDebug>
#include <QtCore/qmath.h>

#include "imgprocess_msg.h"
#include "../_Modules/Algo/localMinimum.h"
#include "../_Modules/Algo/natural_breaks.h"

#include <iostream>
using namespace std;

imgProcess::imgProcess(){
}

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
            valueMatrixOrg[y][x] = colorValue;
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
            valueMatrixOrg[y][x] = colorValue;
            delete color;
        }
}


void imgProcess::constructValueBlackMatrix(QImage image){

    QRgb rgbValue;
    QColor *color;
    int colorValue;
    int c, m, yy, k, al;

    for (int y = 0; y < image.height(); y++)
        for (int x = 0; x < image.width(); x++){
            rgbValue = image.pixel(x,y);
            color = new QColor(rgbValue);
            //color->getCmyk(&c,&m,&yy,&k,&al);
            colorValue = color->black();

            if ( colorValue > 255) colorValue = 255;
            else if (colorValue < 0)  colorValue = 0;

            valueMatrix[y][x] = colorValue;
            valueMatrixOrg[y][x] = colorValue;
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


void imgProcess::normalizeValueMatrix(double factor){

    valueMatrixNorm = new double*[imageHeight];
    for (int j = 0; j < imageHeight; j++)   valueMatrixNorm[j] = new double[imageWidth];
    valueMatrixNormInitSwitch = true;

    for (int y = 0; y < imageHeight; y++)
        for (int x = 0; x < imageWidth; x++){
            valueMatrixNorm[y][x] = valueMatrix[y][x] / factor;
        }
}


float imgProcess::gaussianFn(int x,int y, float stddev){

    if (stddev != 0)
        return exp( (pow(x,2)+pow(y,2))/(-2*pow(stddev,2)) ) / (2*M_PI*pow(stddev,2));
    else
        return 0;
}

void imgProcess::constructGaussianMatrix(int size, float _stddev){

    gaussianMatrixSize = size;
    stdDev = _stddev;

    gaussianMatrix = new float*[gaussianMatrixSize];
    for (int i = 0; i < gaussianMatrixSize; i++) gaussianMatrix[i] = new float[gaussianMatrixSize];
    gaussianMatrixInitSwitch =true;

    int center = (gaussianMatrixSize-1)/2;

    float gaussianMatrixSum = 0;
    for (int x = 0; x < gaussianMatrixSize; x++)
        for (int y = 0; y < gaussianMatrixSize; y++){
            gaussianMatrix[x][y] = gaussianFn(x-center, y-center, stdDev);
            gaussianMatrixSum += gaussianMatrix[x][y];
        }

    /*
    cout << "Sum= " << gaussianMatrixSum << endl;
    for (int x = 0; x < gaussianMatrixSize; x++){
        for (int y = 0; y < gaussianMatrixSize; y++)
            cout << gaussianMatrix[x][y] << " : ";
        cout << endl;
    }
    */
}

void imgProcess::gaussianBlurFixed(){

    int sum;

    for (int y = 2; y < imageHeight - 2; y++){

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

void imgProcess::gaussianBlur(){

    int center = (gaussianMatrixSize-1)/2;
    float sum;

    for (int y = center; y < imageHeight - center; y++){
        for (int x = center; x < imageWidth - center; x++) {
            sum = 0;
            for (int i = 0; i < gaussianMatrixSize; i++)
                for (int j = 0; j < gaussianMatrixSize; j++)
                    sum += gaussianMatrix[i][j] * valueMatrixOrg[y-center+i][x-center+j];
                    //sum += gaussianMask[i][j] * valueMatrix[y-center+i][x-center+j];

            if (gaussianMatrixSum != 0)
                valueMatrix[y][x] = (int) (sum / gaussianMatrixSum);

            if (valueMatrix[y][x] > 255)        valueMatrix[y][x] = 255;
            else if (valueMatrix[y][x] < 0)     valueMatrix[y][x] = 0;
        }
    }

    //to flaten boundary noise except corners
    for (int y = 0; y < imageHeight; y++)
        for (int x = 0; x < imageWidth; x++){
            if ( x<center && (y>=center || y <(imageHeight-center)) )
                valueMatrix[y][x] = valueMatrix[y][center];
            if ( x>=(imageWidth-center) && (y>=center || y <(imageHeight-center)) )
                valueMatrix[y][x] = valueMatrix[y][imageWidth-center-1];

            if ( y<center && (x>=center || x <(imageWidth-center)) )
                valueMatrix[y][x] = valueMatrix[center][x];
            if ( y>=(imageHeight-center) && (x>=center || x <(imageWidth-center)) )
                valueMatrix[y][x] = valueMatrix[imageHeight-center-1][x];
        }

/*corners
 *             if ( (x<center || x>=(imageWidth-center)) &&
                 (y<center || y>=(imageHeight-center)) )*/

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


bool imgProcess::saveMatrix(double **matrix, int width, int height, QString fname){

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

bool imgProcess::saveArray(double *array, int length, QString fname){

    QFile file(fname);
    bool saveStatus = true;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);

        for(int i = 0; i < length; i++) out << array[i] << "\n";
        file.close();
    } else saveStatus = false;

    return saveStatus;
}

bool imgProcess::saveMinCostedLinesArray(minCostedLines *array, int length, QString fname){

    QFile file(fname);
    bool saveStatus = true;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);

        out << "c,cost" << "\n";
        for(int i = 0; i < length; i++)
            out << array[i].c << "," << array[i].cost << "\n";
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

bool imgProcess::saveList(QList<double> array, QString fname){

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

bool imgProcess::saveMinCostedLinesList(QList<minCostedLines> list, QString fname){

    QFile file(fname);
    bool saveStatus = true;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);

        out << "c,cost\n";

        for(int i = 0; i < list.size(); i++)
            out << list[i].c << "," << list[i].cost << "\n";
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


void imgProcess::nonMaximumSuppression(bool suppress){

    for (int y = 0;y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            edgeSuppressedMatrix[y][x] = edgeMatrix[y][x];

    if (suppress) {
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


void imgProcess::thickenEdgeMap(int diameter){

    int xnStart, xnEnd, ynStart, ynEnd;

    for (int y = 0; y < imageHeight; y++)
        for (int x = 0; x < imageWidth; x++)
            edgeThickenedMatrix[y][x] = 0;


    for (int y = 1; y < (imageHeight - 1); y++) {

        ynStart = y - diameter;
        ynEnd = y + diameter;
        if (ynStart < 0) ynStart = 0;
        if (ynEnd >= edgeHeight) ynEnd = edgeHeight;

        for (int x = 1; x < (imageWidth - 1); x++){

            xnStart = x - diameter;
            xnEnd = x + diameter;
            if (xnStart < 0) xnStart = 0;
            if (xnEnd >= edgeWidth) xnEnd = edgeWidth;

            if ( edgeMapMatrix[y - 1][x - 1] )
                for (int xn = xnStart; xn <= xnEnd; xn++)
                    for (int yn = ynStart; yn <= ynEnd; yn++)
                       edgeThickenedMatrix[yn][xn] = 255;
        }
    }

    for (int y = 1; y < imageHeight - 1; y++)
        for (int x = 1; x < imageWidth - 1; x++)
            if (edgeThickenedMatrix[y][x] == 255)
                edgeMapMatrix[y - 1][x - 1] = true;
            else
                edgeMapMatrix[y - 1][x - 1] = false;

}


void imgProcess::scoreLineCrossing(bool orientation){

    // orientation
    // true: vertical line
    // false: horizontal line

    if (mainEdgesList.size() != 0) {

        mainEdgeScore = 0;

        if (orientation) {

            int lineX;

            for (int y = 0; y < edgeHeight; y++){
                lineX = centerX + getLineX((y - centerY), mainEdgesList[0].distance, mainEdgesList[0].angle);

                if (lineX >= 0 && lineX < edgeWidth)
                    if ( edgeMapMatrix[y][lineX]) mainEdgeScore++;
            }

            mainEdgeScorePercent = abs ( mainEdgeScore * sin (R2D * (90 - mainEdgesList[0].angle)) * 100.0 / edgeHeight );

        } else {

            int lineY;

            for (int x = 0; x < edgeWidth; x++){
                lineY = centerY - getLineY((x - centerX), mainEdgesList[0].distance, mainEdgesList[0].angle);

                if (lineY >= 0 && lineY < edgeHeight)
                    if ( edgeMapMatrix[lineY][x]) mainEdgeScore++;

            }

            mainEdgeScorePercent = abs ( mainEdgeScore * cos (R2D * (90 - mainEdgesList[0].angle)) * 100.0 / edgeWidth );

        }

    }


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
void imgProcess::houghTransformFn(int **matrix, int width, int height){

    houghDistanceMax = (int) (sqrt(pow(width, 2) + pow(height, 2)));

    houghThetaSize = (int) ((thetaMax - thetaMin) / thetaStep) + 1;

    houghSpace = new int*[houghDistanceMax];
    for (int i = 0; i < houghDistanceMax; i++)   houghSpace[i] = new int[houghThetaSize];
    houghSpaceInitSwitch = true;

    for (int y = 0; y < houghDistanceMax; y++)
        for (int x = 0; x < houghThetaSize; x++) houghSpace[y][x] = 0;

    int distance, theta;
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            if (matrix[y][x] != 0)
                for (int i = 0; i < houghThetaSize; i++){
                    theta = thetaMin + i * thetaStep;
//                    distance = (int) ((x - centerX) * cos(theta * R2D) + (centerY - y) * sin(theta * R2D));
                    distance = (int) ((x - centerX) * cos(theta * R2D) + (y - centerY) * sin(theta * R2D));
                    if (distance >= 0) houghSpace[distance][i]++;
                }
}

void imgProcess::houghTransform(){

    houghDistanceMax = (int) (sqrt(pow(edgeWidth, 2) + pow(edgeHeight, 2)));

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
//                    distance = (int) ((x - centerX) * cos(theta * R2D) + (centerY - y) * sin(theta * R2D));
                    distance = (int) ((x - centerX) * cos(theta * R2D) + (y - centerY) * sin(theta * R2D));
                    if (distance >= 0) houghSpace[distance][i]++;
                }
}


void imgProcess::houghTransformEdgeMap(){

    houghDistanceMax = (int) (sqrt(pow(edgeWidth, 2) + pow(edgeHeight, 2)));

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
//                    distance = (int) ((x - centerX) * cos(theta * R2D) + (centerY - y) * sin(theta * R2D));
                    distance = (int) ((x - centerX) * cos(theta * R2D) + (y - centerY) * sin(theta * R2D));
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

        //disrance=10 , to elimiate the edges near border
        for (int distance = 10; distance < houghDistanceMax; distance++)
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

                for (int x = 0; x < edgeWidth; x++){
                    lineY = centerY + 1 + getLineY((x - centerX), list[c].distance, list[c].angle);

                    if (lineY >= 0 && lineY < height) matrix[lineY][x+1] = 2555;       // 2555 special code to differenciate line data, arbitrary
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
            //lineY = centerY - getLineY((x-centerX), houghLines[i][0], houghLines[i][1]);
            lineY = getLineY((x-centerX), houghLines[i][0], houghLines[i][1]) - centerY;

            if (lineY >= 0 && lineY < edgeHeight)
                if (houghMatrix[lineY][x] == 0)
                    houghMatrix[lineY][x] = 2555;       // 2555 special code to differeciate line data, arbitrary
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


void imgProcess::constructHoughMatrixMajor2Lines( bool matrixFlag ){

    // matrixFlag: true; value, false; edge
    int matrix_height;

    matrix_height = edgeHeight;

    for (int y = 0; y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            houghMatrix[y][x] = edgeMatrix[y][x];

    //if (matrixFlag) { } else { }    // houghtMattrix definition in constructor, should be revised

    int lineY;

    if (major2Lines.size() == 2){

        for (int x = major2Lines[0].start.x(); x <= major2Lines[0].end.x(); x++){
            lineY = getLineY((x - centerX), major2Lines[0].distance, major2Lines[0].angle);
            //lineY = centerY - getLineY((x - centerX), majorLines[0].distance, majorLines[0].angle);

            if (lineY >= 0 && lineY < matrix_height)
                //if (houghMatrix[lineY][x] == 0)
                    houghMatrix[lineY][x] = 2555;       // 2555 special code to differeciate line data, arbitrary
        }

        for (int x = major2Lines[1].start.x(); x <= major2Lines[1].end.x(); x++){
            lineY = getLineY((x - centerX), major2Lines[1].distance, major2Lines[1].angle);
            //lineY = centerY - getLineY((x - centerX), majorLines[1].distance, majorLines[1].angle);

            if (lineY >= 0 && lineY < matrix_height)
                //if (houghMatrix[lineY][x] == 0)
                    houghMatrix[lineY][x] = 2555;       // 2555 special code to differeciate line data, arbitrary
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


void imgProcess::constructHoughMatrixFindX(){

    int lineX;

    for (int y = 0; y < edgeHeight; y++)
        for (int x = 0; x < edgeWidth; x++)
            houghMatrix[y][x] = edgeMatrix[y][x];

    for (int i = 0; i< houghLineNo; i++)
        for (int y = 0; y < edgeHeight; y++){
            //lineY = centerY - getLineY((x-centerX), houghLines[i][0], houghLines[i][1]);
            lineX = getLineX((y-centerY), houghLines[i][0], houghLines[i][1]) - centerX;

            if (lineX >= 0 && lineX < edgeWidth)
                //if (houghMatrix[lineY][x] == 0)
                    houghMatrix[y][lineX] = 2555;       // 2555 special code to differeciate line data, arbitrary
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

    if (houghLines[0][2] >= voteThreshold){
        primaryLineDetected = true;
        primaryLine.start.setX( 0 );
        primaryLine.start.setY( -1 );
        primaryLine.end.setX( imageWidth-1 );
        primaryLine.end.setY( -1 );
        primaryLine.length = -1;
        primaryLine.distance = distanceAvg;
        primaryLine.angle = thetaAvg;
    }
    else
        primaryLineDetected = false;
    return primaryLineDetected;
}


void imgProcess::detectVoidLines(){

    if (primaryLineDetected){

        voidSpace.clear();
        int lineY = 0, voidCount = 0;
        int fullX = 0;
//        int fullY = centerY - getLineY((fullX - centerX), distanceAvg, thetaAvg);;
        int fullY = getLineY((fullX - centerX), distanceAvg, thetaAvg);;
        int prevValue = 255, currentValue = 0, state = 0;
        voidLine *line;

        for (int x = 0; x < imageWidth + 1; x++){   // +1 for last coordinate
//            lineY = centerY - getLineY((x-centerX), distanceAvg, thetaAvg);
            lineY = getLineY((x-centerX), distanceAvg, thetaAvg);

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
//        int fullY = centerY - getLineY((fullX - centerX), distanceAvg, thetaAvg);;
        int fullY = getLineY((fullX - centerX), distanceAvg, thetaAvg);;
        int prevValue = 255, currentValue = 0, state = 0;
        voidLine *line;

        for (int x = 0; x < edgeWidth + 1; x++){   // +1 for last coordinate
//            lineY = centerY - getLineY((x-centerX), distanceAvg, thetaAvg);
            lineY = getLineY((x-centerX), distanceAvg, thetaAvg);

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


solidLine imgProcess::detectLongestSolidLineVert(float distance, float angle, int yStartOffset, int yEndOffset) {

    solidSpace.clear();

    int width, height;

    width = edgeWidth;
    height = edgeHeight;

    int lineX = 0, lineLength = 0;
    int prevX = 0, prevY = 0, state = 0;

    bool prevValue = false; // begin with empty
    bool currentValue = false;

    solidLine *line;

    for (int y = yStartOffset; y <= (yEndOffset + 1); y++){   // +1 for last coordinate to catch last line shape

        lineX = centerX + getLineX((y - centerY), distance, angle);

        if (lineX >= 0 && lineX < width){

            if (y == (yEndOffset+1) )
                currentValue = false;   // end with empty
            else {
                currentValue = edgeMapMatrix[y][lineX];
            }

            if (!prevValue && currentValue){
                state = 1;  // void to full
            } else
            if (prevValue && !currentValue){
                state = 0;  // full to void
            } else
            if (!prevValue && !currentValue){
                state = 3;  // void unchanged
            } else
            if (prevValue && currentValue){
                state = 4;  // full unchanged
            } else
                state = 5;  // not recognized

            if (state == 1){    // if solid line is STARTED get coor. with prev. coor (last full point coor)
                line = new solidLine();
                line->start.setX(lineX);
                line->start.setY(y);
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
            prevX = lineX;
            prevY = y;
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

solidLine imgProcess::detectLongestSolidLine(float distance, float angle, bool flag, int xOffset, int xEndOffset) {

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

    for (int x = xOffset; x <= (xEndOffset + 1); x++){   // +1 for last coordinate to catch last line shape

        lineY = getLineY((x - centerX), distance, angle);
        //lineY = centerY - getLineY((x - centerX), distance, angle);

        if (lineY >= 0 && lineY < height){

            if (x == (xEndOffset+1) )
                currentValue = 0;   // end with empty
            else {
                if (flag)
                    currentValue = valueMatrix[lineY][x];
                else
                    currentValue = edgeMatrix[lineY][x];
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
void imgProcess::detectLongestSolidLines(bool averaging, bool matrixFlag){

    // matrixFlag: true; value, false; edge
    int matrix_height, matrix_width;

    if (matrixFlag) {
        matrix_height = imageHeight;
        matrix_width = imageWidth;
    } else {
        matrix_height = edgeHeight;
        matrix_width = edgeWidth;
    }

    primaryLineFound = false;
    secondaryLineFound = false;

    //-A----------------------------------------------------------------
    //----- find all solid lines in value matrix
    solidSpaceMain.clear();
    float angle = 0;
    int angleSize = (int) ((thetaMax - thetaMin) / thetaStep) + 1;

    int lo = houghLines[0][0] * 0.50;
    int hi = houghLines[0][0] * 1.50;
//cout << lo << "-" << houghLines[0][0] << "-" << hi << endl;

    //for (int distance = 0; distance < matrix_height; distance++)
        for (int distance = lo; distance < hi; distance++)
        for (int angleIndex = 0; angleIndex < angleSize; angleIndex++){
            angle = thetaMin + angleIndex * thetaStep;
            solidSpaceMain.append( detectLongestSolidLine( distance, angle, matrixFlag, 0, (matrix_width-1) ) );  // in value/edge matrix
        }
    //------------------------------------------------------------------------------------


//cout << solidSpaceMain.size() << endl;
    //----- find max length
    float globalMaxLength = 0;
    for (int x = 0; x < solidSpaceMain.size(); x++)
        if ( solidSpaceMain[x].length > globalMaxLength )
            globalMaxLength = solidSpaceMain[x].length;
    //------------------------------------------------------------------------------------

//cout << globalMaxLength << endl;
    if ( globalMaxLength != 0 ) {

        //-B----------------------------------------------------------------
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


            //-C----------------------------------------------------------------
            //----- devide found lines into 2 main group,
            //----- according to intersection to intersect reference line's coordinates
            primaryGroup.clear();
            secondaryGroup.clear();

            primaryGroup.append( solidSpaceMainTrimmed[index] );    // reference line

            for (int i = 0; i < size; i++)
                if ( i != index ) {

                    if ( solidSpaceMainTrimmed[index].start.x() > solidSpaceMainTrimmed[i].end.x() ||   // no intersection
                         solidSpaceMainTrimmed[index].end.x() < solidSpaceMainTrimmed[i].start.x()
                        )
                        secondaryGroup.append( solidSpaceMainTrimmed[i] );
                    else    // intersection
                        primaryGroup.append( solidSpaceMainTrimmed[i] );
                }
            //------------------------------------------------------------------------------------

            if (primaryGroup.size() != 0)
                primaryLineFound = true;
            else
                primaryLineFound = false;

            if (secondaryGroup.size() != 0)
                secondaryLineFound = true;
            else
                secondaryLineFound = false;

            //-D----------------------------------------------------------------
            float multiplier = 0.75;

            if ( !averaging )
                multiplier = 0.90;

            float primaryLengthThreshold = primaryGroup[0].length * multiplier;


            //----- detect maximum length in secondaryGroup
            int maxSolidLineLengthSecondary = 0;

            for (int i = 0; i < secondaryGroup.size(); i++)
                if ( secondaryGroup[i].length > maxSolidLineLengthSecondary )
                    maxSolidLineLengthSecondary = secondaryGroup[i].length;

            float secondaryLengthThreshold = maxSolidLineLengthSecondary * multiplier;
            //------------------------------------------------------------------------------------

            //----- primaryGroup:
            //----- average above lenght threshold
            primaryGroupMaxs.clear();

            for (int i = 0; i < primaryGroup.size(); i++)
                if ( primaryGroup[i].length >= primaryLengthThreshold ) {
                    primaryGroupMaxs.append( primaryGroup[i] );

                    // DEBUG
                    majorLines.append( primaryGroup[i] );
                }

            //----- secondaryGroup:
            //----- average above lenght threshold
            secondaryGroupMaxs.clear();

            for (int i = 0; i < secondaryGroup.size(); i++)
                if ( secondaryGroup[i].length >= secondaryLengthThreshold ) {
                    secondaryGroupMaxs.append( secondaryGroup[i] );

                    // DEBUG
                    majorLines.append( secondaryGroup[i] );
                }
            //-D-END----------------------------------------------------------------


            major2Lines.clear();

            //-E----------------------------------------------------------------
            // * weighted average
            distanceAvgPrimary = 0;
            thetaAvgPrimary = 0;
            int sum = 0;

            for (int i = 0; i < primaryGroupMaxs.size(); i++ ) {
                distanceAvgPrimary += primaryGroupMaxs[i].length * primaryGroupMaxs[i].distance;
                thetaAvgPrimary += primaryGroupMaxs[i].length * primaryGroupMaxs[i].angle;
                sum += primaryGroupMaxs[i].length;
            }

            if (sum != 0) {
                distanceAvgPrimary /= sum;
                thetaAvgPrimary /= sum;
            }

            // * weighted average
            distanceAvgSecondary = 0;
            thetaAvgSecondary = 0;
            sum = 0;

            for (int i = 0; i < secondaryGroupMaxs.size(); i++ ) {
                distanceAvgSecondary += secondaryGroupMaxs[i].length * secondaryGroupMaxs[i].distance;
                thetaAvgSecondary += secondaryGroupMaxs[i].length * secondaryGroupMaxs[i].angle;
                sum += secondaryGroupMaxs[i].length;
            }

            if (sum != 0) {
                distanceAvgSecondary /= sum;
                thetaAvgSecondary /= sum;
            }
            //-E-END---------------------------------------------------------------

            if (averaging) {

                //-F----------------------------------------------------------------

                /*
                //----- average angle and distance within the same angle band of max. lenght and obtain major line
                float avgAngleUp = thetaAvgPrimary + 0.5;      // +0.5C for deadband
                float avgAngleDown = thetaAvgPrimary - 0.5;    // -0.5C for deadband
                sum = 0;
                distanceAvgPrimary = 0;

                for (int i = 0; i < primaryGroupMaxs.size(); i++ )
                    if ( primaryGroupMaxs[i].angle >= avgAngleDown && primaryGroupMaxs[i].angle <= avgAngleUp ) {
                        distanceAvgPrimary += primaryGroupMaxs[i].length * primaryGroupMaxs[i].distance;
                        sum += primaryGroupMaxs[i].length;
                    }

                if (sum != 0)
                    distanceAvgPrimary /= sum;
                */

                int priXStartOffset = matrix_width - 1;
                for (int i = 0; i < primaryGroupMaxs.size(); i++ ) {

                    if ( primaryGroupMaxs[i].start.x() < priXStartOffset )
                        priXStartOffset = primaryGroupMaxs[i].start.x();
                }

                int priXEndOffset = 0;
                for (int i = 0; i < primaryGroupMaxs.size(); i++ ) {

                    if ( primaryGroupMaxs[i].end.x() > priXEndOffset )
                        priXEndOffset = primaryGroupMaxs[i].end.x();
                }

                major2Lines.append( detectLongestSolidLine( distanceAvgPrimary, thetaAvgPrimary , matrixFlag, priXStartOffset, priXEndOffset ) );


                /*
                //----- average angle and distance within the same angle band of max. lenght and obtain major line
                avgAngleUp = thetaAvgSecondary + 0.5;      // +0.5C for deadband
                avgAngleDown = thetaAvgSecondary - 0.5;    // -0.5C for deadband
                sum = 0;
                distanceAvgSecondary = 0;

                for (int i = 0; i < secondaryGroupMaxs.size(); i++ )
                    if ( secondaryGroupMaxs[i].angle >= avgAngleDown && secondaryGroupMaxs[i].angle <= avgAngleUp ) {
                        distanceAvgSecondary += secondaryGroupMaxs[i].length * secondaryGroupMaxs[i].distance;
                        sum += secondaryGroupMaxs[i].length;
                    }

                if (sum != 0)
                    distanceAvgSecondary /= sum;
                */

                int secXStartOffset = matrix_width - 1;
                for (int i = 0; i < secondaryGroupMaxs.size(); i++ ) {

                    if ( secondaryGroupMaxs[i].start.x() < secXStartOffset )
                        secXStartOffset = secondaryGroupMaxs[i].start.x();
                }

                int secXEndOffset = 0;
                for (int i = 0; i < secondaryGroupMaxs.size(); i++ ) {

                    if ( secondaryGroupMaxs[i].end.x() > secXEndOffset )
                        secXEndOffset = secondaryGroupMaxs[i].end.x();
                }

                major2Lines.append( detectLongestSolidLine( distanceAvgSecondary, thetaAvgSecondary , matrixFlag, secXStartOffset, secXEndOffset ) );

                if ( major2Lines.last().length == -1 )  // in case of produced line dont touch the value matrix
                    secondaryLineFound = false;

                //-F-END---------------------------------------------------------------

            } else {    // NOT AVERAGING

                //-G----------------------------------------------------------------

                int startX = matrix_width - 1;
                int endX = 0;
                int startY = matrix_height - 1;
                int endY = matrix_height - 1;

                for (int i = 0; i < primaryGroupMaxs.size(); i++ ) {

                    if ( primaryGroupMaxs[i].start.x() < startX )   // left most X value
                        startX = primaryGroupMaxs[i].start.x();

                    if ( primaryGroupMaxs[i].end.x() > endX )       // right most X value
                        endX = primaryGroupMaxs[i].end.x();

                    if ( primaryGroupMaxs[i].start.y() < startY )   // left upper(smallest for centerY=0) Y value
                        startY = primaryGroupMaxs[i].start.y();

                    if ( primaryGroupMaxs[i].end.y() < endY )       // right upper(smallest for centerY=0) Y value
                        endY = primaryGroupMaxs[i].end.y();
                }

                solidLine major1;
                major1.start.setX( startX );
                major1.start.setY( startY );
                major1.end.setX( endX );
                major1.end.setY( endY );
                major1.distance = distanceAvgPrimary;
                major1.length = endX - startX + 1;  // rough calculation
                major1.angle = thetaAvgPrimary;

                major2Lines.append( major1 );


                startX = matrix_width - 1;
                endX = 0;
                startY = matrix_height - 1;
                endY = matrix_height - 1;

                for (int i = 0; i < secondaryGroupMaxs.size(); i++ ) {

                    if ( secondaryGroupMaxs[i].start.x() < startX )   // left most X value
                        startX = secondaryGroupMaxs[i].start.x();

                    if ( secondaryGroupMaxs[i].end.x() > endX )       // right most X value
                        endX = secondaryGroupMaxs[i].end.x();

                    if ( secondaryGroupMaxs[i].start.y() < startY )   // left upper(smallest for centerY=0) Y value
                        startY = secondaryGroupMaxs[i].start.y();

                    if ( secondaryGroupMaxs[i].end.y() < endY )       // right upper(smallest for centerY=0) Y value
                        endY = secondaryGroupMaxs[i].end.y();
                }

                solidLine major2;
                major2.start.setX( startX );
                major2.start.setY( startY );
                major2.end.setX( endX );
                major2.end.setY( endY );
                major2.distance = distanceAvgSecondary;
                major2.length = endX - startX + 1;  // rough calculation
                major2.angle = thetaAvgSecondary;

                major2Lines.append( major2 );

                //-G-END---------------------------------------------------------------

            }

//cout << major2Lines[0].start.x() << "," << major2Lines[0].start.y() << "," << major2Lines[0].end.x() << "," << major2Lines[0].end.y() << "," << major2Lines[0].distance << "," << major2Lines[0].angle << "," << major2Lines[0].length << endl;
//cout << major2Lines[1].start.x() << "," << major2Lines[1].start.y() << "," << major2Lines[1].end.x() << "," << major2Lines[1].end.y() << "," << major2Lines[1].distance << "," << major2Lines[1].angle << "," << major2Lines[1].length << endl;

        } // solidSpaceMainTrimmed.size()
    } // globalMaxLength


    //-H----------------------------------------------------------------
    //----- calculate corners & center

    centerDetermined = false;
    angleAvg = 0;

    if (matrixFlag) {   // if value matrix is used
        leftCornerX = 0;
        leftCornerY = imageHeight/2;
        rightCornerX = imageWidth - 1;
        rightCornerY = imageHeight/2;
    } else {            // if edge matrix is used
        leftCornerX = 0;
        leftCornerY = edgeHeight/2;
        rightCornerX = edgeWidth - 1;
        rightCornerY = edgeHeight/2;
    }


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
    } else {
        if ( primaryLineFound ) {

            int distance2Left = major2Lines[0].start.x();
            int distance2Right = (matrix_width - 1) - major2Lines[0].end.x();

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
    }


    trackCenterX = ( leftCornerX + rightCornerX ) / 2;
    trackCenterY = ( leftCornerY + rightCornerY ) / 2;

    if (!matrixFlag) {   // if edge matrix is used
        trackCenterX++;
        trackCenterY++;
    }
    //-H-END---------------------------------------------------------------


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
    anglePrecision = precisionSize;

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

    //-A----------------------------------------------------------------

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
    if (_DEBUG)
        saveMinCostedLinesArray(bestLines, precisionSize, "./data/Algo5_0-bestLines.csv");
    //------------------------------------------------------------------


    //-B----------------------------------------------------------------
    // find global minimum with angle & c among bestLines
    min = 255 * imageHeight;
    index = 0;
    for (int m = 0; m < precisionSize; m++){
        if (bestLines[m].cost < min){
            index = m;
            min = bestLines[m].cost;
        }
    }

    slopeBestIndex = index;
    slopeBest = slope[index];
    //------------------------------------------------------------------


    //-C----------------------------------------------------------------
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
    if (_DEBUG)
        saveMinCostedLinesList(lineList, "./data/Algo5_1-lineList.csv");
    //------------------------------------------------------------------


    //----- alarm preparetion
    primaryLineFound = detected = centerDetermined = true;


    // ----- find the center of the valley
    int minX = imageWidth;
    int maxX = 0;

    if (lineList.size() >= 2) {

        //-D----------------------------------------------------------------
        peakPoints.clear();

        int derivative = 0;
        int startIndex = -1;

        // search for first signed derivative > unequality of value sums
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

            // to determine sign of first derivative
            if (derivative < 0)
                sign = -1;
            else
                sign = 1;

            for (int i = startIndex + 1; i < lineList.size() - 1; i++){

                derivative = lineList[i+1].cost - lineList[i].cost;

                if (sign < 0){  // compare with previous sign
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

            if (_DEBUG)
                saveList(peakPoints, "./data/Algo5_2-peakPoints.csv");
            //------------------------------------------------------------------


            //-E----------------------------------------------------------------
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

            if (_DEBUG) cout << "minCostIndex: " << minCostIndex << " minDerivative: " << derivativeArray[minCostIndex] << endl;
            //------------------------------------------------------------------


            //-F----------------------------------------------------------------
            // calc average brigthness cost and assign valley threshold
            int avgCost = 0;
            for (int i = 0; i < peakPoints.size(); i++)
                avgCost += lineList[ peakPoints[i] ].cost;

            avgCost /= peakPoints.size();

            // threshold = abs(avgCost - costOfMin) / 2 + costOfMin
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
            //------------------------------------------------------------------

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


void imgProcess::detectMainEdges(int method, bool DEBUG){

    trackCenterX = rightCornerX = leftCornerX = 0;
    trackCenterY = rightCornerY = leftCornerY = 0;

    // sort wrt distance low > high
    sortHoughLines_toDistance( houghLineNo );
        // ** HOUGH DATA > SORTED HOUGH DATA WRT DISTANCE

    // 1ST ITERATION
    int *voteArray = new int[houghLineNo];
    for (int i = 0; i < houghLineNo; i++) voteArray[i] = houghLinesSorted[i][2];     // votes

    QList<range> localMaximalist;

    findLocalMinimum(voteArray, houghLineNo, localMaximalist);
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

        //-A----------------------------------------------------------------
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

        QList<QPoint> pointList;

        if ( !localMaximalist2nd.isEmpty() ){

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
            listHoughData2nd.clear();

            //-B----------------------------------------------------------------

            for (int t = 0; t < localMaximalist2nd.size(); t++)
                for (int ts = localMaximalist2nd[t].start; ts <= localMaximalist2nd[t].end; ts++){
                    houghData hd;
                    hd.distance = listHoughData[ ts ].distance;
                    hd.angle = listHoughData[ ts ].angle;
                    hd.voteValue = listHoughData[ ts ].voteValue;
                    listHoughData2nd.append(hd);
                }
            // ** localMaximalist2nd > listHoughData2nd
            //------------------------------------------------------------------

            if (method == 0) {
                int lineX, y = imageHeight/2;
                for (int i=0; i < listHoughData2nd.size(); i++) {
                    //lineX = centerX + 1 + getLineX((centerY + 1 - y), listHoughData2nd[i].distance, listHoughData2nd[i].angle);
                    lineX = centerX + 1 + getLineX((y-centerY), listHoughData2nd[i].distance, listHoughData2nd[i].angle);
                    QPoint p(lineX, listHoughData2nd[i].voteValue);
                    pointList.append(p);
                }
            }

            listHoughData2ndSize = listHoughData2nd.size();

            //if (DEBUG) {
                listHoughData2ndArray = new int*[listHoughData2ndSize];
                for (int d = 0; d < listHoughData2ndSize; d++)    listHoughData2ndArray[d] = new int[3];
                listHoughData2ndArrayInitSwitch = true;

                for (int d = 0; d < listHoughData2ndSize; d++){
                    listHoughData2ndArray[d][0] = listHoughData2nd[d].distance;
                    listHoughData2ndArray[d][1] = listHoughData2nd[d].angle;
                    listHoughData2ndArray[d][2] = listHoughData2nd[d].voteValue;
                }
            //}


            if ( !listHoughData2nd.isEmpty() && method != -1 ) { // BYPASS FOR SOLID LINE ALGO

                pointListSorted.clear();
                QList<int> pointListMap;
                mainPointsList.clear();

                if ( method == 0 ) { // NATURAL BREAKS
                    //------------------------------------------------------------------
                    //
                    // NATURAL BREAKS ALGORITM
                    //
                    // TO ELIMINATE CLOSE POINTS
                    //

                    const int n = listHoughData2nd.size();
                    naturalBreaksNumber = 2;

                    if (n >= naturalBreaksNumber) {
                        // ** qDebug() << "pointList:";  for (int i=0; i<n; i++) qDebug() << i << " " << pointList[i].x() << " " << pointList[i].y() ;

                        // *** Sort point list wrt x position
                        int minx, idx;
                        for (int i=0; i<n; i++) {
                            minx = 2000, idx = 0;
                            for (int j=0; j<n; j++) {
                                if ( pointList[j].x() < minx ) {
                                    minx = pointList[j].x();
                                    idx = j;
                                }
                            }
                            //int lengthVal = detectLongestSolidLineVert( listHoughData2ndArray[pointListMap[idx]][0], listHoughData2ndArray[pointListMap[idx]][1], 0, edgeHeight-1 ).length;
                            //QPoint p( pointList[idx].x(), lengthVal );

                            QPoint p( pointList[idx].x(), pointList[idx].y() );
                            pointListSorted.append(p);
                            pointListMap.append(idx);
                            pointList[idx].setX(2000);
                        }
                        // ** qDebug() << "pointListSorted:"; for (int i=0; i<n; i++) qDebug() << i << " " << pointListSorted[i].x() << " " << pointListSorted[i].y() ;
                        // ***

                        // *** Preparation for Natural Breaks Algorithm
                        std::vector<double> values;
                        values.reserve(n);

                        for (int i=0; i!=n; ++i)
                            values.push_back( pointListSorted[i].x() );

                        assert(values.size() == n);
                        // ***

                        // *** Automatic scan to find optimum breaks (k) number (size)
                        // *** using standard deviations of the regions > mainPointsList
                        bool cont;
                        double varLimit = imageWidth*0.1;
                        int maxValue, maxIdx;

                        do {
                            cont = false;
                            mainPointsList.clear();
                            ValueCountPairContainer sortedUniqueValueCounts;
                            GetValueCountPairs(sortedUniqueValueCounts, &values[0], n);

                            LimitsContainer resultingbreaksArray;
                            ClassifyJenksFisherFromValueCountPairs(resultingbreaksArray, naturalBreaksNumber, sortedUniqueValueCounts);

                            int breaksArrayIdx = 1, pointListSortedIdx = 0, sampleNo = 0;
                            double sum = 0, sampleAve = 0, sampleVar = 0;

                            // ** qDebug() << "-----------"; for (double breakValue: resultingbreaksArray)  qDebug() << breakValue;

                            QList<int> sampleList;
                            maxValue = 0; maxIdx = 0;
                            do {
                                if ( breaksArrayIdx < resultingbreaksArray.size() ) {

                                    if ( pointListSorted[ pointListSortedIdx ].x() >= resultingbreaksArray[ breaksArrayIdx ] ) {

                                        breaksArrayIdx++;
                                        //qDebug() << sum << " " << sampleNo;
                                        if (sampleNo != 0) {
                                            sampleAve = sum / sampleNo;
                                            double powSum = 0;
                                            for (int c=0; c<sampleList.size(); c++)
                                                powSum += pow(sampleAve-sampleList[c], 2);
                                            powSum /= sampleNo;
                                            sampleVar = sqrt(powSum);
                                            if (sampleVar > varLimit) cont = true;

                                            QPoint p( pointListSorted[ maxIdx ].x(), pointListSorted[ maxIdx ].y() );
                                            mainPointsList.append(p);
                                            maxValue = 0; maxIdx = 0;

                                            //qDebug() << sampleVar;
                                        }
                                        sampleList.clear();
                                        sum = 0;
                                        sampleNo = 0;
                                    } else {
                                        sampleList.append( pointListSorted[ pointListSortedIdx ].x() );
                                        sum += pointListSorted[ pointListSortedIdx ].x();

                                        if ( pointListSorted[ pointListSortedIdx ].y() > maxValue ) {
                                            maxValue = pointListSorted[ pointListSortedIdx ].y();
                                            maxIdx = pointListSortedIdx;
                                        }

                                        sampleNo++;
                                        pointListSortedIdx++;
                                    }
                                } else {
                                    // for last break
                                    sampleList.append( pointListSorted[ pointListSortedIdx ].x() );
                                    sum += pointListSorted[ pointListSortedIdx ].x();

                                    if ( pointListSorted[ pointListSortedIdx ].y() > maxValue ) {
                                        maxValue = pointListSorted[ pointListSortedIdx ].y();
                                        maxIdx = pointListSortedIdx;
                                    }

                                    sampleNo++;
                                    pointListSortedIdx++;
                                }

                            } while( pointListSortedIdx < pointListSorted.size() );

                            //qDebug() << sum << " " << sampleNo;
                            if (sampleNo != 0) {
                                sampleAve = sum / sampleNo;
                                double powSum = 0;
                                for (int c=0; c<sampleList.size(); c++)
                                    powSum += pow(sampleAve-sampleList[c], 2);
                                powSum /= sampleNo;
                                sampleVar = sqrt(powSum);
                                if (sampleVar > varLimit) cont = true;

                                QPoint p( pointListSorted[ maxIdx ].x(), pointListSorted[ maxIdx ].y() );
                                mainPointsList.append(p);
                                maxValue = 0; maxIdx = 0;

                                //qDebug() << sampleVar;
                            }
                            naturalBreaksNumber++;

                        } while (cont && naturalBreaksNumber<pointListSorted.size() );

                        naturalBreaksOK = true;
                    } else {
                        naturalBreaksOK = false;
                    }
                    //------------------------------------------------------------------
                }

                detected = true;

                // TRACK CENTER DETECTION

                bool xCoorCalcEnable = false;

                if ( method == 0) {    // NATURAL BREAKS

                    if (naturalBreaksOK) {
                        mainEdgesList.clear();
                        for (int c=0; c<mainPointsList.size(); c++) {
                            int refId = -1;
                            for (int id=0; id<pointList.size(); id++) {
                                if ( mainPointsList[c].x() == pointListSorted[id].x() && mainPointsList[c].y() == pointListSorted[id].y() )
                                    refId = pointListMap[id];

                            }
                            houghData hd;
                            hd.distance =  listHoughData2ndArray[refId][0];
                            hd.angle =  listHoughData2ndArray[refId][1];
                            hd.voteValue =  listHoughData2ndArray[refId][2];
                            mainEdgesList.append(hd);
                            //qDebug() << mainPointsList[c].x() << " - " << mainPointsList[c].y() << " refId: " << refId;
                        }

                        // *** Find maximum (votes) 2 points
                        int _idx = 0;
                        QPoint max1(0,-1);
                        for (int c=0; c<mainPointsList.size(); c++) {
                            if ( mainPointsList[c].y() > max1.y() ) {
                                max1.setY( mainPointsList[c].y() );
                                max1.setX( mainPointsList[c].x() );
                                _idx = c;
                            }
                        }
                        natBreaksMax1.setX( mainPointsList[_idx].x() );
                        natBreaksMax1.setY( mainPointsList[_idx].y() );
                        mainPointsList[_idx].setY(-1);

                        _idx = 0;
                        QPoint max2(0,-1);
                        for (int c=0; c<mainPointsList.size(); c++) {
                            if ( mainPointsList[c].y() > max2.y() ) {
                                max2.setY( mainPointsList[c].y() );
                                max2.setX( mainPointsList[c].x() );
                                _idx = c;
                            }
                        }
                        natBreaksMax2.setX( mainPointsList[_idx].x() );
                        natBreaksMax2.setY( mainPointsList[_idx].y() );
                        mainPointsList[_idx].setY(-1);

                        //qDebug() << "max1 x/vote: " << max1.x() << " / " << max1.y();
                        //qDebug() << "max2 x/vote: " << max2.x() << " / " << max2.y();

                        // *** Determine left & right corners
                        if ( max1.x() > max2.x()) {
                            leftCornerX = max2.x();
                            rightCornerX = max1.x();
                        } else {
                            leftCornerX = max1.x();
                            rightCornerX = max2.x();
                        }
                        // ***

                        trackCenterX = (leftCornerX + rightCornerX)/2.0;

                        centerLine.angle = 0;
                        centerLine.distance = trackCenterX;
                        centerLine.voteValue = 0;
                    } else {
                        detected = false;
                    }
                }

                if ( method == 1 ) {    // MEAN VALUE

                    // APPLY MEAN VALUE THRESHOLD to listHoughData2nd to build mainEdgeList lines

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

                    /* Average of mainEdgeList lines is the **centerLine**
                    if ( mainEdgesList.size() != 0) {

                        float angleSum = 0, distanceSum = 0;
                        for (int i = 0; i < mainEdgesList.size(); i++){
                            angleSum += mainEdgesList[i].angle;
                            distanceSum += mainEdgesList[i].distance;
                        }

                        centerLine.angle = angleSum / mainEdgesList.size();
                        centerLine.distance = distanceSum / mainEdgesList.size();
                    }
                    */

                    xCoorCalcEnable = true;
                }

                if ( method == 2 ) {    // GET NUMBER OF MAXs

                    // SELECT size (thinCornerNum) NUMBER of MAXS to build mainEdgeList

                    int size = thinCornerNum;

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
                    /* Maximum voted line is **centerLine**
                    if ( mainEdgesList.size() != 0) {
                        centerLine.angle = mainEdgesList[0].angle;
                        centerLine.distance = mainEdgesList[0].distance;
                        centerLine.voteValue = mainEdgesList[0].voteValue;
                    }*/

                    xCoorCalcEnable = true;
                    //------------------------------------------------------------------
                }

                // TRACK CENTER DETECTION - END


                if ( !mainEdgesList.isEmpty() && xCoorCalcEnable ){

                    //-E----------------------------------------------------------------
                    // x position calculation of mainEdgesList lines
                    //
                    // track center is the average of Xmin & Xmax
                    //

                    QList<int> xCoors;
                    int yCoor = imageHeight/2;

                    for (int i = 0; i < mainEdgesList.size(); i++){
                        int xCoor = centerX + 1 + getLineX( (yCoor-centerY), mainEdgesList[i].distance, mainEdgesList[i].angle);
                        //int xCoor = centerX + 1 + getLineX((centerY + 1 - yCoor), mainEdgesList[i].distance, mainEdgesList[i].angle);
                        //int xCoor = centerX + getLineX((centerY - yCoor), mainEdgesList[i].distance, mainEdgesList[i].angle);
                        if (xCoor >= 0 && xCoor < imageWidth)
                            xCoors.append( xCoor );
                        //qDebug() << "xCoor: " << xCoors[i];
                    }

                    int max = -1, min = 2000;

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

                        centerLine.angle = mainEdgesList[0].angle;
                        centerLine.distance = mainEdgesList[0].distance;
                        //centerLine.distance = trackCenterX; // since we look to perfect vertical lines
                        centerLine.voteValue = mainEdgesList[0].voteValue;
                    } else {
                        // DETECTION ERROR
                        trackCenterX = rightCornerX = leftCornerX = 0;
                        detected = false;
                    }
                    //------------------------------------------------------------------

                    xCoors.empty();
                } // *** !mainEdgesList.isEmpty()


                trackCenterY = rightCornerY = leftCornerY = imageHeight / 2;


                if (DEBUG && !mainEdgesList.isEmpty()) {
    //                codeLineData(valueMatrix, imageWidth, imageHeight, listHoughData2nd, false);
                    codeLineData(valueMatrix, imageWidth, imageHeight, mainEdgesList, false);
                }

                //----- alarms
                //------------------------------------------------------------------------------------

                // clear vars

            } else {
                // DETECTION ERROR
                trackCenterX = rightCornerX = leftCornerX = 0;
                detected = false;
            } // *** !listHoughData2nd.isEmpty()

            // clear vars
            listHoughData2nd.empty();

        } else {
            // DETECTION ERROR
            trackCenterX = rightCornerX = leftCornerX = 0;
            detected = false;
        } // *** !localMaximalist2nd.isEmpty()

        // clear vars
        delete distArray;
        delete houghDataVotes;
        listHoughData.empty();
        localMaximalist2nd.empty();

    } else {
        // DETECTION ERROR
        trackCenterX = rightCornerX = leftCornerX = 0;
        detected = false;
    } // *** !localMaximalist.isEmpty()

    // clear vars
    delete voteArray;
    localMaximalist.empty();
}

houghData imgProcess::detectMainEdgesSolidLine(float rate, bool debug){

    detectMainEdges(-1, debug);

    mainEdgesList.clear();

    QList<int> lengthList;

    houghData hd;
    hd.distance = -1, hd.angle = -1, hd.voteValue = -1;
    int max = -1, length, index = 0;
    for (int i = 0; i < listHoughData2ndSize; i++) {
        length = detectLongestSolidLineVert( listHoughData2ndArray[i][0], listHoughData2ndArray[i][1], 0, edgeHeight-1 ).length;
        lengthList.append(length);
        if ( length > max ) {
            max = length;
            index = i;
        }
    }

    solidLineLength = max;

    if (solidLineLength != -1){
        hd.distance = centerLine.distance = listHoughData2ndArray[index][0];
        hd.angle = centerLine.angle = listHoughData2ndArray[index][1];
        hd.voteValue = centerLine.voteValue = listHoughData2ndArray[index][2];
        //mainEdgesList.append(hd);

        detected = true;
        int yCoor = edgeHeight/2;
//        trackCenterX = rightCornerX = leftCornerX = centerX + 1 + getLineX((centerY + 1 - yCoor), hd.distance, hd.angle);
        trackCenterX = rightCornerX = leftCornerX = getLineX((yCoor - centerY), hd.distance, hd.angle) - centerX + 1;
        mainEdgeScorePercent = abs ( solidLineLength * sin (R2D * (90 - hd.angle)) * 100.0 / edgeHeight );

    } else {
        detected = false;
        trackCenterX = rightCornerX = leftCornerX = 0;
    }
// * for multiple solid lines
    QList<int> lengthSortedIndex;

    for (int i = 0; i < lengthList.size(); i++) {
        max = -1; index = 0;
        for (int j = 0; j < lengthList.size(); j++) {
            if ( lengthList[j] > max ) {
                index = j;
            }
        }
        lengthSortedIndex.append(index);
        lengthList[index] = -1;
    }

    for (int i=0; i<thinCornerNum; i++) {
        if (i < lengthSortedIndex.size()) {
            houghData hdx;
            hdx.distance = listHoughData2ndArray[ lengthSortedIndex[i] ][0];
            hdx.angle = listHoughData2ndArray[ lengthSortedIndex[i] ][1];
            hdx.voteValue = listHoughData2ndArray[ lengthSortedIndex[i] ][2];
            mainEdgesList.append(hdx);
        }
    }

    return hd;
}

void imgProcess::histogramAnalysis(bool colored, bool invertHist){

    bool state = true;

    // CALCULATE HISTOGRAM
    if (colored)
        valueHistogram(false);
    else
        valueHistogramGray(false);


    histogramFiltered = new int[histogramSize];
    histogramAnlysInitSwitch = true;

    int noiseDia = (maFilterKernelSize-1)/2;
    histogramAvg = 0;
    histogramMin = 2000, histogramMax = 0;

    /* MOVING AVERAGE FILTER
    float sum;
    for (int x = 0; x < histogramSize; x++){
        if (x < noiseDia || x > (histogramSize-noiseDia))
           histogramFiltered[x] = histogram[x] ;
        else {
            sum = 0;
            for (int k = x-noiseDia; k <= x+noiseDia; k++)
                sum += histogram[k];
            histogramFiltered[x] = (sum*1.0) /  maFilterKernelSize;
        }

        histogramAvg += histogramFiltered[x];
        if (histogramFiltered[x] < histogramMin)
            histogramMin = histogramFiltered[x];
        if (histogramFiltered[x] > histogramMax)
            histogramMax = histogramFiltered[x];
    }
    */

    // * RECURSIVE MOVING AVERAGE FILTER
    float initialAvg = 0;
    for (int x = 0; x < maFilterKernelSize; x++)
        initialAvg += histogram[x];
    initialAvg /= (1.0*maFilterKernelSize);

    double value;
    for (int x = 0; x < histogramSize; x++){
        if ( x < noiseDia )
           histogramFiltered[x] = histogram[x] ;
        else if ( x == noiseDia )
            histogramFiltered[x] = initialAvg ;
        else if ( x > (histogramSize-noiseDia-1) )
            histogramFiltered[x] = histogramFiltered[x-1] ;
        else {
            value = histogramFiltered[x-1] + histogram[x+noiseDia] - histogram[x-noiseDia-1];
            histogramFiltered[x] = value;
/*            if (value>=0 && value <= 255)
                histogramFiltered[x] = value;
            else
                histogramFiltered[x] = histogramFiltered[x-1];
  */      }

        histogramAvg += histogramFiltered[x];
        if (histogramFiltered[x] < histogramMin)
            histogramMin = histogramFiltered[x];
        if (histogramFiltered[x] > histogramMax)
            histogramMax = histogramFiltered[x];
    }

    if (histogramSize != 0)
        histogramAvg /= (1.0*histogramSize);
    else
        histogramAvg = -1;


    double yScaleFactor = (histogramSize*1.0) / (histogramMax-histogramMin);

    for (int x = 0; x < histogramSize; x++){
        if (invertHist)
            histogramFiltered[x] = histogramMax - yScaleFactor * (histogramFiltered[x]-histogramMin);
        else
            histogramFiltered[x] = yScaleFactor * (histogramFiltered[x]-histogramMin);
    }

    histogramAvg = 0;
    histogramMin = 2000, histogramMax = 0;

    for (int x = 0; x < histogramSize; x++){
        histogramAvg += histogramFiltered[x];
        if (histogramFiltered[x] < histogramMin)
            histogramMin = histogramFiltered[x];
        if (histogramFiltered[x] > histogramMax)
            histogramMax = histogramFiltered[x];
    }

    if (histogramSize != 0)
        histogramAvg /= histogramSize;
    else
        histogramAvg = -1;

// *-derivative------------------------------------------------------
    histogramDIdx.clear();
    histogramDSum.clear();
    histogramD = new double[histogramSize]; histogramD[0]=0;
    histogramDD = new double[histogramSize]; histogramDD[0]=0; histogramDD[1]=0;
    histogramDDInitSwitch = true;

    histogramDMin = 3000, histogramDMax = -3000;
    for (int i=1; i<histogramSize; i++) {
        histogramD[i] = histogramFiltered[i] - histogramFiltered[i-1];
        if (histogramD[i] < histogramDMin)   histogramDMin = histogramD[i];
        if (histogramD[i] > histogramDMax)   histogramDMax = histogramD[i];
    }

    double ddSum = 0;
    int ddCnt = 0;
    histogramDDMin = 3000, histogramDDMax = -3000;
    for (int i=2; i<histogramSize; i++) {
        histogramDD[i] = std::abs(histogramD[i] - histogramD[i-1]);
        if (histogramDD[i] < histogramDDMin)   histogramDDMin = histogramDD[i];
        if (histogramDD[i] > histogramDDMax)   histogramDDMax = histogramDD[i];
        /*if (histogramDD[i]>0) {
            ddSum += histogramDD[i];
            ddCnt++;
        }*/
    }

    findMaxs(histogramDD, histogramSize, ddPeaks);
    /*QString s="";
    for (int i=0; i<ddPeaks.size(); i++)
        s+="("+QString::number(ddPeaks[i].start)+","+QString::number(ddPeaks[i].end)+") ";
    qDebug() << s;*/

    // FIND AVERAGE OF NON-ZERO VALUES
    for (int i=0; i<ddPeaks.size(); i++) {
        /*for (int j=ddPeaks[i].start; j<=ddPeaks[i].end; j++) {
            if (histogramDD[j]>0) {
                ddSum += histogramDD[j];
                ddCnt++;
            }
        }*/
        if (histogramDD[ddPeaks[i].start]>0) {
            ddSum += histogramDD[ddPeaks[i].start];
            ddCnt++;
        }
    }
    if (ddCnt != 0)
        histDDLimit = ddSum / ddCnt;
    else
        histDDLimit = 0;
    //qDebug()<<"histDDLimit " << histDDLimit;

    histogramExtremes.clear();
    for (int i=0; i<ddPeaks.size(); i++) {
        if (histogramDD[ddPeaks[i].start] > histDDLimit) {
            range p;
            p.start = ddPeaks[i].start;
            p.end = ddPeaks[i].end;
            histogramExtremes.append(p);
        }
    }

    if (!histogramExtremes.isEmpty()) {

        /* CORNER FINDING ALGO BASED ON DERIVATIVE SIGN
            int sum = 0;
            bool signFlag = false;

            int pastSize = 10;
            int pastSum;
            double pastAvg;
            bool dChangeFlag = false;
            int histDThresh = abs((histDmax-histDmin)*0.2);

            for (int i=1; i<histogramSize; i++) {

                if (i>=pastSize) {
                    pastSum = 0;
                    for (int j=1; j<=pastSize; j++)
                        pastSum += histogramD[i-j];
                    pastAvg = abs((1.0*pastSum)/pastSize);
                    if (pastAvg>histDThresh) {
                        if ( abs(histogramD[i]) > (pastAvg*1.5) || abs(histogramD[i]) < (pastAvg*0.5) )
                            dChangeFlag = true;
                    }
                }
                if ( (histogramD[i]*histogramD[i-1]) == 0 ) {
                    if ( (histogramD[i]==0 && histogramD[i-1]!=0) || (histogramD[i]!=0 && histogramD[i-1]==0) )
                        signFlag = true;
                }
                if ( (histogramD[i]*histogramD[i-1]) < 0 ) {
                    signFlag = true;
                }
                if ( (histogramD[i]*histogramD[i-1]) > 0 ) {
                    sum += histogramD[i];
                    signFlag = false;
                }
                if ( signFlag || dChangeFlag) {
                    histogramDIdx.append(i);
                    histogramDSum.append(sum);
                    sum = 0;
                    signFlag = false;
                    dChangeFlag = false;
                }
            }

            histogramExtremes.clear();
            for (int i=1; i<histogramDIdx.size(); i++) {
                range p;
                p.start = histogramDIdx[i];
                p.end = histogramDIdx[i];
                histogramExtremes.append(p);
            }
        */
        /* CORNER FINDING ALGO BASED ON MAX AND MIN PEAK POINTS
            findMaxs(histogramFiltered, histogramSize, histogramPeaks);

            findMins(histogramFiltered, histogramSize, histogramMins);

            //--- MERGE PEAK AND MIN POINTS -----------------------------------------------------
            histogramExtremes.clear();
            int peaksIdx = 0;
            int minsIdx = 0;
            int x = 0;
            do {
                if (peaksIdx < histogramPeaks.size()){
                    if ( histogramPeaks[peaksIdx].start <= x && x <= histogramPeaks[peaksIdx].end ){
                        x = histogramPeaks[peaksIdx].end;
                        range p;
                        p.start = histogramPeaks[peaksIdx].start;
                        p.end = histogramPeaks[peaksIdx].end;
                        //qDebug() << "pp" << QString::number(p.start) << ", " << QString::number(p.end) << ", " << QString::number(histogramFiltered[ p.start ]);
                        histogramExtremes.append(p);
                        peaksIdx++;
                    }
                }

                if (minsIdx < histogramMins.size()){
                    if ( histogramMins[minsIdx].start <= x && x <= histogramMins[minsIdx].end ){
                        range p;
                        p.start = histogramMins[minsIdx].start;
                        p.end = histogramMins[minsIdx].end;
                        //qDebug() << "pp" << QString::number(p.start) << ", " << QString::number(p.end) << ", " << QString::number(histogramFiltered[ p.start ]);
                        histogramExtremes.append(p);
                        x = histogramMins[minsIdx].end;
                        minsIdx++;
                    }
                }
                x++;
            } while (x < histogramSize);
            //-----------------------------------------------------------------------------------
*/

            //--- MERGE CLOSE POINTS ------------------------------------------------------------
            int deltaXThreshold = histogramSize * 0.02;
            int deltaYThreshold = histogramSize * 0.02;
            int deltaX, deltaY;
            histogramExtremesFiltered.clear();

            range zeroPoint;
            zeroPoint.start = histogramExtremes[0].start;
            zeroPoint.end = histogramExtremes[0].end;
            histogramExtremesFiltered.append(zeroPoint);
            int hisExtFltIndex = 0;

            for (int i=0; i<histogramExtremes.size()-1; i++) {

                deltaX = histogramExtremes[i+1].start - histogramExtremes[i].end;
                deltaY = histogramFiltered[ histogramExtremes[i+1].start ] - histogramFiltered[ histogramExtremes[i].end ];

                if (deltaX > deltaXThreshold || abs(deltaY) > deltaYThreshold) {

                    //lenX = histogramExtremes[i].end - histogramExtremesFiltered[hisExtFltIndex].start;
                    histogramExtremesFiltered[hisExtFltIndex].end = histogramExtremes[i].end;

                    range nextPoint;
                    nextPoint.start = histogramExtremes[i+1].start;
                    nextPoint.end = histogramExtremes[i+1].end;
                    histogramExtremesFiltered.append(nextPoint);
                    hisExtFltIndex++;
                }

                //qDebug() << deltaX << " " << deltaY;
                /*
                range nextPoint;
                nextPoint.start = histogramExtremes[i].start;
                nextPoint.end = histogramExtremes[i].end;
                histogramExtremesFiltered.append(nextPoint);*/
            }
            //-----------------------------------------------------------------------------------

            /*
            int max = -1000;
            for (int i=0; i<histogramExtremes.size(); i++) {
                if ( histogramFiltered[ histogramExtremes[i].start ] > max )
                    max = histogramFiltered[ histogramExtremes[i].start ];
            }
            float maxThreshold = max * histogramMaxThreshold;
            */

            //--- FIND THE PEAKS ABOVE AVERAGE --------------------------------------------------
            double maxThreshold = histogramAvg;
            histogramMaxPeaksList.clear();
            for (int i=0; i<histogramExtremesFiltered.size(); i++) {
                if ( histogramFiltered[ histogramExtremesFiltered[i].start ] > maxThreshold )
                    histogramMaxPeaksList.append(i);
            }
            /*for (int i=0; i<histogramExtremes.size(); i++) {
                if ( histogramFiltered[ histogramExtremes[i].start ] > maxThreshold )
                    histogramMaxPeaksList.append(i);
            }*/
            //-----------------------------------------------------------------------------------


            //--- CALCULATE THE ANGLE(TANGENT) BETWEEN PEAK POINTS TO FIND THE STEEPEST DECENTS -
            histogramMaxPointLen.clear();
            histogramMaxPointAng.clear();
            int startIndex, index;
            int maxPoint, pairPoint;
            double angThresh = abs( tan(histogramAngleThreshold*R2D) );
            int yRef = 0, yNext = 0, xRef = 0, xNext = 0;
            double tangent = 0, tangentMax = 0;
            double len = 0, lenMax = 0, rate = 0;
            bool loopFlag;
            double histRange = histogramMax - histogramMin;

            for (int i=0; i<histogramMaxPeaksList.size(); i++) {

                startIndex = histogramMaxPeaksList[i];

                lenMax = 0, tangentMax = 0; maxPoint = 0;

                // RIGHT direction
                xRef = histogramExtremesFiltered[startIndex].end;
                yRef = histogramFiltered[ xRef ];
                index = startIndex;
                loopFlag = true;
                do {
                    index++;
                    if (index < histogramExtremesFiltered.size()) {
                        xNext = histogramExtremesFiltered[index].start;
                        yNext = histogramFiltered[ xNext ];
                        if ( yNext < yRef) {
                            tangent = (1.0*(xNext - xRef)) / (yRef - yNext);
                            if ( abs(tangent) < angThresh ) {
                                len = sqrt( pow(yRef - yNext , 2) + pow(xNext - xRef , 2) );

                                if (len > lenMax) {
                                    maxPoint = startIndex;
                                    pairPoint = index;
                                    lenMax = len;
                                    tangentMax = tangent;
                                }
                            } else {
                                loopFlag = false;
                            }
                        } else {
                            loopFlag = false;
                        }
                    } else {
                        loopFlag = false;
                    }
                } while(loopFlag);

                // LEFT direction
                xRef = histogramExtremesFiltered[startIndex].start;
                yRef = histogramFiltered[ xRef ];
                index = startIndex;
                loopFlag = true;
                do {
                    index--;
                    if (index >= 0) {
                        xNext = histogramExtremesFiltered[index].end;
                        yNext = histogramFiltered[ xNext ];
                        if ( yNext < yRef) {
                            tangent = (1.0*(xNext - xRef)) / (yRef - yNext);
                            if ( abs(tangent) < angThresh ) {
                                len = sqrt( pow(yRef - yNext , 2) + pow(xNext - xRef , 2) );

                                if (len > lenMax) {
                                    maxPoint = startIndex;
                                    pairPoint = index;
                                    lenMax = len;
                                    tangentMax = tangent;
                                }
                            } else {
                                loopFlag = false;
                            }
                        } else {
                            loopFlag = false;
                        }
                    } else {
                        loopFlag = false;
                    }
                } while(loopFlag);

                rate = lenMax / histRange;
                if (rate > lenRateThr) {
                    if (tangentMax>=0) {
                        histogramMaxPoint.append( QPoint( histogramExtremesFiltered[maxPoint].end, histogramFiltered[ histogramExtremesFiltered[maxPoint].end ]) );
                        histogramMaxPointPair.append( QPoint( histogramExtremesFiltered[pairPoint].start, histogramFiltered[ histogramExtremesFiltered[pairPoint].start ]) );
                    } else {
                        histogramMaxPoint.append( QPoint( histogramExtremesFiltered[maxPoint].start, histogramFiltered[ histogramExtremesFiltered[maxPoint].start ]) );
                        histogramMaxPointPair.append( QPoint( histogramExtremesFiltered[pairPoint].end, histogramFiltered[ histogramExtremesFiltered[pairPoint].end ]) );
                    }

                    histogramMaxPointLen.append(lenMax);
                    histogramMaxPointAng.append(tangentMax);
                }
            } // for
            //qDebug() << "-----------------------";
            //qDebug() << "histogramMaxPoint: " << histogramMaxPoint;
            //qDebug() << "histogramMaxPointPair: " << histogramMaxPointPair;
            //qDebug() << "histogramMaxPointLen: " << histogramMaxPointLen;
            //qDebug() << "histogramMaxPointAng: " << histogramMaxPointAng;
            //-----------------------------------------------------------------------------------


            //--- EVALUATION -

            bandCheck_errorState = 0;

            // ** MAX POINT NUMBER (LENGTH RATE>THRESH) SHOULD BE >=2
            if ( histogramMaxPoint.size() < 2 ) {
                state = false;
                bandCheck_errorState = 2;
            } else {

                //--- NATURAL BREAKS ALGORITHM -
                const int n = histogramMaxPoint.size();

                // *** Preparation for Natural Breaks Algorithm
                std::vector<double> values;
                values.reserve(n);

                for (int i=0; i!=n; ++i)
                    values.push_back( histogramMaxPoint[i].x() );

                assert(values.size() == n);

                // *** Automatic scan to find optimum breaks (k) number (size)
                // *** using standard deviations of the regions > mainPointsList
                naturalBreaksNumber = 2;
                bool cont;
                double varLimit = 10;   // variance limit to identify closeness
                int maxValue, maxIdx;
                QList<int> mainPointToHistMaxIndx;

                breakPointList.clear();
                do {
                    cont = false;
                    mainPointsList.clear(); // holds x & length values
                    ValueCountPairContainer sortedUniqueValueCounts;
                    GetValueCountPairs(sortedUniqueValueCounts, &values[0], n);

                    LimitsContainer resultingbreaksArray;
                    ClassifyJenksFisherFromValueCountPairs(resultingbreaksArray, naturalBreaksNumber, sortedUniqueValueCounts);

                    int breaksArrayIdx = 1, pointListSortedIdx = 0, sampleNo = 0;
                    double sum = 0, sampleAve = 0, sampleVar = 0;

                    // ** qDebug() << "-----------"; for (double breakValue: resultingbreaksArray)  qDebug() << breakValue;

                    breakPointList.clear();
                    mainPointToHistMaxIndx.clear();
                    QList<int> sampleList;
                    maxValue = 0; maxIdx = 0;
                    do {
                        if ( breaksArrayIdx < resultingbreaksArray.size() ) {

                            if ( histogramMaxPoint[ pointListSortedIdx ].x() >= resultingbreaksArray[ breaksArrayIdx ] ) {

                                breaksArrayIdx++;
                                //qDebug() << sum << " " << sampleNo;
                                if (sampleNo != 0) {
                                    sampleAve = sum / sampleNo;
                                    double powSum = 0;
                                    for (int c=0; c<sampleList.size(); c++)
                                        powSum += pow(sampleAve-sampleList[c], 2);
                                    powSum /= sampleNo;
                                    sampleVar = sqrt(powSum);
                                    if (sampleVar > varLimit) cont = true;

                                    //QPoint p( histogramMaxPoint[ maxIdx ].x(), histogramMaxPoint[ maxIdx ].y() );
                                    breakPointList.append( sampleList[0] );
                                    QPoint p( histogramMaxPoint[ maxIdx ].x(), histogramMaxPointLen[ maxIdx ] );
                                    mainPointsList.append(p);
                                    mainPointToHistMaxIndx.append( maxIdx );
                                    maxValue = 0; maxIdx = 0;
                                    //qDebug() << sampleVar;
                                }
                                sampleList.clear();
                                sum = 0;
                                sampleNo = 0;
                            } else {
                                sampleList.append( histogramMaxPoint[ pointListSortedIdx ].x() );
                                sum += histogramMaxPoint[ pointListSortedIdx ].x();

                                if ( histogramMaxPointLen[ pointListSortedIdx ] > maxValue ) {
                                    maxValue = histogramMaxPointLen[ pointListSortedIdx ];
                                    maxIdx = pointListSortedIdx;
                                }

                                sampleNo++;
                                pointListSortedIdx++;
                            }
                        } else {
                            // for last break
                            sampleList.append( histogramMaxPoint[ pointListSortedIdx ].x() );
                            sum += histogramMaxPoint[ pointListSortedIdx ].x();

                            if ( histogramMaxPointLen[ pointListSortedIdx ] > maxValue ) {
                                maxValue = histogramMaxPointLen[ pointListSortedIdx ];
                                maxIdx = pointListSortedIdx;
                            }

                            sampleNo++;
                            pointListSortedIdx++;
                        }

                    } while( pointListSortedIdx < histogramMaxPoint.size() );

                    //qDebug() << sum << " " << sampleNo;
                    if (sampleNo != 0) {
                        sampleAve = sum / sampleNo;
                        double powSum = 0;
                        for (int c=0; c<sampleList.size(); c++)
                            powSum += pow(sampleAve-sampleList[c], 2);
                        powSum /= sampleNo;
                        sampleVar = sqrt(powSum);
                        if (sampleVar > varLimit) cont = true;

                        //QPoint p( histogramMaxPoint[ maxIdx ].x(), histogramMaxPoint[ maxIdx ].y() );
                        breakPointList.append( sampleList[0] );
                        QPoint p( histogramMaxPoint[ maxIdx ].x(), histogramMaxPointLen[ maxIdx ] );
                        mainPointsList.append(p);
                        mainPointToHistMaxIndx.append( maxIdx );
                        maxValue = 0; maxIdx = 0;

                        //qDebug() << sampleVar;
                    }
                    naturalBreaksNumber++;

                } while (cont && naturalBreaksNumber < histogramMaxPoint.size() );

                //qDebug() << breakPointList;
                //qDebug() << "mainPointsList(x/len): " << mainPointsList;
                //qDebug() << "mainPointToHistMaxIndx: " << mainPointToHistMaxIndx;

                //-----------------------------------------------------------------------------------

                // ** MAIN POINTS LIST SHOULD BE >=2
                if (mainPointsList.size() < 2) {
                    state = false;
                    bandCheck_errorState = 1;
                } else {

                    // ** ANGLE TRACING
                    // ** 1st (LEFT) ANGLE SHOULD EXTENDS TO RIGHT, 2nd (RIGHT) ANGLE SHOULD EXTENDS TO LEFT  { SHAPE: \  / }
                    double angle1 = 0, angle2 = 0, range;
                    int xFirst = 0, xNext = 0, sum;

                    // CALCULATE SEGMENT AVERAGES
                    QList<int> segmentLength;
                    QList<int> segmentAvg;
                    for (int i=0; i<mainPointsList.size()-1; i++) {
                        angle1 = histogramMaxPointAng[ mainPointToHistMaxIndx[i] ];
                        angle2 = histogramMaxPointAng[ mainPointToHistMaxIndx[i+1] ];
                        if (angle1 >= 0)
                            xFirst = histogramMaxPointPair[ mainPointToHistMaxIndx[i] ].x();
                        else
                            xFirst = histogramMaxPoint[ mainPointToHistMaxIndx[i] ].x();

                        if (angle2 >= 0)
                            xNext = histogramMaxPoint[ mainPointToHistMaxIndx[i+1] ].x();
                        else
                            xNext = histogramMaxPointPair[ mainPointToHistMaxIndx[i+1] ].x();
                        range = xNext-xFirst;
                        segmentLength.append(range);

                        if (range > 0) {
                            sum = 0;
                            for (int j=xFirst+1; j<xNext; j++)
                                sum += histogramFiltered[j];
                            segmentAvg.append(sum/range);
                        } else
                            segmentAvg.append( histogramFiltered[xFirst]);

                    }
                    //qDebug() << "mean: " << histogramAvg << "len: " << segmentLength << "avg: " << segmentAvg;


                    if (!segmentAvg.isEmpty()) {

                        // IF FIRST OR LAST SEGMENT IS ABOVE THE AVERGE DONT TAKE THIS POINT INTO ACCOUNT
                        QList<QPoint> edgeList;
                        QList<int> segmentLength2;
                        QList<int> segmentAvg2;

                        for (int i=0; i<mainPointsList.size()-1; i++) {
                            angle1 = histogramMaxPointAng[ mainPointToHistMaxIndx[i] ];
                            xFirst = histogramMaxPointPair[ mainPointToHistMaxIndx[i] ].x();

                            if (angle1 >=0 && segmentAvg[i] < histogramAvg) {

                                for (int j=i+1; j<mainPointsList.size(); j++) {

                                    angle2 = histogramMaxPointAng[ mainPointToHistMaxIndx[j] ];
                                    xNext = histogramMaxPointPair[ mainPointToHistMaxIndx[j] ].x();

                                    if (angle2 <= 0 && segmentAvg[j-1] < histogramAvg){

                                        edgeList.append(QPoint(i,j));

                                        range = xNext-xFirst;
                                        segmentLength2.append(range);

                                        if (range > 0) {
                                            sum = 0;
                                            for (int j=xFirst+1; j<xNext; j++)
                                                sum += histogramFiltered[j];
                                            segmentAvg2.append(sum/range);
                                        } else
                                            segmentAvg2.append( histogramFiltered[xFirst]);
                                    }
                                }
                            }
                        }
                        //qDebug() << "edgeList: " << edgeList;
                        //qDebug() << "len2: " << segmentLength2 << "avg2: " << segmentAvg2;
                        //if (edgeList.size()<1) qDebug() << mainPointsList.size() << " " << edgeList.size();

                        // FIND THE PAIR; BELOW HIST AVG AND HAS THE MAX WIDTH
                        if (!edgeList.isEmpty()) {
                            int maxWidth = -1, maxIdx = -1;
                            int minWidth = 2000, minIdx = -1;
                            for (int i=0; i<edgeList.size(); i++) {
                                if (segmentAvg2[i] < histogramAvg){
                                    if (segmentLength2[i] > maxWidth){
                                        maxWidth = segmentLength2[i];
                                        maxIdx = i;
                                    }
                                    if (segmentLength2[i] < minWidth){
                                        minWidth = segmentLength2[i];
                                        minIdx = i;
                                    }
                                }
                            }

                            //int selectionId = maxIdx;   // for maximum width
/**********/                int selectionId = minIdx;   // for minimum width

                            // ** THERE SHOULD BE SEGMENT(S) BELOW HIST AVG
                            if (selectionId >=0) {
                                int leftIndex = mainPointToHistMaxIndx[ edgeList[selectionId].x() ];
                                int rightIndex = mainPointToHistMaxIndx[ edgeList[selectionId].y() ];

                                natBreaksMax1.setX( mainPointsList[edgeList[selectionId].x()].x() );
                                natBreaksMax2.setX( mainPointsList[edgeList[selectionId].y()].x() );

                                bandWidth = abs( histogramMaxPoint[leftIndex].x() - histogramMaxPoint[rightIndex].x() );
                                bandCenter = histogramMaxPoint[leftIndex].x() + bandWidth/2 - imageWidth/2;
                                int bottomWidth = abs( histogramMaxPointPair[leftIndex].x() - histogramMaxPointPair[rightIndex].x() );
                                bandShape = (1.0*bottomWidth) / bandWidth;
                                /*qDebug() << histogramMaxPoint[leftIndex].x() << "," << histogramMaxPoint[rightIndex].x() << " " <<
                                            histogramMaxPointPair[leftIndex].x() << "," << histogramMaxPointPair[rightIndex].x() <<
                                            " topD: " << bandWidth << " btmD: " << bottomWidth  << " shape: " << bandShape << " center: " << bandCenter;    */

                                // ** BAND WIDTH SHOULD BE WIDE ENOUGH
                                if ( ((1.0*bandWidth)/imageWidth) < bandWidthMin) {
                                    state = false;
                                    bandCheck_errorState = 4;
                                } else {
                                    // ** BAND CENTER SHOULD BE CLOSE ENOUGH TO CENTER (CONSTANT)
                                    if ( ((1.0*abs(bandCenter))/imageWidth) > bandCenterMax ) {
                                        state = false;
                                        bandCheck_errorState = 5;
                                    } else {
                                        // ** BAND SHAPE SHOULD NOT BE TRIANGULAR, SHOULD BE CLOSE TO RECTANGLE
                                        if ( bandShape < bandShapeMin ) {
                                            state = false;
                                            bandCheck_errorState = 6;
                                        }
                                    }
                                }
                            } else {
                                state = false;
                                bandCheck_errorState = 7;
                            }
                        } else {
                            state = false;
                            bandCheck_errorState = 1;
                        }
                    } else {
                        state = false;
                        bandCheck_errorState = 3;
                    }
                }
            }
    } else {
        state = false;
        bandCheck_errorState = 10;
    }

    detected = state;
}

void imgProcess::histogramAnalysisDarkTracks(bool colored, bool invertHist){

    bool state = true;

    // CALCULATE HISTOGRAM
    if (colored)
        valueHistogram(false);
    else
        valueHistogramGray(false);


    histogramFiltered = new int[histogramSize];
    histogramAnlysInitSwitch = true;

    int noiseDia = (maFilterKernelSize-1)/2;
    histogramAvg = 0;
    histogramMin = 2000, histogramMax = 0;

    /* MOVING AVERAGE FILTER
    float sum;
    for (int x = 0; x < histogramSize; x++){
        if (x < noiseDia || x > (histogramSize-noiseDia))
           histogramFiltered[x] = histogram[x] ;
        else {
            sum = 0;
            for (int k = x-noiseDia; k <= x+noiseDia; k++)
                sum += histogram[k];
            histogramFiltered[x] = (sum*1.0) /  maFilterKernelSize;
        }

        histogramAvg += histogramFiltered[x];
        if (histogramFiltered[x] < histogramMin)
            histogramMin = histogramFiltered[x];
        if (histogramFiltered[x] > histogramMax)
            histogramMax = histogramFiltered[x];
    }
    */

    // * RECURSIVE MOVING AVERAGE FILTER
    float initialAvg = 0;
    for (int x = 0; x < maFilterKernelSize; x++)
        initialAvg += histogram[x];
    initialAvg /= (1.0*maFilterKernelSize);

    double value;
    for (int x = 0; x < histogramSize; x++){
        if ( x < noiseDia )
           histogramFiltered[x] = histogram[x] ;
        else if ( x == noiseDia )
            histogramFiltered[x] = initialAvg ;
        else if ( x > (histogramSize-noiseDia-1) )
            histogramFiltered[x] = histogramFiltered[x-1] ;
        else {
            value = histogramFiltered[x-1] + histogram[x+noiseDia] - histogram[x-noiseDia-1];
            histogramFiltered[x] = value;
        }

        histogramAvg += histogramFiltered[x];
        if (histogramFiltered[x] < histogramMin)
            histogramMin = histogramFiltered[x];
        if (histogramFiltered[x] > histogramMax)
            histogramMax = histogramFiltered[x];
    }

    if (histogramSize != 0)
        histogramAvg /= (1.0*histogramSize);
    else
        histogramAvg = -1;


    double yScaleFactor = (histogramSize*1.0) / (histogramMax-histogramMin);

    for (int x = 0; x < histogramSize; x++){
        if (invertHist)
            histogramFiltered[x] = histogramMax - yScaleFactor * (histogramFiltered[x]-histogramMin);
        else
            histogramFiltered[x] = yScaleFactor * (histogramFiltered[x]-histogramMin);
    }

    histogramAvg = 0;
    histogramMin = 2000, histogramMax = 0;

    for (int x = 0; x < histogramSize; x++){
        histogramAvg += histogramFiltered[x];
        if (histogramFiltered[x] < histogramMin)
            histogramMin = histogramFiltered[x];
        if (histogramFiltered[x] > histogramMax)
            histogramMax = histogramFiltered[x];
    }

    if (histogramSize != 0)
        histogramAvg /= histogramSize;
    else
        histogramAvg = -1;

    // CORNER FINDING ALGO BASED ON MAX AND MIN PEAK POINTS
    findMaxs(histogramFiltered, histogramSize, histogramPeaks);

    findMins(histogramFiltered, histogramSize, histogramMins);

    //--- MERGE PEAK AND MIN POINTS -----------------------------------------------------
    histogramExtremes.clear();
    int peaksIdx = 0;
    int minsIdx = 0;
    int x = 0;
    do {
        if (peaksIdx < histogramPeaks.size()){
            if ( histogramPeaks[peaksIdx].start <= x && x <= histogramPeaks[peaksIdx].end ){
                x = histogramPeaks[peaksIdx].end;
                range p;
                p.start = histogramPeaks[peaksIdx].start;
                p.end = histogramPeaks[peaksIdx].end;
                //qDebug() << "pp" << QString::number(p.start) << ", " << QString::number(p.end) << ", " << QString::number(histogramFiltered[ p.start ]);
                histogramExtremes.append(p);
            histogramExtremesFiltered.append(p);
                peaksIdx++;
            }
        }

        if (minsIdx < histogramMins.size()){
            if ( histogramMins[minsIdx].start <= x && x <= histogramMins[minsIdx].end ){
                range p;
                p.start = histogramMins[minsIdx].start;
                p.end = histogramMins[minsIdx].end;
                //qDebug() << "pp" << QString::number(p.start) << ", " << QString::number(p.end) << ", " << QString::number(histogramFiltered[ p.start ]);
                histogramExtremes.append(p);
            histogramExtremesFiltered.append(p);
                x = histogramMins[minsIdx].end;
                minsIdx++;
            }
        }
        x++;
    } while (x < histogramSize);
    //-----------------------------------------------------------------------------------


    if (!histogramExtremes.isEmpty()) {
/*
        //--- MERGE CLOSE POINTS ------------------------------------------------------------
        int deltaXThreshold = histogramSize * 0.02;
        int deltaYThreshold = histogramSize * 0.02;
        int deltaX, deltaY;
        histogramExtremesFiltered.clear();

        range zeroPoint;
        zeroPoint.start = histogramExtremes[0].start;
        zeroPoint.end = histogramExtremes[0].end;
        histogramExtremesFiltered.append(zeroPoint);
        int hisExtFltIndex = 0;

        for (int i=0; i<histogramExtremes.size()-1; i++) {

            deltaX = histogramExtremes[i+1].start - histogramExtremes[i].end;
            deltaY = histogramFiltered[ histogramExtremes[i+1].start ] - histogramFiltered[ histogramExtremes[i].end ];

            if (deltaX > deltaXThreshold || abs(deltaY) > deltaYThreshold) {

                //lenX = histogramExtremes[i].end - histogramExtremesFiltered[hisExtFltIndex].start;
                histogramExtremesFiltered[hisExtFltIndex].end = histogramExtremes[i].end;

                range nextPoint;
                nextPoint.start = histogramExtremes[i+1].start;
                nextPoint.end = histogramExtremes[i+1].end;
                histogramExtremesFiltered.append(nextPoint);
                hisExtFltIndex++;
            }

            //qDebug() << deltaX << " " << deltaY;
            /*
            range nextPoint;
            nextPoint.start = histogramExtremes[i].start;
            nextPoint.end = histogramExtremes[i].end;
            histogramExtremesFiltered.append(nextPoint);* /
        }
        //-----------------------------------------------------------------------------------

*/



            //--- EVALUATION -

            //bandCheck_errorState = 0;

    } else {
        state = false;
        //bandCheck_errorState = 10;
    }

    detected = state;
}

void imgProcess::detectScanHorizontal(int y){

    horLineVotes = new float*[edgeWidth];
    for (int j = 0; j < edgeWidth; j++)   horLineVotes[j] = new float[3];
    horLineVotesInitSwitch = true;

    int dist, max, maxDistance;
    float angle = 0, maxTheta = 0;

    for (int x = 0; x < edgeWidth; x++){
        max = 0;
        maxDistance = 0;
        maxTheta = 0;
        //if (edgeMapMatrix[y][x]){
            for (int i = 0; i < houghThetaSize; i++){
                angle = thetaMin + i * thetaStep;
                dist = (int) ((x - centerX) * cos(angle * R2D) + (y - centerY) * sin(angle * R2D));
                //qDebug() << "dist: " << dist << " angIdx: " << i;
                if (dist >= 0){
                    if (houghSpace[dist][i] > max){
                        max = houghSpace[dist][i];
                        maxDistance = dist;
                        maxTheta = angle;
                    }
                }
            }
            horLineVotes[x][0] = maxDistance;   // distance
            horLineVotes[x][1] = maxTheta;      // angle
            horLineVotes[x][2] = max;           // vote value
        /*} else {
            horLineVotes[x][0] = 0;
            horLineVotes[x][1] = 0;
            horLineVotes[x][2] = 0;
        }*/
    }

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
    if (sine >= 0.001)
        y = (int) ( (distance - x*cos(theta*R2D)) / sine );
    return y;
}


int imgProcess::getLineX(int y, float distance, float theta){

    int x = -1;
    float cosine = cos(theta*R2D);
    if (cosine >= 0.1)
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


int* imgProcess::valueHistogram(bool axis){

    int traceAxis, valueAxis;
    if (!axis){ // along X
        traceAxis = imageWidth;
        valueAxis = imageHeight;
    } else {    // along Y
        traceAxis = imageHeight;
        valueAxis = imageWidth;
    }
    histogram = new int[traceAxis];
    histogramInitSwitch = true;
    histogramSize = traceAxis;

    int sum;

    for(int t = 0; t < traceAxis; t++){
        sum = 0;
        for(int v = 0; v < valueAxis; v++) {
            if (!axis)
                sum += valueMatrixOrg[v][t];   //[y][x]
            else
                sum += valueMatrixOrg[t][v];   //[y][x]
        }
        histogram[t] = (1.0*sum) / valueAxis;
    }
    return histogram;
}

int* imgProcess::valueHistogramGray(bool axis){

    QImage image = imgOrginal.convertToFormat(QImage::Format_Grayscale8);
    //image.save("gray.jpg");
    QRgb rgbValue;
    QColor *color;
    int colorValue;

    int **_valueMatrix = new int*[image.height()];
    for (int i = 0; i < image.height(); i++) _valueMatrix[i] = new int[image.width()];

    for (int y = 0; y < image.height(); y++)
        for (int x = 0; x < image.width(); x++){
            rgbValue = image.pixel(x,y);
            color = new QColor(rgbValue);
            colorValue = color->value();

            if ( colorValue > 255) colorValue = 255;
            else if (colorValue < 0)  colorValue = 0;

            _valueMatrix[y][x] = colorValue;
            delete color;
        }

    int traceAxis, valueAxis;
    if (!axis){ // along X
        traceAxis = imageWidth;
        valueAxis = imageHeight;
    } else {    // along Y
        traceAxis = imageHeight;
        valueAxis = imageWidth;
    }
    histogram = new int[traceAxis];
    histogramInitSwitch = true;
    histogramSize = traceAxis;

    int sum;

    for(int t = 0; t < traceAxis; t++){
        sum = 0;
        for(int v = 0; v < valueAxis; v++) {
            if (!axis)
                sum += _valueMatrix[v][t];   //[y][x]
            else
                sum += _valueMatrix[t][v];   //[y][x]
        }
        histogram[t] = sum / valueAxis;
    }

    for (int y = 0; y < image.height(); y++) delete []_valueMatrix[y];
    delete []_valueMatrix;

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

    tLo = ( loValue + 0.33 * delta ) * 100  / 255.0;
    tHi = ( loValue + 0.67 * delta ) * 100  / 255.0;
}


QImage imgProcess::cornerImage( bool matrixFlag ){

    // matrixFlag; true: value, false: edge
    imgCorner = imgOrginal.copy();

//    if (detected){
        QRgb value;
        value = qRgb(255, 0, 0);        // red

        int xOffset = 0, yOffset = 0;

        if (!matrixFlag){   // edge matrix
            xOffset = yOffset = 1;
        }

        int X, Y;
        int deltaX = imageWidth * 0.03;
        int deltaY = imageHeight * 0.03;
        for (int x = -1*deltaX; x <= deltaX; x++){

            if (leftCornerX != -1 && leftCornerY != -1){
                X = leftCornerX + x + xOffset;
                Y = leftCornerY + yOffset;
                if ( X >= 0 && X < imgCorner.width() && Y >= 0 && Y < (imgCorner.height()-1)){
                    imgCorner.setPixel( X, Y-1, value );
                    imgCorner.setPixel( X, Y, value );
                    imgCorner.setPixel( X, Y+1, value );
                }
            }

            if (rightCornerX != -1 && rightCornerY != -1){
                X = rightCornerX + x + xOffset;
                Y = rightCornerY + yOffset;
                if ( X >= 0 && X < imgCorner.width() && Y >= 0 && Y < (imgCorner.height()-1)){
                    imgCorner.setPixel( X, Y-1, value );
                    imgCorner.setPixel( X, Y, value );
                    imgCorner.setPixel( X, Y+1, value );
                }
            }
        }

        for (int y = -1*deltaY; y <= deltaY; y++){

            if (leftCornerX != -1 && leftCornerY != -1){
                X = leftCornerX + xOffset;
                Y = leftCornerY + y + yOffset;
                if ( X >= 0 && X < (imgCorner.width()-1) && Y >= 0 && Y < imgCorner.height()){
                    imgCorner.setPixel( X-1, Y, value);
                    imgCorner.setPixel( X, Y, value);
                    imgCorner.setPixel( X+1, Y, value);
                }
            }

            if (rightCornerX != -1 && rightCornerY != -1){
                X = rightCornerX + xOffset;
                Y = rightCornerY + y + yOffset;
                if ( X >= 1 && X < imgCorner.width() && Y >= 0 && Y < imgCorner.height()){
                    imgCorner.setPixel( X-1, Y, value);
                    imgCorner.setPixel( X, Y, value);
                    imgCorner.setPixel( X+1, Y, value);
                }
            }

            X = trackCenterX + xOffset;
            Y = trackCenterY + y + yOffset;
            if ( X >= 0 && X < imgCorner.width() && Y >= 0 && Y < imgCorner.height()){
                imgCorner.setPixel( X-1, Y, value);
                imgCorner.setPixel( X, Y, value);
                imgCorner.setPixel( X+1, Y, value);
            }
        }
    //}
    return imgCorner;
}


QImage imgProcess::cornerAndPrimaryLineImage( solidLine line1, solidLine line2, int line2offset, bool matrixFlag ){

    imgCornerAndPrimaryLines = imgOrginal.copy();

    QRgb valueCorner, valuePrimary;

    valuePrimary = qRgb(0, 0, 255);     // blue

    int xOffset = 0, yOffset = 0;

    if (!matrixFlag){   // edge matrix
        xOffset = yOffset = 1;
    }

    int lineY;

    if ( line1.distance > 0 ) {

        for (int x = line1.start.x(); x <= line1.end.x(); x++){
            lineY = getLineY((x - centerX), line1.distance, line1.angle) + yOffset - centerY;

            if ( x >= 0 && x < imgCornerAndPrimaryLines.width() && lineY >= 0 && lineY < imgCornerAndPrimaryLines.height())
                imgCornerAndPrimaryLines.setPixel( x + xOffset, lineY, valuePrimary );
        }
    }

    if ( line2.distance > 0 ) {

        int startX = line2offset + line2.start.x();
        int endX = line2offset + line2.end.x();

        for (int x = startX; x <= endX; x++){
            lineY = getLineY((x - centerX), line2.distance, line2.angle) + yOffset - centerY;

            if ( x >= 0 && x < imgCornerAndPrimaryLines.width() && lineY >= 0 && lineY < imgCornerAndPrimaryLines.height())
                imgCornerAndPrimaryLines.setPixel( x + xOffset, lineY, valuePrimary );
        }
    }

    // draw corners and center
    valueCorner = qRgb(255, 0, 0);        // red
    int X, Y;

    for (int x = -8; x <= 8; x++){

        X = leftCornerX + x + xOffset;
        Y = leftCornerY + yOffset;
        if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height()){
            imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );
            imgCornerAndPrimaryLines.setPixel( X, Y+1, valueCorner );
        }

        X = rightCornerX + x + xOffset;
        Y = rightCornerY + yOffset;
        if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height()){
            imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );
            imgCornerAndPrimaryLines.setPixel( X, Y+1, valueCorner );
        }
    }

    for (int y = -8; y <= 8; y++){

        X = leftCornerX + xOffset;
        Y = leftCornerY + y + yOffset;
        if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height()){
            imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );
            imgCornerAndPrimaryLines.setPixel( X+1, Y, valueCorner );
        }

        X = rightCornerX + xOffset;
        Y = rightCornerY + y + yOffset;
        if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height())
            imgCornerAndPrimaryLines.setPixel( X-1, Y, valueCorner );
            imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );

        X = trackCenterX + xOffset;
        Y = trackCenterY + y + yOffset;
        if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height()){
            imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );
        }
    }

    return imgCornerAndPrimaryLines;
}

QImage imgProcess::getImageMainEdges( int number, bool matrixFlag ){

    imgSolidLines = imgOrginal.copy();

    int size;
    if (number == 0)                    // only center line
        size = 0;
    else
        size = number;                  // number of major lines + center line


    //QRgb valueCorner;
    QRgb valuePrimary;
    QRgb valueCenter;
    QRgb valueBlue;

    valuePrimary = qRgb(255, 0, 0);     // red
    valueCenter = qRgb(0, 255, 0);      // green
    valueBlue = qRgb(0, 0, 255);        // blue

    int xOffset = 0, yOffset = 0;
    int width = 0, height = 0;

    if (!matrixFlag){   // edge matrix
        xOffset = yOffset = 1;
        width = edgeWidth;
        height = edgeHeight;
    } else {
        width = imageWidth;
        height = imageHeight;
    }

    if ( !mainEdgesList.isEmpty() ){

        int lineX;

        if (size > mainEdgesList.size())
            size = mainEdgesList.size();

        for (int c = 0; c < size; c++){

            for (int y = 0; y < height; y++){
                //lineY = centerY - getLineY((x-centerX), houghLines[i][0], houghLines[i][1]);
                lineX = getLineX((y-centerY), mainEdgesList[c].distance, mainEdgesList[c].angle) - centerX;

                if (lineX > 0 && lineX < (width-1) )
                    for (int xx = -1; xx <= 1; xx++) {
                        if ( naturalBreaksNumber != 0 && (natBreaksMax1.y() == mainEdgesList[c].voteValue || natBreaksMax2.y() == mainEdgesList[c].voteValue) )
                            imgSolidLines.setPixel( lineX + xOffset + xx, y, valueBlue );
                        else
                            imgSolidLines.setPixel( lineX + xOffset + xx, y, valuePrimary );
                    }
            }
        }

        for (int y = 0; y < height; y++){
            //lineY = centerY - getLineY((x-centerX), houghLines[i][0], houghLines[i][1]);
            lineX = getLineX((y-centerY), centerLine.distance, centerLine.angle) - centerX;

            if (lineX > 0 && lineX < (width-1))
                for (int xx = -1; xx <= 1; xx++)
                    imgSolidLines.setPixel( lineX + xOffset + xx, y, valueCenter );
        }

    /*
        // draw corners and center
        valueCorner = qRgb(0, 255, 0);        // green
        int X, Y;

        for (int x = -4; x <= 4; x++){

            X = leftCornerX + x + xOffset;
            Y = leftCornerY + yOffset;
            if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height())
                imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );

            X = rightCornerX + x + xOffset;
            Y = rightCornerY + yOffset;
            if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height())
                imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );
        }

        for (int y = -4; y <= 4; y++){

            X = leftCornerX + xOffset;
            Y = leftCornerY + y + yOffset;
            if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height())
                imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );

            X = rightCornerX + xOffset;
            Y = rightCornerY + y + yOffset;
            if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height())
                imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );

            X = trackCenterX + xOffset;
            Y = trackCenterY + y + yOffset;
            if ( X >= 0 && X < imgCornerAndPrimaryLines.width() && Y >= 0 && Y < imgCornerAndPrimaryLines.height())
                imgCornerAndPrimaryLines.setPixel( X, Y, valueCorner );
        }
    */

    }
    return imgSolidLines;
}

QImage imgProcess::getImageMainEdges_2ndList( bool enableCenter, bool matrixFlag ) {

    imgSolidLines = imgOrginal.copy();

    //QRgb valueCorner;
    QRgb valuePrimary;
    QRgb valueCenter;

    valuePrimary = qRgb(255, 0, 0);     // red
    valueCenter = qRgb(0, 255, 0);     // green

    int xOffset = 0, yOffset = 0;
    int width = 0, height = 0;

    if (!matrixFlag){   // edge matrix
        xOffset = yOffset = 1;
        width = edgeWidth;
        height = edgeHeight;
    } else {
        width = imageWidth;
        height = imageHeight;
    }

    if ( !listHoughData2nd.isEmpty() ){

        int lineX;

        for (int c = 0; c < listHoughData2nd.size(); c++){

            for (int y = 0; y < height; y++){
                //lineY = centerY - getLineY((x-centerX), houghLines[i][0], houghLines[i][1]);
                lineX = getLineX((y-centerY), listHoughData2nd[c].distance, listHoughData2nd[c].angle) - centerX;

                if (lineX >= 0 && lineX < width)
                    imgSolidLines.setPixel( lineX + xOffset, y, valuePrimary );
            }
        }

        if (enableCenter){
            for (int y = 0; y < height; y++){
                //lineY = centerY - getLineY((x-centerX), houghLines[i][0], houghLines[i][1]);
                lineX = getLineX((y-centerY), centerLine.distance, centerLine.angle) - centerX;

                if (lineX >= 0 && lineX < width)
                    imgSolidLines.setPixel( lineX + xOffset, y, valueCenter );
            }
        }
    }
    return imgSolidLines;
}

QImage imgProcess::getImageContrast() {

    QImage img = imgOrginal.copy();
    QRgb black, white;
    black = qRgb(0, 0, 0);
    white = qRgb(255, 255, 255);
    img.fill(black);

    for(int y = 0; y < imageHeight; y++)
        for(int x = 0; x < imageWidth; x++)
            if (contrastMatrix[y][x] == 1) img.setPixel(x, y, white);

    return img;
}

QImage imgProcess::drawSolidLines( QList<solidLine> lineList ){

    imgSolidLines = imgOrginal.copy();

    QRgb valuePrimary;

//    valuePrimary = qRgb(255, 0, 0);     // red
    valuePrimary = qRgb(0, 0, 255);     // blue
    int lineY;

    for (int c = 0; c < lineList.size(); c++) {

        if ( lineList[c].distance > 0 ) {

            for (int x = lineList[c].start.x(); x <= lineList[c].end.x(); x++){
                lineY = getLineY((x - centerX), lineList[c].distance, lineList[c].angle) - centerY;

                if ( x >= 0 && x < imgSolidLines.width() && lineY >= 0 && lineY < imgSolidLines.height())
                    imgSolidLines.setPixel( x, lineY, valuePrimary );
            }
        }
    }
    return imgSolidLines;
}


QImage* imgProcess::drawSolidLines2EdgeMatrix( solidLine line, QImage::Format format ){

    QImage *image = new QImage(edgeWidth, edgeHeight, format);
    QRgb trackValue = qRgb(255, 255, 255);
    QRgb lineValue = qRgb(255, 0, 0);

    int lineY;

    for(int y = 0; y < edgeHeight; y++)
        for(int x = 0; x < edgeWidth; x++)
            if ( edgeMatrix[y][x] == 255 )
                image->setPixel(x, y, trackValue);

    if ( line.distance > 0 ) {

        for (int x = line.start.x(); x <= line.end.x(); x++){
            lineY = getLineY((x - centerX), line.distance, line.angle) - centerY;

            if ( x >= 0 && x < image->width() && lineY >= 0 && lineY < image->height())
                image->setPixel( x, lineY, lineValue );
        }
    }
    return image;
}

QImage imgProcess::drawLine2OrginalImage( solidLine line, QImage::Format format ){

    QImage image = imgOrginal.copy();
    QRgb rgbRed;
    rgbRed = qRgb(255, 0, 0);     // red

    int lineY;
    if ( line.distance > 0 ) {
        for (int x = line.start.x(); x <= line.end.x(); x++){
            lineY = getLineY((x - centerX), line.distance, line.angle) - centerY;

            if ( x >= 0 && x < image.width() && lineY >= 0 && lineY < image.height())
                image.setPixel( x, lineY, rgbRed );
        }
    }
    return image;
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


double imgProcess::membershipFn(double in, double k){

    return ( 1 / (1+abs(in-k)) );
}


double imgProcess::entropyFn(double in){

    return ( -1 * in * log(in) );
}


double imgProcess::calcEntropyMatrix(int windowSize){

    FEM_height = imageHeight - windowSize + 1;
    FEM_width = imageWidth - windowSize + 1;

    int winHalf = (windowSize - 1) / 2;
    int winSqr = pow(windowSize,2);

    fuzzyEntropyMatrix = new double*[FEM_height];
    for (int j = 0; j < FEM_height; j++)   fuzzyEntropyMatrix[j] = new double[FEM_width];
    fuzzyEntropyMatrixInitSwitch = true;

    double globalSum = 0;
    double sum;

    for (int y = 0; y < FEM_height; y++)
        for (int x = 0; x < FEM_width; x++){
            sum = 0;

            for (int yy = y-winHalf; yy <= (y+winHalf); yy++)
                for (int xx = x-winHalf; xx <= (x+winHalf); xx++)
                    sum += entropyFn( membershipFn(valueMatrixNorm[yy+winHalf][xx+winHalf], valueMatrixNorm[y+winHalf][x+winHalf]) );

            fuzzyEntropyMatrix[y][x] = sum / winSqr;
            globalSum += fuzzyEntropyMatrix[y][x];
        }

    return globalSum;
}

void imgProcess::findMaxs(int *array, int array_size, QList<range> &list){
    list.empty();

    int startIndex = 0;
    int refPoint, prevPoint;

    for (int i = 0; i < array_size; i++){

        if (i == 0){
            refPoint =  array[i];
            prevPoint = 0;
        } else {
            refPoint =  array[i];
            prevPoint =  array[i-1];
        }

        if ( refPoint > prevPoint ){
            startIndex = i;

            if (i == (array_size - 1)){
                range x;
                x.start = startIndex;
                x.end = startIndex;
                list.append(x);
            }

            for (int j = startIndex + 1; j < array_size; j++){
                if ( array[i] < array[j] ){  // < next, not maximum point
                    break;
                }
                else
                if ( array[i] > array[j] ){  // > next, maximum point

                    i = j;

                    range x;
                    x.start = startIndex;
                    x.end = j - 1;
                    list.append(x);
                    break;
                } else if (j == (array_size - 1)) {
                    range x;
                    x.start = startIndex;
                    x.end = j;
                    list.append(x);
                    break;
                }
            }
        }
    } // main for
}

void imgProcess::findMaxs(double *array, int array_size, QList<range> &list){
    list.empty();

    int startIndex = 0;
    double refPoint, prevPoint;

    for (int i = 0; i < array_size; i++){

        if (i == 0){
            refPoint =  array[i];
            prevPoint = 0;
        } else {
            refPoint =  array[i];
            prevPoint =  array[i-1];
        }

        if ( refPoint > prevPoint ){
            startIndex = i;

            if (i == (array_size - 1)){
                range x;
                x.start = startIndex;
                x.end = startIndex;
                list.append(x);
            }

            for (int j = startIndex + 1; j < array_size; j++){
                if ( array[i] < array[j] ){  // < next, not maximum point
                    break;
                }
                else
                if ( array[i] > array[j] ){  // > next, maximum point

                    i = j;

                    range x;
                    x.start = startIndex;
                    x.end = j - 1;
                    list.append(x);
                    break;
                } else if (j == (array_size - 1)) {
                    range x;
                    x.start = startIndex;
                    x.end = j;
                    list.append(x);
                    break;
                }
            }
        }
    } // main for
}

void imgProcess::findMins(int *array, int array_size, QList<range> &list){
    list.empty();

    int startIndex = 0;
    int refPoint, prevPoint;

    for (int i = 0; i < array_size; i++){

        if (i == 0){
            refPoint =  array[i];
            prevPoint = 0;
        } else {
            refPoint =  array[i];
            prevPoint =  array[i-1];
        }

        if ( refPoint < prevPoint ){
            startIndex = i;

            if (i == (array_size - 1)){
                range x;
                x.start = startIndex;
                x.end = startIndex;
                list.append(x);
            }

            for (int j = startIndex + 1; j < array_size; j++){
                if ( array[i] > array[j] ){  // < next, not maximum point
                    break;
                }
                else
                if ( array[i] < array[j] ){  // > next, maximum point

                    i = j;

                    range x;
                    x.start = startIndex;
                    x.end = j - 1;
                    list.append(x);
                    break;
                } else if (j == (array_size - 1)) {
                    range x;
                    x.start = startIndex;
                    x.end = j;
                    list.append(x);
                    break;
                }
            }
        }
    } // main for
}

void imgProcess::findMins(double *array, int array_size, QList<range> &list){
    list.empty();

    int startIndex = 0;
    double refPoint, prevPoint;

    for (int i = 0; i < array_size; i++){

        if (i == 0){
            refPoint =  array[i];
            prevPoint = 0;
        } else {
            refPoint =  array[i];
            prevPoint =  array[i-1];
        }

        if ( refPoint < prevPoint ){
            startIndex = i;

            if (i == (array_size - 1)){
                range x;
                x.start = startIndex;
                x.end = startIndex;
                list.append(x);
            }

            for (int j = startIndex + 1; j < array_size; j++){
                if ( array[i] > array[j] ){  // < next, not maximum point
                    break;
                }
                else
                if ( array[i] < array[j] ){  // > next, maximum point

                    i = j;

                    range x;
                    x.start = startIndex;
                    x.end = j - 1;
                    list.append(x);
                    break;
                } else if (j == (array_size - 1)) {
                    range x;
                    x.start = startIndex;
                    x.end = j;
                    list.append(x);
                    break;
                }
            }
        }
    } // main for
}

imgProcess::~imgProcess(){

    for (int y = 0; y < imageHeight; y++) delete []valueMatrix[y];
    delete []valueMatrix;

    for (int y = 0; y < imageHeight; y++) delete []valueMatrixOrg[y];
    delete []valueMatrixOrg;

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

    if ( histogramAnlysInitSwitch ) {
        delete []histogramFiltered;
    }

    if ( histogramDDInitSwitch ) {
        delete []histogramD;
        delete []histogramDD;
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

    if ( gaussianMatrixInitSwitch ) {
        for (int y = 0; y < gaussianMatrixSize; y++) delete []gaussianMatrix[y];
        delete []gaussianMatrix;
    }

    if ( horLineVotesInitSwitch ) {
        for (int y = 0; y < edgeWidth; y++) delete []horLineVotes[y];
        delete []horLineVotes;
    }

    if ( valueMatrixNormInitSwitch ) {
        for (int y = 0; y < imageHeight; y++) delete []valueMatrixNorm[y];
        delete []valueMatrixNorm;
    }

    if ( fuzzyEntropyMatrixInitSwitch ) {
        for (int y = 0; y < FEM_height; y++) delete []fuzzyEntropyMatrix[y];
        delete []fuzzyEntropyMatrix;
    }

    mainEdgesList.empty();

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
