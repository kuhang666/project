#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>//串口编程头文件
#include <string.h>
#include </home/hang/3rdlib/include/MQTTAsync.h>//mqtt编译头文件
#include <pthread.h>//多线程编程头文件
#include <mysql/mysql.h>//mysql编译头文件
#include <sys/socket.h>//tcp需要的头文件
#include <arpa/inet.h>
#include <netinet/in.h>
#define MAXCONN 8


#define  SERVERIP "121.37.14.128"
#define Clientid "hang"

int isConnected=0;
static int fd1;
static char buffer[30];


//mqtt订阅消息回调函数，接收来自客户端的消息
int onMessrecv(void* context,char*topicName,int topicLen,MQTTAsync_message *message){

    
    printf("topic:%s,payload:%s\n",topicName,(char*)message->payload);
    write(fd1,(char*)message->payload,strlen((char*)message->payload));   //订阅主题收到的信息发送至串口 
    printf("%s\n",(char*)message->payload);

    MQTTAsync_free(topicName);//释放空间
    MQTTAsync_free(message);//释放空间
    return 1;         
}
void onConnect(void *context,MQTTAsync_successData* response)
{
    MQTTAsync client =(MQTTAsync)context;
    int ret;
    MQTTAsync_responseOptions response_opt=MQTTAsync_responseOptions_initializer;
    printf("Succeed in connecting to mqtt-server!\n");
    ret=MQTTAsync_subscribe(client,"deng",1,&response_opt);//订阅主题消息
    if(ret!=MQTTASYNC_SUCCESS){
        printf("fail to sub!\n");
    }
    isConnected=1;
}
void disConnect(void *context,MQTTAsync_failureData* response)
{
    printf("Failed to connect  mqtt-server!\n");
}

void mqtt_port(void)
{	
	
    MQTTAsync client;
    int ret;
    MQTTAsync_connectOptions conn_opt = MQTTAsync_connectOptions_initializer;//初始化连接选项
    ret=MQTTAsync_create(&client,SERVERIP,Clientid,MQTTCLIENT_PERSISTENCE_NONE,NULL);
    if(ret!=MQTTASYNC_SUCCESS)
    {
        printf("Cannot create mqtt client!\n");
    }
    
    //初始化接收消息回调
    ret=MQTTAsync_setCallbacks(client,NULL,NULL,onMessrecv,NULL);
    if(ret!=MQTTASYNC_SUCCESS){
        printf("cannnot set call back function!\n");
        
    }
    conn_opt.onSuccess=onConnect;
    conn_opt.onFailure=disConnect;
    conn_opt.automaticReconnect=1;
    conn_opt.context=client;
    conn_opt.cleansession=0;
    conn_opt.username="hang";//设置用户名密码
    conn_opt.password="hang";
    conn_opt.cleansession=0;
    ret=MQTTAsync_connect(client,&conn_opt);
    //因为是异步的，当MQTTAsync_connect返回的时候只是代表底层代码对参数进行了检查
    //当正确返回时，表示底层代码接收了该connect连接命令
    if(ret!=MQTTASYNC_SUCCESS)
    {
        printf("Cannot start a mqttt server connect!\n");
        
    }

    
}   





int main()
{   
    //mqtt_port();//线程处理下行数据处理
    pthread_t tid;
    int ret1;
    ret1=pthread_create(&tid,NULL,(void*)mqtt_port,NULL);
    if (ret1!=0)
    {
       printf("create pthread error!\n");
    }
    




    //主函数处理上行数据处理
    //串口配置
	//××××××××××××××××××××××××××××××××××××××××××××
	
	int count=0;
	
	struct termios uart_cfg;//声明串口配置结构
	fd1=open("/dev/ttyS1",O_RDWR|O_NONBLOCK|O_NOCTTY);
	if(fd1<0)
	{
		perror("Failed to open serial:");
	}
    fcntl(fd1,F_SETFL,0);
	cfmakeraw(&uart_cfg);//设置串口为原始模式
	cfsetspeed(&uart_cfg,B115200);	//设置波特率
    uart_cfg.c_cflag|=CLOCAL|CREAD;//本地连接和接收使能
	tcflush(fd1,TCIOFLUSH);//处理目前串口缓冲中的数据，清除缓冲区数据
	tcsetattr(fd1,TCSANOW,&uart_cfg);//激活配置



	//数据库配置
	MYSQL com_mysql;//声明一个数据库连接句柄
    char sqlcommand[100];
    MYSQL_RES *pRes;
    MYSQL_ROW hs;
    if(mysql_init(&com_mysql)==NULL)
    {
        printf("Cannot init mysql!\n");
   	}

	if(mysql_real_connect(&com_mysql,"121.37.14.128","root","Aa87468792","django",0,NULL,0)==NULL)
    {
		printf("%s\n",mysql_error(&com_mysql));
   	}


	//tcp服务器
	int listen_fd,comm_fd;
	int ret;
	int i=1;
	struct sockaddr_in server_addr,client_addr;
	int sock_size=sizeof(struct sockaddr_in);
	listen_fd=socket(AF_INET,SOCK_STREAM,0);
	if(listen_fd<0)
	{
		perror("Failed to create socket:");

	}
	bzero(&server_addr,sock_size);
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(1024);
	server_addr.sin_addr.s_addr=INADDR_ANY;
	setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&i,sizeof(int));
	ret=bind(listen_fd,(struct sockaddr*)&server_addr,sock_size);
	if(ret==0)
	{
		printf("Bind Successfully!\n");
	}
	ret=listen(listen_fd,MAXCONN);
	if(ret==0)
	{
		printf("Listen Successfully!\n");
	}
	comm_fd=accept(listen_fd,(struct sockaddr*)&client_addr,&sock_size);
	
	char ipaddr[16];
	inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,ipaddr,16);
		

	while(1)
	{
		//读串口
		count=read(fd1,buffer,99);
		if(count>0)
		{
			buffer[count]=0;
			printf("%s\r\n",buffer);
            
		}
	
	 	//tcpserver发送串口数据
	 	send(comm_fd,(char*)buffer,strlen(buffer),0);

		// //处理串口数据，并发送至mysql
		char *delim = ",";
		char *p;
		char *data[6];
		//text
		p=strtok(buffer, delim);
		//printf("%s\n", p);
		data[0]=p;
		//deng
		p=strtok(NULL, delim);
		//printf("%s\n", p);
		data[1]=p;
		//feng
		p=strtok(NULL, delim);
		//printf("%s\n", p);
		data[2]=p;
		//ren
		p=strtok(NULL, delim);
		//printf("%s\n", p);
		data[3]=p;
		sprintf(sqlcommand,"insert into keshe_note(text,deng,ren,feng) values('%s',%s,%s,%s)",data[0],data[1],data[2],data[3]);
		printf("sqlcommand:%s\n",sqlcommand);
		if(mysql_query(&com_mysql,sqlcommand)!=0)
		{
			printf("%s\n",mysql_error(&com_mysql));
        	break;
		}

	}
	mysql_close(&com_mysql);
	close(fd1);






}



