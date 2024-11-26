#include "mainwindow.h"
#include <QFileDialog>
#include <QImageReader>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    resize(800, 600);

    imageLabelBefore = new QLabel(this);
    imageLabelAfter = new QLabel(this);
    histogramLabelBefore = new QLabel(this);
    histogramLabelAfter = new QLabel(this);
    loadImageButton = new QPushButton("Load Image", this);
    histogramEqualizationButton = new QPushButton("Histogram Equalization", this);
    linearContrastButton = new QPushButton("Linear Contrast", this);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(loadImageButton);
    buttonLayout->addWidget(histogramEqualizationButton);
    buttonLayout->addWidget(linearContrastButton);

    QHBoxLayout *imageLayout = new QHBoxLayout;
    imageLayout->addWidget(imageLabelBefore);
    imageLayout->addWidget(imageLabelAfter);

    QHBoxLayout *histogramLayout = new QHBoxLayout;
    histogramLayout->addWidget(histogramLabelBefore);
    histogramLayout->addWidget(histogramLabelAfter);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(imageLayout);
    mainLayout->addLayout(histogramLayout);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    connect(loadImageButton, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(histogramEqualizationButton, &QPushButton::clicked, this, [this]() { applyHistogramEqualization(); });
    connect(linearContrastButton, &QPushButton::clicked, this, [this]() { applyLinearContrast(); });
}

MainWindow::~MainWindow() {}

void MainWindow::loadImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image File", "", "Images (*.png *.xpm *.jpg)");
    if (!fileName.isEmpty()) {
        QImageReader reader(fileName);
        reader.setAutoTransform(true);
        image = reader.read();
        if (image.isNull()) {
            QMessageBox::information(this, "Image Processing App",
                                     tr("Cannot load %1: %2")
                                         .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
            return;
        }

        int maxWidth = 800;
        int maxHeight = 600;
        if (image.width() > maxWidth || image.height() > maxHeight) {
            image = image.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio);
        }

        imageLabelBefore->setPixmap(QPixmap::fromImage(image));

        QVector<int> histogram = calculateHistogram(image);
        histogramLabelBefore->setPixmap(QPixmap::fromImage(drawHistogram(histogram)));
    }
}

void MainWindow::applyHistogramEqualization() {
    if (!image.isNull()) {
        QImage result = histogramEqualization(image);
        imageLabelAfter->setPixmap(QPixmap::fromImage(result));

        QVector<int> histogram = calculateHistogram(result);
        histogramLabelAfter->setPixmap(QPixmap::fromImage(drawHistogram(histogram)));
    }
}

void MainWindow::applyLinearContrast() {
    if (!image.isNull()) {
        QImage result = linearContrastEnhancement(image);
        imageLabelAfter->setPixmap(QPixmap::fromImage(result));

        QVector<int> histogram = calculateHistogram(result);
        histogramLabelAfter->setPixmap(QPixmap::fromImage(drawHistogram(histogram)));
    }
}

QVector<int> MainWindow::calculateHistogram(const QImage &image) {
    QVector<int> histogram(256, 0);
    int width = image.width();
    int height = image.height();

    for (int y = 0; y < height; ++y) {
        const QRgb *line = (const QRgb*)image.scanLine(y);
        for (int x = 0; x < width; ++x) {
            int gray = qGray(line[x]);
            histogram[gray]++;
        }
    }

    return histogram;
}

QImage MainWindow::drawHistogram(const QVector<int> &histogram) {
    int histogramHeight = 100;
    QImage histogramImage(256, histogramHeight, QImage::Format_RGB32);
    histogramImage.fill(Qt::white);

    QPainter painter(&histogramImage);
    painter.setPen(Qt::black);

    int maxCount = *std::max_element(histogram.begin(), histogram.end());

    for (int i = 0; i < 256; ++i) {
        int barHeight = static_cast<int>((static_cast<double>(histogram[i]) / maxCount) * histogramHeight);
        painter.drawLine(i, histogramHeight, i, histogramHeight - barHeight);
    }

    return histogramImage;
}

QImage MainWindow::histogramEqualization(const QImage &source) {
    QImage result = source;
    int width = source.width();
    int height = source.height();

    QVector<int> histogram = calculateHistogram(source);
    QVector<int> cdf(256, 0);
    cdf[0] = histogram[0];
    for (int i = 1; i < 256; ++i) {
        cdf[i] = cdf[i - 1] + histogram[i];
    }

    int cdfMin = 0;
    for (int i = 0; i < 256; ++i) {
        if (cdf[i] != 0) {
            cdfMin = cdf[i];
            break;
        }
    }

    int totalPixels = width * height;
    QVector<int> equalized(256, 0);
    for (int i = 0; i < 256; ++i) {
        equalized[i] = (cdf[i] - cdfMin) * 255 / (totalPixels - cdfMin);
        equalized[i] = qBound(0, equalized[i], 255);
    }

    for (int y = 0; y < height; ++y) {
        QRgb *line = (QRgb*)result.scanLine(y);
        for (int x = 0; x < width; ++x) {
            int gray = qGray(line[x]);
            int newGray = equalized[gray];
            line[x] = qRgb(newGray, newGray, newGray);
        }
    }

    return result;
}

QImage MainWindow::linearContrastEnhancement(const QImage &source) {
    QImage result = source;
    int width = source.width();
    int height = source.height();

    int minR = 255, maxR = 0;
    int minG = 255, maxG = 0;
    int minB = 255, maxB = 0;

    for (int y = 0; y < height; ++y) {
        const QRgb *line = (const QRgb*)source.scanLine(y);
        for (int x = 0; x < width; ++x) {
            int r = qRed(line[x]);
            int g = qGreen(line[x]);
            int b = qBlue(line[x]);

            minR = std::min(minR, r);
            maxR = std::max(maxR, r);
            minG = std::min(minG, g);
            maxG = std::max(maxG, g);
            minB = std::min(minB, b);
            maxB = std::max(maxB, b);
        }
    }

    if (minR == maxR) minR = 0, maxR = 255;
    if (minG == maxG) minG = 0, maxG = 255;
    if (minB == maxB) minB = 0, maxB = 255;

    for (int y = 0; y < height; ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < width; ++x) {
            int r = qRed(line[x]);
            int g = qGreen(line[x]);
            int b = qBlue(line[x]);

            int newR = 255 * (r - minR) / (maxR - minR);
            int newG = 255 * (g - minG) / (maxG - minG);
            int newB = 255 * (b - minB) / (maxB - minB);

            line[x] = qRgb(newR, newG, newB);
        }
    }

    return result;
}
