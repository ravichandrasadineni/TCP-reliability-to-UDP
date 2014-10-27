#include"portutility.h"
int is_valid_port (char* port_string) {
        int port_number = atoi(port_string);
        if((port_number > 0) && (port_number < 65535)) {
            return 1;
        }
    return 0;
}
