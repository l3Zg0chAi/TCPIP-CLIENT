#include "Application.h"
#include "TCPCommunicator.h"
#include "Logger.h"
#include <thread>
#include <chrono>

DltContext main_dltCxt; // define context

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

int main() {
    std::cout << "before DLT_REGISTER_APP and context\n";
    DLT_REGISTER_APP("TCPC", "TCP Client Application"); // register app with DLT Daemon
    DLT_REGISTER_CONTEXT(main_dltCxt, "MAIN", "Main application context"); // register context of app with DLT Daemon

    std::cout << "register complete\n";
    DLT_LOG(main_dltCxt, DLT_LOG_ERROR, DLT_CSTRING("DLT smoke test TCPC MAIN"));
    Application::get_instance()->init();
    Application::get_instance()->execute();

    DLT_UNREGISTER_CONTEXT(main_dltCxt); // unregister context
    DLT_UNREGISTER_APP(); // unregister app
    return 0;
}