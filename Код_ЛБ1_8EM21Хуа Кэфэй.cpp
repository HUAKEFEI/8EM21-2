﻿// Circle_1.cpp: 定义应用程序的入口点。
//

#include "circle.h"
#include <iomanip>
#include <string>
#include <cmath>
#include <fstream>
#include <matplotlibcpp.h>
#include <vector>
#include <chrono>



#define PI 3.1415926
namespace plt = matplotlibcpp;

class MyRobotino {
public:
	MyRobotino(std::string ip_addr) {
		init_motors();
		this->ip_addr = ip_addr;
	}
	std::string ip_addr;
	size_t col_width = 9;

	void init_motors() {
		for (unsigned int i = 0; i < motor_num; i++) {
			motor[i].setMotorNumber(i);
		}
	}

	class MyCom : public Com {
	public:
		MyCom() : Com() {}
		void errorEvent(const char* errorString) {
			std::cerr << "Error : " << errorString << std::endl;
		}
		void connectedEvent() {
			std::cout << " Connected ." << std::endl;
		}
		void connectionClosedEvent() {
			std::cout << " Connection closed ." << std::endl;
		}
	};

	MyCom com;
	size_t motor_num = 3;
	Motor motor[3];
	float gear_ratio = 16;
	float wheel_rad = 0.04f; //m
	float L = 0.125; // dist from robot center to wheel center[m]
	float K = (60 * gear_ratio) / (2 * PI * wheel_rad);
	float speed_vec_task[3];
	float speed_vec_calc[3];
	float wheel_freq_vec_calc[3];
	float wheel_freq_vec_real[3];
	float x_cor_task = 0; //in m
	float y_cor_task = 0; // in m
	float x_cor_current = 0; //in m
	float y_cor_current = 0; // in m

	void connect() {
		// Connect
		std::cout << " Connecting ...";
		com.setAddress(ip_addr.c_str());
		com.connectToServer(true);
		if (!com.isConnected()) {
			std::cout << std::endl << " Could not connect to " <<
				com.address() << std::endl;
			rec::robotino::api2::shutdown();
			exit(1);
		}
		else {
			std::cout << " success " << std::endl;
		}
	}





	void go_trajectory() {

		const unsigned int duration_ms = 7000;
		const unsigned int sleep_ms = 20;
		const float start_x_speed = 0.0f; // in m/s
		const float start_y_speed = -0.1f; // in m/s
		float x_speed = start_x_speed; // in m/s
		float y_speed = start_y_speed; // in m/s
		x_cor_task = 0; //in m
		y_cor_task = 0; // in m
		float circle_R = 0.10f; // in m

		x_cor_current = 0.3f * std::sin(1 * 0 + PI / 2); //in
		y_cor_current = 0.3f * std::sin(2 * 0); // in m
		float rad = 0.0f;
		unsigned int start_time = com.msecsElapsed();
		unsigned int time = com.msecsElapsed();
		unsigned int time_prev = time;
		//print_headline_to_csv();
		//print_headline();
		speed_vec_task[0] = x_speed;
		speed_vec_task[1] = y_speed;
		speed_vec_task[2] = 0;
		while ((time - start_time) <= duration_ms) {

			//Треактория Лиссажу (восьмерка)
			rad = 2 * (time - start_time) / duration_ms;
			x_cor_task = 0.3f * std::sin(1 * rad);
			y_cor_task = 0.3f * std::sin(2 * rad);

			//calc needed wheel freqs
			inverse_kinematics(speed_vec_task);
			//set needed wheel freqs

			motor[0].setSpeedSetPoint(wheel_freq_vec_calc[0]);

			motor[1].setSpeedSetPoint(wheel_freq_vec_calc[1]);

			motor[2].setSpeedSetPoint(wheel_freq_vec_calc[2]);

			//sleep to give robot time to change whee speed

			rec::robotino::api2::msleep(sleep_ms);

			//get real wheel freqs
			for (size_t i = 0; i < motor_num; i++) {
				wheel_freq_vec_real[i] = motor[i].actualVelocity();
			}
			//calc real speed
			forward_kinematics(wheel_freq_vec_real);
			//integrate coors
			time_prev = time;
			time = com.msecsElapsed();
			//time should be converted from ms to sec
			x_cor_current += speed_vec_calc[0] * (float)(time - time_prev) / 1000;
			y_cor_current += speed_vec_calc[1] * (float)(time - time_prev) / 1000;

			//x_points.push_back(x_cor_current);
			//y_points.push_back(y_cor_current);


			std::cout << x_cor_task << std::endl;
			std::cout << y_cor_task << std::endl;
			std::cout << x_cor_current << std::endl;
			std::cout << y_cor_current << std::endl;
			std::ofstream outputFile("output.txt");
			if (outputFile.is_open()) {
				outputFile << x_cor_current << std::endl;
				outputFile.close();
			}
			else {
				//std::cout << "无法创建文件 Невозможно создать файл output.txt" << std::endl;
			}
		}
	}
	//обратная кинематика
	void inverse_kinematics(float* speed_vec) {
		wheel_freq_vec_calc[0] = K * (-sin(PI / 3) * speed_vec[0] + cos(PI / 3) * speed_vec[1] +
			L * speed_vec[2]);
		wheel_freq_vec_calc[1] = K * (-sin(PI) * speed_vec[0] +
			cos(PI) * speed_vec[1] + L * speed_vec[2]);
		wheel_freq_vec_calc[2] = K * (-sin(5 * PI / 3) * speed_vec[0] + cos(5 * PI / 3) * speed_vec[1] + L * speed_vec[2]);
	}

	//прямая кинематика
	void forward_kinematics(float* wheel_freq_vec) {

		speed_vec_calc[0] = 1 / K * (-0.577f * wheel_freq_vec[0] + 0.577f * wheel_freq_vec[2]);
		speed_vec_calc[1] = 1 / K * (0.333f * wheel_freq_vec[0] + (-0.667f) * wheel_freq_vec[1] + 0.333f * wheel_freq_vec[2]);
		speed_vec_calc[2] = 1 / K * (2.667f * wheel_freq_vec[0] + 2.667f * wheel_freq_vec[1] + 2.667f * wheel_freq_vec[2]);
	}
	void disconnect() {
		com.disconnectFromServer();
		rec::robotino::api2::shutdown();
	}



};

int main(int argc, char** argv) {
	// Default IP address Robotino 4
	std::string ip_addr = "192.168.0.1";
	MyRobotino robotino(ip_addr);
	robotino.connect();
	robotino.go_trajectory();
	robotino.disconnect();


	return 0;
}




