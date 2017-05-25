#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "F:\Programs\opencv\build\include\opencv2\core\core.hpp"
#include "F:\Programs\opencv\build\include\opencv2\highgui/highgui.hpp"
#include "F:\Programs\opencv\build\include\opencv\cv.h"
#include <QTCore>
#include <fstream>
#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//asbdkajsbdkajsndla
    if(!capWebcam.open(0)){
        ui->textArea->appendPlainText("error on webcam open");
    }

    cv::waitKey(1000);

    if(capWebcam.isOpened() == false)
    {
        ui->textArea->appendPlainText("error opening webcam");
        return;
    }

    cv::waitKey(1000);

    tmrTimer = new QTimer(this);
    connect(tmrTimer , SIGNAL(timeout()) , this , SLOT(processFrameAndUpdateGUI()));
    tmrTimer->start(200);

    connect(ui->captureImageButton, SIGNAL(released()), this, SLOT(captureImage()));
    connect(ui->reognizeButton, SIGNAL(released()), this, SLOT(recognizeLetters()));
}

void MainWindow::updateCamera(){
    if(!capWebcam.read(matOriginal)){
        ui->textArea->appendPlainText("Cannot read frame");
    }


    if(matOriginal.empty() == true){
        ui->textArea->appendPlainText("empty");
        return;
    }


    cv::cvtColor(matOriginal,matOriginal,CV_BGR2RGB);
    cv::Mat element;

    cv::inRange(matOriginal, cv::Scalar(0,0,0), cv::Scalar(100,100,100), matContours);

    element = getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(matContours, matContours, cv::MORPH_GRADIENT, element);

    cv::findContours(matContours.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    count = 0;

    for( size_t i = 0; i < contours.size(); i++ )
    {
        cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true) * 0.02,  true);

        if (std::fabs(cv::contourArea(contours[i])) < 300 || std::fabs(cv::contourArea(contours[i])) > 2000)
            continue;

        cv::Rect objectBoundingRectangle;
        objectBoundingRectangle = boundingRect(contours.at(i));

        if(objectBoundingRectangle.area() > 5000)
            continue;
        cv::rectangle(matOriginal, objectBoundingRectangle, cv::Scalar(255,0,0));

        letters.push_back(objectBoundingRectangle);
        count++;

        for(size_t j = 0; j < approx.size() - 1; j++)
            cv::line(matOriginal,approx[j],approx[j+1],cv::Scalar(0,255,0),2,8);
    }
    letters.clear();
}

double MainWindow::compareLetters(QString path, cv::Mat letter){
    cv::Mat templImg;
    templImg = cvLoadImage(path.toStdString().c_str(), CV_LOAD_IMAGE_GRAYSCALE);

    if(templImg.empty()){
        //ui->textArea->appendPlainText(QString("Could not load image"));
        return -1;
    }

    cv::resize(letter, letter, cv::Size(templImg.cols,templImg.rows));
    //cv::resize(templImg, templImg, cv::Size(letter.cols,letter.rows));
    //cv::compare(letter, templImg, overlap, CV_CMP_EQ);

    int matchingPixels = 0;

    for(int i = 0; i < letter.rows; i++){
        for(int j = 0; j < letter.cols; j++){
            if(letter.at<uchar>(i,j) != 1 && letter.at<uchar>(i,j) == templImg.at<uchar>(i,j))
                matchingPixels++;
        }
    }

//    if(matchingPixels > 14000){
//        return matchingPixels;
//    }
    return (double)matchingPixels / (templImg.cols * templImg.rows);
}

void MainWindow::recognizeLetters(){
    foundText.clear();
    sortRectangles();

    for(int i = 0; i < count; i++){
        int index;
        double maxPixels = 0;

        cv::Mat croppedImage;
        char *path = "E:\\work\\recognizeWriting\\trainingData\\";

        croppedImage = matContours(letters[i]).clone();

        for(int j = 0; j < 27; j++){
            char folder = 'A' + j;

            QString fullPath = path;
            fullPath.append(folder);
            fullPath.append("\\");

            const char * dataCount = getDataCount();
            for(int k = 1; k < std::atoi(dataCount); k++){
                QString newPath = fullPath;
                newPath.append(k + '0');
                newPath.append(".bmp");

                double currentPixels = compareLetters(newPath, croppedImage);

                if(currentPixels > maxPixels){
                    //ui->textArea->appendPlainText(QString::number(k));
                    maxPixels = currentPixels;
                    index = j;
                }
            }
        }

        cv::putText(matOriginal, QString((char) index + 'A').toStdString().c_str(), cv::Point(letters[i].x, letters[i].y), CV_FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(255,0,0));
        printf("%c%f\n",((char) index + 'A'), maxPixels);

        foundText.append((char) index + 'A');
        if(i < count - 1)
            if(letters[i+1].y - letters[i].y > 10)
                foundText.append('\n');

        //ui->textArea->appendPlainText(QString((char) index + 'A') + QString::number(maxPixels));
    }

    QImage image((uchar*)matOriginal.data, matOriginal.cols, matOriginal.rows, matOriginal.step, QImage::Format_RGB888);

    QImage qimgOriginal((uchar*)matOriginal.data, matOriginal.cols, matOriginal.rows, matOriginal.step, QImage::Format_RGB888);

    ui->lblContours->setPixmap(QPixmap::fromImage(qimgOriginal));

    if(image.save("E:\\work\\recognizeWriting\\pics\\img1.jpg", 0, -1)){
        //ui->textArea->appendPlainText("saved");
    }else{
        ui->textArea->appendPlainText("save failed");
    }

    ui->textArea->appendPlainText(foundText);
}

void MainWindow::sortRectangles(){
    //sort by rows
    cv::Rect aux;
    for(int i = 0; i < count - 1; i++){
        for(int j = i + 1; j < count; j++){
            if(letters[j].y < letters[i].y){
                aux = letters[i];
                letters[i] = letters[j];
                letters[j] = aux;
            }
        }
    }
    //sort by columns
    for(int i = 0; i < count - 1; i++){
        for(int j = i + 1; j < count; j++){
            if(letters[j].x < letters[i].x && abs(letters[j].y - letters[i].y) < 10){
                aux = letters[i];
                letters[i] = letters[j];
                letters[j] = aux;
            }
        }
    }
    /*char *text = (char *)malloc(5);

    for(int i = 0; i < count; i++){
        itoa(i, text, 10);
        cv::putText(matOriginal, text, cv::Point(letters[i].x, letters[i].y), CV_FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(255,0,0));
    }*/
}

void MainWindow::captureImage(){
    if(count != 26){
        ui->textArea->appendPlainText("invalid format: not 26 detected chars");
        return;
    }
    sortRectangles();

    QImage image((uchar*)matOriginal.data, matOriginal.cols, matOriginal.rows, matOriginal.step, QImage::Format_RGB888);

    if(image.save("E:\\work\\recognizeWriting\\pics\\img.jpg", 0, -1)){
        ui->textArea->appendPlainText("saved");
    }else{
        ui->textArea->appendPlainText("save failed");
    }

    cv::Mat croppedImage;
    char *path = "E:\\work\\recognizeWriting\\trainingData\\";

    const char * dataCount = getDataCount();
    for(int i = 0; i < count; i++){
        croppedImage = matContours(letters[i]).clone();
        cv::cvtColor(croppedImage, croppedImage, CV_GRAY2RGB);
        cv::resize(croppedImage,croppedImage,cv::Size(croppedImage.cols * 4, croppedImage.rows * 4));

        QImage image((uchar*)croppedImage.data, croppedImage.cols, croppedImage.rows, croppedImage.step, QImage::Format_RGB888);
        char folder = 'A' + i;

        QString fullPath = path;
        fullPath.append(folder);
        fullPath.append("\\");
        fullPath.append(getDataCount());
        fullPath.append(".bmp");
        ui->textArea->appendPlainText(fullPath);


        if(image.save(fullPath, 0, -1)){
            ui->textArea->appendPlainText("saved");
        }else{
            ui->textArea->appendPlainText("save failed");
        }
    }

    int c = std::atoi(getDataCount()) + 1;
    std::ofstream file;
    file.open("E:\\work\\recognizeWriting\\trainingData\\data.txt");

    ui->textArea->appendPlainText(QString::number(c));
    file << "input data count: " << c;

    file.close();
}

const char* MainWindow::getDataCount(){
    std::ifstream file;
    file.open("E:\\work\\recognizeWriting\\trainingData\\data.txt");

    std::string line;

    if (file.is_open())
      {
        getline(file,line);
        file.close();

        //ui->textArea->appendPlainText(QString(line.substr(line.size() - 1, line.size()).c_str()));

        return line.substr(line.size() - 1, line.size()).c_str();
    }

    //return nullptr;
}

void MainWindow::processFrameAndUpdateGUI()
{
    updateCamera();

    //ui->textArea->appendPlainText("asd");

    QImage qimgOriginal((uchar*)matOriginal.data, matOriginal.cols, matOriginal.rows, matOriginal.step, QImage::Format_RGB888);
    //QImage qimgContours((uchar*)matContours.data, matContours.cols, matContours.rows, matContours.step, QImage::Format_Indexed8);

    ui->cameraFeed->setPixmap(QPixmap::fromImage(qimgOriginal));
    //ui->lblContours->setPixmap(QPixmap::fromImage(qimgContours));
}

MainWindow::~MainWindow()
{
    delete ui;
}
