#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::string devicePath = "/dev/video0";  // Явно указываем путь к устройству
    cv::VideoCapture cap(devicePath);

    if (!cap.isOpened()) {
        std::cerr << "Ошибка: Не удалось открыть камеру по пути " << devicePath << std::endl;
        return -1;
    }

    std::cout << "Камера открыта: " << devicePath << std::endl;

    cv::Mat frame;
    std::string windowName = "Камера";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL); // Разрешаем менять размер окна

    // Попробуем установить более низкое разрешение
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "❌ Пустой кадр — возможно, камера не даёт изображение\n";
            continue;
        }

        std::cout << "✅ Кадр: " << frame.cols << "x" << frame.rows << ", Тип: " << frame.type() << std::endl;
        cv::imshow(windowName, frame);

        if (cv::waitKey(1) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
