// #include<iostream>
// #include<stdio.h>
// #include"timer_lst.h"
// #include<unistd.h>
// #define CLIENT_SIZE 1024
// #define CONSUME_TIME 5
// client_data* clilst=new client_data[CLIENT_SIZE];
// void cd_func(client_data* client ){
//     printf("CALLBACK TODO FROM : %d,THE TEXT: %s\n",client->sockfd,client->buf);
// }
// int main(int argv,char* argc[]){
//     timer_set* sdata=new timer_set;
//     for(int i=0;i<20;i++){
//         usleep(1000);
//         time_t curtime=time(nullptr);
//         client_data* clt=new client_data();
//         util_timer* node=new util_timer();
//         sprintf(clt->buf,"I am client form : %d\n",i);
//         clt->sockfd=i;
//         clt->timer=node;
//         node->expire=curtime+i*CONSUME_TIME;
//         node->cb_func=cd_func;
//         node->user_data=clt;
//         sdata->add_timer(*node);
//     }
//     sdata->print();
//     for(int i=0;i<10;i++){
//         sleep(1);
//         sdata->tick();
//         sdata->print();
//     }
//     return 0;
// } 
