#include <iostream>
#include "configure.hpp"

std::string executive_file = HTML_FILE;
Socket *sock = new Socket(HTTP1_PORT);
sock->set_socket();

int body_length = 0;
int is_file_exist;

int accfd = -1;

