/*
 * EGT_modbus_ADM00879_Master.cpp
 *
 *  Created on: May 6, 2024
 *      Author: miles
 */

/*
 * EGT_modbus_ADM00879_Slave.cpp
 *
 *  Created on: May 2, 2024
 *      Author: miles
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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
//#include <i2c/smbus.h>
#include <sys/ioctl.h>
//#include <linux/i2c.h>
//#include <linux/i2c-dev.h>

extern "C"
{
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
}

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define DEV_I2C "/dev/i2c-0"
#define SLAVE_ADDR_MCP9800 0x49 /* EMC1438 I2C slave address */
#define SLAVE_ADDR_EMC1438 0x4C /* EMC1438 I2C slave address */
#define SLAVE_ADDR_EMC2305 0x4D /* EMC2305 I2C slave address */
#include <linux/input.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <thread>
#include <modbus/modbus.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "modbus_ex.h"

#define LOOP          1
#define SERVER_ID     0
#define ADDRESS_START 0
#define ADDRESS_END   1
#define Modbus_Slave_IP "192.168.0.131"
typedef struct MODBUS_TASK_PARAM__
{
	char slave_addr[128];
	int slave_port;
}MODBUS_TASK_PARAM, *pMODBUS_TASK_PARAM;

pthread_mutex_t gMutex;

int writeLed(modbus_t *ctx, POS_ADDR addr, BIT_VALUE val)
{
	int rc = 0;
	uint8_t read_data;
	
	pthread_mutex_lock(&gMutex);
	rc = modbus_write_bit(ctx, addr, val);
    if (rc != 1) {
        printf("[writeLed] FAILED\n");
        return -1;
    }

    rc = modbus_read_bits(ctx, addr, 1, &read_data);
    pthread_mutex_unlock(&gMutex);
    
    if (rc != 1) {
        printf("[writeLed] FAILED (nb points %d)\n", rc);
        return -1;
    }
    
    return 0;
}

int readKey(modbus_t *ctx, POS_ADDR addr, BIT_VALUE *val)
{
	int rc = 0;
	pthread_mutex_lock(&gMutex);
	rc = modbus_read_input_bits(ctx, addr, 1, (uint8_t*)val);
	pthread_mutex_unlock(&gMutex);
	
	if (rc != 1) {
        printf("[readKey] FAILED (nb points %s)\n", modbus_strerror(errno));
        return -1;
    }
    
    return 0;
}


uint16_t *Fan_RPM,*FAN_setting;
void monitorSlaveStatus(void *param)
{
	pMODBUS_TASK_PARAM pkeyTaskParam = (pMODBUS_TASK_PARAM)param;
	modbus_t *ctx;
	int rc;
	BIT_VALUE keyValue;
	
	
	printf("[monitorSlaveStatus] thread created\r\n");
	
	ctx = modbus_new_tcp(pkeyTaskParam->slave_addr, pkeyTaskParam->slave_port);
	
	if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "monitorSlaveStatus] Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return;
    }
	
	while(1)
	{
		rc = readKey(ctx, POS_KEY_RED, &keyValue);
		if( rc < 0 )
		{
			printf("[monitorSlaveStatus] Read key error (%d)\r\n", POS_LED_RED);
			goto close;
		}
		
		if( keyValue != 0 )
		{
			printf("[monitorSlaveStatus] Key pressed\r\n");
		}
		
		
		usleep(100*1000);
	}
	
close:
	modbus_close(ctx);
    modbus_free(ctx);
}
unsigned char EMC1438_init(void)
{
	int fd;
	int ret;
	unsigned char buf[2];
	// open device node
	fd = open(DEV_I2C, O_RDWR);

	if (fd < 0) {
		printf("ERROR open %s ret=%d\n", DEV_I2C, fd);
		return 0;
	}
	
	if (ioctl(fd, I2C_SLAVE, SLAVE_ADDR_EMC1438) < 0) {
		printf("ERROR ioctl() set slave address\n");
		return 0;
	}
		buf[1] = 0; // clear receive buffer
		ret = read(fd, &buf[1], 1);
		
		if (ret != 1) {
			printf("ERROR read() data\n");
			return 0;
		}	
		//printf("EMC1438 of Temperature is %d\n", buf[1]);
	// close device node
	close(fd);
	return buf[1];
}

unsigned char MCP9800_init(void)
{
	int fd;
	int ret;
	unsigned char buf[2];
	//union i2c_smbus_data buf;
	// open device node
	fd = open(DEV_I2C, O_RDWR);

	if (fd < 0) {
		printf("ERROR open %s ret=%d\n", DEV_I2C, fd);
		return 0;
	}
	
	if (ioctl(fd, I2C_SLAVE, SLAVE_ADDR_MCP9800) < 0) {
		printf("SLAVE_ADDR_MCP9800 ERROR ioctl() set slave address\n");
		return 0;
	}
		buf[1] = 0; // clear receive buffer
		ret = read(fd, &buf[1], 1);
		
		if (ret != 1) {
			printf("ERROR read() data\n");
			return 0;
		}	
		//printf("MCP9800 of Temperature is %d\n", buf[1]);
	// close device node
	close(fd);
	return buf[1];
}
//unsigned char* EMC2305_init(unsigned char* reg)
uint16_t * EMC2305_init(uint8_t *Fan_rpm_setting)
{
	int fd;
	//int ret;
	//uint8_t byte_date;
	//uint8_t *data;
	uint16_t *buf =(uint16_t*) malloc(5 * sizeof(uint16_t));
	// open device node
	fd = open(DEV_I2C, O_RDWR);

	if (fd < 0) {
		printf("ERROR open %s ret=%d\n", DEV_I2C, fd);
		return 0;
	}
	
	if (ioctl(fd, I2C_SLAVE, SLAVE_ADDR_EMC2305) < 0) {
		printf("ERROR ioctl() set slave address\n");
		return 0;
	}
			//byte_date = i2c_smbus_read_byte_data(fd,fan1_rpm);
			//printf("EMC2305 FAN1 reg 0x%x is 0x%x\n",fan1_rpm, byte_date);
			
			
			/*printf("EMC2305 FAN1 of RPM is %d\n", (uint16_t) ((i2c_smbus_read_byte_data(fd,0x3E)<< 5) | i2c_smbus_read_byte_data(fd,0x3F) >> 3));
			printf("EMC2305 FAN2 of RPM is %d\n", (uint16_t) ((i2c_smbus_read_byte_data(fd,0x4E) << 5) | i2c_smbus_read_byte_data(fd,0x4F) >> 3));
			printf("EMC2305 FAN3 of RPM is %d\n", (uint16_t) ((i2c_smbus_read_byte_data(fd,0x5E) << 5) | i2c_smbus_read_byte_data(fd,0x5F) >> 3));
			printf("EMC2305 FAN4 of RPM is %d\n", (uint16_t) ((i2c_smbus_read_byte_data(fd,0x6E)<< 5) | i2c_smbus_read_byte_data(fd,0x6F) >> 3));
			printf("EMC2305 FAN5 of RPM is %d\n", (uint16_t) ((i2c_smbus_read_byte_data(fd,0x7E) << 5) | i2c_smbus_read_byte_data(fd,0x7F) >> 3));*/
			
			/*//block 最多只能讀32筆 byte ( I2C_SMBUS_BLOCK_MAX = 32 Linux 預設)
			ret = i2c_smbus_read_i2c_block_data(fd, 0x30, 0x20, data);
			printf("EMC2305 FAN1 reg 0x30 is 0x%x (byte_data)\n", i2c_smbus_read_byte_data(fd,0x30));
			printf("EMC2305 FAN1 reg 0x3%x is 0x%x (block_data)\n",0,data[0]);
			printf("EMC2305 FAN1 of RPM is %d (block_data)\n", 8190 -(0x1FFF & ((uint16_t) (data[0x0E] << 8| data[0x0F]))>> 3));
			printf("EMC2305 FAN2 of RPM is %d (block_data)\n", (uint16_t) 8190 - ((uint16_t) ((data[0x1E] << 8| data[0x1F])>> 3)));
			ret = i2c_smbus_read_i2c_block_data(fd, 0x50, 0x20, data);
			printf("EMC2305 FAN3 of RPM is %d (block_data)\n", (uint16_t) 8190 - ((uint16_t) ((data[0x0E] << 8| data[0x0F])>> 3)));
			printf("EMC2305 FAN4 of RPM is %d (block_data)\n", (uint16_t) 8190 - ((uint16_t) ((data[0x1E] << 8| data[0x1F])>> 3)));
			ret = i2c_smbus_read_i2c_block_data(fd, 0x70, 0x10, data);
			printf("EMC2305 FAN5 of RPM is %d (block_data)\n", (uint16_t) 8190 - ((uint16_t) ((data[0x0E] << 8| data[0x0F])>> 3)));*/
			
			//usleep(10000);
			
			//s32 i2c_smbus_write_block_data(const struct i2c_client *client, u8 command, u8 length, const u8 *values)
			//s32 i2c_smbus_write_byte_data(const struct i2c_client *client, u8 command, u8 value)
			
			i2c_smbus_write_byte_data(fd, 0x30, Fan_rpm_setting[0]);
			i2c_smbus_write_byte_data(fd, 0x40, Fan_rpm_setting[1]);
			i2c_smbus_write_byte_data(fd, 0x50, Fan_rpm_setting[2]);
			i2c_smbus_write_byte_data(fd, 0x60, Fan_rpm_setting[3]);
			i2c_smbus_write_byte_data(fd, 0x70, Fan_rpm_setting[4]);
			
			//printf("EMC2305 FAN1 of RPM is %d\n",  0x1FFF & ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x3E)<< 5) | i2c_smbus_read_byte_data(fd,0x3F)) >> 3));
			buf[0] = 0x1FFF & ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x3E)<< 5) | i2c_smbus_read_byte_data(fd,0x3F)) >> 3);
			buf[1] = 0x1FFF & ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x4E)<< 5) | i2c_smbus_read_byte_data(fd,0x4F)) >> 3);
			buf[2] = 0x1FFF & ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x5E)<< 5) | i2c_smbus_read_byte_data(fd,0x5F)) >> 3);
			buf[3] = 0x1FFF & ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x6E)<< 5) | i2c_smbus_read_byte_data(fd,0x6F)) >> 3);
			buf[4] = 0x1FFF & ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x7E)<< 5) | i2c_smbus_read_byte_data(fd,0x7F)) >> 3);
			
			
		/*	uint8_t rpm_setting[32] = {0};
			rpm_setting[0] = fan1_rpm;
			rpm_setting[1] = fan1_rpm;
			i2c_smbus_write_block_data(fd, 0x3C, 2, rpm_setting);*/
	
	// close device node
	close(fd);
	return (buf);
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
        auto angle1 = egt::detail::to_radians<float>(180, 0);
	
       // auto min = std::min(widget.starting(), widget.ending());
        //auto max = std::max(widget.starting(), widget.ending());
		auto min = std::min(0,0);
       // auto max = std::max(8192,8192);
	   auto max = std::max(100,100);
        auto angle2 = egt::detail::to_radians<float>(180.0f,
                                                egt::detail::normalize_to_angle(static_cast<float>(widget.value()),
                                                        static_cast<float>(min), static_cast<float>(max), 0.0f, 360.0f, true));


		painter.line_width(linew);
        painter.set(widget.color(egt::Palette::ColorId::button_fg));
		painter.set(egt::Color::rgb(0xee82ee));
        painter.draw(egt::Arc(widget.center(), radius, 0.0f, 2 * egt::detail::pi<float>()));
        painter.stroke();


		//painter.set(widget.color(egt::Palette::ColorId::button_fg,egt::Palette::GroupId::disabled));
       painter.set(widget.color(egt::Palette::ColorId::button_fg));
	   painter.set(egt::Color::rgb(0xd3d3d3));
	  // painter.set(egt::Palette::yellow);
        painter.draw(egt::Arc(widget.center(), radius, angle1, angle2));
        painter.stroke();

        if (widget.show_label())
        {
            std::string text = std::to_string((static_cast<int> (widget.value())));
            auto f = egt::TextWidget::scale_font(egt::Size(dim * 0.9f, dim * 0.9f), text, widget.font());
            painter.set(f);
            painter.set(widget.color(egt::Palette::ColorId::text));
			painter.set(egt::Color::rgb(0x40e0d0));
			painter.set(egt::Font(30, egt::Font::Weight::bold));
            auto size = painter.text_size(text);
            auto target = egt::detail::align_algorithm(size, b, egt::AlignFlag::center);
            painter.draw(target.point());
            painter.draw(text);
        }
    }
};

   
	
int main(int argc, char** argv)
{
	uint8_t RPM_setting[5]={0x0F,0x0F,0x0F,0x0F,0x0F,};
	uint16_t *RPM_Fan;
    egt::Application app(argc, argv);  //This call will cost ~270ms on 9x60ek board

#ifdef EXAMPLEDATA
    egt::add_search_path(EXAMPLEDATA);
#endif

	egt::TopWindow win;
    // win.padding(10);
	win.show();
    win.color(egt::Palette::ColorId::bg, egt::Color::css("#1b1d43")); //#1b1d43 //#3b0030
	
	egt::experimental::Gauge gauge;
	//center(gauge);
	win.add(gauge);
	
		
	struct ifaddrs *ifaddr, *ifa;
    int  s;
    char host[NI_MAXHOST],Slave_IP[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        s=getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if( (strcmp(ifa->ifa_name,"wlan0")==0)&&(  ifa->ifa_addr->sa_family==AF_INET)  )
        {
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("\tInterface : <%s>\n",ifa->ifa_name );
            printf("\t  Address : <%s>\n", host);
			
			std::string str0 = ifa->ifa_name;
			std::string str1 = host;
			auto Address = std::make_shared<egt::Label>();
			Address->box(egt::Rect(5.0,75.0, 50.0, 10.0));
			Address->font(egt::Font(egt::Font("Serif", 25, egt::Font::Weight::normal  , egt::Font::Slant::italic)));
			Address->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0x00FFFF));
			win.add(Address);
			Address->text("(Client:)" + str0 + " : " + str1);
        }
    }
	
    freeifaddrs(ifaddr);
	
	
				
	auto logo = std::make_shared<egt::ImageLabel>(egt::Image("file:TW_Master.jpg"));
	//logo->margin(40);
	//logo->align(egt::AlignFlag::bottom | egt::AlignFlag::right);
	logo->align(egt::AlignFlag::bottom | egt::AlignFlag::expand_horizontal);
	//win.add(bottom(right(logo)));
	win.add(logo);
	
	auto title = std::make_shared<egt::Label>(win, "Microchip 2024 Taiwan MASTERs");
	title->align(egt::AlignFlag::top | egt::AlignFlag::expand_horizontal);
	title->font(egt::Font(40));
	title->margin(2);
	title->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xC71585));
	
	auto demoname = std::make_shared<egt::Label>(win, "MODBUS/TCP Client only");
	demoname->align(egt::AlignFlag::top | egt::AlignFlag::expand_horizontal);
	demoname->font(egt::Font(35));
	demoname->margin(42);
	demoname->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xFF69B4));
	
	auto Author = std::make_shared<egt::Label>(win, "Author : Miles");
	Author->align(egt::AlignFlag::top | egt::AlignFlag::right);
	Author->font(egt::Font(15));
	Author->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0xfffacd));


	auto FAN1_slider= std::make_shared<egt::Slider>(0,255);
	FAN1_slider->color(egt::Palette::ColorId::button_bg , egt::Palette::white);
	FAN1_slider->box(egt::Rect(0.0, 120.0, 50.0, 10.0));
	int read_FAN1_RPM = FAN1_slider->value();
	win.add(FAN1_slider);
	
	auto value1 = std::make_shared<egt::Label>();
	value1->box(egt::Rect(80.0,100.0, 50.0, 10.0));
	value1->font(egt::Font(egt::Font("Serif", 30, egt::Font::Weight::normal  , egt::Font::Slant::italic)));
	value1->color(egt::Palette::ColorId::label_text, egt::Palette::white);
	win.add(value1);
	
	auto FAN2_slider= std::make_shared<egt::Slider>(0,255);
	FAN2_slider->color(egt::Palette::ColorId::button_bg , egt::Palette::white);
	FAN2_slider->box(egt::Rect(160, 120.0, 50.0, 10.0));
	int read_FAN2_RPM = FAN2_slider->value();
	win.add(FAN2_slider);
	
	auto value2 = std::make_shared<egt::Label>();
	value2->box(egt::Rect(240.0,100.0, 50.0, 10.0));
	value2->font(egt::Font(egt::Font("Serif", 30, egt::Font::Weight::normal  , egt::Font::Slant::italic)));
	value2->color(egt::Palette::ColorId::label_text, egt::Palette::white);
	win.add(value2);
	
	auto FAN3_slider= std::make_shared<egt::Slider>(0,255);
	FAN3_slider->color(egt::Palette::ColorId::button_bg , egt::Palette::white);
	FAN3_slider->box(egt::Rect(320.0, 120.0, 50.0, 10.0));
	int read_FAN3_RPM = FAN3_slider->value();
	win.add(FAN3_slider);
	
	auto value3 = std::make_shared<egt::Label>();
	value3->box(egt::Rect(400.0,100.0, 50.0, 10.0));
	value3->font(egt::Font(egt::Font("Serif", 30, egt::Font::Weight::normal  , egt::Font::Slant::italic)));
	value3->color(egt::Palette::ColorId::label_text, egt::Palette::white);
	win.add(value3);
	
	auto FAN4_slider= std::make_shared<egt::Slider>(0,255);
	FAN4_slider->color(egt::Palette::ColorId::button_bg , egt::Palette::white);
	FAN4_slider->box(egt::Rect(480.0, 120.0, 50.0, 10.0));
	int read_FAN4_RPM = FAN4_slider->value();
	win.add(FAN4_slider);
	
	auto value4 = std::make_shared<egt::Label>();
	value4->box(egt::Rect(560.0,100.0, 50.0, 10.0));
	value4->font(egt::Font(egt::Font("Serif", 30, egt::Font::Weight::normal  , egt::Font::Slant::italic)));
	value4->color(egt::Palette::ColorId::label_text, egt::Palette::white);
	win.add(value4);
	
	auto FAN5_slider= std::make_shared<egt::Slider>(0,255);
	FAN5_slider->color(egt::Palette::ColorId::button_bg , egt::Palette::white);
	FAN5_slider->box(egt::Rect(640.0, 120.0, 50.0, 10.0));
	int read_FAN5_RPM = FAN5_slider->value();
	win.add(FAN5_slider);
	
	auto value5 = std::make_shared<egt::Label>();
	value5->box(egt::Rect(720.0,100.0, 50.0, 10.0));
	value5->font(egt::Font(egt::Font("Serif", 30, egt::Font::Weight::normal  , egt::Font::Slant::italic)));
	value5->color(egt::Palette::ColorId::label_text, egt::Palette::white);
	win.add(value5);
	
	egt::PeriodicTimer time_timer_slider1(std::chrono::milliseconds(50));
	time_timer_slider1.on_timeout([&FAN1_slider,&read_FAN1_RPM,&value1,&FAN2_slider,&read_FAN2_RPM,&value2,&FAN3_slider,&read_FAN3_RPM,&value3,&FAN4_slider,&read_FAN4_RPM,&value4,&FAN5_slider,&read_FAN5_RPM,&value5,&RPM_setting,&RPM_Fan]()
    {
		//RPM_Fan = EMC2305_init(RPM_setting);
		value1->text(std::to_string(static_cast<int>(FAN1_slider->value())));
		value2->text(std::to_string(static_cast<int>(FAN2_slider->value())));
		value3->text(std::to_string(static_cast<int>(FAN3_slider->value())));
		value4->text(std::to_string(static_cast<int>(FAN4_slider->value())));
		value5->text(std::to_string(static_cast<int>(FAN5_slider->value())));
		
	/*	RPM_setting[0] = FAN1_slider->value();
		RPM_setting[1] = FAN2_slider->value();
		RPM_setting[2] = FAN3_slider->value();
		RPM_setting[3] = FAN4_slider->value();
		RPM_setting[4] = FAN5_slider->value();*/
		
		read_FAN1_RPM = RPM_Fan[0];
		read_FAN2_RPM = RPM_Fan[1];
		read_FAN3_RPM = RPM_Fan[2];
		read_FAN4_RPM = RPM_Fan[3];
		read_FAN5_RPM = RPM_Fan[4];
		
		//std::cout <<  " FAN1_setting " << std::endl;
    });
    time_timer_slider1.start();
	
	auto thread_Modbus = [&RPM_setting,&RPM_Fan,&FAN1_slider,&FAN2_slider,&FAN3_slider,&FAN4_slider,&FAN5_slider,&win,&argc,&argv,&read_FAN1_RPM,&read_FAN2_RPM,&read_FAN3_RPM,&read_FAN4_RPM,&read_FAN5_RPM,&host,&Slave_IP]() {	
				modbus_t *ctx;
				//int rc;
				//int nb_fail;
				//int nb_loop;
				int nb=5;
				uint8_t *tab_rq_bits;
				uint8_t *tab_rp_bits;
				uint16_t *tab_rq_registers;
				uint16_t *tab_rw_rq_registers;
				uint16_t *tab_rp_registers;
				//char* ip= "192.168.77.1";
				MODBUS_TASK_PARAM keyTaskParam;
				//pthread_t keyMonitorId;
			//	pthread_mutex_init(&gMutex, NULL);
				
				printf("host ip : %s \r\n",host);
				int a,b,c,d;
				sscanf(host,"%d.%d.%d.%d",&a,&b,&c,&d);
				printf("a = %d , b= %d , c=%d, d=%d\r\n",a,b,c,d);
				//Slave_IP
				d=131;
				sprintf(Slave_IP,"%d.%d.%d.%d",a,b,c,d);
				printf("Copy host ip to prepare searching Slave_IP : %s \r\n",Slave_IP);
				//if( argc == 2 || argc == 3)
				if(argc == 3)
				{
					printf("argc  %d\r\n",argc);
					printf("The IP as your type  : %s\r\n",argv[1]);
					sprintf(Slave_IP,"%s",argv[1]);
					strcpy(keyTaskParam.slave_addr, argv[1]);
					ctx = modbus_new_tcp(argv[1], 1502);
				}
				/*else if( argc != 2 || argc==1)
				{
					printf("argc = %d, (argc over 2 or argc = 1)\r\n",argc);
					printf("Modbus_Slave_IP(code fixed) : %s\r\n",Modbus_Slave_IP);
					strcpy(keyTaskParam.slave_addr, Modbus_Slave_IP);
					ctx = modbus_new_tcp(Modbus_Slave_IP, 1502);
				}	*/
				else
				{
					printf("starting serach Server IP\r\n");
				
					while (modbus_connect(ctx) == -1) {
						sprintf(Slave_IP,"%d.%d.%d.%d",a,b,c,d);
						strcpy(keyTaskParam.slave_addr, Slave_IP);
						ctx = modbus_new_tcp(Slave_IP, 1502);
						
						if(d < 255)
							d++;
						else if(d>254)
							d = 1; 
						else
							d=1;
						
						
					}
					printf("Modbus_ServerIP(auto search Server ip) : %s\r\n",Slave_IP);
					
						std::string str_ip = Slave_IP;
						auto found_slave_ip = std::make_shared<egt::Label>();
						found_slave_ip->box(egt::Rect(360.0,75.0, 50.0, 10.0));
						found_slave_ip->font(egt::Font(egt::Font("Serif", 25, egt::Font::Weight::normal  , egt::Font::Slant::italic)));
						found_slave_ip->color(egt::Palette::ColorId::label_text, egt::Color::rgb(0x32CD32));
						found_slave_ip->text("Slave ip :" + str_ip );
						win.add(found_slave_ip);
				}	
				
				
				
				
				// ****************************************************	
				// key check task
				// ****************************************************	
				//argv[1] = "192.168.77.42";
				//strcpy(keyTaskParam.slave_addr, argv[1]);
				
				keyTaskParam.slave_port = 1502;
			#if 0	
				if( pthread_create(&keyMonitorId, NULL, (void*)monitorSlaveStatus, &keyTaskParam) != 0 )
				{
					printf("Create slave status monitor thread fail\r\n");
				}
				pthread_detach(keyMonitorId);
				pthread_join(keyMonitorId, NULL);
			#endif	
				// ****************************************************	
				// end of task process
				// ****************************************************	
				
				// ****************************************************	
				// master connection process
				// ****************************************************	
				//
				
				//modbus_set_debug(ctx, TRUE);
				modbus_set_error_recovery(ctx,
										  (modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK |
										  MODBUS_ERROR_RECOVERY_PROTOCOL));

				if (modbus_connect(ctx) == -1) {
					fprintf(stderr, "[main] Connection failed: %s\n", modbus_strerror(errno));
					modbus_free(ctx);
					return -1;
				}
				
				
				
				
				 /* Allocate and initialize the different memory spaces */
				//nb = ADDRESS_END - ADDRESS_START;

				tab_rq_bits = (uint8_t *) malloc(nb * sizeof(uint8_t));
				memset(tab_rq_bits, 0, nb * sizeof(uint8_t));

				tab_rp_bits = (uint8_t *) malloc(nb * sizeof(uint8_t));
				memset(tab_rp_bits, 0, nb * sizeof(uint8_t));

				tab_rq_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
				memset(tab_rq_registers, 0, nb * sizeof(uint16_t));

				tab_rp_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
				memset(tab_rp_registers, 0, nb * sizeof(uint16_t));

				tab_rw_rq_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
				memset(tab_rw_rq_registers, 0, nb * sizeof(uint16_t));

				//nb_loop = nb_fail = 0;
			   // while (nb_loop++ < LOOP) {
				   while (1) 
				   {
					   usleep(30*1000);
					for (int addr = ADDRESS_START; addr < ADDRESS_END; addr++) 
					{
						//3 read RPM_setting
						modbus_read_registers(ctx, addr, nb, tab_rp_registers);
						
						//16 silder write 
						if(RPM_setting[0] != FAN1_slider->value() || RPM_setting[1] != FAN2_slider->value() || RPM_setting[2] != FAN3_slider->value() || RPM_setting[3] != FAN4_slider->value() || RPM_setting[4] != FAN5_slider->value())
						//if((uint8_t)tab_rp_registers[0] != FAN1_slider->value() || (uint8_t)tab_rp_registers[1] != FAN2_slider->value() || (uint8_t)tab_rp_registers[2] != FAN3_slider->value() || (uint8_t)tab_rp_registers[3] != FAN4_slider->value() || (uint8_t)tab_rp_registers[4] != FAN5_slider->value())	
						{
							tab_rw_rq_registers[0] = FAN1_slider->value();
							tab_rw_rq_registers[1] = FAN2_slider->value();
							tab_rw_rq_registers[2] = FAN3_slider->value();
							tab_rw_rq_registers[3] = FAN4_slider->value();
							tab_rw_rq_registers[4] = FAN5_slider->value();
							modbus_write_registers(ctx, addr, nb, tab_rw_rq_registers);
							//usleep(100*1000);
							RPM_setting[0] = FAN1_slider->value();
							RPM_setting[1] = FAN2_slider->value();
							RPM_setting[2] = FAN3_slider->value();
							RPM_setting[3] = FAN4_slider->value();
							RPM_setting[4] = FAN5_slider->value();
							//rc = modbus_write_registers(ctx, addr, nb, tab_rw_rq_registers);
						}
						else 
						{
							FAN1_slider->value((static_cast<int> (tab_rp_registers[0])));
							FAN2_slider->value((static_cast<int> (tab_rp_registers[1])));
							FAN3_slider->value((static_cast<int> (tab_rp_registers[2])));
							FAN4_slider->value((static_cast<int> (tab_rp_registers[3])));
							FAN5_slider->value((static_cast<int> (tab_rp_registers[4])));
							// usleep(100*1000);
						}
						/*
						if((uint8_t)tab_rp_registers[0] != FAN1_slider->value() &&(uint8_t)tab_rp_registers[1] != FAN2_slider->value() && (uint8_t)tab_rp_registers[2] != FAN3_slider->value() && (uint8_t)tab_rp_registers[3] != FAN4_slider->value() && (uint8_t)tab_rp_registers[4] != FAN5_slider->value())	*/
						//4 RPM
						//rc = modbus_read_input_registers(ctx, addr, nb, tab_rq_registers);
						modbus_read_input_registers(ctx, addr, nb, tab_rq_registers);
						RPM_Fan[0] = tab_rq_registers[0];
						RPM_Fan[1] = tab_rq_registers[1];
						RPM_Fan[2] = tab_rq_registers[2];
						RPM_Fan[3] = tab_rq_registers[3];
						RPM_Fan[4] = tab_rq_registers[4];
						
						read_FAN1_RPM = RPM_Fan[0];
						read_FAN2_RPM = RPM_Fan[1];
						read_FAN3_RPM = RPM_Fan[2];
						read_FAN4_RPM = RPM_Fan[3];
						read_FAN5_RPM = RPM_Fan[4];
					}
				}	
	
				//while(1);
			//close:

				//pthread_mutex_destroy(&gMutex);
				
				 /* Free the memory */
				free(tab_rq_bits);
				free(tab_rp_bits);
				free(tab_rq_registers);
				free(tab_rp_registers);
				free(tab_rw_rq_registers);

				/* Close the connection */
				modbus_close(ctx);
				modbus_free(ctx);
	};
	std::thread thread_Modbus_thread(thread_Modbus);
	
	
	
	
	//int v;
	//MySpinProgressF::ValueRangeWidget Fan1_bar(egt::Rect(5,170, 160,160), 0 , 8192, v);
	MySpinProgressF temp_ADM00879(egt::Rect(5,330, 160,160));
	temp_ADM00879.color(egt::Palette::ColorId::button_bg, egt::Color::rgb(0xffefd5), egt::Palette::GroupId::disabled);
	temp_ADM00879.font(egt::Font(30, egt::Font::Weight::bold));
	temp_ADM00879.alpha(0.8);
	
	egt::PeriodicTimer timer_temp_ADM00879(std::chrono::milliseconds(100));
	timer_temp_ADM00879.on_timeout([&temp_ADM00879,&read_FAN1_RPM]()
	{
		//Fan1_bar.value( (static_cast<float> (motoValue.moto_speed++)/9));
		temp_ADM00879.value( (static_cast<int> (MCP9800_init())));
		 //std::cout <<  "FAN1.RPM : " << static_cast<int> (motoValue.moto_speed) << std::endl;
	});
	timer_temp_ADM00879.start();
	win.add(temp_ADM00879);
	
	auto Fan1_bar = std::make_shared<egt::SpinProgress>(egt::Rect(5,170, 150,150), 0 , 8192,(static_cast<int> (5000)));
	Fan1_bar->color(egt::Palette::ColorId::text , egt::Palette::white);
	Fan1_bar->color(egt::Palette::ColorId::button_fg , egt::Palette::red);
	Fan1_bar->font(egt::Font(30, egt::Font::Weight::bold));
	win.add(Fan1_bar);
	auto Fan2_bar = std::make_shared<egt::SpinProgress>(egt::Rect(162,170, 150,150), 0 , 8192,(static_cast<int> (5000)));
	Fan2_bar->color(egt::Palette::ColorId::text , egt::Palette::white);
	Fan2_bar->color(egt::Palette::ColorId::button_fg , egt::Palette::orange);
	Fan2_bar->font(egt::Font(30, egt::Font::Weight::bold));
	win.add(Fan2_bar);
	auto Fan3_bar = std::make_shared<egt::SpinProgress>(egt::Rect(318,170, 150,150), 0 , 8192,(static_cast<int> (5000)));
	Fan3_bar->color(egt::Palette::ColorId::text , egt::Palette::white);
	Fan3_bar->color(egt::Palette::ColorId::button_fg , egt::Palette::yellow);
	Fan3_bar->font(egt::Font(30, egt::Font::Weight::bold));
	win.add(Fan3_bar);
	auto Fan4_bar = std::make_shared<egt::SpinProgress>(egt::Rect(475,170, 150,150), 0 , 8192,(static_cast<int> (5000)));
	Fan4_bar->color(egt::Palette::ColorId::text , egt::Palette::white);
	Fan4_bar->color(egt::Palette::ColorId::button_fg , egt::Palette::green);
	Fan4_bar->font(egt::Font(30, egt::Font::Weight::bold));
	win.add(Fan4_bar);
	auto Fan5_bar = std::make_shared<egt::SpinProgress>(egt::Rect(635,170, 150,150), 0 , 8192,(static_cast<int> (5000)));
	Fan5_bar->color(egt::Palette::ColorId::text , egt::Palette::white);
	Fan5_bar->color(egt::Palette::ColorId::button_fg , egt::Palette::blue);
	Fan5_bar->font(egt::Font(30, egt::Font::Weight::bold));
	win.add(Fan5_bar);
	egt::PeriodicTimer timer_Fan1_bar(std::chrono::milliseconds(50));
	timer_Fan1_bar.on_timeout([&Fan1_bar,&read_FAN1_RPM,&Fan2_bar,&read_FAN2_RPM,&Fan3_bar,&read_FAN3_RPM,&Fan4_bar,&read_FAN4_RPM,&Fan5_bar,&read_FAN5_RPM]()
	{
		Fan1_bar->value( (static_cast<int> (read_FAN1_RPM)));
		Fan2_bar->value( (static_cast<int> (read_FAN2_RPM)));
		Fan3_bar->value( (static_cast<int> (read_FAN3_RPM)));
		Fan4_bar->value( (static_cast<int> (read_FAN4_RPM)));
		Fan5_bar->value( (static_cast<int> (read_FAN5_RPM)));
	});
	timer_Fan1_bar.start();
    win.show();
	return app.run();
	
	
	
	

}



