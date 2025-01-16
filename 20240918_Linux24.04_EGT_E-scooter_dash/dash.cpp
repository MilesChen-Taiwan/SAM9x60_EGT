/*
 * dash.cpp
 *
 *  Created on: Sep 18, 2024
 *      Author: miles
 */

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
#include <vector>
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

#include <egt/ui>
#include <egt/uiloader.h>
#include <egt/form.h>
#include <egt/shapes.h>
#include <egt/themes/midnight.h>
#include <egt/themes/shamrock.h>
#include <egt/themes/sky.h>
#include <egt/painter.h>


#define ID_MIN 310
#define ID_MAX 390
#define STEPPER 2
#define MAX_NEEDLE_INDEX 40
#define HIGH_Q_TIME_THRESHHOLD 30000  //30ms
#define LOW_Q_TIME_THRESHHOLD  20000  //20ms
using namespace std;

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
	uint VDC;
	uint Iq;
	uint wheel_pulses;
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
 
bool win_type =0; 
//float score[5] = {1.878,1.878,1.878,1.878,999.99};
//char name[5][256] = {"kph_ratio_v", "RPM_ratio_v", "I_bus_ratio_v", "Iq_ratio_v","motoValue.milage"};
//float score_w[5] = {0.87,0.87,0.87,0.87,9999.9};

static std::string last_line(const std::string& str)
{
    std::stringstream ss(str);
    std::string line;
    while (std::getline(ss, line, '\n'))
    {}
    return line;
}

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
	   //painter.set(egt::Color::css("#C0C0C0"));
	   //#6667AB
	   painter.set(egt::Color::css("#FDF200"));
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


uint rpm1_r,counter;;
uint assistance_mode_r,regen_mode_r , moto_on_r=moto_Disable;
uint assistance_mode_r_last,regen_mode_r_last  , moto_on_r_last=moto_Disable ;
bool Over_MSOFET_temp, Over_voltage, Under_voltage,moto_enable,CAN_BUS_Error;

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
	
		int i=0;
		char send_char[11] = {0x10,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x1F};
		//char send_char[8] = {0x12,0x13,0x12,0x13,0x12,0x13,0x12,0x13};
		char send_char1[11] = {0x10,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};
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
			usleep(100);

			while(1) {
				int nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
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

					if(i==10)
						i=0;
					frame.can_dlc = 1;
					strcpy((char *)frame.data, send_char);
					nbytes = sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
					
					
					if (nbytes < 0) {
						//printf("tCan_send");
						std::cout << "Can bus Write Error_sendto" << std::endl;
						usleep(1000*1000);
						motoValue.battery_dot  = 0;
						motoValue.drive_temp = 0;
						motoValue.rpm1 = 0; 
						motoValue.moto_speed =0;
						//motoValue.VDC = 0;
						//motoValue.Iq = 0; 
						//motoValue.wheel_pulses =0;
						CAN_BUS_Error = true;
					}
					else
					CAN_BUS_Error = false;
				}
				/*
				//while(sendto(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr)) < 0);
				if (nbytes < 0) {
					std::cout << "Can bus Write Error #2" << std::endl;
					usleep(1000*1000);
				}*/
				
				
				usleep(1000*25);
				
			}
			
	};

	//std::vector<std::unique_ptr<egt::PeriodicTimer>> timers1;
	std::thread tCan_send(TimerCAN, std::ref(motoValue));
	//tCan_send.detach();

	



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
		usleep(100);
		while(1) {
		
			
			nbytes = read(s, &frame, sizeof(struct can_frame));
			//while((read(s, &frame, sizeof(struct can_frame)) <0));
			
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
					//uint VDC;
					//uint Iq;
					//uint wheel_pulses;
				case 0x16:	
					//16h : read VDC Bus current
					val.VDC = (frame.data[4] | (frame.data[3]<<8) | (frame.data[2] <<16) | (frame.data[1] <<24));
				break;
				case 0x17:
					//17h: read Iq current
					val.Iq  = (frame.data[4] | (frame.data[3]<<8) | (frame.data[2] <<16) | (frame.data[1] <<24));
				break;
				case 0x18:
					//18h: read wheel pulses / Sec
					val.wheel_pulses = (frame.data[4] | (frame.data[3]<<8) | (frame.data[2] <<16) | (frame.data[1] <<24));
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
				//default:
					//std::cout << "invaild command!" << std::endl;
					//std::cout <<  "invaild command: can bus read : " << static_cast<int> (frame.data[0]) << std::endl;
               // break;
			}

			//motoValue.moto_speed = val.moto_speed;
			/*val.speed_index = (frame.data[0]*15)/15;
			val.headlight = (LIGHT)frame.data[1];
			val.turnRightlight = (LIGHT)frame.data[2];
			val.turnLeftlight = (LIGHT)frame.data[3];*/
			usleep(50);
			
		}
	};
	// end of CAN Process
	// end of CAN Process

	std::thread tCan(threadCAN, std::ref(motoValue));
	//tCan.detach();
    //auto logo = std::make_shared<egt::ImageLabel>(egt::Image("icon:egt_logo_white.png;128"));
    //win.add(top(center(logo)));

    // the gauge
    egt::experimental::Gauge gauge,gauge1;
    center(gauge);center(gauge1);

	MySpinProgressF speed_follower(egt::Rect(8.542,92.231,317.658,322.538));
	speed_follower.color(egt::Palette::ColorId::button_fg, egt::Palette::transparent, egt::Palette::GroupId::disabled);
	speed_follower.alpha(0.3);
	egt::PeriodicTimer time_timer_S(std::chrono::milliseconds(10));
	time_timer_S.on_timeout([&speed_follower]()
    {
	   speed_follower.value( (static_cast<float> (motoValue.moto_speed)/9));
    });
    time_timer_S.start();
	gauge.add(speed_follower);


	MySpinProgressF rpm_follower(egt::Rect(466.416,85.479,331.576,336.042));
	rpm_follower.color(egt::Palette::ColorId::button_fg, egt::Palette::transparent, egt::Palette::GroupId::disabled);
	rpm_follower.alpha(0.3);
	egt::PeriodicTimer time_timer_m(std::chrono::milliseconds(10));
	time_timer_m.on_timeout([&rpm_follower]()
    {
	   rpm_follower.value(((static_cast<float>(motoValue.rpm1))/12));
    });
    time_timer_m.start();
	gauge.add(rpm_follower);

	auto dash_background1 = std::make_unique<egt::SvgImage>("file:eng_mode.svg", egt::Size(win1.width()*1.0,  win1.height()*1.0));
	auto gauge_background1 = std::make_shared<egt::experimental::GaugeLayer>(dash_background1->render("#svg1"));
	gauge1.add(gauge_background1);
	
	auto dash_background = std::make_unique<egt::SvgImage>("file:Car_20240924_800X480.svg", egt::Size(win.width()*1.0,  win.height()*1.0));
	//auto dash_background = std::make_unique<egt::SvgImage>("file:Car_20240110_1.svg", egt::Size(win.width()*1.0,  win.height()*1.0));
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


	egt::PeriodicTimer time_timer44(std::chrono::milliseconds(10));
	time_timer44.on_timeout([&rpm_needle]()
    {
      rpm_needle->value(motoValue.rpm1);
    });
    time_timer44.start();

	egt::PeriodicTimer time_timer55(std::chrono::milliseconds(10));
	time_timer55.on_timeout([&mph_needle]()
    {
      mph_needle->value((static_cast<float> (motoValue.moto_speed)));
    });
    time_timer55.start();
*/
	/*egt::PeriodicTimer time_timer6(std::chrono::milliseconds(10));
	time_timer6.on_timeout([&fuel_needle]()
    {
      fuel_needle->value(motoValue.battery_dot);
    });
    time_timer6.start();*/


   win.add(gauge);
   win1.add(gauge1);
	float kph_ratio_v=12.111,RPM_ratio_v=22.222 ,I_bus_ratio_v=33.333 ,Iq_ratio_v=44.444;
	
    auto rpm_text = std::make_shared<egt::Label>("#XMLID_188_");
	rpm_text->box(egt::Rect(425.0,215.0,10.0,10.0));
	rpm_text->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFFFFFF));
	rpm_text->font(egt::Font(60, egt::Font::Weight::bold));
    rpm_text->text(" RPM");
    gauge.add(rpm_text);

    auto speed_text = std::make_shared<egt::Label>();
    speed_text->box(egt::Rect(95.0, 215.0, 10.0,10.0));
	speed_text->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFFFFFF));
	speed_text->font(egt::Font(60, egt::Font::Weight::bold));
    speed_text->text("0 kph");
    gauge.add(speed_text);

    auto middle_box = dash_background->id_box("#middle");
    auto middle_text = std::make_shared<egt::Label>();
   //middle_text->text_align(egt::AlignFlag::center);
    middle_text->box(egt::Rect(middle_box.x()+375.0, middle_box.y()+185.0, middle_box.width(), middle_box.height()));
    middle_text->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    middle_text->text("25°C");
	middle_text->font(egt::Font(20, egt::Font::Weight::bold));
    gauge.add(middle_text);

	//auto voltage_box = dash_background->id_box("#middle");
    auto voltage_text = std::make_shared<egt::Label>();
   //middle_text->text_align(egt::AlignFlag::center);
    //voltage_text->box(egt::Rect(middle_box.x()+10.0, middle_box.y(), middle_box.width(), middle_box.height()));
	voltage_text->box(egt::Rect(middle_box.x()+365.0, middle_box.y()+140.0,10,10));
    voltage_text->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    voltage_text->text("Battery");
	voltage_text->font(egt::Font(20, egt::Font::Weight::bold));
    gauge.add(voltage_text);

	//val.VDC;
	auto VDC = std::make_shared<egt::Label>();
	VDC->box(egt::Rect(middle_box.x()+5.0, middle_box.y()+450.0,10,10));
    VDC->color(egt::Palette::ColorId::label_text, egt::Palette::white);
	VDC->font(egt::Font(25, egt::Font::Weight::bold));
    //VDC->text("VDC");
	/*auto VDC_n = std::make_shared<egt::Label>();
	VDC_n->box(egt::Rect(middle_box.x()+10.0, middle_box.y()+252.0,10,10));
    VDC_n->color(egt::Palette::ColorId::label_text, egt::Palette::white);
	VDC_n->font(egt::Font(14, egt::Font::Weight::bold));
    VDC_n->text("I_bus:  ");
	*/
    gauge.add(VDC);
	//gauge.add(VDC_n);

	//val.Iq;
	auto Iq = std::make_shared<egt::Label>();
	Iq->box(egt::Rect(middle_box.x()+250.0, middle_box.y()+450.0,10,10));
    Iq->color(egt::Palette::ColorId::label_text, egt::Palette::white);
   // Iq->text("Iq");
	Iq->font(egt::Font(25, egt::Font::Weight::bold));
	gauge.add(Iq);

	//val.wheel_pulses;
	auto wheel_pulses = std::make_shared<egt::Label>();
 
	wheel_pulses->box(egt::Rect(middle_box.x()+500.0, middle_box.y()+450.0,10,10));
    wheel_pulses->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    wheel_pulses->text("wheel_pulses");
	wheel_pulses->font(egt::Font(25, egt::Font::Weight::bold));
    gauge.add(wheel_pulses);

	auto milage =  std::make_shared<egt::Label>();
	milage->box(egt::Rect(300.0,400.0, 10.0,10.0));
    milage->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFF4500));
    milage->text("1000.00KM");
	milage->font(egt::Font(40, egt::Font::Weight::bold));
    gauge.add(milage);

	auto milage1 =  std::make_shared<egt::Label>();
	milage1->box(egt::Rect(600.0,60.0, 10.0,10.0));
    milage1->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFF4500));
    milage1->text("9999.99 KM");
	milage1->font(egt::Font(35, egt::Font::Weight::bold));
    gauge1.add(milage1);

	auto clear_km = std::make_shared<egt::ImageButton>(egt::Image("file:zero_icon.png"));
	clear_km->color(egt::Palette::ColorId::button_bg , egt::Palette::transparent);
	clear_km->box(egt::Rect(630.0, 0.0, 50.0, 50.0));
	gauge1.add(clear_km);
	
	float score[5] = {1.878,1.878,1.878,1.878,999.99};
	clear_km->on_event([&](egt::Event& event_clear_km)
	{
		if(event_clear_km.id() == egt::EventId::raw_pointer_down)
		{
			std::cout <<  "clear_km clicked!"  << std::endl;
			motoValue.milage = 0.0;
			score[5] = 0.0;
		}

	});
	/*auto sec_test = std::make_shared<egt::Label>();
	sec_test->box(egt::Rect(middle_box.x()+50.0, middle_box.y()+400.0,10,10));
    sec_test->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    sec_test->text("sec_test");
	sec_test->font(egt::Font(25, egt::Font::Weight::bold));
    gauge.add(sec_test);
	*/
	egt::PeriodicTimer time_timer9(std::chrono::milliseconds(100));
	time_timer9.on_timeout([/*&sec_test,*/&wheel_pulses,&Iq,&Iq_ratio_v,&VDC,&I_bus_ratio_v,&voltage_text,&middle_text,&rpm_text,&RPM_ratio_v,&speed_text,&kph_ratio_v,&milage,&milage1]()
    {
		
		rpm_text->text(std::to_string(static_cast<int>(motoValue.rpm1*RPM_ratio_v)));
       // speed_text->text(std::to_string(static_cast<int>(motoValue.moto_speed/10*kph_ratio_v)));
		 
		std::stringstream ss_RPM_ratio_v;
		ss_RPM_ratio_v << std::fixed << std::setprecision(1) << static_cast<float>(motoValue.moto_speed/10*kph_ratio_v);
		std::string str1_RPM_ratio_v = ss_RPM_ratio_v.str();
		speed_text->text(str1_RPM_ratio_v);
		
		float Vout,R2,N1,N2,N3,N4;
		Vout = (motoValue.drive_temp*(3.3/1024));
		R2 = (((3300*Vout)/(3.3-Vout)))/1000;
		N1 = (log(10)-log(R2))/3988;
		N2 = (1/298.15) - N1;
		N3 = 1/N2;
		N4 = N3-273.15;
		std::string str1_middle_text = std::to_string(static_cast<float>(N4));
		std::stringstream ss_middle_text;
		ss_middle_text << std::setprecision(3) << N4;
		str1_middle_text = ss_middle_text.str();
		middle_text->text(str1_middle_text+ "°C");
     // middle_text->text(std::to_string(static_cast<int>(motoValue.drive_temp)) + "°C");
		
		//std::string str1 = std::to_string(static_cast<float>(motoValue.battery_dot));
		std::stringstream ss_battery_dot;
		ss_battery_dot << std::setprecision(3) << motoValue.battery_dot;
		std::string str1_battery_dot = ss_battery_dot.str();
		voltage_text->text(str1_battery_dot+ "V");
		
		float VDC_t=static_cast<float>(motoValue.VDC);
		if(motoValue.VDC > 32767)
			VDC_t = (VDC_t - 65536);
		std::stringstream ss_VDC;
		ss_VDC << std::fixed << std::setprecision(2) <<(static_cast<float>(VDC_t*0.01*I_bus_ratio_v));
		std::string str1_VDC = ss_VDC.str();
		VDC->text("I_bus : "+ str1_VDC+ " A");

		float Iq_t=static_cast<float>(motoValue.Iq);
		if(Iq_t > 32767)
			Iq_t = (Iq_t - 65535);

		std::stringstream ss_Iq_t;
		ss_Iq_t << std::fixed << std::setprecision(2) << static_cast<float>(Iq_t*0.01*Iq_ratio_v);
		std::string str1_Iq_t = ss_Iq_t.str();
		Iq->text("Iq : " + str1_Iq_t + " A");
		
		//std::string str1 = std::to_string(static_cast<float>(motoValue.wheel_pulses));
		std::stringstream ss_wheel_pulses;
		ss_wheel_pulses << std::setprecision(4) << motoValue.wheel_pulses;
		std::string str1_wheel_pulses = ss_wheel_pulses.str();
		wheel_pulses->text("wheel_pulses:   "+str1_wheel_pulses+ " p/sec");
		
		motoValue.milage += ((motoValue.moto_speed/360000*kph_ratio_v));
		if(motoValue.milage > 9999.99)
			motoValue.milage = 0.0;
		
		/*
		std::stringstream ss_sec_test;
		counter++;
		ss_sec_test << std::setprecision(4) << counter;
		std::string str1_sec_test = ss_sec_test.str();
		if(thread_terminate)
			sec_test->text("test 0.1 sec : "+str1_sec_test + " 1");
		else
			sec_test->text("test 0.1 sec : "+str1_sec_test + " 0");
		*/
    });
    time_timer9.start();




//auto sys_date_box = dash_background->id_box("#XMLID_192_-1");
	//auto sys_time_box = dash_background->id_box("#XMLID_192_");

	
	
	//auto Remaining =  std::make_shared<egt::Label>();
	
	auto sys_date =  std::make_shared<egt::Label>();
	sys_date->box(egt::Rect(320.0,340.0, 10.0, 10.0 ));
    sys_date->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    sys_date->text("12.12.2023");
	sys_date->font(egt::Font(30, egt::Font::Weight::bold));
    gauge.add(sys_date);

	auto sys_time =  std::make_shared<egt::Label>();
	sys_time->box(egt::Rect(320.0,370.0, 10.0,10.0));
    sys_time->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    sys_time->text("12.12.2023");
	sys_time->font(egt::Font(30, egt::Font::Weight::bold));
    gauge.add(sys_time);

	egt::PeriodicTimer time_timer7(std::chrono::milliseconds(500));
	time_timer7.on_timeout([&sys_time,&sys_date]()
    {
		sys_date->text(std::to_string(static_cast<int>( 1900 + ltm->tm_year)) + "/" + std::to_string(static_cast<int>( 1 + ltm->tm_mon)) + "/" + std::to_string(static_cast<int>( ltm->tm_mday )));
		sys_time->text(std::to_string(static_cast<int>( ltm->tm_hour))+":"+ std::to_string(static_cast<int>( ltm->tm_min))+":"+std::to_string(static_cast<int>( ltm->tm_sec )));
		now = time(0);
		ltm = localtime(&now);
	});
    time_timer7.start();



    auto console_text = std::make_shared<egt::Button>();
	console_text->box(egt::Rect(415.0, 40.0, 1.0, 1.0));
	console_text->font(egt::Font("Serif", 35, egt::Font::Weight::normal ,egt::Font::Slant::italic));
	console_text->text_align(egt::AlignFlag::left);
	console_text->color(egt::Palette::ColorId::button_bg, egt::Palette::transparent);


	gauge.add(console_text);

	auto regen_mode = std::make_shared<egt::Label>();
	regen_mode->box(egt::Rect(415.0, 40.0, 1.0, 1.0));
	regen_mode->font(egt::Font("Serif", 35, egt::Font::Weight::normal  , egt::Font::Slant::italic));
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
			if(win_type)
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
	console1_text->box(egt::Rect(290.0, 40.0, 10.0, 10.0));
	console1_text->font(egt::Font("Serif",35, egt::Font::Weight::normal  , egt::Font::Slant::italic));
	console1_text->text_align(egt::AlignFlag::left);
	gauge.add(console1_text);

	auto assistance_mode = std::make_shared<egt::Label>();
	assistance_mode->box(egt::Rect(290.0, 40.0, 10.0, 10.0));
	assistance_mode->font(egt::Font("Serif", 35, egt::Font::Weight::normal  , egt::Font::Slant::italic));
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
			if(win_type)
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
	Staus_massages->box(egt::Rect(270.0, 5.0, 1.0, 10.0));
	//Staus_massages->font(egt::Font(25, egt::Font::Weight::bold));
	Staus_massages->font(egt::Font("Serif", 40, egt::Font::Weight::normal  , egt::Font::Slant::italic));
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
			{"CAN BUS Error!",egt::Palette::yellow}
        };

        Staus_massages->color(egt::Palette::ColorId::label_text, states1[state_Staus_massages].second);
        Staus_massages->text(states1[state_Staus_massages].first);


		//Over_MSOFET_temp, Over_voltage, Under_voltage,moto_enable;
		if(CAN_BUS_Error)
			 state_Staus_massages = 5;
		else if(Over_MSOFET_temp)
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

//*************
//gauge1
//*************
//-----------------------------------
//|  1.kph  |  2.  | 3.Iq	 |
//|  XXX    |      |   XXX   |
//-------------------------
//| 4.RPM   |  5.  | 6.      |
//|  XXX    |      |         |
//-------------------------
//|  7.VDC  |  8.  | 9.Pluses|
//|  XXX    |      |         |
//-----------------------------------

	for (auto x = 40; x < 480; x=x+50)
    {
		auto painter = std::make_shared<egt::Label>();
		painter->box(egt::Rect(0.0, static_cast<float>(x), 10.0,10.0));
		painter->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFFFFFF));
		painter->font(egt::Font(30, egt::Font::Weight::bold));
		bool a;
		if(a)
		{	
			painter->text("——————————————————————————");
			a=false;
		}
		else
		{
			
			painter->text("---------------------------------------------------------------------------------");
			a=true;
		}
		gauge1.add(painter);
		
	}

/*
	for (auto x = 120; x < 190; x++)
    {
			auto painter2 = std::make_shared<egt::Label>();
			painter2->box(egt::Rect(100.0, static_cast<float>(x), 0.0,0.0));
			painter2->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFFFFFF));
			painter2->font(egt::Font(10, egt::Font::Weight::bold));
			painter2->text("-");
			gauge1.add(painter2);
	}
		
	for (auto x = -1; x < 262; x++)
    {
			auto painter2 = std::make_shared<egt::Label>();
			painter2->box(egt::Rect(150.0, static_cast<float>(x), 0.0,0.0));
			painter2->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFFFFFF));
			painter2->font(egt::Font(10, egt::Font::Weight::bold));
			painter2->text("-");
			gauge1.add(painter2);
	}
	for (auto y = -1; y < 262; y++)
    {
			auto painter3 = std::make_shared<egt::Label>();
			painter3->box(egt::Rect(320.0, static_cast<float>(y), 0.0,0.0));
			painter3->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFFFFFF));
			painter3->font(egt::Font(10, egt::Font::Weight::bold));
			painter3->text("-");
			gauge1.add(painter3);
	}
	

	//kph_ratio
	auto kph_ratio = std::make_shared<egt::Slider>(-1000, 1000);
	kph_ratio->color(egt::Palette::ColorId::button_bg , egt::Color::rgb(0xFFFFFF));
	kph_ratio->box(egt::Rect(152.0, 40.0, 170.0, 10.0));
	kph_ratio->value(1000);
	gauge1.add(kph_ratio);
	auto slider_text = std::make_shared<egt::Label>();
	slider_text->box(egt::Rect(198.0, 5.0, 10.0, 10.0));
    slider_text->color(egt::Palette::ColorId::label_text,  egt::Color::rgb(0xFFFFFF));
	slider_text->font(egt::Font(25, egt::Font::Weight::bold));
    gauge1.add(slider_text);
	auto kph_ratio_n = std::make_shared<egt::Label>();
	kph_ratio_n->box(egt::Rect(180.0, 65.0, 10.0, 10.0));
    kph_ratio_n->color(egt::Palette::ColorId::label_text,  egt::Color::rgb(0xFFFFFF));
	kph_ratio_n->font(egt::Font(25, egt::Font::Weight::bold));
	kph_ratio_n->text("kph ratio");
    gauge1.add(kph_ratio_n);
	
	//RPM_ratio
	auto RPM_ratio = std::make_shared<egt::Slider>(-1000, 1000);
	RPM_ratio->color(egt::Palette::ColorId::button_bg , egt::Color::rgb(0xFFFFFF));
	RPM_ratio->box(egt::Rect(152.0, 135.0, 170.0, 10.0));
	RPM_ratio->value(1000);
	gauge1.add(RPM_ratio);
	auto RPM_ratio_text = std::make_shared<egt::Label>();
	RPM_ratio_text->box(egt::Rect(198.0, 105.0, 10.0, 10.0));
    RPM_ratio_text->color(egt::Palette::ColorId::label_text,  egt::Color::rgb(0xFFFFFF));
	RPM_ratio_text->font(egt::Font(22, egt::Font::Weight::bold));
    gauge1.add(RPM_ratio_text);
	auto RPM_ratio_n = std::make_shared<egt::Label>();
	RPM_ratio_n->box(egt::Rect(170.0, 155.0, 10.0, 10.0));
    RPM_ratio_n->color(egt::Palette::ColorId::label_text,  egt::Color::rgb(0xFFFFFF));
	RPM_ratio_n->font(egt::Font(22, egt::Font::Weight::bold));
	RPM_ratio_n->text("RPM ratio");
    gauge1.add(RPM_ratio_n);
	
	//I_Bus
	auto I_Bus_ratio = std::make_shared<egt::Slider>(-1000, 1000);
	I_Bus_ratio->color(egt::Palette::ColorId::button_bg , egt::Color::rgb(0xFFFFFF));
	I_Bus_ratio->box(egt::Rect(152.0, 215.0, 170.0, 5.0));
	I_Bus_ratio->value(1000);
	gauge1.add(I_Bus_ratio);
	auto I_Bus_ratio_text = std::make_shared<egt::Label>();
	I_Bus_ratio_text->box(egt::Rect(198.0, 180.0, 10.0, 10.0));
    I_Bus_ratio_text->color(egt::Palette::ColorId::label_text,  egt::Color::rgb(0xFFFFFF));
	I_Bus_ratio_text->font(egt::Font(25, egt::Font::Weight::bold));
    gauge1.add(I_Bus_ratio_text);
	auto I_Bus_ratio_n = std::make_shared<egt::Label>();
	I_Bus_ratio_n->box(egt::Rect(170.0, 235.0, 10.0, 10.0));
    I_Bus_ratio_n->color(egt::Palette::ColorId::label_text,  egt::Color::rgb(0xFFFFFF));
	I_Bus_ratio_n->font(egt::Font(25, egt::Font::Weight::bold));
	I_Bus_ratio_n->text("I_Bus ratio");
    gauge1.add(I_Bus_ratio_n);
	
	//Iq
	auto Iq_ratio = std::make_shared<egt::Slider>(-1000, 1000);
	Iq_ratio->color(egt::Palette::ColorId::button_bg , egt::Color::rgb(0xFFFFFF));
	Iq_ratio->box(egt::Rect(320.0, 135.0, 170.0, 5.0));
	Iq_ratio->value(1000);
	gauge1.add(Iq_ratio);
	auto Iq_ratio_text = std::make_shared<egt::Label>();
	Iq_ratio_text->box(egt::Rect(350.0, 105.0, 10.0, 10.0));
    Iq_ratio_text->color(egt::Palette::ColorId::label_text,  egt::Color::rgb(0xFFFFFF));
	Iq_ratio_text->font(egt::Font(22, egt::Font::Weight::bold));
    gauge1.add(Iq_ratio_text);
	auto Iq_ratio_n = std::make_shared<egt::Label>();
	Iq_ratio_n->box(egt::Rect(350.0, 155.0, 10.0, 10.0));
    Iq_ratio_n->color(egt::Palette::ColorId::label_text,  egt::Color::rgb(0xFFFFFF));
	Iq_ratio_n->font(egt::Font(22, egt::Font::Weight::bold));
	Iq_ratio_n->text("Iq ratio");
    gauge1.add(Iq_ratio_n);
	
	egt::PeriodicTimer time_timer_ratio(std::chrono::milliseconds(100));
	time_timer_ratio.on_timeout([&kph_ratio,&slider_text,&RPM_ratio,&RPM_ratio_text,&I_Bus_ratio,&I_Bus_ratio_text,&Iq_ratio,&Iq_ratio_text]()
    {
		std::string str1 = std::to_string(static_cast<float>(kph_ratio->value()*0.001));
		std::stringstream ss1;
		ss1 << std::fixed << std::setprecision(3) << kph_ratio->value()*0.001;
		str1 = ss1.str();
		slider_text->text(str1);
		
		std::string str2 = std::to_string(static_cast<float>(RPM_ratio->value()*0.001));
		std::stringstream ss2;
		ss2 << std::fixed << std::setprecision(3) << RPM_ratio->value()*0.001;
		str2 = ss2.str();
		RPM_ratio_text->text(str2);
		
		std::string str3 = std::to_string(static_cast<float>(I_Bus_ratio->value()*0.001));
		std::stringstream ss3;
		ss3 << std::fixed << std::setprecision(3) << I_Bus_ratio->value()*0.001;
		str3 = ss3.str();
		I_Bus_ratio_text->text(str3);
		
		std::string str4 = std::to_string(static_cast<float>(Iq_ratio->value()*0.001));
		std::stringstream ss4;
		ss4 << std::fixed << std::setprecision(3) << Iq_ratio->value()*0.001;
		str4 = ss4.str();
		Iq_ratio_text->text(str4);
		
    });
    time_timer_ratio.start();
*/	
	
	//static_cast<float>(kph_ratio->value()*0.001)
	//static_cast<float>(RPM_ratio->value()*0.001)
	//static_cast<float>(I_Bus_ratio->value()*0.001)
	//static_cast<float>(Iq_ratio->value()*0.001)
	//
	auto speed_text2 = std::make_shared<egt::Label>();
    speed_text2->box(egt::Rect(50.0, 5.0, 10.0,10.0));
	speed_text2->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFFFFFF));
	speed_text2->font(egt::Font(40, egt::Font::Weight::bold));
    speed_text2->text("kph");
    gauge1.add(speed_text2);
	auto speed_text1 = std::make_shared<egt::Label>();
    speed_text1->box(egt::Rect(50.0, 60.0, 10.0,10.0));
	speed_text1->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFFFFFF));
	speed_text1->font(egt::Font(40, egt::Font::Weight::bold));
   // speed_text1->text("0 kph");
    gauge1.add(speed_text1);
	
	
	
	//RPM
	auto rpm_text2 = std::make_shared<egt::Label>();
	//rpm_text2->box(egt::Rect(200.0,50.0,10.0,10.0));
	rpm_text2->box(egt::Rect(50.0,115.0,10.0,10.0));
	rpm_text2->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFFFFFF));
	rpm_text2->font(egt::Font(40, egt::Font::Weight::bold));
    rpm_text2->text("RPM");
    gauge1.add(rpm_text2);
	auto rpm_text1 = std::make_shared<egt::Label>();
	//rpm_text1->box(egt::Rect(200.0,5.0,10.0,10.0));
	rpm_text1->box(egt::Rect(50.0,170.0,10.0,10.0));
	rpm_text1->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFFFFFF));
	rpm_text1->font(egt::Font(40, egt::Font::Weight::bold));
    //rpm_text1->text("RPM");
    gauge1.add(rpm_text1);
	
	//val.Iq;
	auto Iq_e_n = std::make_shared<egt::Label>();
	Iq_e_n->box(egt::Rect(50.0,210.0,10.0,10.0));
    Iq_e_n->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    Iq_e_n->text("Iq");
	Iq_e_n->font(egt::Font(40, egt::Font::Weight::bold));
	gauge1.add(Iq_e_n);
	auto Iq_e = std::make_shared<egt::Label>();
	Iq_e->box(egt::Rect(2.0,265.0,10.0,10.0));
    Iq_e->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    //Iq->text("Iq");
	Iq_e->font(egt::Font(40, egt::Font::Weight::bold));
	gauge1.add(Iq_e);
	
	

    //val.VDC;
	auto VDC_e_n = std::make_shared<egt::Label>();
	VDC_e_n->box(egt::Rect(50.0,310.0,10.0,10.0));
    VDC_e_n->color(egt::Palette::ColorId::label_text, egt::Palette::white);
	VDC_e_n->font(egt::Font(40, egt::Font::Weight::bold));
    VDC_e_n->text("I_bus");
	gauge1.add(VDC_e_n);
	auto VDC_e = std::make_shared<egt::Label>();
	VDC_e->box(egt::Rect(2.0,365.0,10.0,10.0));
	VDC_e->color(egt::Palette::ColorId::label_text, egt::Palette::white);
	VDC_e->font(egt::Font(40, egt::Font::Weight::bold));
    //VDC->text("VDC");
	gauge1.add(VDC_e);

	//val.wheel_pulses;
	auto wheel_pulses_e_n = std::make_shared<egt::Label>();
	wheel_pulses_e_n->box(egt::Rect(610.0,115.0,10.0,10.0));
    wheel_pulses_e_n->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    wheel_pulses_e_n->text("Wheel Pulses");
	wheel_pulses_e_n->font(egt::Font(30, egt::Font::Weight::bold));
    gauge1.add(wheel_pulses_e_n);
	auto wheel_pulses_e = std::make_shared<egt::Label>();
	wheel_pulses_e->box(egt::Rect(610.0,160.0,10.0,10.0));
    wheel_pulses_e->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    //wheel_pulses_e->text("wheel_pulses");
	wheel_pulses_e->font(egt::Font(30, egt::Font::Weight::bold));
    gauge1.add(wheel_pulses_e);

	//kph
	
	egt::PeriodicTimer time_timer22(std::chrono::milliseconds(10));
	time_timer22.on_timeout([&rpm_text1,&speed_text1,&kph_ratio_v,&RPM_ratio_v,&VDC_e,&I_bus_ratio_v,&Iq_e,&Iq_ratio_v,&wheel_pulses_e]()
    {
		//kph_ratio
		//speed_text1->text(std::to_string(static_cast<int>(motoValue.moto_speed/10*kph_ratio_v)));
		
		std::stringstream ss;
		ss << std::fixed << std::setprecision(1) << static_cast<float>(motoValue.moto_speed/10*kph_ratio_v);
		std::string str1 = ss.str();
		speed_text1->text(str1);
		
		
	    //RPM_ratio
		rpm_text1->text(std::to_string(static_cast<int>(motoValue.rpm1*RPM_ratio_v)));
		
		//I_bus
		float VDC_t=static_cast<float>(motoValue.VDC);
		if(motoValue.VDC > 32767)
			VDC_t = (VDC_t - 65535);
		std::stringstream ss_I_bus;
		ss_I_bus << std::fixed << std::setprecision(2) << static_cast<float>(VDC_t*0.01*I_bus_ratio_v);
		std::string str1_I_bus = ss_I_bus.str();
		VDC_e->text(str1_I_bus+ "A");
		
		//Iq
		float Iq_t=static_cast<float>(motoValue.Iq);
		if(motoValue.Iq > 32767)
			Iq_t = (Iq_t - 65535);
		std::stringstream ss_q;
		ss_q << std::fixed << std::setprecision(2) << static_cast<float>(Iq_t*0.01*Iq_ratio_v);
		std::string str1_q = ss_q.str();
		Iq_e->text(str1_q+ "A");
		
		//wheel_pulses
		std::string str1_w = std::to_string(static_cast<float>(motoValue.wheel_pulses));
		std::stringstream ss_w;
		ss_w << std::setprecision(4) << motoValue.wheel_pulses;
		str1_w = ss_w.str();
		wheel_pulses_e->text(str1_w+" P/sec");
    });
    time_timer22.start();


/*
	auto send_value = std::make_shared<egt::ImageButton>(egt::Image("file:btn_icon_green_turn_down_left.png"));
	send_value->color(egt::Palette::ColorId::button_bg , egt::Palette::transparent);
	send_value->box(egt::Rect(180.0, 100.0, 50.0, 50.0));
	gauge1.add(send_value);
*/


	auto PreviousKey = std::make_shared<egt::ImageButton>(egt::Image("file:previous.png")); //<------
	PreviousKey->color(egt::Palette::ColorId::button_bg , egt::Palette::transparent);
	PreviousKey->box(egt::Rect(750.0, 0.0, 50.0, 50.0));
	gauge1.add(PreviousKey);

	auto NextKey = std::make_shared<egt::ImageButton>(egt::Image("file:next.png")); // --->
	NextKey->color(egt::Palette::ColorId::button_bg , egt::Palette::transparent);
	NextKey->box(egt::Rect(750.0, 0.0, 50.0, 50.0));
	gauge.add(NextKey);

	PreviousKey->on_event([&](egt::Event& PreviousKeyEvent)
	{
		if(PreviousKeyEvent.id() == egt::EventId::raw_pointer_down)
		{
			std::cout <<  "PreviousKey On clicked!"  << std::endl;
			win_type = 1;
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
			win_type = 0; 
			win1.show();
			win.hide();

			PreviousKey->show();
			NextKey->hide();
			NextKey->fill_flags().clear();
		}
	});
	
	egt::VerticalBoxSizer vsizer;
    vsizer.align(egt::AlignFlag::center_horizontal);
    win1.add(expand_vertical(vsizer));
	
	egt::TextBox text("0", egt::TextBox::TextFlag::multiline);
    text.text_align(egt::AlignFlag::center_vertical | egt::AlignFlag::right);
    text.font(egt::Font(80));
    text.color(egt::Palette::ColorId::bg, egt::Color(0x272727ff));
    text.color(egt::Palette::ColorId::text, egt::Palette::white);
    text.readonly(true);
    vsizer.add(expand(text));
	
	egt::StaticGrid grid({4, 4});
    grid.align(egt::AlignFlag::bottom | egt::AlignFlag::center_horizontal);
    egt::DefaultDim width;
    if (1)
    {
        // Buttons size is 60x60 for a 800x480 screen.
        width = 130 * grid.n_col() * 480 / 800;
    }
    else
    {
        // The grid expands horizontally.
        width = 480;
    }
    // Buttons are squares.
    auto grid_size = egt::Size(width*1.2, (grid.n_row() * width / grid.n_col()));
   //auto grid_size = egt::Size(width, 100);
    grid.resize(grid_size);
    vsizer.add(grid);

    const std::vector<std::vector<std::string>> buttons =
    {
        //{"?", "?", "?", "C"},
        {"7", "8", "9", "kph"},
        {"4", "5", "6", "RPM"},
        {"1", "2", "3", "I_bus"},
        {"0", ".", "C", "Iq"},
    };

    const std::vector<std::vector<std::pair<egt::Palette::ColorId, egt::Color>>> colors =
    {
       /* {
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::gray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::gray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::gray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::orange)
        },*/

        {
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::lightgray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::lightgray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::lightgray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::orange)
        },

        {
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::lightgray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::lightgray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::lightgray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::orange)
        },

        {
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::lightgray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::lightgray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::lightgray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::orange)
        },

        {
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::lightgray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::lightgray),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::blue),
            std::make_pair(egt::Palette::ColorId::button_bg, egt::Palette::orange)
        },
    };

	
	
    for (size_t r = 0; r < buttons.size(); r++)
    {
        for (size_t c = 0; c < buttons[r].size(); c++)
        {
            std::string label = buttons[r][c];
            if (label.empty())
                continue;

            auto b = std::make_shared<egt::Button>(label);
            b->border(1);
            b->font(egt::Font(35, egt::Font::Weight::bold));
            b->color(colors[r][c].first, colors[r][c].second);
            b->color(egt::Palette::ColorId::border, egt::Palette::gray);
            grid.add(expand(b), c, r);

            b->on_click([&text, b, &kph_ratio_v,&RPM_ratio_v,&I_bus_ratio_v,&Iq_ratio_v](egt::Event&)
            {
                static bool do_clear = false;
				
                if (b->text() == "kph")
				{
					//char*  kph_ratio_s = text.text();
					std::stringstream kph_ratio_s;				
					kph_ratio_s << std::setprecision(7) << text.text();
					//std::cout <<  "last_line(text.value()) =  "<<  kph_ratio_s.str()  <<std::endl;
					std::string kph_s = kph_ratio_s.str();
					kph_ratio_v =  std::stof(kph_s,0);						
					//std::cout <<  "kph_ratio_v = "<<  static_cast<float> (kph_ratio_v) <<  std::endl;
					do_clear = true;
				}	
				//std::cout <<  "invaild command: regen_mode_r :" << static_cast<int> (frame.data[0]) << std::endl;
				else if (b->text() == "RPM")
				{
					std::stringstream RPM_ratio_s;				
					RPM_ratio_s << std::setprecision(7) << text.text();
					RPM_ratio_v =  std::stof(RPM_ratio_s.str(),0);
					//std::cout <<  "RPM = "<<  static_cast<float> (RPM_ratio_v) <<  std::endl;
				
					do_clear = true;
				}	
				else if (b->text() == "I_bus")
				{
					std::stringstream I_bus_ratio_s;				
					I_bus_ratio_s << std::setprecision(7) << text.text();
					I_bus_ratio_v =  std::stof(I_bus_ratio_s.str(),0);				
					//std::cout <<  "I_bus = "<<  static_cast<float> (I_bus_ratio_v) <<  std::endl;
					do_clear = true;
				}
				else if (b->text() == "Iq")
				{
					std::stringstream Iq_ratio_s;				
					Iq_ratio_s << std::setprecision(7) << text.text();
					Iq_ratio_v =  std::stof(Iq_ratio_s.str(),0);						
					//std::cout <<  "Iq = "<<  static_cast<float> (Iq_ratio_v) <<  std::endl;
					do_clear = true;
				}
				else if (b->text() == "=")
                {
                    auto line = egt::detail::replace_all(
                                    egt::detail::replace_all(
                                        last_line(text.text()), "x", "*"), "÷", "/");
                    if (!line.empty())
                    {
                        std::ostringstream ss;
                        ss << "\n= " <<
                           egt::experimental::lua_evaluate(line) <<
                           "\n";
                        text.append(ss.str());
                    }
                    do_clear = true;
                }
                else if (b->text() == "C")
                {
                    text.clear();
					text.text("0");
                }
                else
                {
                    if (do_clear)
                    {
                        text.clear();
                        do_clear = false;
                    }
                    text.append(b->text());
                }
            });
        }
    }

	//ratio display
	/*auto ratio = std::make_shared<egt::Label>();
	ratio->box(egt::Rect(620.0,180.0,10.0,10.0));
    ratio->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    ratio->text("ratio");
	ratio->font(egt::Font(50, egt::Font::Weight::bold));
	gauge1.add(ratio);*/
	auto kph_ratio_display = std::make_shared<egt::Label>();
	kph_ratio_display->box(egt::Rect(600.0,210.0,10.0,10.0));
    kph_ratio_display->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    kph_ratio_display->text("kph ratio");
	kph_ratio_display->font(egt::Font(25, egt::Font::Weight::bold));
    gauge1.add(kph_ratio_display);
	auto RPM_ratio_display = std::make_shared<egt::Label>();
	RPM_ratio_display->box(egt::Rect(600.0,260.0,10.0,10.0));
    RPM_ratio_display->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    RPM_ratio_display->text("RPM ratio");
	RPM_ratio_display->font(egt::Font(25, egt::Font::Weight::bold));
    gauge1.add(RPM_ratio_display);
	auto I_bus_ratio_display = std::make_shared<egt::Label>();
	I_bus_ratio_display->box(egt::Rect(600.0,310.0,10.0,10.0));
    I_bus_ratio_display->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    I_bus_ratio_display->text("I_bus ratio");
	I_bus_ratio_display->font(egt::Font(25, egt::Font::Weight::bold));
    gauge1.add(I_bus_ratio_display);
	auto Iq_ratio_display = std::make_shared<egt::Label>();
	Iq_ratio_display->box(egt::Rect(600.0,360.0,10.0,10.0));
    Iq_ratio_display->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    Iq_ratio_display->text("Iq ratio");
	Iq_ratio_display->font(egt::Font(25, egt::Font::Weight::bold));
    gauge1.add(Iq_ratio_display);
	
	auto kph_ratio = std::make_shared<egt::Label>();
	kph_ratio->box(egt::Rect(600.0,235.0,10.0,10.0));
    kph_ratio->color(egt::Palette::ColorId::label_text, egt::Palette::white);
    //kph_ratio->text("kph");
	kph_ratio->font(egt::Font(25, egt::Font::Weight::bold));
    gauge1.add(kph_ratio);
	auto RPM_ratio = std::make_shared<egt::Label>();
	RPM_ratio->box(egt::Rect(600.0,280.0,10.0,10.0));
    RPM_ratio->color(egt::Palette::ColorId::label_text, egt::Palette::white);
   // RPM_ratio->text("RPM");
	RPM_ratio->font(egt::Font(25, egt::Font::Weight::bold));
    gauge1.add(RPM_ratio);
	auto I_bus_ratio = std::make_shared<egt::Label>();
	I_bus_ratio->box(egt::Rect(600.0,335.0,10.0,10.0));
    I_bus_ratio->color(egt::Palette::ColorId::label_text, egt::Palette::white);
   // I_bus_ratio->text("I_bus");
	I_bus_ratio->font(egt::Font(25, egt::Font::Weight::bold));
    gauge1.add(I_bus_ratio);
	auto Iq_ratio = std::make_shared<egt::Label>();
	Iq_ratio->box(egt::Rect(600.0,385.0,10.0,10.0));
    Iq_ratio->color(egt::Palette::ColorId::label_text, egt::Palette::white);
   // Iq_ratio->text("Iq");
	Iq_ratio->font(egt::Font(25, egt::Font::Weight::bold));
    gauge1.add(Iq_ratio);
	
	bool power_on=true;
	
	egt::PeriodicTimer timer_ratio(std::chrono::milliseconds(1000));
	timer_ratio.on_timeout([&power_on,&score,/*&score_w,&name,*/&kph_ratio,&RPM_ratio,&I_bus_ratio,&Iq_ratio,&kph_ratio_v,&RPM_ratio_v,&I_bus_ratio_v,&Iq_ratio_v,&milage,&milage1]()
    {
		float score_w[5] = {0.87,0.87,0.87,0.87,9999.9};
		char name[5][256] = {"kph_ratio_v", "RPM_ratio_v", "I_bus_ratio_v", "Iq_ratio_v","motoValue.milage"};
		std::string str1 = std::to_string(static_cast<float>(kph_ratio_v));
		std::stringstream ss1;
		ss1 << std::setprecision(7) << kph_ratio_v;
		str1 = ss1.str();
		kph_ratio->text(str1);
		
		std::string str2 = std::to_string(static_cast<float>(RPM_ratio_v));
		std::stringstream ss2;
		ss2 << std::setprecision(7) << RPM_ratio_v;
		str2 = ss2.str();
		RPM_ratio->text(str2);
		
		std::string str3 = std::to_string(static_cast<float>(I_bus_ratio_v));
		std::stringstream ss3;
		ss3 << std::setprecision(7) << I_bus_ratio_v;
		str3 = ss3.str();
		I_bus_ratio->text(str3);
		
		std::string str4 = std::to_string(static_cast<float>(Iq_ratio_v));
		std::stringstream ss4;
		ss4 << std::setprecision(7) << Iq_ratio_v;
		str4 = ss4.str();
		Iq_ratio->text(str4);
		
		std::stringstream ss5;
		ss5 << std::fixed << std::setprecision(2) << static_cast<float>(motoValue.milage);
		std::string str5 = ss5.str();
		milage->text(str5+ " KM");
		milage1->text(str5+ " KM");

			std::string name_r;
			//int test;
			//float score_r[5]= {0.878,0.878,0.878,0.878,9999.99};;
			float score_o;
			std::vector<std::string> names;
			std::vector<float> scores;
			std::fstream ifs;int j=0;
			if(
					(static_cast<float>(kph_ratio_v) != static_cast<float>(score[0])) 
				||  (static_cast<float>(RPM_ratio_v) != static_cast<float>(score[1])) 
				||  (static_cast<float>(I_bus_ratio_v) != static_cast<float>(score[2])) 
				||  (static_cast<float>(Iq_ratio_v) != static_cast<float>(score[3])) 
				||  (static_cast<float>(motoValue.milage) >= static_cast<float>(score[4]+0.1)) 
				||  (static_cast<float>(motoValue.milage) < static_cast<float>(score[4]))
				)
			{	
				ifs.open("/root/ratio.txt", std::ios::in);
		
				if (!(ifs.is_open())) 
				{
					std::cout <<  "open and read  ratio.txt file Fail!!! " <<  std::endl;
					power_on = true;
					//return 1; // EXIT_FAILURE
					std::ofstream file("/root/ratio.txt");
					for (int i = 0; i < 5; i++) 
						{
						file << name[i] << " " << static_cast<float>(score[i]) << "\n";
						}
						std::cout<< "write a o data " << "\n";
					file.close();
					//ifs.is_open();
					std::cout <<  "reopen ratio.txt " <<  std::endl;
					//ifs.close();
				}
					ifs.close();
			
			
				if(power_on)
				{	
					std::cout <<  "power_on " <<  std::endl;
					ifs.open("/root/ratio.txt", ios::in);
					j=0;
					while(ifs >> name_r >> score_o){
						std::cout<< "read file :"<< "score_o[" << j <<"]" << name_r << " " << score_o << "\n";
						//score_r[j] = static_cast<float>(score_o);
						score[j] = static_cast<float>(score_o);
						names.push_back(name_r);
						scores.push_back(score_o);
						power_on = false;	
						j++;
						kph_ratio_v = static_cast<float>(score[0]);
						RPM_ratio_v = static_cast<float>(score[1]);
						I_bus_ratio_v = static_cast<float>(score[2]);
						Iq_ratio_v = static_cast<float>(score[3]);
						motoValue.milage = static_cast<float>(score[4]);
						
					}
					ifs.close();
					
					ifs.open("/root/ratio.txt", ios::out);
					if(j==0)
					{
						for (int i = 0; i < 5; i++) 
						{
						ifs << name[i] << " " << static_cast<float>(score[i]) << "\n";
						}
						std::cout<< "write a o data " << "\n";
						
					}	
					ifs.close();
				}	
						
				ifs.open("/root/ratio.txt", ios::in);
				while(ifs >> name_r >> score_o){
				//	std::cout<< "read file :"<< "score_o[" << j <<"]" << name_r << " " << score_o << "\n";
					//score_r[j] = static_cast<float>(score_o);
					score[j] = static_cast<float>(score_o);
					names.push_back(name_r);
					scores.push_back(score_o);
					j++;
				}
				j=0;
				ifs.close();
				
				ifs.open("/root/ratio.txt", ios::out);
				score_w[0] = static_cast<float>(kph_ratio_v);
				score_w[1] = static_cast<float>(RPM_ratio_v);
				score_w[2] = static_cast<float>(I_bus_ratio_v);
				score_w[3] = static_cast<float>(Iq_ratio_v);
				score_w[4] = static_cast<float>(motoValue.milage);
				
				for (int i = 0; i < 5; i++) 
				{
					ifs << name[i] << " " << static_cast<float>(score_w[i]) << "\n";
					//std::cout <<  "ofs Write file :score_w["<< i << "] : " << static_cast<float>(score_w[i]) <<  std::endl;
				}
				ifs.close();
				/*
				for (int i = 0; i < 5; i++) 
				{
					std::cout <<  "ofs Write file :score["<< i << "] : " << static_cast<float>(score[i]) <<  std::endl;
				}

				
				std::cout <<  "ofs Write file kph_ratio_v : " << static_cast<float>(kph_ratio_v) <<  std::endl;
				std::cout <<  "ofs Write file RPM_ratio_v : " << static_cast<float>(RPM_ratio_v) <<  std::endl;
				std::cout <<  "ofs Write file I_bus_ratio_v : " << static_cast<float>(I_bus_ratio_v) <<  std::endl;
				std::cout <<  "ofs Write fileIq_ratio_v : " << static_cast<float>(Iq_ratio_v) <<  std::endl;
				std::cout <<  "ofs Write file motoValue.milage : " << static_cast<float>(motoValue.milage) <<  std::endl;
				*/
			}
		
		
    });
    timer_ratio.start();
	
	
    dash_background.reset(nullptr);
    win1.show();
	//ifs.close();
	
	
	return app.run();
	
	tCan_send.join();
    tCan.join();
	return 0;
}



