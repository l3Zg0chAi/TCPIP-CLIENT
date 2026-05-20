#include "Application.h"
#include "TCPCommunicator.h"
#include "Logger.h"
#include <thread>
#include <chrono>

void Application::init()
{
    TCPCommunicator::get_instance()->start();
}

void Application::execute()
{
    while (true){
        DEBUG_LOG("start execute");
        receive_from_server();
        DEBUG_LOG("end execute");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void Application::receive_from_server()
{
    Packet packet;
    while(TCPCommunicator::get_instance()->receive_packet(packet)){
        // handle data here
    }
    DEBUG_LOG("not element in queue any more");
}

DLT_DEFINE_CONTEXT(main_dltCxt); // define context

int main() {
    DLT_REGISTER_APP("TCPC", "TCP Client Application"); // register app with DLT Daemon
    DLT_REGISTER_CONTEXT(main_dltCxt, "MAIN", "Main application context"); // register context of app with DLT Daemon
    Application::get_instance()->init();
    Application::get_instance()->execute();

    DLT_UNREGISTER_CONTEXT(main_dltCxt); // unregister context
    return 0;
}