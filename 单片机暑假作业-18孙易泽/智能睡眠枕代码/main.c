/*******************说明:**************************
智能睡眠枕代码 18数据科学孙易泽
**************************************************/

#include <reg52.h>
#include <intrins.h>
#define uchar unsigned char
#define uint  unsigned int
uchar x,y,z,n,m,a=0;
//压力传感器变量
uint num=0x1F;
uint c=0x1F;
uint b,d;
//睡眠记录数组
uint e,f,g,h;
uint deep=0;
uchar condition[60]={0};
//闹钟时间变量设置
int time_hour_1=0;
int time_hour_2=7;
int time_min_1=5;
int time_min_2=0;
//蜂鸣器延时变量
uint time;
//LCD1602引脚定义
sbit RS=P3^5;
sbit RW=P3^6;
sbit CS=P3^7;
#define LCDDATA P0
//DS1302引脚定义
sbit RST=P1^5;
sbit IO=P1^6;
sbit SCK=P1^7;
//4个独立按键的引脚定义
sbit key1=P2^7;
sbit key2=P2^6;
sbit key3=P2^5;
sbit key4=P2^4;
//蜂鸣器引脚定义
sbit buzz=P2^0;
//压力传感器（fsr）引脚定义
sbit f1=P1^0;
sbit f2=P1^1;
sbit f3=P1^2;
sbit f4=P1^3;
sbit f5=P1^4;
//DS1302地址定义
#define ds1302_sec_add			0x80		//秒数据地址
#define ds1302_min_add			0x82		//分数据地址
#define ds1302_hr_add			0x84		//时数据地址
#define ds1302_date_add			0x86		//日数据地址
#define ds1302_month_add		0x88		//月数据地址
#define ds1302_day_add			0x8a		//星期数据地址
#define ds1302_year_add			0x8c		//年数据地址
#define ds1302_control_add		0x8e		//控制数据地址
#define ds1302_charger_add		0x90 					 
#define ds1302_clkburst_add		0xbe
//时间数组定义
uchar time_buf[8] = {0};
uchar dis_time_buf[16]={0};

/*****************
延时函数
*****************/

//1ms延时
void Delay_xms(uint x)
{
  uint i,j;
  for(i=0;i<x;i++)
    for(j=0;j<112;j++);
}
//12us延时
void Delay_xus(uint t)	  		 		
{ 
  for(;t>0;t--)
   {
	 _nop_();
   }
}

/*****************
LCD设置
*****************/

//控制LCD写时序
void LCD_en_write(void)       
{
   CS=1;    
    Delay_xus(20);
   CS=0;   
	Delay_xus(20);
} 
//写指令函数
void Write_Instruction(uchar command)
{
  RS=0;
  RW=0;
  CS=1;
  LCDDATA=command;
  LCD_en_write();//写入指令数据
}
//写数据函数
void Write_Data(uchar Wdata)
{
  RS=1;
  RW=0;
  CS=1;
  LCDDATA=Wdata;
  LCD_en_write();//写入数据
}
//字符显示初始地址设置
void LCD_SET_XY(uchar X,uchar Y)
{
  uchar address;
  if(Y==0)
    address=0x80+X;//Y=0,表示在第一行显示，地址基数为0x80
  else 
    address=0xc0+X;//Y非0时，表时在第二行显示，地址基数为0xC0
  Write_Instruction(address);//写指令，设置显示初始地址
}
//在第X行Y列开始显示Wdata所对应的单个字符
void LCD_write_char(uchar X,uchar Y,uchar Wdata)
{
  LCD_SET_XY(X,Y);//写地址
  Write_Data(Wdata);//写入当前字符并显示
}
//LCD清屏函数
void LCD_clear(void)
{
  Write_Instruction(0x01);
  Delay_xms(5);
}
//显示屏初始化函数
void LCD_init(void) 
{	
	Write_Instruction(0x38);
	Delay_xms(5);
	Write_Instruction(0x38);	
	Delay_xms(5);
	Write_Instruction(0x38);	

	Write_Instruction(0x08);	//关显示，不显光标，光标不闪烁
	Write_Instruction(0x01);	//清屏
	Delay_xms(5);
	
	Write_Instruction(0x04);	//写一字符，整屏显示不移动
	Delay_xms(5);
	Write_Instruction(0x0C);	//开显示，光标、闪烁都关闭
}
//闹钟设置界面时的十位
void LCD_hour_write_1(void)
{
	if(time_hour_1<0)
	{
		time_hour_1+=3;
	}
	if(0<=time_hour_1<3)
	{
		LCD_write_char(11,1,time_hour_1+'0');
	}
	if(time_hour_1>=3)
	{
		time_hour_1-=3;
		LCD_write_char(11,1,'0');
	}
}
//闹钟设置界面时的个位
void LCD_hour_write_2(void)
{
	if(time_hour_2<0)
	{
		if(time_hour_1==0)
		{
			time_hour_1=2;
			time_hour_2=3;
		}
		else
		{	
			time_hour_2+=10;
			time_hour_1--;
		}
	}
	if(0<=time_hour_2<10)
	{
		if(time_hour_1==2)
		{
			if(time_hour_2==4)
			{
				time_hour_1=0;
				time_hour_2=0;
				LCD_write_char(12,1,time_hour_2+'0');
			}
			else
			{
				LCD_write_char(12,1,time_hour_2+'0');
			}
		}
		else
		{
			LCD_write_char(12,1,time_hour_2+'0');
		}
	}
	if(time_hour_2>=10)
	{
		time_hour_2-=10;
		time_hour_1++;
		LCD_write_char(12,1,'0');
	}
}

//闹钟设置界面分钟的十位
void LCD_min_write_1(void)
{
	if(time_min_1<0)
	{
		time_min_1+=6;
	}
	if(0<=time_min_1<6)
	{
		LCD_write_char(14,1,time_min_1+'0');
	}
	if(time_min_1>=6)
	{
		time_min_1-=6;
		LCD_write_char(14,1,'0');
	}
}
//闹钟设置界面分钟的个位
void LCD_min_write_2(void)
{
	if(time_min_2<0)
	{
		time_min_2+=10;
	}
	if(0<=time_min_2<10)
	{
		LCD_write_char(15,1,time_min_2+'0');
	}
	if(time_min_2>=10)
	{
		time_min_2-=10;
		LCD_write_char(15,1,'0');
	}
}

/************************
DS1302时钟模块设置代码
************************/

//DS1302初始化函数
void ds1302_init(void) 
{
	RST=0;			//RST脚置低
	SCK=0;			//SCK脚置低
}
//从DS1302读出一字节数据
uchar ds1302_read_byte(uchar addr) {

	uchar i,temp;	
	RST=1;					//启动DS1302总线
	//写入目标地址：addr
	addr = addr | 0x01;    //最低位置高，寄存器0位为0时写，为1时读
	for (i = 0; i < 8; i ++) {
		if (addr & 0x01) {
			IO=1;
			}
		else {
			IO=0;
			}
		SCK=1;
		SCK=0;
		addr = addr >> 1;
		}	
	//输出数据：temp
	for (i = 0; i < 8; i ++) {
		temp = temp >> 1;
		if (IO) {
			temp |= 0x80;
			}
		else {
			temp &= 0x7F;
			}
		SCK=1;
		SCK=0;
		}	
	RST=0;					//停止DS1302总线
	return temp;
}

//从DS302读出时钟数据
void ds1302_read_time(void)  
{
	time_buf[1]=ds1302_read_byte(ds1302_year_add);		//年 
	time_buf[2]=ds1302_read_byte(ds1302_month_add);		//月 
	time_buf[3]=ds1302_read_byte(ds1302_date_add);		//日 
	time_buf[4]=ds1302_read_byte(ds1302_hr_add);		//时 
	time_buf[5]=ds1302_read_byte(ds1302_min_add);		//分 
	time_buf[6]=(ds1302_read_byte(ds1302_sec_add))&0x7f;//秒，屏蔽秒的第7位，避免超出59
	time_buf[7]=ds1302_read_byte(ds1302_day_add);		//周 	
}

/************************
独立按键代码
************************/
//独立按键初始化
void scanstart(void)
{
	key1=1;
	key2=1;
	key3=1;
	key4=1;
}
//按键扫描
void scan(void)
{
	if(key1==0||key2==0||key3==0||key4==0)
	{
		Delay_xms(10);
		if(key1==0||key2==0||key3==0||key4==0)
		{
			if(key1==0)
			{
				x=1;
			}
			if(key2==0)
			{
				y=1;
			}
			if(key3==0)
			{
				z=1;
			}
			if(key4==0)
			{
				n=1;
			}
		}
	}
}

/*********************
fsr压力传感器
*********************/

//压力传感器检测扫描
void fsr()
{
	b=P1&num;
	if(b!=c)
	{
		c=b;
		d++;
	}
}

/*********************
LCD显示界面
*********************/
//默认显示界面
void Display1(void)
{
   
   LCD_write_char(1,0,dis_time_buf[4]+'0');
   LCD_write_char(2,0,dis_time_buf[5]+'0');
   LCD_write_char(3,0,'/');
   LCD_write_char(4,0,dis_time_buf[6]+'0');
   LCD_write_char(5,0,dis_time_buf[7]+'0');
   LCD_write_char(7,0,dis_time_buf[8]+'0');
   LCD_write_char(8,0,dis_time_buf[9]+'0');
	 LCD_write_char(9,0,':');
   LCD_write_char(10,0,dis_time_buf[10]+'0');
   LCD_write_char(11,0,dis_time_buf[11]+'0');
	 LCD_write_char(12,0,':');
	 LCD_write_char(13,0,dis_time_buf[12]+'0');
   LCD_write_char(14,0,dis_time_buf[13]+'0');
   //第2行显示  
   LCD_write_char(0,1,'a');
	 LCD_write_char(1,1,'l');
	 LCD_write_char(2,1,'a');
	 LCD_write_char(3,1,'r');
	 LCD_write_char(4,1,'m');
	 LCD_write_char(5,1,'|');
	 LCD_write_char(10,1,'|');
	 LCD_write_char(11,1,'s');
	 LCD_write_char(12,1,'t');
	 LCD_write_char(13,1,'a');
	 LCD_write_char(14,1,'r');
	 LCD_write_char(15,1,'t');
}
//闹钟设置界面
void Display2(void)
{
	LCD_write_char(0,0,'s');
	LCD_write_char(1,0,'e');
	LCD_write_char(2,0,'t');
	LCD_write_char(3,0,'t');
	LCD_write_char(4,0,'i');
	LCD_write_char(5,0,'n');
	LCD_write_char(6,0,'g');
	LCD_write_char(8,0,'a');
	LCD_write_char(9,0,'l');
	LCD_write_char(10,0,'a');
	LCD_write_char(11,0,'r');
	LCD_write_char(12,0,'m');
	LCD_write_char(13,0,':');
	LCD_write_char(13,1,':');

	LCD_hour_write_2();
	LCD_hour_write_1(); //小时的显示
	LCD_min_write_1(); //分的十位显示
	LCD_min_write_2(); //分的个位显示
	
}
//睡眠界面
void Display3(void)
{
	LCD_write_char(3,0,'g');
	LCD_write_char(4,0,'o');
	LCD_write_char(5,0,'o');
	LCD_write_char(6,0,'d');
	LCD_write_char(8,0,'n');
	LCD_write_char(9,0,'i');
	LCD_write_char(10,0,'g');
	LCD_write_char(11,0,'h');
	LCD_write_char(12,0,'t');
	LCD_write_char(13,0,'!');
	
	LCD_write_char(11,1,'|');
	LCD_write_char(12,1,'s');
	LCD_write_char(13,1,'t');
	LCD_write_char(14,1,'o');
	LCD_write_char(15,1,'p');

}
//闹钟叫醒界面
void Display4(void)
{
	LCD_write_char(2,0,'g');
	LCD_write_char(3,0,'o');
	LCD_write_char(4,0,'o');
	LCD_write_char(5,0,'d');
	LCD_write_char(7,0,'m');
	LCD_write_char(8,0,'o');
	LCD_write_char(9,0,'r');
	LCD_write_char(10,0,'n');
	LCD_write_char(11,0,'i');
	LCD_write_char(12,0,'n');
	LCD_write_char(13,0,'g');
	LCD_write_char(14,0,'!');
	
	LCD_write_char(11,1,'|');
	LCD_write_char(12,1,'s');
	LCD_write_char(13,1,'t');
	LCD_write_char(14,1,'o');
	LCD_write_char(15,1,'p');
}
//睡眠状况查看界面
void Display5(void)
{
	LCD_write_char(0,0,'s');
	LCD_write_char(1,0,'l');
	LCD_write_char(2,0,'e');
	LCD_write_char(3,0,'e');
	LCD_write_char(4,0,'p');
	LCD_write_char(6,0,'h');
	LCD_write_char(7,0,'o');
	LCD_write_char(8,0,'u');
	LCD_write_char(9,0,'r');
	LCD_write_char(10,0,':');
	LCD_write_char(11,0,f/60+'0');
	LCD_write_char(12,0,f/6+'0');
	LCD_write_char(13,0,':');
	LCD_write_char(14,0,f%6+'0');
	LCD_write_char(15,0,'0');
	
	LCD_write_char(0,1,'d');
	LCD_write_char(1,1,'e');
	LCD_write_char(2,1,'e');
	LCD_write_char(3,1,'p');
	LCD_write_char(4,1,':');
	LCD_write_char(5,1,deep/60+'0');
	LCD_write_char(6,1,deep/6+'0');
	LCD_write_char(7,1,':');
	LCD_write_char(8,1,deep%6+'0');
	LCD_write_char(9,1,'0');
}
//蓝牙发送界面
void Display6(void)
{
	LCD_write_char(2,0,'s');
	LCD_write_char(3,0,'e');
	LCD_write_char(4,0,'n');
	LCD_write_char(5,0,'d');
	LCD_write_char(7,0,'m');
	LCD_write_char(8,0,'e');
	LCD_write_char(9,0,'s');
	LCD_write_char(10,0,'s');
	LCD_write_char(11,0,'a');
	LCD_write_char(12,0,'g');
	LCD_write_char(13,0,'e');
}
//蓝牙发送成功界面
void Display7(void)
{	
	LCD_write_char(9,1,'s');
	LCD_write_char(10,1,'u');
	LCD_write_char(11,1,'c');
	LCD_write_char(12,1,'c');
	LCD_write_char(13,1,'e');
	LCD_write_char(14,1,'s');
	LCD_write_char(15,1,'s');
}


/****************
蜂鸣器代码
****************/
void buzzer()
{
		 for(time=0;time<10000;time++);
		 buzz=1;
		 for(time=0;time<10000;time++);
		 buzz=0;
		 for(time=0;time<10000;time++);
		 buzz=1;
		 for(time=0;time<10000;time++);
		 buzz=0;
		 for(time=0;time<10000;time++);
		 buzz=1;
		 for(time=0;time<10000;time++);
		 buzz=0;
		 for(time=0;time<10000;time++);
		 buzz=1;
		 for(time=0;time<10000;time++);
		 buzz=0;
		 for(time=0;time<10000;time++);
		 buzz=1;
		 for(time=0;time<10000;time++);
}	
/******************
蓝牙函数
******************/
//蓝牙数据发送函数
void sendDate(char date)
{
    SBUF=date;		  //接收到的数据放入发送缓存器发送
    while(!TI);       //等待发送数据完成
    TI=0;			  //清除发送完成标志位
}

/******************
定时器和中断
******************/
//定时器0初始化
void Init_timer0()
{
	TMOD|=0X01;//选择为定时器0模式，工作方式1，仅用TR0打开启动。
	TH0=0X00;	//给定时器赋初值，定时1ms
	TL0=0X00;	
	TR0=1;//打开定时器	
	ET0=1;//打开定时器0中断允许
	EA=1;//打开总中断
}
//定时器1初始化
void Init_timer1()  
{
    TMOD=0x20;      //设置计数器1的工作方式2
    TH1=0xfd;	    //设置计数器1的初值，决定波特率
    TL1=0xfd;		//设置计数器1的初值，决定波特率
    PCON=0x00;      // 波特率倍增0x00不加倍	 0x80加倍
    SCON=0x50;		//设置工作方式1 开启接受允许
    EA=1;		    //开启总中断
    ES=1;			//开启串口接受中断
    TR1=1;			//计数器1开始运行
}
//定时器2初始化
void Init_timer2(void)
{
 RCAP2H=0x3c;//赋T2初始值0x3cb0，溢出20次为1秒,每次溢出时间为50ms
 RCAP2L=0xb0;
 TR2=1;	     //启动定时器2
 ET2=1;		 //打开定时器2中断
 EA=1;		 //打开总中断
}
//定时器0中断函数
void Timer0() interrupt 1
{
	TH0=0X00;	//给定时器赋初值，定时1ms
	TL0=0X00;
	if(g==1)
	{
		e++;
		if(e==8435)
		{
			if(d<4)
			{
				deep++;
			}
			e=0;
			condition[f]=d;
			f++;
			d=0;
		}
	}

}	
//蓝牙中断
void Uart() interrupt 4
{
		uchar date;
    date=SBUF;        //取出接受到的数据
    RI=0;			  //清除接受中断标志位
}

//定时器2中断函数
void Timer2() interrupt 5	  //定时器2是5号中断
{
 static uchar t;
 TF2=0;
 t++;
 if(t==4)               //间隔200ms(50ms*4)读取一次时间
  {
   t=0;
   ds1302_read_time();  //读取时间 
   dis_time_buf[0]=(time_buf[0]>>4); //年   
   dis_time_buf[1]=(time_buf[0]&0x0f);
   
   dis_time_buf[2]=(time_buf[1]>>4);   
   dis_time_buf[3]=(time_buf[1]&0x0f);
  
   dis_time_buf[4]=(time_buf[2]>>4); //月  
   dis_time_buf[5]=(time_buf[2]&0x0f);
   
   dis_time_buf[6]=(time_buf[3]>>4); //日   
   dis_time_buf[7]=(time_buf[3]&0x0f);
   
   dis_time_buf[14]=(time_buf[7]&0x07); //星期
   
   //第2行显示  
   dis_time_buf[8]=(time_buf[4]>>4); //时   
   dis_time_buf[9]=(time_buf[4]&0x0f);   

   dis_time_buf[10]=(time_buf[5]>>4); //分   
   dis_time_buf[11]=(time_buf[5]&0x0f);   

   dis_time_buf[12]=(time_buf[6]>>4); //秒   
   dis_time_buf[13]=(time_buf[6]&0x0f);
  }
}

/***************************
智能睡眠枕软件程序
***************************/

//闹钟设置函数
void setalarm() 
{
	Display2();
	while(1)
	{
		scan();
		//闹钟时的设置界面
		if(n==1)
		{			
			Delay_xms(200);
			charlie:
			n=0;
			LCD_clear();
			break;
		}
		if(y==1)
		{
			y=0;
			Delay_xms(200);
			time_hour_2+=1;//小时加1
			LCD_clear();
			Display2();
		}
		if(z==1)
		{
			z=0;
			Delay_xms(200);
			time_hour_2-=1;//小时减1
			LCD_clear();
			Display2();
		}
		if(x==1)
		{
			x=0;
			Delay_xms(200);
			//闹钟分的十位设置界面
			while(1)
			{
				scan();
				if(y==1)
				{
					y=0;
					Delay_xms(200);
					time_min_1+=1;//分的十位加1
					LCD_clear();
					Display2();
				}
				if(z==1)
				{
					z=0;
					Delay_xms(200);
					time_min_1-=1;//分的十位减1
					LCD_clear();
					Display2();
				}
				if(n==1)
				{
					n=0;
					LCD_clear();
					goto charlie;
				}
				if(x==1)
				{
					x=0;
					Delay_xms(200);
					//闹钟分的个位设置界面
					while(1)
					{
						scan();
						if(y==1)
						{
							y=0;
							Delay_xms(200);
							time_min_2+=1;//分的个位加1
							LCD_clear();
							Display2();
						}
						if(z==1)
						{
							z=0;
							Delay_xms(200);
							time_min_2-=1;//分的个位减1
							LCD_clear();
							Display2();
						}
						if(n==1)
						{
							Delay_xms(200);
							n=0;
							LCD_clear();
							goto charlie;
						}
						if(x==1)
						{
							Delay_xms(200);
							x=0;
							m=1;
							LCD_clear();
							Display3();
							goto charlie;
						}
					}
				}
			}
		}
	}
}
//睡眠模式
void sleepmode()
{
	m=0;
	LCD_clear();
	Display3();
	Delay_xms(200);
	while(1)
	{
		g=1;
		scan();
		fsr();
		if(n==1)
		{
			begin:
			n=0;
			LCD_clear();
			buzz=1;
			Delay_xms(200);
			g=0;
			break;
		}
		//到达闹钟设置时间，开始叫醒
		if(time_hour_1==dis_time_buf[8]&&time_hour_2==dis_time_buf[9]&&time_min_1==dis_time_buf[10]&&time_min_2==dis_time_buf[11])
		{
			while(1)
			{
				scan();
				fsr();
				//检测到压力传感器有压力
				if(c!=num)
				{
					LCD_clear();
					Display4();
					Delay_xms(200);
					buzzer();
				}
				//检测到压力传感器无压力
				if(c==num)
				{
					LCD_clear();
					g=0;
					goto begin;
				}
				if(n==1)
				{
					n=0;
					LCD_clear();
					buzz=1;
					Delay_xms(200);
					g=0;
					goto begin;
				}
			}
		}
	}
}
//蓝牙数据发送
void bluetooth()
{
	Display6();
	while(1)
	{
		scan();
		if(n==1)
		{
			n=0;
			LCD_clear();
			Delay_xms(200);
			break;
		}
		if(z==1)
		{
		  z=0;
		  for(h=0;h<60;h++)
		  {
				sendDate(condition[h]+'0');
			}
			Delay_xms(200);
			Display7();
		}
	}
}
//睡眠状况查看
void sleepcondition()
{
	Display5();
	while(1)
	{
		scan();
		if(n==1)
		{
		  n=0;
			LCD_clear();
			Delay_xms(200);
			break;
		}
	}
}

//主函数
void main(void)
{
	Delay_xms(50);//等待系统稳定
	LCD_init();   //LCD初始化
	LCD_clear();  //清屏   
	ds1302_init();  //DS1302初始化
	Delay_xms(10); //延时
	Init_timer0(); //定时器0初始化
	Init_timer1(); //定时器1初始化
	Init_timer2(); //定时器2初始化 
	scanstart(); //按键初始化
	while(1)
	{
		Display1(); //初始界面
		scan(); //按键扫描
		if(m==1)
	  {
		 sleepmode(); //m=1时进入睡眠模式
		}
		if(x==1) //进入闹钟设置界面
		{
			x=0;
			Delay_xms(200);
			LCD_clear();
			setalarm();		 
		}
		if(y==1) //进入睡眠记录界面
		{
			y=0;
			Delay_xms(200);
			LCD_clear();
			sleepcondition();	 
		}
		if(z==1) //进入蓝牙发送界面
		{
			z=0;
			Delay_xms(200);
			LCD_clear();
			bluetooth();		
		}
		if(n==1) //进入睡眠模式
		{
			n=0;
			m=1; 
			Delay_xms(200);
			LCD_clear();
		}	
	}
}