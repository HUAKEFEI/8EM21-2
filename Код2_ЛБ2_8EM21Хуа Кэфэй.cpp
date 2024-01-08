#include <iostream>
#include <fstream>
#include <vector>
#include "matplotlibcpp.h"

namespace plt = matplotlibcpp;

int main() {
    std::ifstream file1("a1.txt");
    std::ifstream file2("b1.txt");
    std::ifstream file3("a2.txt");
    std::ifstream file4("b2.txt");

    if (file1.is_open() && file2.is_open() && file3.is_open() && file4.is_open()) {
        std::vector<double> a1, b1, a2, b2;
        double value1, value2, value3, value4;
        while (file1 >> value1, file2 >> value2) {
            a1.push_back(value1); // x坐标递增
            b1.push_back(value2); // 从文件1中读取的值作为y坐标
        }
        file1.close();

        while (file3 >> value3, file4 >> value4) {
            a2.push_back(value3); // x坐标递增
            b2.push_back(value4); // 从文件1中读取的值作为y坐标
        }
        file2.close();

        plt::plot(a1, b1, "b-"); // 第一条线用蓝色
        plt::plot(a2, b2, "r-"); // 第二条线用红色
        plt::xlabel("Index");
        plt::ylabel("Value");
        plt::title("Comparison Plot");

        plt::show();
    }
    else {
        std::cout << "Unable to open files" << std::endl;
    }

    return 0;
}

