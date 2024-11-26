#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVector>
#include <QImage>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadImage();
    void applyHistogramEqualization();
    void applyLinearContrast();

private:
    QLabel *imageLabelBefore;
    QLabel *imageLabelAfter;
    QLabel *histogramLabelBefore;
    QLabel *histogramLabelAfter;
    QPushButton *loadImageButton;
    QPushButton *histogramEqualizationButton;
    QPushButton *linearContrastButton;
    QImage image;

    QVector<int> calculateHistogram(const QImage &image);
    QImage drawHistogram(const QVector<int> &histogram);
    QImage histogramEqualization(const QImage &source);
    QImage linearContrastEnhancement(const QImage &source);
};

#endif // MAINWINDOW_H
