#include "Master.h"
int main() {
    Master master("127.0.0.1",9000,1);
    master.run();
    return 0;
}