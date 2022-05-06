#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/queue.h>
#include <math.h>
#include <string.h>
#include <netinet/in.h>
#include <signal.h>



struct Location_
{
    int X;
    int Y;
};

struct PictureBox
{
	int sockname;
    bool Visible;
    char *Name;
    struct Location_ Location;
    int Height,Width;
};

struct PictureBox ball_picture_box_list[20], bonus_picture_box_list[20];
int ball_picture_box_list_Count = 0, bonus_picture_box_list_Count = 0;
int SocketFD = 0;
int stepX = 10, stepY = 10, initial_size = 44;
int fdmax = 0;
int newfd = 0;
int nbytes = 0;

void del_list(int client){
	int del_idx = -1;
	for(int i=0;i<ball_picture_box_list_Count;i++){
		if(ball_picture_box_list[i].sockname == client){
			del_idx = i;
			break;
		}
	}
	if(del_idx>-1){
		for(int i=del_idx;i<ball_picture_box_list_Count - 1;i++){
			ball_picture_box_list[i] = ball_picture_box_list[i + 1];
		}
		struct PictureBox temp;
		ball_picture_box_list[ball_picture_box_list_Count] =  temp;
		ball_picture_box_list_Count--;
	}
}

void move_ball(char active_ball[], char active_move[], int active_idx)
{
    if (active_move == "L")
    {
        ball_picture_box_list[active_idx].Location.Y = ball_picture_box_list[active_idx].Location.Y;
        ball_picture_box_list[active_idx].Location.X = ball_picture_box_list[active_idx].Location.X - stepX;
    }
    if (active_move == "R")
    {
        ball_picture_box_list[active_idx].Location.Y = ball_picture_box_list[active_idx].Location.Y;
        ball_picture_box_list[active_idx].Location.X = ball_picture_box_list[active_idx].Location.X;
    }
    if (active_move == "U")
    {
        ball_picture_box_list[active_idx].Location.Y = ball_picture_box_list[active_idx].Location.Y - stepY;
        ball_picture_box_list[active_idx].Location.X = ball_picture_box_list[active_idx].Location.X;
    }
    if (active_move == "D")
    {
        ball_picture_box_list[active_idx].Location.X = ball_picture_box_list[active_idx].Location.X;
        ball_picture_box_list[active_idx].Location.Y = ball_picture_box_list[active_idx].Location.Y + stepY;
    }
}


void get_move_ball(char str[])
{
    bool check_active = false;
    char ball_name[20], active_name[20];
    int ball_name_idx=0, active_name_idx=0;
    for (int i = 1; i < sizeof str; i++)
    {
        if (str[i] == '_')
        {
            check_active = true;
        }
        else if (!check_active)
        {
            ball_name[ball_name_idx] = str[i];
            ball_name_idx++;
        }
        else if (check_active)
        {
            active_name[active_name_idx] = str[i];
            active_name_idx++;
        }
        if(str[i] == 0)
        {
            break;
        }
    }
    int active_idx = -1;

    for (int i=0; i < ball_picture_box_list_Count; i++)
    {
        if(ball_picture_box_list[i].Name == ball_name)
        {
            active_idx = i;
        }
        if(ball_picture_box_list[i].Name == 0)
        {
            break;
        }
    }

    if(active_idx > -1)
    {
        move_ball(ball_name, active_name, active_idx);
    }

    return;
}

void Generate_ball(char str[], int client)
{
    char ball_name[20];
    for(int i = 1; i < sizeof str; i++)
    {
        ball_name[i - 1] = str[i];
        if(str[i] == 0)
        {
            break;
        }
    }

    ball_picture_box_list[ball_picture_box_list_Count].Name = ball_name;
    ball_picture_box_list[ball_picture_box_list_Count].Visible = true;
    srand(time(NULL));
    int x = rand()%1415;
    int y = rand()%646;
    ball_picture_box_list[ball_picture_box_list_Count].Location.X = x;
    ball_picture_box_list[ball_picture_box_list_Count].Location.Y = y;
    ball_picture_box_list[ball_picture_box_list_Count].Height = initial_size;
    ball_picture_box_list[ball_picture_box_list_Count].Width = initial_size;
    ball_picture_box_list[ball_picture_box_list_Count].sockname = client;
    ball_picture_box_list_Count++;
    return;
}
void Receive_Msg(int client, char str[])
{
    while (true) {
        if(str[0] == '0')
        {
            bool check_name = false;
            for(int i = 0; i < ball_picture_box_list_Count; i++)
            {
                if(str == ball_picture_box_list[i].Name)
                {
                    char Msg[] = "###";
                    send(client, Msg, sizeof(Msg), 0);
                    check_name = true;
                    break;
                }
            }
            //建立ball
            if (check_name == false){
                Generate_ball(str, client);
            }

        }
        if(str[0] == '1')
        {
            //取得移動指令
            get_move_ball(str);
        }


    }
    return;
}

void collision()
{
    //palyer and bonus
    for (int i = 0; i < ball_picture_box_list_Count; i++)
    {
        if (ball_picture_box_list[i].Visible == true)
        {
            int x1 = ball_picture_box_list[i].Location.X + ball_picture_box_list[i].Width / 2, y1 = ball_picture_box_list[i].Location.Y + ball_picture_box_list[i].Height / 2, r1 = ball_picture_box_list[i].Height / 2;
            for (int j = 0; j < bonus_picture_box_list_Count; j++)
            {
                if (bonus_picture_box_list[j].Visible == true)
                {
                    int x2 = bonus_picture_box_list[j].Location.X + bonus_picture_box_list[j].Width / 2, y2 = bonus_picture_box_list[j].Location.Y + bonus_picture_box_list[j].Height / 2, r2 = bonus_picture_box_list[j].Height / 2;
                    if (sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) <= r1 + r2)
                    {
                        ball_picture_box_list[i].Height += 10;
                        ball_picture_box_list[i].Width += 10;
                        bonus_picture_box_list[j].Visible = false;
                    }
                }
            }
        }
    }

    //player and player
    for (int i = 0; i < ball_picture_box_list_Count; i++)
    {
        if (ball_picture_box_list[i].Visible == true)
        {
            int x1 = ball_picture_box_list[i].Location.X + ball_picture_box_list[i].Width / 2, y1 = ball_picture_box_list[i].Location.Y + ball_picture_box_list[i].Height / 2, r1 = ball_picture_box_list[i].Height / 2;
            for (int j = 0; j < ball_picture_box_list_Count; j++)
            {
                if (i != j && ball_picture_box_list[j].Visible == true)
                {
                    int x2 = ball_picture_box_list[j].Location.X + ball_picture_box_list[j].Width / 2, y2 = ball_picture_box_list[j].Location.Y + ball_picture_box_list[j].Height / 2, r2 = ball_picture_box_list[j].Height / 2;
                    if (sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) <= r1 + r2)
                    {
                        if (r1 >= r2)
                        {
                            ball_picture_box_list[i].Width += ball_picture_box_list[j].Width / 2;
                            ball_picture_box_list[i].Height += ball_picture_box_list[j].Height / 2;
                            ball_picture_box_list[j].Height = initial_size;
                            ball_picture_box_list[j].Width = initial_size;
                            srand(time(NULL) + j);
                            int x = rand()%1415;
                            int y = rand()%646;
                            ball_picture_box_list[j].Location.X = x;
                            ball_picture_box_list[j].Location.Y = y;

                        }
                        if (r1 < r2)
                        {
                            ball_picture_box_list[j].Width += ball_picture_box_list[i].Width / 2;
                            ball_picture_box_list[j].Height += ball_picture_box_list[i].Height / 2;
                            ball_picture_box_list[i].Width = initial_size;
                            ball_picture_box_list[i].Height = initial_size;
                            srand(time(NULL) + j);
                            int x = rand()%1415;
                            int y = rand()%646;
                            ball_picture_box_list[i].Location.X = x;
                            ball_picture_box_list[i].Location.Y = y;
                        }
                    }
                }
            }
        }
    }
}

void Send_Ball_pos()
{
    char str[1024];
    memset(str, '\0', 1024);
    strcat(str, '@');
    for(int j = 0; j < ball_picture_box_list_Count; j++)
    {
        char visible_ = '0';
        if (ball_picture_box_list[j].Visible)
        {
            visible_ = '1';
        }

        strcat(str, ball_picture_box_list[j].Name + '_' + ball_picture_box_list[j].Location.X + '_' + ball_picture_box_list[j].Location.Y + '_' + ball_picture_box_list[j].Height + '_' + visible_  + '|');
    }

    //bonus
    for(int j = 0; j < bonus_picture_box_list_Count; j++)
    {
        char visible_ = '0';
        if (bonus_picture_box_list[j].Visible)
        {
            visible_ = '1';
        }

        strcat(str, bonus_picture_box_list[j].Name + '_' + bonus_picture_box_list[j].Location.X + '_' + bonus_picture_box_list[j].Location.Y + '_' + bonus_picture_box_list[j].Height + '_' + visible_ + '|');
    }
    for(int i = 0; i <= fdmax; i++)
    {
        send(i, str, 256, 0);
    }
}
void erase_ball(){
    int del_num=0;
    for(int i=0;i<bonus_picture_box_list_Count;i++){
        if(bonus_picture_box_list[i].Visible == false){
            del_num++;
        }
    }
    if(del_num > 0){
        for(int i=0;i<del_num;i++){
            int del_idx = -1;
            for(int j=0;j<bonus_picture_box_list_Count;j++){
                if(bonus_picture_box_list[j].Visible == false){
                    del_idx = j;
                    break;
                }
            }
            if(del_idx>-1){
                for(int j=del_idx;j<ball_picture_box_list_Count - 1;j++){
                    ball_picture_box_list[j] = ball_picture_box_list[j + 1];
                }
                struct PictureBox temp;
                bonus_picture_box_list[bonus_picture_box_list_Count] =  temp;
                bonus_picture_box_list_Count--;
            }
        }
    }
}

void * timer_clock(void *arg)
{
    clock_t start_t,finish_t, start_t2;
    double total_t = 0,total_t2 = 0;
    start_t = clock();
    start_t2 = clock();
    while(true){
        finish_t = clock();
        total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;
        total_t2 = (double)(finish_t - start_t2) / CLOCKS_PER_SEC;
        if(total_t2 >= 0.03){
        	srand(time(NULL));
        	int intRandom = rand()%11;
        	if(intRandom > 5 && bonus_picture_box_list_Count <= 5){
        		for(int i =0;i < intRandom;i++){
        			srand(time(NULL) + 1);
        			int x = rand()%1415;
                    int y = rand()%646;
                    struct PictureBox bonus;
                    bonus.Name = "bonus";
                    bonus.Visible = true;
                    bonus.Location.X = x;
                    bonus.Location.Y = y;
                    bonus.sockname = 1234;
                    bonus.Height = initial_size;
                    bonus.Width = initial_size;
					bonus_picture_box_list[bonus_picture_box_list_Count] = bonus;
					bonus_picture_box_list_Count++;
				}
				start_t2 = clock();
			}
		}
        if(total_t >= 0.01)
        {
            if(ball_picture_box_list_Count > 0)
            {
                collision();
            }
            Send_Ball_pos();
            erase_ball();
            start_t = clock();
        }
    }
}

int main(int argc, char *argv[]) {
    //set TCP
    struct sockaddr_in stSockAddr;
    SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (SocketFD == -1){
        printf("Fail to create a socket.");
    }
    else{
        printf("Create a socket");
    }
    //bind socket
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(5566);
    stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(stSockAddr));

    //thread timer
    pthread_t timer_clock_;
    pthread_create(&timer_clock_, NULL, timer_clock, NULL);

    //listen
    socklen_t addrlen;
    struct sockaddr_storage remoteaddr;
    char remoteIP[INET6_ADDRSTRLEN];
    fd_set master;
    fd_set read_fds;
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    listen(SocketFD, 10);
    FD_SET(SocketFD, &master);
    fdmax = SocketFD;
    while (true)
    {
        read_fds = master; // 複製 master
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }
        for(int i = 0; i <= fdmax; i++) {
            char buf[1024];
            if (FD_ISSET(i, &read_fds)) { // 我們找到一個！！
                if (i == SocketFD) {
                // handle new connections
                addrlen = sizeof remoteaddr;
                newfd = accept(SocketFD,(struct sockaddr *)&remoteaddr,&addrlen);
                if (newfd == -1) {
                    perror("accept");
                }
                else
                {
                    FD_SET(newfd, &master); // 新增到 master set
                    if (newfd > fdmax) { // 持續追蹤最大的 fd
                        fdmax = newfd;
                    }

                }
                }
                else
                {
                    // 處理來自 client 的資料
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0)
                    {

                        if (nbytes == 0) {
                        // 關閉連線
                            printf("selectserver: socket %d hung up\n", i);
                        }
                        else {
                            perror("recv");
                        }
                        close(i); // bye!
                        del_list(i);
                        FD_CLR(i, &master); // 從 master set 中移除

                    }
                    else
                    {
                        Receive_Msg(i, buf);
                    }
                }
            }
        }
    }
    pthread_cancel(timer_clock_);
	return 0;
}

