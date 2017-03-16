#include "hFiles/Master.h"
int main() {
    Master master("127.0.0.1",9000,1);
    if(!master.run()){
        exit(1);
    }
    return 0;
}