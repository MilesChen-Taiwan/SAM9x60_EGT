/*
 * dash.cpp
 *
 *  Created on: Feb 1, 2024
 *      Author: miles
 */
/*
 * New_dashboard_20231120.cpp
 *
 *  Created on: Nov 20, 2023
 *      Author: miles
 */

/*
 * Copyright (C) 2018 Microchip Technology Inc.  All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 /*
 * dash.cpp
 *
 *  Created on: Jul 18, 2023
 *      Author: miles
 */
/*
 * Copyright (C) 2018 Microchip Technology Inc.  All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/*
 * Copyright (C) 2018 Microchip Technology Inc.  All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <chrono>
#include <egt/ui>
#include <memory>
#include <egt/ui>
#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <sys/time.h>
#include <random>
#include <chrono>
#include <queue>
#include <poll.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include <thread>
#include <errno.h>
#include <time.h>

class MySpinProgressF : public egt::SpinProgressF::SpinProgressType
{
public:
	explicit MySpinProgressF(const egt::Rect& rect) : egt::SpinProgressF::SpinProgressType(rect)
	{}
	void draw( egt::Painter& painter, const egt::Rect& rect) override
	{
		 egt::SpinProgressF::SpinProgressType::draw( painter, rect);
		 //egt::Drawer<egt::SpinProgressType<float>>::draw(*this, painter, rect);
		 Mydefault_draw(*this, painter, rect);
	}

private:
	void  Mydefault_draw( egt::SpinProgressType<float>& widget, egt::Painter& painter, const egt::Rect& rect)
    {
		egt::detail::ignoreparam(rect);

        widget.draw_box(painter,egt::Palette::ColorId::bg, egt::Palette::ColorId::border);

        auto b = widget.content_area();

        auto dim = std::min(b.width() * 0.9f, b.height()* 0.9f);
        float linew = dim / 5.0f;
        float radius = dim / 2.0f - (linew / 2.0f);
        auto angle1 = egt::detail::to_radians<float>(149, 0);

        auto min = std::min(widget.starting(), widget.ending());
        auto max = std::max(widget.starting(), widget.ending());
        auto angle2 = egt::detail::to_radians<float>(149.0f,
                                                egt::detail::normalize_to_angle(static_cast<float>(widget.value()),
                                                        static_cast<float>(min), static_cast<float>(max), 0.0f, 360.0f, true));


		painter.line_width(linew);
        painter.set(widget.color(egt::Palette::ColorId::button_fg,egt::Palette::GroupId::disabled));
        painter.draw(egt::Arc(widget.center(), radius, 0.0f, 2 * egt::detail::pi<float>()));
        painter.stroke();


		//painter.set(widget.color(egt::Palette::ColorId::button_fg,egt::Palette::GroupId::disabled));
       painter.set(widget.color(egt::Palette::ColorId::button_fg));
	   painter.set(egt::Color::css("#C0C0C0"));
	  // painter.set(egt::Palette::yellow);
        painter.draw(egt::Arc(widget.center(), radius, angle1, angle2));
        painter.stroke();

        if (widget.show_label())
        {
            std::string text = std::to_string(widget.value());
            auto f = egt::TextWidget::scale_font(egt::Size(dim * 0.9f, dim * 0.9f), text, widget.font());
            painter.set(f);
            painter.set(widget.color(egt::Palette::ColorId::text));
            auto size = painter.text_size(text);
            auto target = egt::detail::align_algorithm(size, b, egt::AlignFlag::center);
            painter.draw(target.point());
            painter.draw(text);
        }
    }
};
/*
void  Mydefault_draw( egt::SpinProgressType<float>& widget, egt::Painter& painter, const egt::Rect& rect)
{
      egt::detail::ignoreparam(rect);

        widget.draw_box(painter,egt::Palette::ColorId::bg, egt::Palette::ColorId::border);

        auto b = widget.content_area();

        auto dim = std::min(b.width(), b.height());
        float linew = dim / 10.0f;
        float radius = dim / 2.0f - (linew / 2.0f);
        auto angle1 = egt::detail::to_radians<float>(180, 0);

        auto min = std::min(widget.starting(), widget.ending());
        auto max = std::max(widget.starting(), widget.ending());
        auto angle2 = egt::detail::to_radians<float>(180.0f,
                                                egt::detail::normalize_to_angle(static_cast<float>(widget.value()),
                                                        static_cast<float>(min), static_cast<float>(max), 0.0f, 360.0f, true));

        painter.line_width(linew);
        painter.set(widget.color(egt::Palette::ColorId::button_fg, egt::Palette::GroupId::disabled));
        painter.draw(egt::Arc(widget.center(), radius, 0.0f, 2 * egt::detail::pi<float>()));
        painter.stroke();

        painter.set(widget.color(egt::Palette::ColorId::button_fg));
        painter.draw(egt::Arc(widget.center(), radius, angle1, angle2));
        painter.stroke();

        if (widget.show_label())
        {
            std::string text = std::to_string(widget.value());
            auto f = egt::TextWidget::scale_font(egt::Size(dim * 0.75f, dim * 0.75f), text, widget.font());
            painter.set(f);
            painter.set(widget.color(egt::Palette::ColorId::text));
            auto size = painter.text_size(text);
            auto target = egt::detail::align_algorithm(size, b, egt::AlignFlag::center);
            painter.draw(target.point());
            painter.draw(text);
        }
}
*/


#define ID_MIN 310
#define ID_MAX 390
#define STEPPER 2
#define MAX_NEEDLE_INDEX 40
#define HIGH_Q_TIME_THRESHHOLD 30000  //30ms
#define LOW_Q_TIME_THRESHHOLD  20000  //20ms


typedef enum __CAN_DEVICE__ {
  CAN_NONE = 0,
  CAN_NEEDLE,
  CAN_SPEED,
  CAN_GEAR,
  CAN_TEMP,
  CAN_FUEL,
  CAN_ENG5TC,
  CAN_PHONECALL,
  CAN_BAR,
  CAN_VSC,
  CAN_WIFI,
  CAN_BLINK
} CAN_DEVICE;
typedef enum __LIGHT__
{
	LIGHT_OFF = 0,
	LIGHT_ON = 1
}LIGHT;
union f2b
{
	float f;
	char b[4];
}F2B;
typedef struct __MOTO_STRUCT__
{
	uint needle_index;
	uint command;
	//uint battery_v;
	float battery_dot;
	float milage;
	float moto_speed;
	uint regen_mode;
	uint assistance_mode;
	uint drive_temp;
	uint rpm1;
	uint Status;

} MOTO_STRUCT, *pMORO_STRUCT;
MOTO_STRUCT motoValue;

#define assistance_Eco 1
#define assistance_Normal 2
#define assistance_Sport 3

#define Regen_Disable 0
#define Regen_Mild 1
#define Regen_Normal 2
#define Regen_Strong 3

#define moto_Disable 1
#define moto_Enable 2
uint rpm1_r;
uint assistance_mode_r,regen_mode_r , moto_on_r=moto_Disable;
uint assistance_mode_r_last,regen_mode_r_last  , moto_on_r_last=moto_Disable ;
bool Over_MSOFET_temp, Over_voltage, Under_voltage,moto_enable;

time_t now = time(0);
tm* ltm = localtime(&now);
int main(int argc, char** argv)
{
    egt::Application app(argc, argv);  //This call will cost ~270ms on 9x60ek board

#ifdef EXAMPLEDATA
    egt::add_search_path(EXAMPLEDATA);
#endif

    egt::TopWindow win,win1;
   // win.padding(10);
	win.show();
    win.color(egt::Palette::ColorId::bg, egt::Color::css("#1b1d43")); //#1b1d43 //#3b0030
	win1.color(egt::Palette::ColorId::bg, egt::Color::css("#3b0030")); //#1b1d43 //#3b0030
	win1.resize(egt::Size(win.width()*1.0,  win.height()*1.0));
	//std::_V2::rotate(10,100,100);
	//rotate_left();
	// CAN Socket process

	//motoValue.headlight = LIGHT_OFF;
	//motoValue.turnRightlight = LIGHT_OFF;
	//motoValue.turnLeftlight = LIGHT_OFF;
	auto TimerCAN = [](MOTO_STRUCT& val1) {
		int s;
		int nbytes,i=0;
		char send_char[8] = {0x10,0x10,0x11,0x12,0x13,0x14,0x15,0x1F};
		//char send_char[8] = {0x12,0x13,0x12,0x13,0x12,0x13,0x12,0x13};
		char send_char1[8] = {0x10,0x10,0x11,0x12,0x13,0x14,0x15};
		struct sockaddr_can addr;
		struct ifreq ifr;
		struct can_frame frame;

		if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
			perror("Socket");
			return 1;
		}
		//
		/*设置CAN帧的ID号，可区分为标准帧和扩展帧的ID号*/
		frame.can_id = 0x100;
		val1.needle_index = 0;
		strcpy((char *)(ifr.ifr_name), "can0");
		ioctl(s, SIOCGIFINDEX, &ifr);
		//printf("can0 can_ifindex = %x\n", ifr.ifr_ifindex);
		memset(&addr, 0, sizeof(addr));
		addr.can_family = AF_CAN;
		addr.can_ifindex = ifr.ifr_ifindex;

		/*将刚生成的套接字与CAN套接字地址进行绑定*/
		if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			perror("Bind");
			return 1;
		}
			//int can_comm=0;
				std::cout << "Message from CAN Thread >> can sendTo" << std::endl;

			//char *args3[] = {"sudo","ip","link","set","can0","down",NULL};
			//char *args4[] = {"sudo","ip","link","set","can0","up","type","can","bitrate","500000","restart-ms","10",NULL};
			usleep(1000);
			while(1) {



				frame.can_dlc = 5;
				if( moto_on_r !=  moto_on_r_last)
				{
					switch(moto_on_r)
					{
						case (moto_Disable):
							send_char1[0] = 0x24;send_char1[1] = 0xFF;send_char1[2] = 0xFF;send_char1[3] = 0xFF;send_char1[4] = 0x00;
							strcpy((char *)frame.data, send_char1);
							nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
						break;
						case (moto_Enable):
							send_char1[0] = 0x24;send_char1[1] = 0xFF;send_char1[2] = 0xFF;send_char1[3] = 0xFF;send_char1[4] = 0x01;
							strcpy((char *)frame.data, send_char1);
							nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
						break;
						default:
							send_char1[0] = 0x24;send_char1[1] = 0xFF;send_char1[2] = 0xFF;send_char1[3] = 0xFF;send_char1[4] = 0x00;
							strcpy((char *)frame.data, send_char1);
							usleep(1000);
							nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
						break;
					}
					moto_on_r_last =  moto_on_r;

				}

				if(assistance_mode_r_last != assistance_mode_r)
				{
					switch(assistance_mode_r)
					{
						case (assistance_Eco):
							send_char1[0] = 0x23;send_char1[1] = 0xFF;send_char1[2] = 0xFF;send_char1[3] = 0xFF;send_char1[4] = 0x01;
							strcpy((char *)frame.data, send_char1);
							nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
							std::cout <<  "assistance_Eco" << std::endl;
						break;
						case (assistance_Normal):
							send_char1[0] = 0x23;send_char1[1] = 0xFF;send_char1[2] = 0xFF;send_char1[3] = 0xFF;send_char1[4] = 0x02;
							strcpy((char *)frame.data, send_char1);
							nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
							std::cout <<  "assistance_Normal" << std::endl;
						break;
						case (assistance_Sport):
							send_char1[0] = 0x23;send_char1[1] = 0xFF;send_char1[2] = 0xFF;send_char1[3] = 0xFF;send_char1[4] = 0x03;
							strcpy((char *)frame.data, send_char1);
							nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
							std::cout <<  "assistance_Sport" << std::endl;
						break;
						default:
							send_char1[0] = 0x23;send_char1[1] = 0xFF;send_char1[2] = 0xFF;send_char1[3] = 0xFF;send_char1[4] = 0x00;
							strcpy((char *)frame.data, send_char1);
							nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
								std::cout <<  "invaild command: assistance_mode_r  " << static_cast<int> (frame.data[0]) << std::endl;
						break;
					}
					assistance_mode_r_last = assistance_mode_r;
				}

				else if(regen_mode_r_last != regen_mode_r)
				{
					switch(regen_mode_r)
					{

						case (Regen_Disable):
							send_char1[0] = 0x22;send_char1[1] = 0xFF;send_char1[2] = 0xFF;send_char1[3] = 0xFF;send_char1[4] = 0x00;
							strcpy((char *)frame.data, send_char1);
							nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
							std::cout <<  "Regen_Disable" << std::endl;
						break;
						case (Regen_Mild):
							send_char1[0] = 0x22;send_char1[1] = 0xFF;send_char1[2] = 0xFF;send_char1[3] = 0xFF;send_char1[4] = 0x01;
							strcpy((char *)frame.data, send_char1);
							nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
							std::cout <<  "Regen_Mild" << std::endl;
						break;
						case (Regen_Normal):
							send_char1[0] = 0x22;send_char1[1] = 0xFF;send_char1[2] = 0xFF;send_char1[3] = 0xFF;send_char1[4] = 0x02;
							strcpy((char *)frame.data, send_char1);
							nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
							std::cout <<  "Regen_Normal" << std::endl;
						break;
						case (Regen_Strong):
							send_char1[0] = 0x22;send_char1[1] = 0xFF;send_char1[2] = 0xFF;send_char1[3] = 0xFF;send_char1[4] = 0x03;
							strcpy((char *)frame.data, send_char1);
							nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
							std::cout <<  "Regen_Strong" << std::endl;
						break;
						default:
							send_char1[0] = 0x22;send_char1[1] = 0xFF;send_char1[2] = 0xFF;send_char1[3] = 0xFF;send_char1[4] = 0x00;
							strcpy((char *)frame.data, send_char1);
							nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
							std::cout <<  "invaild command: regen_mode_r :" << static_cast<int> (frame.data[0]) << std::endl;
						break;
					}
				regen_mode_r_last = regen_mode_r;
				}
				else
				{
					i++;
					send_char[0] = send_char[i];

					if(i==7)
						i=0;
					frame.can_dlc = 1;
					strcpy((char *)frame.data, send_char);
					nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
					if (nbytes < 0) {
						//printf("tCan_send");
						std::cout << "Can bus Write Error" << std::endl;
						usleep(1000*1000);
					}
				}

				if (nbytes < 0) {
					std::cout << "Can bus Write Error" << std::endl;
					usleep(1000*1000);
				}
				usleep(1000*10);

			}
	};

	//std::vector<std::unique_ptr<egt::PeriodicTimer>> timers1;
	std::thread tCan_send(TimerCAN, std::ref(motoValue));
	//tCan_send.join();



	auto threadCAN = [](MOTO_STRUCT& val) {

		int s;
		int nbytes;
		//char send_char[5] = {0x10,0x11,0x12,0x13,0x14};
		struct sockaddr_can addr;
		struct ifreq ifr;
		struct can_frame frame;

		if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
			perror("Socket");
			return 1;
		}


		//strcpy(ifr.ifr_name, "vcan0" );
		strcpy(ifr.ifr_name, "can0" );
		ioctl(s, SIOCGIFINDEX, &ifr);
		memset(&addr, 0, sizeof(addr));
		addr.can_family = AF_CAN;
		addr.can_ifindex = ifr.ifr_ifindex;
		if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			perror("Bind");
			return 1;
		}
		frame.can_dlc = 5;
		std::cout << "Message from CAN Thread" << std::endl;
		val.needle_index = 0;
		//char *args1[] = {"sudo","ip","link","set","can0","down",NULL};
		//char *args2[] = {"sudo","ip","link","set","can0","up","type","can","bitrate","500000","restart-ms","10",NULL};
		usleep(1000);
		while(1) {


			nbytes = read(s, &frame, sizeof(struct can_frame));

			if (nbytes < 0) {
				//printf("Read");
				//can bus recovery
				std::cout << "Can bus read Error" << std::endl;
				//execvp("/sbin/ip",args1);
					usleep(1000*1000);
				//execvp("/sbin/ip",args2);
				//return 1;
			}


			//val.command = frame.data[0];
			//if(!((frame.data[4]==0x13) && (frame.data[3]==0x12) && (frame.data[2]==0x11) && (frame.data[1]==0x10)))
			if(frame.can_id == 0x200)
			switch(frame.data[0])
			{
				case 0x10:
					//val.battery_v = (frame.data[4] | (frame.data[3]<<8));
					/*F2B.b[0]=frame.data[1];
					F2B.b[1]=frame.data[2];
					F2B.b[2]=frame.data[3];
					F2B.b[3]=frame.data[4];
					val.battery_dot =F2B.f; //(float*)(frame.data);*/

					val.battery_dot =  ((float)(frame.data[4] | frame.data[3]<<8) * 0.051625);

					//val.battery_dot = (frame.data[1] | (frame.data[2]<<8) | (frame.data[3] <<16) | (frame.data[4] <<24));
				break;
				case 0x11:
					//val.moto_speed = (frame.data[4] | (frame.data[3]<<8) | (frame.data[2] <<16) | (frame.data[1] <<24));
				break;
				case 0x12:
					val.regen_mode = (frame.data[4] | (frame.data[3]<<8) | (frame.data[2] <<16) | (frame.data[1] <<24));
					//std::cout <<  "val.regen_mode : " << static_cast<uint> (val.regen_mode) << std::endl;
					//std::cout <<  "val.regen_mode  : 0X" <<  std::hex << (val.regen_mode) << std::endl;
				break;
				case 0x13:
					val.assistance_mode = (frame.data[4] | (frame.data[3]<<8) | (frame.data[2] <<16) | (frame.data[1] <<24));
					//std::cout <<  "val.assistance_mode : " << static_cast<uint> (val.assistance_mode) << std::endl;
					//std::cout <<  "val.assistance_mode : 0X" <<  std::hex <<  (val.assistance_mode) << std::endl;
				break;
				case 0x14:
					val.drive_temp = (frame.data[4] | (frame.data[3]<<8) | (frame.data[2] <<16) | (frame.data[1] <<24));

				break;
				case 0x15:
					val.rpm1 = (frame.data[4] | (frame.data[3]<<8) | (frame.data[2] <<16) | (frame.data[1] <<24));
					// rpm1_r  val.rpm1
					val.moto_speed = static_cast<float> (val.rpm1 * 0.4147);
					if(val.rpm1)
					{
						if(rpm1_r > val.rpm1)
							val.rpm1 -= 1;
						else if(rpm1_r < val.rpm1)
							val.rpm1 += 1;
					}
					rpm1_r = val.rpm1;
				break;
				case 0x1F:
					//Over_MSOFET_temp, Over_voltage, Under_voltage,moto_enable;
					val.Status = (frame.data[4] | (frame.data[3]<<8) | (frame.data[2] <<16) | (frame.data[1] <<24));
					//std::cout <<  "A: Staus_massages (frame.data[1]) " << static_cast<int> (motoValue.Status) << std::endl;
					if(motoValue.Status & 0x00000001)
					//if((frame.data[4] && 0x01) )
						Over_MSOFET_temp = true;
					else
						Over_MSOFET_temp = false;
					if(motoValue.Status & 0x00000002)
					//if((frame.data[4] && 0x02))
						Over_voltage = true;
					else
						Over_voltage = false;
					if(motoValue.Status & 0x00000004)
					//if((frame.data[4] && 0x04) )
						Under_voltage = true;
					else
						Under_voltage = false;
					//if(motoValue.Status & 0x00000010)
					if(motoValue.Status & 0x01000000)
					//if((frame.data[4] && 0x10))
						moto_enable = true;
					else
						moto_enable = false;
					//std::cout <<  "B: Staus_massages (frame.data[1]) " << static_cast<int> (motoValue.Status) << std::endl;
				break;
				case 0x22:
					std::cout <<  "0x22 : " << std::hex <<  (frame.data[0]) << std::endl;
				break;
				case 0x23:
					std::cout <<  "0x23 : " << std::hex <<  (frame.data[0]) << std::endl;
				break;
				case 0x24:
					std::cout <<  "0x24 : " << std::hex <<  (frame.data[0]) << std::endl;
				break;
				default:
					//std::cout << "invaild command!" << std::endl;
					std::cout <<  "invaild command: can bus read : " << static_cast<int> (frame.data[0]) << std::endl;
                break;
			}

			//motoValue.moto_speed = val.moto_speed;
			/*val.speed_index = (frame.data[0]*15)/15;
			val.headlight = (LIGHT)frame.data[1];
			val.turnRightlight = (LIGHT)frame.data[2];
			val.turnLeftlight = (LIGHT)frame.data[3];*/
			usleep(1000*1);
		}
	};
	// end of CAN Process
	// end of CAN Process

	std::thread tCan(threadCAN, std::ref(motoValue));
    //auto logo = std::make_shared<egt::ImageLabel>(egt::Image("icon:egt_logo_white.png;128"));
    //win.add(top(center(logo)));

    // the gauge
    egt::experimental::Gauge gauge,gauge1;
    center(gauge);center(gauge1);

	MySpinProgressF speed_follower(egt::Rect(6.351,53.799,189.445,182.428));
	speed_follower.color(egt::Palette::ColorId::button_fg, egt::Palette::transparent, egt::Palette::GroupId::disabled);
	speed_follower.alpha(0.5);
	egt::PeriodicTimer time_timer_S(std::chrono::milliseconds(100));
	time_timer_S.on_timeout([&speed_follower]()
    {
	   speed_follower.value( (static_cast<float> (motoValue.moto_speed)/9));
    });
    time_timer_S.start();
	gauge.add(speed_follower);


	MySpinProgressF rpm_follower(egt::Rect(283.793,50.002,192.779,190.023));
	rpm_follower.color(egt::Palette::ColorId::button_fg, egt::Palette::transparent, egt::Palette::GroupId::disabled);
	rpm_follower.alpha(0.5);
	egt::PeriodicTimer time_timer_m(std::chrono::milliseconds(100));
	time_timer_m.on_timeout([&rpm_follower]()
    {
	   rpm_follower.value(((static_cast<float>(motoValue.rpm1))/12));
    });
    time_timer_m.start();
	gauge.add(rpm_follower);

	auto dash_background1 = std::make_unique<egt::SvgImage>("file:eng_mode.svg", egt::Size(win1.width()*1.0,  win1.height()*1.0));
	auto gauge_background1 = std::make_shared<egt::experimental::GaugeLayer>(dash_background1->render("#svg86"));
	gauge1.add(gauge_background1);

	auto dash_background = std::make_unique<egt::SvgImage>("file:Car_20240110_1.svg", egt::Size(win.width()*1.0,  win.height()*1.0));
	auto gauge_background = std::make_shared<egt::experimental::GaugeLayer>(dash_background->render("#Layer_1"));
	gauge.add(gauge_background);

    std::vector<std::unique_ptr<egt::PeriodicTimer>> timers;
/*
	egt::experimental::SVGDeserial smplab(win);
	smplab.move(egt::Point(0.0,0.0));
	smplab.resize(egt::Size(win.width()*1.0,  win.height()*1.0));

    auto circlePtr1 = smplab.AddWidgetByID("/root/eraw_Car_10/XMLID_64_.eraw", true);
	auto needle_point1 = circlePtr1->box().center();
	auto rpm_needle = smplab.AddRotateWidgetByID("/root/eraw_Car_10/XMLID_234_.eraw", 0, 1000, 0, 300, true, needle_point1);

	auto circlePtr2 = smplab.AddWidgetByID("/root/eraw_Car_10/XMLID_62_.eraw", true); //XMLID_9_.eraw //path5010-7.eraw
	auto needle_point2 = circlePtr2->box().center();
	auto mph_needle = smplab.AddRotateWidgetByID("/root/eraw_Car_10/XMLID_9_.eraw", 0, 600, 0, 235, true, needle_point2);

	rpm_needle->value(motoValue.rpm1);
	mph_needle->value((static_cast<float> (motoValue.moto_speed)));
	//fuel_needle->value(motoValue.battery_dot);


	egt::PeriodicTimer time_timer4(std::chrono::milliseconds(10));
	time_timer4.on_timeout([&rpm_needle]()
    {
      rpm_needle->value(motoValue.rpm1);
    });
    time_timer4.start();

	egt::PeriodicTimer time_timer5(std::chrono::milliseconds(10));
	time_timer5.on_timeout([&mph_needle]()
    {
      mph_needle->value((static_cast<float> (motoValue.moto_speed)));
    });
    time_timer5.start();
*/
	/*egt::PeriodicTimer time_timer6(std::chrono::milliseconds(10));
	time_timer6.on_timeout([&fuel_needle]()
    {
      fuel_needle->value(motoValue.battery_dot);
    });
    time_timer6.start();*/


   win.add(gauge);
	win1.add(gauge1);

    auto rpm_text = std::make_shared<egt::Label>("#XMLID_188_");
	rpm_text->box(egt::Rect(235.0,115.0,10.0,10.0));
	rpm_text->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFFFFFF));
	rpm_text->font(egt::Font(43, egt::Font::Weight::bold));
    rpm_text->text(" RPM");
    gauge.add(rpm_text);

	egt::PeriodicTimer time_timer(std::chrono::milliseconds(10));
	time_timer.on_timeout([&rpm_text]()
    {
       rpm_text->text(std::to_string(static_cast<int>(motoValue.rpm1)));
    });
    time_timer.start();


    auto speed_text = std::make_shared<egt::Label>();
    speed_text->box(egt::Rect(45.0, 115.0, 10.0,10.0));
	speed_text->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFFFFFF));
	speed_text->font(egt::Font(43, egt::Font::Weight::bold));
    speed_text->text("0 kph");
    gauge.add(speed_text);

	egt::PeriodicTimer time_timer1(std::chrono::milliseconds(10));
	time_timer1.on_timeout([&speed_text]()
    {
       speed_text->text(std::to_string(static_cast<int>(motoValue.moto_speed/10)));
    });
    time_timer1.start();

   auto rpm_text1 = std::make_shared<egt::Label>();
	rpm_text1->box(egt::Rect(40.0,25.0,10.0,10.0));
	rpm_text1->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0x32a852));
	rpm_text1->font(egt::Font(43, egt::Font::Weight::bold));
    rpm_text1->text(" RPM");
    gauge1.add(rpm_text1);

	egt::PeriodicTimer time_timer11(std::chrono::milliseconds(10));
	time_timer11.on_timeout([&rpm_text1]()
    {
       rpm_text1->text(std::to_string(static_cast<int>(motoValue.rpm1)));
    });
    time_timer11.start();


    auto speed_text1 = std::make_shared<egt::Label>();
    speed_text1->box(egt::Rect(120.0, 25.0, 10.0,10.0));
	speed_text1->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0x32a852));
	speed_text1->font(egt::Font(43, egt::Font::Weight::bold));
    speed_text1->text("0 kph");
    gauge1.add(speed_text1);

	egt::PeriodicTimer time_timer22(std::chrono::milliseconds(10));
	time_timer22.on_timeout([&speed_text1]()
    {
       speed_text1->text(std::to_string(static_cast<int>(motoValue.moto_speed/10)));
    });
    time_timer22.start();





    auto middle_box = dash_background->id_box("#middle");
    auto middle_text = std::make_shared<egt::Label>();
   //middle_text->text_align(egt::AlignFlag::center);
    middle_text->box(egt::Rect(middle_box.x()+220.0, middle_box.y()+105.0, middle_box.width(), middle_box.height()));
    middle_text->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    middle_text->text("25°C");
	middle_text->font(egt::Font(15, egt::Font::Weight::bold));
    gauge.add(middle_text);

	egt::PeriodicTimer time_timer2(std::chrono::milliseconds(10));
	time_timer2.on_timeout([&middle_text]()
    {
		float Vout,R2,N1,N2,N3,N4;
		Vout = (motoValue.drive_temp*(3.3/1024));
		R2 = (((3300*Vout)/(3.3-Vout)))/1000;
		N1 = (log(10)-log(R2))/3988;
		N2 = (1/298.15) - N1;
		N3 = 1/N2;
		N4 = N3-273.15;
		std::string str1 = std::to_string(static_cast<float>(N4));
		std::stringstream ss;
		ss << std::setprecision(3) << N4;
		str1 = ss.str();
		middle_text->text(str1+ "°C");
     // middle_text->text(std::to_string(static_cast<int>(motoValue.drive_temp)) + "°C");
    });
    time_timer2.start();


	//auto voltage_box = dash_background->id_box("#middle");
    auto voltage_text = std::make_shared<egt::Label>();
   //middle_text->text_align(egt::AlignFlag::center);
    //voltage_text->box(egt::Rect(middle_box.x()+10.0, middle_box.y(), middle_box.width(), middle_box.height()));
	voltage_text->box(egt::Rect(middle_box.x()+220.0, middle_box.y()+77.0,10,10));
    voltage_text->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    voltage_text->text("Battery");
	voltage_text->font(egt::Font(14, egt::Font::Weight::bold));
    gauge.add(voltage_text);

	egt::PeriodicTimer time_timer3(std::chrono::milliseconds(10));
	time_timer3.on_timeout([&voltage_text]()
    {
     std::string str1 = std::to_string(static_cast<float>(motoValue.battery_dot));
		std::stringstream ss;
		ss << std::setprecision(3) << motoValue.battery_dot;
		str1 = ss.str();

		voltage_text->text(str1+ "V");
    });
    time_timer3.start();








	//auto sys_date_box = dash_background->id_box("#XMLID_192_-1");
	//auto sys_time_box = dash_background->id_box("#XMLID_192_");

	auto sys_date =  std::make_shared<egt::Label>();
	auto sys_time =  std::make_shared<egt::Label>();
	//auto Remaining =  std::make_shared<egt::Label>();
	auto milage =  std::make_shared<egt::Label>();

	sys_date->box(egt::Rect(210.0,185.0, 10.0, 10.0 ));
    sys_date->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    sys_date->text("12.12.2023");
	sys_date->font(egt::Font(12, egt::Font::Weight::bold));
    gauge.add(sys_date);

	sys_time->box(egt::Rect(210.0,195.0, 10.0,10.0));
    sys_time->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    sys_time->text("12.12.2023");
	sys_time->font(egt::Font(12, egt::Font::Weight::bold));
    gauge.add(sys_time);


	auto sys_time1 =  std::make_shared<egt::Label>();
	sys_time1->box(egt::Rect(210.0,195.0, 10.0,10.0));
    sys_time1->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0x32a852));
    sys_time1->text("12.12.2023");
	sys_time1->font(egt::Font(12, egt::Font::Weight::bold));
	gauge1.add(sys_time1);


	/*Remaining->box(egt::Rect(227.0,133.0, 10.0,10.0));
    Remaining->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    Remaining->text("1000 KM");
	Remaining->font(egt::Font(12, egt::Font::Weight::bold));
    gauge.add(Remaining);*/

	milage->box(egt::Rect(225.0,160.0, 10.0,10.0));
    milage->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    milage->text("1000 KM");
	milage->font(egt::Font(12, egt::Font::Weight::bold));
    gauge.add(milage);

	egt::PeriodicTimer time_timer8(std::chrono::milliseconds(100));
	time_timer8.on_timeout([&milage]()
    {
		//float speed;
		//speed = float (motoValue.moto_speed);
		std::string str1 = std::to_string(static_cast<float>(motoValue.milage));
		std::stringstream ss;
		ss << std::setprecision(4) << motoValue.milage;
		str1 = ss.str();
		milage->text(str1+ " KM");

		//now = time(0);
		//ltm = localtime(&now);
		motoValue.milage += ((motoValue.moto_speed/36) * 0.1);
		if(motoValue.milage > 9999.9)
			motoValue.milage = 0.0;
	});
    time_timer8.start();


	egt::PeriodicTimer time_timer6(std::chrono::milliseconds(500));
	time_timer6.on_timeout([&sys_date]()
    {

		sys_date->text(std::to_string(static_cast<int>( 1900 + ltm->tm_year)) + "/" + std::to_string(static_cast<int>( 1 + ltm->tm_mon)) + "/" + std::to_string(static_cast<int>( ltm->tm_mday )));
		now = time(0);
		ltm = localtime(&now);
	});
    time_timer6.start();

	egt::PeriodicTimer time_timer7(std::chrono::milliseconds(500));
	time_timer7.on_timeout([&sys_time]()
    {
		sys_time->text(std::to_string(static_cast<int>( ltm->tm_hour))+":"+ std::to_string(static_cast<int>( ltm->tm_min))+":"+std::to_string(static_cast<int>( ltm->tm_sec )));
		now = time(0);
		ltm = localtime(&now);
	});
    time_timer7.start();



    auto console_text = std::make_shared<egt::Button>();
	console_text->box(egt::Rect(175.0, 30.0, 1.0, 1.0));
	console_text->font(egt::Font("Serif", 18, egt::Font::Weight::normal ,egt::Font::Slant::italic));
	console_text->text_align(egt::AlignFlag::left);
	console_text->color(egt::Palette::ColorId::button_bg, egt::Palette::transparent);


	gauge.add(console_text);

	auto regen_mode = std::make_shared<egt::Label>();
	regen_mode->box(egt::Rect(175.0, 30.0, 1.0, 1.0));
	regen_mode->font(egt::Font("Serif", 18, egt::Font::Weight::normal  , egt::Font::Slant::italic));
	regen_mode->text("ReGen");
	regen_mode->color(egt::Palette::ColorId::label_text, egt::Palette::aquamarine);
	gauge.add(regen_mode);

    auto change_state = [&console_text]()
	{
		static size_t state0 = 0;
		if(motoValue.regen_mode == 0xFFFFFF01)
		{
				state0 = 0;
		}
		else if(motoValue.regen_mode == 0xFFFFFF02)
		{
				state0 = 1;
		}
		else if(motoValue.regen_mode == 0xFFFFFF03)
		{
				state0 = 2;
		}
		else if(motoValue.regen_mode == 0xFFFFFF00)
		{
				state0 = 3;
		}
		else
		{
				regen_mode_r = Regen_Disable;
				regen_mode_r_last = Regen_Strong;
				state0 = 3;
		}

		console_text->on_event([&](egt::Event& event)
		{
			if(event.id() == egt::EventId::raw_pointer_down)
			{
				std::cout << "regen_mode clicked!" << std::endl;
				std::cout << "state0 =" << state0 << std::endl;
					if(state0 == 0)
					{
						regen_mode_r = Regen_Normal;
						regen_mode_r_last = Regen_Mild;
						state0=1;

					}
					else if(state0 == 1)
					{
						regen_mode_r = Regen_Strong;
						regen_mode_r_last = Regen_Normal;
						state0=2;

					}
					else if(state0 == 2)
					{
						regen_mode_r = Regen_Disable;
						regen_mode_r_last = Regen_Strong;
						state0=3;

					}
					else if(state0 == 3)
					{
						regen_mode_r = Regen_Mild;
						regen_mode_r_last = Regen_Disable;
						state0=0;

					}
					else
						std::cout <<  "regen_mode_r Error"  << std::endl;
					std::cout << "state0 =" << state0 << std::endl;

					//state0++;
					console_text->fill_flags().clear();
			}
		});

		const std::pair<const char*, egt::Color> states[] =
		{
            //{"P", egt::Palette::white},
            //{"R", egt::Palette::red},
           // {"N", egt::Palette::orange},
            {"Mild", egt::Palette::blue}, //Mild
            {"Normal", egt::Palette::white},//Normal
            {"Strong", egt::Palette::orange},//Strong
			{"Disable", egt::Palette::green},//Disable
			{"Error", egt::Palette::red},//Error
		};
        console_text->color(egt::Palette::ColorId::button_text, states[state0].second);
        console_text->text(states[state0].first);


	};
    change_state();

    timers.emplace_back(std::make_unique<egt::PeriodicTimer>(std::chrono::milliseconds(100)));
    timers.back()->on_timeout(change_state);
    timers.back()->start();






//=========================
    //auto console_box = dash_background->id_box("#console1");
    auto console1_text = std::make_shared<egt::Button>();
	//auto console1_text2 = std::make_shared<egt::ImageButton>();
	//egt::ImageButton console1_text(egt::Image("res:fullscreen_png"));


	//console_text->text_align(egt::AlignFlag::center);
    //console1_text->box(egt::Rect(console_box.x(), console_box.y(), console_box.width(), console_box.height()));
	//console1_text->box(egt::Rect(console_box.x()+200.0, console_box.y()+40.0, console_box.width(), console_box.height()));
	//console1_text->box(egt::Rect(rectangle_of_rotated());
    //console1_text->font(egt::Font(30, egt::Font::Weight::bold));
	console1_text->box(egt::Rect(252.0, 30.0, 10.0, 10.0));
	console1_text->font(egt::Font("Serif",18, egt::Font::Weight::normal  , egt::Font::Slant::italic));
	console1_text->text_align(egt::AlignFlag::left);
	gauge.add(console1_text);

	auto assistance_mode = std::make_shared<egt::Label>();
	assistance_mode->box(egt::Rect(252.0, 30.0, 10.0, 10.0));
	assistance_mode->font(egt::Font("Serif", 18, egt::Font::Weight::normal  , egt::Font::Slant::italic));
	assistance_mode->text("Assist");
	assistance_mode->color(egt::Palette::ColorId::label_text, egt::Palette::aquamarine);
	gauge.add(assistance_mode);

    auto change_state1 = [&console1_text]()
    {
        static size_t state1 = 0;

		if(motoValue.assistance_mode == 0xFFFFFF01)
				state1 = 0;
		else if(motoValue.assistance_mode == 0xFFFFFF02)
				state1 = 1;
		else if(motoValue.assistance_mode == 0xFFFFFF03)
				state1 = 2;
		else if(motoValue.assistance_mode == 0xFFFFFF00)
				state1 = 3;
		else
				state1 = 0;

		console1_text->on_event([&](egt::Event& event1)
		{
			if(event1.id() == egt::EventId::raw_pointer_down)
			{
				std::cout <<  "assistance_mode clicked!"  << std::endl;


				if(state1 == 0)
				{
					assistance_mode_r = assistance_Normal;
					assistance_mode_r_last = assistance_Eco;
					state1 = 1;
				}
				else if(state1 == 1)
				{
					assistance_mode_r = assistance_Sport;
					assistance_mode_r_last = assistance_Normal;
					state1 = 2;
				}
				else if(state1 == 2)
				{
					assistance_mode_r = assistance_Eco;
					assistance_mode_r_last = assistance_Sport;
					state1 = 0;
				}
				else if(state1 == 3)
					state1=0;
				else
					std::cout <<  "assistance_mode Error"  << std::endl;
				//state1++;
				console1_text->fill_flags().clear();
			}
		});


        const std::pair<const char*, egt::Color> states1[] =
        {
            //{"P", egt::Palette::white},
            //{"R", egt::Palette::red},
           // {"N", egt::Palette::orange},
            {"Eco", egt::Palette::blue}, //Eco
            {"Normal", egt::Palette::white},//Normal
            {"Sport", egt::Palette::orange},//Sport
			{"None", egt::Palette::green},//Inactive
			{"Error", egt::Palette::red},//Error
        };

        console1_text->color(egt::Palette::ColorId::button_text, states1[state1].second);
        console1_text->text(states1[state1].first);
		console1_text->color(egt::Palette::ColorId::button_bg , egt::Palette::transparent);
       // if (++state >= egt::detail::size(states))
         //   state = 0;





    };
    change_state1();

    timers.emplace_back(std::make_unique<egt::PeriodicTimer>(std::chrono::milliseconds(100)));
    timers.back()->on_timeout(change_state1);
    timers.back()->start();


	auto power_key_on = std::make_shared<egt::ImageButton>(egt::Image("file:power_on.png"));
	power_key_on->color(egt::Palette::ColorId::button_bg , egt::Palette::transparent);
	power_key_on->box(egt::Rect(0.0, 0.0, 50.0, 50.0));

	gauge.add(power_key_on);


	auto power_key_off = std::make_shared<egt::ImageButton>(egt::Image("file:power_off.png"));
	power_key_off->color(egt::Palette::ColorId::button_bg , egt::Palette::transparent);
	power_key_off->box(egt::Rect(0.0, 0.0, 50.0, 50.0));

	gauge.add(power_key_off);


	power_key_on->on_event([&](egt::Event& event_power)
	{
		if(event_power.id() == egt::EventId::raw_pointer_down)
		{
			std::cout <<  "event_power On clicked!"  << std::endl;
			std::cout <<  "moto_on_r:"  << static_cast<int> (moto_on_r) << std::endl;
			if(moto_on_r == moto_Disable)
			{
				power_key_on->show();
				power_key_off->hide();
				moto_on_r = moto_Enable;
			}
			else if(moto_on_r == moto_Enable)
			{
				moto_on_r = moto_Disable;
				power_key_on->hide();
				power_key_off->show();
			}
			power_key_on->fill_flags().clear();


		}

	});

	power_key_off->on_event([&](egt::Event& event_power1)
	{
		if(event_power1.id() == egt::EventId::raw_pointer_down)
		{
			std::cout <<  "event_power off clicked!"  << std::endl;
			std::cout <<  "moto_on_r:"  << static_cast<int> (moto_on_r) << std::endl;

			if(moto_on_r == moto_Enable)
			{
				moto_on_r = moto_Disable;
				power_key_on->hide();
				power_key_off->show();
			//	win.show();
			//	win1.hide();
			}
			else if(moto_on_r == moto_Disable)
			{
				power_key_on->show();
				power_key_off->hide();
				moto_on_r = moto_Enable;
			//	win.hide();
			//	win1.show();
			}
			power_key_off->fill_flags().clear();
		}

	});

	auto Staus_massages = std::make_shared<egt::Label>();
	Staus_massages->box(egt::Rect(180.0, 220.0, 1.0, 10.0));
	//Staus_massages->font(egt::Font(25, egt::Font::Weight::bold));
	Staus_massages->font(egt::Font("Serif", 20, egt::Font::Weight::normal  , egt::Font::Slant::italic));
	Staus_massages->text_align(egt::AlignFlag::left);
	gauge.add(Staus_massages);

    auto change_state_Staus_massages = [&Staus_massages]()
    {
        static size_t state_Staus_massages = 0;
        const std::pair<const char*, egt::Color> states1[] =
        {
            {"MOS Overtemp", egt::Palette::red},
            {"Over Voltage", egt::Palette::white},
            {"UnderVoltage", egt::Palette::orange},
			{"MotorEnabled", egt::Palette::green},
			{"MotorDisabled", egt::Palette::blue},
			{"Initial!",egt::Palette::yellow}
        };

        Staus_massages->color(egt::Palette::ColorId::label_text, states1[state_Staus_massages].second);
        Staus_massages->text(states1[state_Staus_massages].first);


		//Over_MSOFET_temp, Over_voltage, Under_voltage,moto_enable;
		if(Over_MSOFET_temp)
			state_Staus_massages = 0;
		else if(Over_voltage)
			state_Staus_massages = 1;
		else if(Under_voltage)
			state_Staus_massages = 2;
		else if(moto_enable)
			state_Staus_massages = 3;
		else
			state_Staus_massages = 4;
		/*
		state_Staus_massages++;
		if(state_Staus_massages == 5)
			state_Staus_massages=0;*/

    };
    change_state_Staus_massages();

    timers.emplace_back(std::make_unique<egt::PeriodicTimer>(std::chrono::milliseconds(100)));
    timers.back()->on_timeout(change_state_Staus_massages);
    timers.back()->start();








	auto slider1 = std::make_shared<egt::Slider>(-32678, 32767);
	slider1->color(egt::Palette::ColorId::button_bg , egt::Palette::transparent);
	slider1->box(egt::Rect(20.0, 150.0, 150.0, 20.0));
    slider1->value(0);
	//slider1->slider_flags().set({egt::Slider::SliderFlag::round_handle, egt::Slider::SliderFlag::show_label});
    //slider1->slider_flags().set({egt::Slider::SliderFlag::show_labels});
	//slider1->show_label(false);
	int slider_value = slider1->value();
	gauge1.add(slider1);



	egt::PeriodicTimer time_timer_slider1(std::chrono::milliseconds(100));
	time_timer_slider1.on_timeout([&slider1,&slider_value]()
    {
		slider_value = slider1->value();
    });
    time_timer_slider1.start();


   	auto slider_text = std::make_shared<egt::Label>();
	slider_text->box(egt::Rect(20.0, 90.0, 10.0, 10.0));
    slider_text->color(egt::Palette::ColorId::label_text,  egt::Color::rgb(0x32a852));
	slider_text->font(egt::Font(50, egt::Font::Weight::bold));
    gauge1.add(slider_text);

	egt::PeriodicTimer time_timer_slider(std::chrono::milliseconds(100));
	time_timer_slider.on_timeout([&slider_text,&slider_value]()
    {
		slider_text->text(std::to_string(static_cast<int>(slider_value)));

    });
    time_timer_slider.start();


	auto send_value = std::make_shared<egt::ImageButton>(egt::Image("file:btn_icon_green_turn_down_left.png"));
	send_value->color(egt::Palette::ColorId::button_bg , egt::Palette::transparent);
	send_value->box(egt::Rect(180.0, 100.0, 50.0, 50.0));

	gauge1.add(send_value);

	//egt::ProgressBarType test(egt::Rect(260, 200, 0,), 0.0 , 100.0 , 10.0);
	//gauge.add(test);
	//egt::ProgressBarF test1();
	//gauge.add(test1);

	/*
	auto battery_bar = std::make_shared<egt::ProgressBarF>();
	battery_bar->color(egt::Palette::ColorId::label_text, egt::Palette::white);
	battery_bar->font(egt::Font("Serif", 10, egt::Font::Weight::normal  , egt::Font::Slant::italic));
	battery_bar->box(egt::Rect(260.0, 200.0, 0.0, 0.0));
	battery_bar->value(100);
    battery_bar->show_label(false);
   // grid0->add(egt::expand(battery_bar));
	gauge.add(battery_bar);
	*/


/*
	// Construct the SVG handler instance
    egt::experimental::SVGDeserial evdash(win);

	 // Define the local variables
    //std::queue<QueueCallback> high_pri_q;
    //std::queue<QueueCallback> low_pri_q;
    std::vector<std::shared_ptr<egt::experimental::GaugeLayer>> NeedleBase;
    std::string lbd_ret = "0";
    std::ostringstream path;
    bool is_increasing = true;
    bool is_needle_finish = false;
    //bool is_high_q_quit = true;
   // bool is_low_q_quit = true;
    int needle_index = 0;
    //int speed_index = 0;
    //int timer_cnt = 0;

	  // Deserial the background and some blinking widgets
   // evdash.AddWidgetByID("/root/eraw/png.eraw", true);
    auto bluetooth = evdash.AddWidgetByID("/root/eraw/bluetooth.eraw", true);
    auto loading = evdash.AddWidgetByID("/root/eraw/loading.eraw", true);

    // Deserial the progress bar of battery
    for (auto i = ID_MIN, j =0; i <= ID_MAX; i += STEPPER, j++)
    {
        path.str("");
        if (316 == i)
            path << "eraw/polygon" << std::to_string(i) << ".eraw";
        else
            path << "eraw/path" << std::to_string(i) << ".eraw";

        NeedleBase.push_back(evdash.AddWidgetByID(path.str(), true));
    }

    // Handle the battery progress bar animation
    auto needle_move = [&]()
    {
        for (int i=0; i<1; i++)
        {
            if (is_increasing)
            {
                NeedleBase[needle_index]->show();
            }
            else
            {
                NeedleBase[needle_index]->hide();
            }

            if (is_increasing && MAX_NEEDLE_INDEX == needle_index)
            {
                is_increasing = false;
                is_needle_finish = true;
            }
            else if (!is_increasing && 0 == needle_index)
            {
                is_increasing = true;
                is_needle_finish = true;
            }
            else
            {
                needle_index = is_increasing ? needle_index + 1 : needle_index - 1;
            }
        }

        return "needle_move";
    };
	needle_move();

    timers.emplace_back(std::make_unique<egt::PeriodicTimer>(std::chrono::milliseconds(20)));
    timers.back()->on_timeout(needle_move);
    timers.back()->start();


	*/




	auto PreviousKey = std::make_shared<egt::ImageButton>(egt::Image("file:previous.png")); //<------
	PreviousKey->color(egt::Palette::ColorId::button_bg , egt::Palette::transparent);
	PreviousKey->box(egt::Rect(430.0, 0.0, 50.0, 50.0));

	gauge1.add(PreviousKey);

	auto NextKey = std::make_shared<egt::ImageButton>(egt::Image("file:next.png")); // --->
	NextKey->color(egt::Palette::ColorId::button_bg , egt::Palette::transparent);
	NextKey->box(egt::Rect(430.0, 0.0, 50.0, 50.0));

	gauge.add(NextKey);

	PreviousKey->on_event([&](egt::Event& PreviousKeyEvent)
	{
		if(PreviousKeyEvent.id() == egt::EventId::raw_pointer_down)
		{
			std::cout <<  "PreviousKey On clicked!"  << std::endl;

			win.show();
			win1.hide();

			NextKey->show();
			PreviousKey->hide();

			PreviousKey->fill_flags().clear();
		}

	});




	NextKey->on_event([&](egt::Event& NextKeyEvent)
	{
		if(NextKeyEvent.id() == egt::EventId::raw_pointer_down)
		{
			std::cout <<  "NextKey On clicked!"  << std::endl;

			win1.show();
			win.hide();

			PreviousKey->show();
			NextKey->hide();
			NextKey->fill_flags().clear();
		}

	});




    dash_background.reset(nullptr);
    win.show();

    return app.run();

}




