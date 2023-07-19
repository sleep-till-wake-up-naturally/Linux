#include "SelectServer.hpp"
#include <memory>

std::string transaction(const std::string &request)
{
    return request;
}

int main()
{
    std::unique_ptr<SelectServer_ns::SelectServer> ptr(new SelectServer_ns::SelectServer(transaction));
    ptr->init();
    ptr->start();
    
    return 0;
}