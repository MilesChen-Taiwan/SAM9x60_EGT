
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
#include <thread>
#include <egt/ui>
#include <iostream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <planes/plane.h>
#include "egt/detail/screen/kmsoverlay.h"
#include "egt/detail/screen/kmsscreen.h"
#include "djpeg.h"
#include <time.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <unistd.h>
#include <limits.h>

#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#define MAX_WIDTH	800
#define MAX_HEIGHT	360
int width = MAX_WIDTH;
int height = MAX_HEIGHT;
#define WELCOME_IMAGE	"test.jpg"//"honda.jpg"
//#define WELCOME_IMAGE	"0000.jpg"
#define PORT 7533
#define SA struct sockaddr

struct ImageFrame {
    char* buffer;
   size_t bufferSize;
};

int show_ip(const char *adapter)
{
	unsigned int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	//I want to get an IPv4 IP address 
	ifr.ifr_addr.sa_family = AF_INET;

	// I want IP address attached to "eth0"
	strncpy(ifr.ifr_name, adapter, IFNAMSIZ-1);

	if( ioctl(fd, SIOCGIFADDR, &ifr) < 0 )
	{
		std::cout << adapter << " is not available" << std::endl;
	}
	else
	{
		std::cout << "Server IP (this board) : " << inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr) << std::endl;
	}

	close(fd);

	return 0;
}

bool readJPEG(const char* filename, ImageFrame& frame) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    std::cout << "Reading: " << filename << std::endl;

    frame.bufferSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    frame.buffer = static_cast<char*>(malloc(frame.bufferSize));
    if (!frame.buffer) {
        std::cerr << "Error allocating memory for buffer." << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(frame.buffer), frame.bufferSize);
    file.close();

    return true;
}

bool showJPEG(const char *filename, ImageFrame frame, char* disp_buffer)
{
    if( !readJPEG(filename, frame)) {
        std::cerr << "Failed to read JPEG file: " << filename << std::endl;
        return false;
    }

    int width = MAX_WIDTH;
    int height = MAX_HEIGHT;

    djpeg_yuv((char *)frame.buffer, frame.bufferSize, disp_buffer, &width, &height);

    return true;
}
#define packet 1024
// Function designed for chat between client and server.
void server_proc(int connfd, char* disp_buffer)
{
    unsigned char *file_ptr, *init_p;
    unsigned int file_size, recived_total_size=0, packet_size =packet,packet_size_d =packet ;
    unsigned int next_command=1;
    unsigned int count=0;
    char filename[80];
    int size;
    //remove("test.jpg");
    
	//FILE *fp  = fopen("test.jpg", "w+");
	// 1. read file size
	write(connfd,&packet_size_d,sizeof(packet_size_d));
	read(connfd, &file_size, sizeof(file_size));
    file_ptr = (unsigned char*)malloc(file_size);
	init_p = file_ptr;
    memset(file_ptr,0,file_size);
	std::cout << "Read to receive file total size  " << file_size << " bytes data : " <<std::endl;
	// infinite loop for chat
	
	while(1) {
	   	// 2. read packet_size
		write(connfd,&packet_size_d,sizeof(packet_size_d));
		//usleep(1000);
        read(connfd, &packet_size, sizeof(packet_size));
		
        // print read data
       
		//std::cout << "Read to receive packet_size : " << packet_size << " bytes data, #" <<  next_command <<std::endl;
		next_command++;
		//std::cout << "cont. #1 " <<std::endl;
		if(packet_size >= packet)
			while(packet_size != packet)
			{	
				
				write(connfd,&packet_size,sizeof(packet_size));
				read(connfd,&packet_size, sizeof(packet_size));
			
				std::cout << "cont. #while packet_size : " << packet_size <<std::endl;
			}
		//std::cout << "cont. #2 " <<std::endl;
		//*file_ptr = *(file_ptr+packet);
        // 3. read file binary
        //bzero(file_ptr, file_size);
		write(connfd,&packet_size,sizeof(packet_size));
		//usleep(1000);
        read(connfd, file_ptr, packet_size);
		//std::cout << "cont. #3 " <<std::endl;
		recived_total_size = recived_total_size+packet_size;
		//std::cout<< std::hex <<std::nouppercase << (file_ptr) <<std::endl;
		//std::cout<<  "file data :  " << (file_ptr) <<std::endl;
        // fwrite(file_ptr, packet_size, 1, fp);
        // usleep(1000);
        // 4. check last packet_size
       // read(connfd, &next_command, sizeof(next_command));
	   //usleep(100000);
        if(file_size <= recived_total_size)//packet_size < packet
        {
        	//size = ftell(fp);
        	//djpeg_yuv((char*)fp, size, disp_buffer, &width, &height);
			next_command = next_command-2;
			std::cout << "Read to receive 'LAST' packet_size : " << packet_size <<std::endl;
			std::cout << "Read to receive 'Total' packet_size : " << (packet*(next_command))+packet_size <<std::endl;
			//*file_ptr = *(file_ptr-(packet*next_command)-packet_size);
			//file_ptr = init_p; 
			djpeg_yuv((char*)file_ptr, file_size, disp_buffer, &width, &height);
        	//fclose(fp); 
			free(file_ptr);
        	//exit(0);
        	return; 
			//break;
        }
		//next_command = 0;
       // djpeg_yuv((char*)file_ptr, file_size, disp_buffer, &width, &height);
       // write(connfd,"I got\n",sizeof("I got\n"));
       
    }

}

class EGT_API OverlayWindow : public egt::Window
{
public:
    OverlayWindow(const egt::Rect& rect,
              egt::PixelFormat format_hint = egt::PixelFormat::yuv420,
              egt::WindowHint hint = egt::WindowHint::overlay)
        : egt::Window(rect, format_hint, hint)
    {
        allocate_screen();
        m_overlay = reinterpret_cast<egt::detail::KMSOverlay*>(screen());
        assert(m_overlay);
        plane_set_pos(m_overlay->s(), 0, 0);
        plane_apply(m_overlay->s());
    }

    egt::detail::KMSOverlay* GetOverlay()
    {
        return m_overlay;
    }

private:
    egt::detail::KMSOverlay* m_overlay;
};

int main(int argc, char** argv)
{
    egt::Application app(argc, argv);
    egt::add_search_path("/usr/share/libegt");
    egt::TopWindow window;

    ///-- Read the JPEG file to local buffer
    std::vector<std::string> filenames;

    auto descriptions = std::make_shared<egt::Label>(window, "Decode JPEG to YUV on HEO layer        800x360");
    descriptions->align(egt::AlignFlag::bottom | egt::AlignFlag::left);
    descriptions->font(egt::Font(30));
    descriptions->margin(7);

    egt::Label lfps("FPS: ---");
    lfps.align(egt::AlignFlag::bottom | egt::AlignFlag::right);
    lfps.color(egt::Palette::ColorId::label_text, egt::Palette::black);
    lfps.color(egt::Palette::ColorId::label_bg, egt::Palette::transparent);
    lfps.font(egt::Font(30));
    lfps.margin(7);
    window.add(lfps);

    OverlayWindow heoWin(egt::Rect(0, 60, MAX_WIDTH, MAX_HEIGHT));
    window.add(heoWin);

    egt::experimental::FramesPerSecond fps;
    fps.start();

    char* yuv_ptr = (char*)heoWin.GetOverlay()->raw();
    ImageFrame frame;
    std::string filename;
    unsigned int idx = 0;

    if( !showJPEG(WELCOME_IMAGE, frame, yuv_ptr) ) {
    	std::cout << "Read first JPEG file error" << std::endl;
    	return -1;
    }

    heoWin.GetOverlay()->schedule_flip();
	show_ip("wlan0");
	//std::cout << "test1 " << std::endl;
	 std::vector<std::unique_ptr<egt::PeriodicTimer>> timers;
	auto ServerProc ([&heoWin, &frame, &yuv_ptr, &lfps, &fps](){
		//char* yuv_ptr = (char*)heoWin.GetOverlay()->raw();

	//	std::cout << "test2-1 " << std::endl;
		unsigned int sockfd, connfd;
		unsigned int len;
		struct sockaddr_in servaddr, cli;
	  //  std::cout << "test2 " << std::endl;
		// socket create and verification
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
		    std::cout << "socket creation failed..." << std::endl;
		    exit(0);
		}
		//else {
		//    std::cout << "Socket successfully created.." << std::endl;
	//	}
		// std::cout << "test3 " << std::endl;
		bzero(&servaddr, sizeof(servaddr));
		// std::cout << "test4 " << std::endl;
		// assign IP, PORT
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(PORT);
	  //  std::cout << "test5 " << std::endl;


		// Binding newly created socket to given IP and verification
		if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		    std::cout << "socket bind failed..." << std::endl;
		    while(((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0));
			//exit(0);
		}
		//else {
		//	std::cout << "Socket successfully binded.." << std::endl;
		//}

		// Now server is ready to listen and verification
		//usleep(500000);
		if ((listen(sockfd, 5)) != 0) {
		    std::cout << "Listen failed..." << std::endl;
		    exit(0);
		}
		else {
			std::cout << "Server listening.." << std::endl;
		}

		len = sizeof(cli);
	   //usleep(500000);
		// Accept the data packet from client and verification
		connfd = accept(sockfd, (struct sockaddr*)&cli, &len);

		if (connfd < 0) {
		    std::cout << "server accept failed..." << std::endl;

			if((errno == ENETDOWN || errno == EPROTO || errno == ENOPROTOOPT || errno == EHOSTDOWN ||
            errno == ENONET || errno == EHOSTUNREACH || errno == EOPNOTSUPP || errno == ENETUNREACH)) {
				std::cout << errno  << std::endl;
			}

			exit(0);
		}
		else
		{
			std::cout << "server accept the client..." << std::endl;
			std::cout <<"errno : " << static_cast<int> (errno)  << std::endl;
		}
		//usleep(150000);
		// Function for chatting between client and server
		server_proc(connfd, yuv_ptr);

	//	if( !showJPEG(WELCOME_IMAGE, frame, yuv_ptr) ) {
//
	//		std::cout << "Read first JPEG file error" << std::endl;
		//	return -1;
		//}


		close(sockfd);
		close(connfd);

		if (fps.ready())
        {
            std::ostringstream ss;
            ss << "FPS: " << std::round(fps.fps());
            lfps.text(ss.str());
        }

        //heoWin.GetOverlay()->schedule_flip();

        fps.end_frame();

	});
	ServerProc();

	timers.emplace_back(std::make_unique<egt::PeriodicTimer>(std::chrono::milliseconds(60)));
    timers.back()->on_timeout(ServerProc);
    timers.back()->start();


    window.show();

    auto ret = app.run();

    return ret;
}
/*
#include <chrono>
#include <thread>
#include <egt/ui>
#include <iostream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <planes/plane.h>
#include "egt/detail/screen/kmsoverlay.h"
#include "egt/detail/screen/kmsscreen.h"
#include "djpeg.h"
#include <time.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <unistd.h>
#include <limits.h>

#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#define MAX_WIDTH	800
#define MAX_HEIGHT	360
int width = MAX_WIDTH;
int height = MAX_HEIGHT;
#define WELCOME_IMAGE	"honda.jpg"
//#define WELCOME_IMAGE	"0000.jpg"
#define PORT 123
#define SA struct sockaddr

struct ImageFrame {
    char* buffer;
   size_t bufferSize;
};

int show_ip(const char *adapter)
{
	unsigned int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	//I want to get an IPv4 IP address 
	ifr.ifr_addr.sa_family = AF_INET;

	// I want IP address attached to "eth0"
	strncpy(ifr.ifr_name, adapter, IFNAMSIZ-1);

	if( ioctl(fd, SIOCGIFADDR, &ifr) < 0 )
	{
		std::cout << adapter << " is not available" << std::endl;
	}
	else
	{
		std::cout << "Server IP (this board) : " << inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr) << std::endl;
	}

	close(fd);

	return 0;
}

bool readJPEG(const char* filename, ImageFrame& frame) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    std::cout << "Reading: " << filename << std::endl;

    frame.bufferSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    frame.buffer = static_cast<char*>(malloc(frame.bufferSize));
    if (!frame.buffer) {
        std::cerr << "Error allocating memory for buffer." << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(frame.buffer), frame.bufferSize);
    file.close();

    return true;
}

bool showJPEG(const char *filename, ImageFrame frame, char* disp_buffer)
{
    if( !readJPEG(filename, frame)) {
        std::cerr << "Failed to read JPEG file: " << filename << std::endl;
        return false;
    }

    int width = MAX_WIDTH;
    int height = MAX_HEIGHT;

    djpeg_yuv((char *)frame.buffer, frame.bufferSize, disp_buffer, &width, &height);

    return true;
}

// Function designed for chat between client and server.
void server_proc(int connfd, char* disp_buffer)
{
    unsigned char *file_ptr;
    unsigned int file_size;
    unsigned int next_command;
    unsigned int count=0;
     char filename[80];

    // infinite loop for chat
   // for (;;) {
        // 1. read file size
        read(connfd, &file_size, sizeof(file_size));
        // print read data
        std::cout << "Read to receive " << file_size << " bytes data"  << std::endl;
        //usleep(100000);
        // 2. read file binary
        file_ptr = (unsigned char*)malloc(file_size);
        bzero(file_ptr, file_size);
        read(connfd, file_ptr, file_size);
		//usleep(100000);




#if 0
        // debug
        sprintf(filename, "%04d.jpg", count++);
        std::cout << "Writting " << filename << std::endl;
        FILE *fp;
        fp = fopen(filename, "wb");
        fwrite(file_ptr, file_size, 1, fp);
        fclose(fp);
#endif
		djpeg_yuv((char*)file_ptr, file_size, disp_buffer, &width, &height);
        //usleep(100000);
		free(file_ptr);

        // 3. read next command
        read(connfd, &next_command, sizeof(next_command));
        if( next_command == 1 )
        {
        	 exit(0);
        	//break;
        }
		next_command = 0;
   // }
}

class EGT_API OverlayWindow : public egt::Window
{
public:
    OverlayWindow(const egt::Rect& rect,
              egt::PixelFormat format_hint = egt::PixelFormat::yuv420,
              egt::WindowHint hint = egt::WindowHint::overlay)
        : egt::Window(rect, format_hint, hint)
    {
        allocate_screen();
        m_overlay = reinterpret_cast<egt::detail::KMSOverlay*>(screen());
        assert(m_overlay);
        plane_set_pos(m_overlay->s(), 0, 0);
        plane_apply(m_overlay->s());
    }

    egt::detail::KMSOverlay* GetOverlay()
    {
        return m_overlay;
    }

private:
    egt::detail::KMSOverlay* m_overlay;
};

int main(int argc, char** argv)
{
    egt::Application app(argc, argv);
    egt::add_search_path("/usr/share/libegt");
    egt::TopWindow window;

    ///-- Read the JPEG file to local buffer
    std::vector<std::string> filenames;

    auto descriptions = std::make_shared<egt::Label>(window, "Decode JPEG to YUV on HEO layer        800x360");
    descriptions->align(egt::AlignFlag::bottom | egt::AlignFlag::left);
    descriptions->font(egt::Font(30));
    descriptions->margin(7);

    egt::Label lfps("FPS: ---");
    lfps.align(egt::AlignFlag::bottom | egt::AlignFlag::right);
    lfps.color(egt::Palette::ColorId::label_text, egt::Palette::black);
    lfps.color(egt::Palette::ColorId::label_bg, egt::Palette::transparent);
    lfps.font(egt::Font(30));
    lfps.margin(7);
    window.add(lfps);

    OverlayWindow heoWin(egt::Rect(0, 60, MAX_WIDTH, MAX_HEIGHT));
    window.add(heoWin);

    egt::experimental::FramesPerSecond fps;
    fps.start();

    char* yuv_ptr = (char*)heoWin.GetOverlay()->raw();
    ImageFrame frame;
    std::string filename;
    unsigned int idx = 0;

    if( !showJPEG(WELCOME_IMAGE, frame, yuv_ptr) ) {
    	std::cout << "Read first JPEG file error" << std::endl;
    	return -1;
    }

    heoWin.GetOverlay()->schedule_flip();
	show_ip("eth0");
	//std::cout << "test1 " << std::endl;
	 std::vector<std::unique_ptr<egt::PeriodicTimer>> timers;
	auto ServerProc ([&heoWin, &frame, &yuv_ptr, &lfps, &fps](){
		//char* yuv_ptr = (char*)heoWin.GetOverlay()->raw();

	//	std::cout << "test2-1 " << std::endl;
		unsigned int sockfd, connfd;
		unsigned int len;
		struct sockaddr_in servaddr, cli;
	  //  std::cout << "test2 " << std::endl;
		// socket create and verification
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
		    std::cout << "socket creation failed..." << std::endl;
		    exit(0);
		}
		//else {
		//    std::cout << "Socket successfully created.." << std::endl;
	//	}
		// std::cout << "test3 " << std::endl;
		bzero(&servaddr, sizeof(servaddr));
		// std::cout << "test4 " << std::endl;
		// assign IP, PORT
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(PORT);
	  //  std::cout << "test5 " << std::endl;


		// Binding newly created socket to given IP and verification
		if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		    std::cout << "socket bind failed..." << std::endl;
		    exit(0);
		}
		//else {
		//	std::cout << "Socket successfully binded.." << std::endl;
		//}

		// Now server is ready to listen and verification
		//usleep(500000);
		if ((listen(sockfd, 5)) != 0) {
		    std::cout << "Listen failed..." << std::endl;
		    exit(0);
		}
		else {
			std::cout << "Server listening.." << std::endl;
		}

		len = sizeof(cli);
	   //usleep(500000);
		// Accept the data packet from client and verification
		connfd = accept(sockfd, (struct sockaddr*)&cli, &len);

		if (connfd < 0) {
		    std::cout << "server accept failed..." << std::endl;

			if((errno == ENETDOWN || errno == EPROTO || errno == ENOPROTOOPT || errno == EHOSTDOWN ||
            errno == ENONET || errno == EHOSTUNREACH || errno == EOPNOTSUPP || errno == ENETUNREACH)) {
				std::cout << errno  << std::endl;
			}

			exit(0);
		}
		else
		{
			std::cout << "server accept the client..." << std::endl;
			std::cout <<"errno : " << static_cast<int> (errno)  << std::endl;
		}
		//usleep(150000);
		// Function for chatting between client and server
		server_proc(connfd, yuv_ptr);

	//	if( !showJPEG(WELCOME_IMAGE, frame, yuv_ptr) ) {
//
	//		std::cout << "Read first JPEG file error" << std::endl;
		//	return -1;
		//}
		close(sockfd);
		close(connfd);

		if (fps.ready())
        {
            std::ostringstream ss;
            ss << "FPS: " << std::round(fps.fps());
            lfps.text(ss.str());
        }

        //heoWin.GetOverlay()->schedule_flip();

        fps.end_frame();

	});
	ServerProc();

	timers.emplace_back(std::make_unique<egt::PeriodicTimer>(std::chrono::milliseconds(200)));
    timers.back()->on_timeout(ServerProc);
    timers.back()->start();


    window.show();

    auto ret = app.run();

    return ret;
}

*/



/*
#include <egt/ui>
#include <iostream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <planes/plane.h>
#include "egt/detail/screen/kmsoverlay.h"
#include "egt/detail/screen/kmsscreen.h"
#include "djpeg.h"


#define MAX_WIDTH	800
#define MAX_HEIGHT	360


struct ImageFrame {
    char* buffer;
    size_t bufferSize;
};

bool readJPEG(const char* filename, ImageFrame& frame) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    frame.bufferSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    frame.buffer = static_cast<char*>(malloc(frame.bufferSize));
    if (!frame.buffer) {
        std::cerr << "Error allocating memory for buffer." << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(frame.buffer), frame.bufferSize);
    file.close();

    return true;
}

class EGT_API OverlayWindow : public egt::Window
{
public:
    OverlayWindow(const egt::Rect& rect,
              egt::PixelFormat format_hint = egt::PixelFormat::yuv420,
              egt::WindowHint hint = egt::WindowHint::overlay)
        : egt::Window(rect, format_hint, hint)
    {
        allocate_screen();
        m_overlay = reinterpret_cast<egt::detail::KMSOverlay*>(screen());
        assert(m_overlay);
        plane_set_pos(m_overlay->s(), 0, 60);
        plane_apply(m_overlay->s());
    }

    egt::detail::KMSOverlay* GetOverlay()
    {
        return m_overlay;
    }

private:
    egt::detail::KMSOverlay* m_overlay;
};

int main(int argc, char** argv)
{
    egt::Application app(argc, argv);

    egt::add_search_path("/usr/share/libegt");

    egt::TopWindow window;

    ///-- Read the JPEG file to local buffer
    std::vector<ImageFrame> frames;
    std::vector<std::string> filenames;

    // Since this code is tested on sam9x60-curiosity D1G, the memory is limited.
    // So we only decode 641 images to ram.
    for (auto i=3000; i<3641; i++) {
        filenames.push_back("images/" + std::to_string(i+1) + ".jpg");
    }

    for (std::string filename : filenames) {
        ImageFrame frame;
        if (readJPEG(filename.c_str(), frame)) {
            frames.push_back(frame);
        } else {
            std::cerr << "Failed to read JPEG file: " << filename << std::endl;
        }
    }

    auto label = std::make_shared<egt::ImageLabel>(
                     egt::Image("icon:egt_logo_black.png;128"));
    label->fill_flags().clear();
    label->margin(7);
    label->align(egt::AlignFlag::left);
    label->image_align(egt::AlignFlag::center);
    window.add(label);

    auto txt = std::make_shared<egt::TextBox>("EGT JPEG decoding YUV demo");
    txt->align(egt::AlignFlag::top | egt::AlignFlag::center_horizontal);
    txt->font(egt::Font(25, egt::Font::Weight::bold));
    txt->text_align(egt::AlignFlag::center);
    txt->margin(5);
    txt->border_flags({egt::Theme::BorderFlag::bottom, egt::Theme::BorderFlag::top});
    txt->disable();
    window.add(txt);

    auto descriptions = std::make_shared<egt::Label>(window, "Decode JPEG to YUV on HEO layer        800x360");
    descriptions->align(egt::AlignFlag::bottom | egt::AlignFlag::left);
    descriptions->font(egt::Font(30));
    descriptions->margin(7);

    egt::Label lfps("FPS: ---");
    lfps.align(egt::AlignFlag::bottom | egt::AlignFlag::right);
    lfps.color(egt::Palette::ColorId::label_text, egt::Palette::black);
    lfps.color(egt::Palette::ColorId::label_bg, egt::Palette::transparent);
    lfps.font(egt::Font(30));
    lfps.margin(7);
    window.add(lfps);

    OverlayWindow heoWin(egt::Rect(0, 60, MAX_WIDTH, MAX_HEIGHT));
    window.add(heoWin);

    egt::experimental::FramesPerSecond fps;
    fps.start();

    std::chrono::milliseconds ms(1);
    fprintf(stderr, "%s \n", argv[1]);
    int fps_set = 10;
    fps_set = atoi(argv[1]);
    if(fps_set < 1 || ( fps_set > 60))
	fps_set = 33;
    fprintf(stderr, "%d \n", fps_set);

    egt::PeriodicTimer timer(std::chrono::milliseconds(1000/fps_set * ms));

    timer.on_timeout([&heoWin, &frames, &fps, &lfps]() {
        static int idx = 0;
        char* yuv_ptr = (char*)heoWin.GetOverlay()->raw();

        int width = MAX_WIDTH;
        int height = MAX_HEIGHT;
       // djpeg_yuv((char *)frames[idx].buffer, frames[idx].bufferSize, yuv_ptr, &width, &height);

        if (fps.ready())
        {
            std::ostringstream ss;
            ss << "FPS: " << std::round(fps.fps());
            lfps.text(ss.str());
        }

        heoWin.GetOverlay()->schedule_flip();

        fps.end_frame();

        if (++idx >= 641)
            idx = 0;
    });
    timer.start();

    window.show();

    auto ret = app.run();

    for (ImageFrame& frame : frames) {
        free(frame.buffer);
    }

    return ret;
}

*/




/*
 * Copyright (C) 2018 Microchip Technology Inc.  All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/*
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <egt/detail/string.h>
#include <egt/ui>
#include <sstream>
#include <string>

#include <iomanip>
#include <iostream>
#include <libintl.h>

#include <egt/ui>
#include <egt/uiloader.h>
#include <egt/form.h>
#include <egt/shapes.h>
#include <egt/themes/midnight.h>
#include <egt/themes/shamrock.h>
#include <egt/themes/sky.h>
#include <egt/painter.h>
#include <egt/uiloader.h>

#include <egt/ui>
#include <iostream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <planes/plane.h>
#include "egt/detail/screen/kmsoverlay.h"
#include "egt/detail/screen/kmsscreen.h"
#include "djpeg.h"
#include <chrono>
//#include "../external/cxxopts/cxxopts.hpp"

#define MAX_WIDTH	800
#define MAX_HEIGHT	360


struct ImageFrame {
    char* buffer;
    size_t bufferSize;
};

bool readJPEG(const char* filename, ImageFrame& frame) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    frame.bufferSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    frame.buffer = static_cast<char*>(malloc(frame.bufferSize));
    if (!frame.buffer) {
        std::cerr << "Error allocating memory for buffer." << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(frame.buffer), frame.bufferSize);
    file.close();

    return true;
}

class EGT_API OverlayWindow : public egt::Window
{
public:
    OverlayWindow(const egt::Rect& rect, 
              egt::PixelFormat format_hint = egt::PixelFormat::yuv420,
              egt::WindowHint hint = egt::WindowHint::overlay )
        : egt::Window(rect, format_hint, hint)
    {
        allocate_screen();
        m_overlay = reinterpret_cast<egt::detail::KMSOverlay*>(screen());
        assert(m_overlay);
        plane_set_pos(m_overlay->s(), 0, 60);
        plane_apply(m_overlay->s());
    }
    egt::detail::KMSOverlay* GetOverlay()
    {
        return m_overlay;
    }
private:
    egt::detail::KMSOverlay* m_overlay;
};

int main(int argc, char** argv)
{
    egt::Application app(argc, argv);

    egt::add_search_path("/usr/share/libegt");

    egt::TopWindow window;
	
	
	
    ///-- Read the JPEG file to local buffer
    std::vector<ImageFrame> frames,frames_camera;
    std::vector<std::string> filenames;

    // Since this code is tested on sam9x60-curiosity D1G, the memory is limited.
    // So we only decode 641 images to ram.raw

    for (auto i=3000; i<3641; i++) {
        filenames.push_back("images/" + std::to_string(i+1) + ".jpg");
    }

    for (std::string filename : filenames) {
        ImageFrame frame;
        if (readJPEG(filename.c_str(), frame)) {
            frames.push_back(frame);
        } else {
            std::cerr << "Failed to read JPEG file: " << filename << std::endl;
        }
    }

    auto label = std::make_shared<egt::ImageLabel>(
                     egt::Image("icon:egt_logo_black.png;128"));
    label->fill_flags().clear();
    label->margin(7);
    label->align(egt::AlignFlag::left);
    label->image_align(egt::AlignFlag::center);
    window.add(label);

    auto txt = std::make_shared<egt::TextBox>("EGT JPEG decoding YUV demo");
    txt->align(egt::AlignFlag::top | egt::AlignFlag::center_horizontal);
    txt->font(egt::Font(25, egt::Font::Weight::bold));
    txt->text_align(egt::AlignFlag::center);
    txt->margin(5);
    txt->border_flags({egt::Theme::BorderFlag::bottom, egt::Theme::BorderFlag::top});
    txt->disable();
    window.add(txt);

    auto descriptions = std::make_shared<egt::Label>(window, "Decode JPEG to YUV on HEO layer        800x360");
    descriptions->align(egt::AlignFlag::bottom | egt::AlignFlag::left);
    descriptions->font(egt::Font(30));
    descriptions->margin(7);

    egt::Label lfps("FPS: ---");
    lfps.align(egt::AlignFlag::bottom | egt::AlignFlag::right);
    lfps.color(egt::Palette::ColorId::label_text, egt::Palette::black);
    lfps.color(egt::Palette::ColorId::label_bg, egt::Palette::transparent);
    lfps.font(egt::Font(30));
    lfps.margin(7);
    window.add(lfps);

    OverlayWindow heoWin(egt::Rect(0, 60, MAX_WIDTH, MAX_HEIGHT));
    window.add(heoWin);
    
    egt::experimental::FramesPerSecond fps;
    fps.start();


 // Setting camera
#define CAM_WIDTH 320
#define CAM_HEIGHT 240
#define CAM_FORMAT (v4l2_fourcc('Y', 'U', 'Y', 'V'))

//#define CAM_FORMAT (v4l2_fourcc('M', 'J', 'P', 'G'))

//#define CAM_FORMAT (v4l2_fourcc('Y', 'V', 'Y', 'U'))
//#define CAM_FORMAT (v4l2_fourcc('B', 'G', 'R', '3'))
//#define CAM_FORMAT (v4l2_fourcc('B', 'A', '2', '4'))
//#define CAM_FORMAT (v4l2_fourcc('B','G','R','A'))
//#define CAM_FORMAT (v4l2_fourcc('A', 'R', '2', '4'))

#define CAM_BUFFERS 1
 int cam_fd; 
   struct v4l2_capability cap;
    struct v4l2_requestbuffers reqbuf;
    struct v4l2_format fmt;
    struct v4l2_buffer buffinfo;
    enum v4l2_buf_type bufftype;

    char *cam_buffers[8];
    int cam_buffer_size;
    int cam_buffer_index = -1;

    char *fb_p;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
 
    cam_fd = open("/dev/video0", O_RDWR | O_NONBLOCK, 0);
    if(!cam_fd){
        fprintf(stderr, "%s:%i: Couldn't open device\n", __FILE__, __LINE__);
        return -1;
    }
    if(ioctl(cam_fd, VIDIOC_QUERYCAP, &cap))
    {
        fprintf(stderr, "%s:%i: Couldn't retreive device capabilities\n", __FILE__, __LINE__);
        return -1;
    }
    if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE == 0)
    {
        fprintf(stderr, "%s:%i: Device is not a capture device\n", __FILE__, __LINE__);
        return -1;
    }
    if(cap.capabilities & V4L2_CAP_STREAMING == 0)
    {
        fprintf(stderr, "%s:%i: Device is not available for streaming", __FILE__, __LINE__);
        return -1;
    }
// Set image format
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = CAM_WIDTH;
    fmt.fmt.pix.height = CAM_HEIGHT;
    fmt.fmt.pix.pixelformat = CAM_FORMAT;
    fmt.fmt.pix.field =  V4L2_FIELD_NONE;
    if(ioctl(cam_fd, VIDIOC_S_FMT, &fmt) == -1)
    {
        fprintf(stderr, "%s:%i: Unable to set image format\n", __FILE__, __LINE__);
        return -1;
    }
    cam_buffer_size = fmt.fmt.pix.sizeimage;

    // Request buffers
    memset(&reqbuf, 0, sizeof(reqbuf));
    reqbuf.count = CAM_BUFFERS;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    if(ioctl(cam_fd, VIDIOC_REQBUFS, &reqbuf) == -1)
    {
        fprintf(stderr, "%s:%i: Mmap streaming not supported\n", __FILE__, __LINE__);
        return -1;
    }
    if(reqbuf.count != CAM_BUFFERS)
    {
        fprintf(stderr, "%S:%i: Not all requared buffers are allocated\n", __FILE__, __LINE__);
        return -1;
    }
    
	 fprintf(stderr, "CAM_BUFFERS \n");
	
    //Query and Mmap buffers
    for (int i=0; i < CAM_BUFFERS; i++)
    {
        memset(&buffinfo, 0, sizeof(buffinfo));
        buffinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffinfo.memory = V4L2_MEMORY_MMAP;
        buffinfo.index = i;
        
        if(ioctl(cam_fd, VIDIOC_QUERYBUF, &buffinfo) == -1)
        {
            fprintf(stderr, "%s:%i: Unable to query buffers\n", __FILE__, __LINE__);
            return -1;
        }
        
        cam_buffers[i] = (char *) mmap(NULL, buffinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, cam_fd, buffinfo.m.offset);
    
        if(cam_buffers[i] == MAP_FAILED)
        {
            fprintf(stderr, "%s:%i: Unable to enqueue buffers\n", __FILE__, __LINE__);
            return -1;
        }
    }   
    
   // Enqueue buffers
        for (int i=0; i < CAM_BUFFERS; i++)
        {
                memset(&buffinfo, 0, sizeof(buffinfo));
                buffinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buffinfo.memory = V4L2_MEMORY_MMAP;
                buffinfo.index = i;

                if(ioctl(cam_fd, VIDIOC_QBUF, &buffinfo) == -1)
                {
                        fprintf(stderr, "%s:%i: Unable to enqueue buffers\n", __FILE__, __LINE__);
                        return -1;
                }
        }    

    // Start Streaming
    bufftype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(cam_fd, VIDIOC_STREAMON, &bufftype) == -1)
    {
        fprintf(stderr, "%s:%i: Unable to start streaming\n", __FILE__, __LINE__);
        return -1;
    }

	std::chrono::milliseconds ms(1);
	fprintf(stderr, "%s \n", argv[1]);
	int fps_set = 10; 
	//fps_set = atoi(argv[1]);
	if(fps_set < 1 || ( fps_set > 60))
		 fps_set = 33;
	fprintf(stderr, "%d \n", fps_set);
	
    //egt::PeriodicTimer timer(std::chrono::milliseconds(33));
    egt::PeriodicTimer timer(std::chrono::milliseconds(1000/fps_set * ms));

   // egt::PeriodicTimer timer(std::chrono::milliseconds(10));
    //egt::PeriodicTimer timer(std::chrono::milliseconds(50));
    timer.on_timeout([&heoWin, &frames, &fps, &lfps,&cam_fd, &cam_buffer_index,&buffinfo,&cam_buffers, &vinfo]() {
        static int idx = 0;
        char* yuv_ptr = (char*)heoWin.GetOverlay()->raw();

        int width = MAX_WIDTH;
        int height = MAX_HEIGHT;
		 
		fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(cam_fd, &fds);
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        
		// fprintf(stderr, " entry while\n");
		
        r = select(cam_fd+1, &fds, NULL, NULL, &tv);
		 
        memset(&buffinfo, 0, sizeof(buffinfo));
        buffinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffinfo.memory = V4L2_MEMORY_MMAP;	
		ioctl(cam_fd, VIDIOC_DQBUF, &buffinfo);
        cam_buffer_index = buffinfo.index;
		
       
	   //djpeg_yuv((char *)frames[idx].buffer, frames[idx].bufferSize, yuv_ptr, &width, &height);
	 //  djpeg_yuv( cam_buffer_index, sizeof(buffinfo), yuv_ptr, &width, &height);
        
		//memcpy(yuv_ptr, cam_buffers[cam_buffer_index], CAM_WIDTH*CAM_HEIGHT*2); 
		//memcpy(yuv_ptr, cam_buffers[cam_buffer_index], CAM_WIDTH*CAM_HEIGHT*1); 
		
		int offset_data = 0;
		
		for (int i = 0; i < 360; i++)
		{
			for (int j = 0; j <400; j+=2)
			{
				int idx = i*800 + j;
				//int idx = 0;
				if(j < 320 && i< 240)
				{	
					//yuv_ptr[idx] = cam_buffers[cam_buffer_index][offset_data+0];
					//yuv_ptr[idx+1] = cam_buffers[cam_buffer_index][offset_data+1];
					//yuv_ptr[idx+2] = cam_buffers[cam_buffer_index][offset_data+2];
					//yuv_ptr[idx+3] =cam_buffers[cam_buffer_index][offset_data+3];
					
					float Y0 = cam_buffers[cam_buffer_index][offset_data+0];
					float U = cam_buffers[cam_buffer_index][offset_data+1];
					float Y1 = cam_buffers[cam_buffer_index][offset_data+2];
					float V = cam_buffers[cam_buffer_index][offset_data+3];
					
					char R0 = (1.164*(Y0-16.0)+1.596*(V-128.0));
					char G0 = (1.164*(Y0-16.0)-0.813*(V-128.0)-0.391*(U-128.0));
					char B = (1.164*(Y0-16.0)+2.018*(U-128.0));
					
					//char R1 = (1.164*(Y1-16.0)+1.596*(V-128.0));
					char G1 = (1.164*(Y1-16.0)-0.813*(V-128.0)-0.391*(U-128.0));
					char B1 = (1.164*(Y1-16.0)+2.018*(U-128.0));

					//yuv_ptr[idx+0] = ((B & 0xF8) | (G0>>5));  
					//yuv_ptr[idx+1] = ((G0 & 0x1C) << 3) | (R0 >> 3);  
				
					yuv_ptr[idx+0] = Y0+1.402*V;
					yuv_ptr[idx+1] = Y1+1.402*V;
					

					yuv_ptr[idx+1] = (1.164*(Y0-16.0)+1.596*(V-128.0));
					yuv_ptr[idx+2] = (1.164*(Y0-16.0)-0.813*(V-128.0)-0.391*(U-128.0));
					yuv_ptr[idx+3] = (1.164*(Y0-16.0)+2.018*(U-128.0));
					yuv_ptr[idx+5] = (1.164*(Y1-16.0)+1.596*(V-128.0));
					yuv_ptr[idx+6] = (1.164*(Y1-16.0)-0.813*(V-128.0)-0.391*(U-128.0));
					yuv_ptr[idx+7] = (1.164*(Y1-16.0)+2.018*(U-128.0));
					//yuv_ptr[idx+6] = (1.164*(Y1-16.0)+2.018*(U-128.0));
					//yuv_ptr[idx+7] = 192;
					

					yuv_ptr[idx] = 255;
					yuv_ptr[idx+1] = Y0+1.402*V;
					yuv_ptr[idx+2] = Y0-0.344*U-0.792*V; 
					yuv_ptr[idx+3] = Y0+1.722*U;
					yuv_ptr[idx+4] = 255;
					yuv_ptr[idx+5] = Y1+1.402*V;
					yuv_ptr[idx+6] = Y1-0.344*U-0.792*V; 
					yuv_ptr[idx+7] = Y1+1.722*U;
					offset_data += 4;
				}
				else 
					yuv_ptr[idx] = yuv_ptr[idx];
					
			}
		}
			
				
		memset(&buffinfo, 0, sizeof(buffinfo));
                buffinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buffinfo.memory = V4L2_MEMORY_MMAP;
        buffinfo.index = cam_buffer_index;
		ioctl(cam_fd, VIDIOC_QBUF, &buffinfo);
		
		if (fps.ready())
        {
            std::ostringstream ss;
            ss << "FPS: " << std::round(fps.fps());
            lfps.text(ss.str());
        }

        heoWin.GetOverlay()->schedule_flip();
		
        fps.end_frame();

        if (++idx >= 641)
            idx = 0;
    });
    timer.start();
 
    window.show();

    auto ret = app.run();

    for (ImageFrame& frame : frames) {
        free(frame.buffer);
    }

    return ret;
}*/
