#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "F:\Programs\opencv\build\include\opencv2\core\core.hpp"
#include "F:\Programs\opencv\build\include\opencv2\highgui/highgui.hpp"
#include "F:\Programs\opencv\build\include\opencv/cv.h"

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    int count;

    cv::VideoCapture capWebcam;

    QTimer* tmrTimer;

    cv::Mat matOriginal;
    cv::Mat matContours;

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Point> approx;
    std::vector<cv::Rect> letters;
    QString foundText;

    void updateCamera();
    void sortRectangles();
    double compareLetters(QString, cv::Mat);
    const char * getDataCount();

private slots:
    void processFrameAndUpdateGUI();
    void captureImage();
    void recognizeLetters();
};

#endif // MAINWINDOW_H
