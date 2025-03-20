#include "main.h"

#define usart2_baund  57600

SysPara AS608Para;	// 指纹模块AS608参数
u16 ValidN;			// 模块内有效指纹个数
u8** kbd_tbl;
int Error;
u8 ensure;
char str[64];

void Add_FR(void);
int press_FR(void);

void Peripherals_Init(void)
{
	// 设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	OLED_Init();
	Delay_init();
	PS_StaGPIO_Init();
	usart2_init(usart2_baund);
}


int main(void)
{
	Peripherals_Init();
	
	OLED_Clear();
	OLED_ShowString(0, 0, "Connecting to AS608", OLED_8X16);	// 连接指纹模块
	OLED_Update();
	Delay_ms(400);
	while(PS_HandShake(&AS608Addr))								// 与AS608模块握手
	{
		Delay_ms(400);
		OLED_Clear();
		OLED_ShowString(0, 0, "Connect Failed", OLED_8X16);		// 连接失败
		OLED_Update();
		OLED_Clear();
	}
	OLED_Clear();
	
	OLED_ShowString(0, 0, "Connect Success", OLED_8X16);		// 连接成功
	OLED_Update();
	Delay_ms(1500);
	OLED_Clear();
	OLED_ShowNum(0,0,usart2_baund,6, OLED_8X16);
	OLED_ShowHexNum(0,20,AS608Addr,16 ,OLED_8X16);
	OLED_ShowString(0,40,str, OLED_8X16);
	OLED_Update();
	
	ensure=PS_ValidTempleteNum(&ValidN);						// 读库指纹个数
	if(ensure!=0x00)
	{
		OLED_Clear();
	}
	ensure=PS_ReadSysPara(&AS608Para);							// 读参数
	OLED_ShowString(25,4,str, OLED_8X16);
	OLED_Update();
	
	while (1)
	{
		OLED_Clear(); 
		while(1)
		{
			OLED_ShowString(0, 0, "Unlock Failed", OLED_8X16);	// 未开锁	
			OLED_Clear();			
			// 指纹解锁
			if(PS_Sta)											// 检测PS_Sta状态，如果有手指按下
			{
				while(PS_Sta)
				{
					Error=press_FR();							// 刷指纹
					if(Error==0)
					{
						
					}								
					else
					{
						// 未开锁
						OLED_ShowString(0, 0, "Unlock Failed", OLED_8X16);
						OLED_Update();
					}
				}
			}
		}
	}
}


// 录指纹
void Add_FR(void)
{
	Delay_ms(500);
	u8 i,ensure ,processnum=0;
	int ID=2;
	OLED_Clear();					// 清屏
	while(1)
	{
		switch (processnum)
		{
			case 0:
				OLED_Clear();		// 清屏
				i++;
                OLED_ShowString(0, 0, "Recording fingerprint", OLED_8X16);		// 录入指纹
				OLED_ShowString(0, 20, "Recording fingerprint", OLED_8X16);		// 请按指纹
				OLED_Update();
                Delay_ms(6000);
                OLED_Clear();		// 清屏	
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
                    Delay_ms(500);
					ensure=PS_GenChar(CharBuffer1);								// 生成特征
					if(ensure==0x00)
					{
						OLED_ShowString(0, 20, "Fingerprint OK", OLED_8X16);	// 指纹正常
						OLED_Update();
                        Delay_ms(5000);	
						i=0;
						processnum=1;											// 跳到第二步						
					}
					else
					{
						OLED_ShowString(0, 20, "Error", OLED_8X16);				// 错误
						OLED_Update();
                    }				
				}
				else
				{
					OLED_ShowString(0, 20, "Error", OLED_8X16);					// 错误
					OLED_Update();
				}
				OLED_Clear();													// 清屏
				break;
			case 1:
				i++;
                OLED_ShowString(0, 20, "Please press again", OLED_8X16);		// 请再按一次指纹
				OLED_Update();
                Delay_ms(5000);			
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
                    Delay_ms(500);
					ensure=PS_GenChar(CharBuffer2);								// 生成特征
					if(ensure==0x00)
					{
						OLED_ShowString(0, 20, "Fingerprint OK", OLED_8X16);	// 指纹正常
						OLED_Update();
                        Delay_ms(5000);	
						i=0;
						processnum=2;	// 跳到第三步
					}
					else
					{
                        OLED_ShowString(0, 20, "Error", OLED_8X16);				// 错误
						OLED_Update();
                    }	
				}
				else
				{
					OLED_ShowString(0, 20, "Error", OLED_8X16);					// 错误
					OLED_Update();
				}	
				OLED_Clear();//清屏
				break;
			case 2:		
				OLED_ShowString(0, 20, "Checking fingerprint", OLED_8X16);		// 对比指纹
				OLED_Update();
				Delay_ms(5000);	
				ensure=PS_Match();
				if(ensure==0x00) 
				{
					OLED_ShowString(0, 20, "fingerprint correct", OLED_8X16);	// 指纹一样
					OLED_Update();
					Delay_ms(5000);	
					processnum=3;		// 跳到第四步
				}
				else 
				{   
					OLED_ShowString(0, 20, "fingerprint error", OLED_8X16);		// 指纹错误
					OLED_Update();
					Delay_ms(5000);	
                    OLED_Clear();		// 清屏
					OLED_ShowString(0, 20, "Error", OLED_8X16);					// 错误
					OLED_Update();
					i=0;
                    Delay_ms(3000);
					OLED_Clear();		// 清屏
					processnum=0;		// 跳回第一步		
				}
				Delay_ms(1200);
				OLED_Clear();			// 清屏
				break;
			case 3:
			    OLED_ShowString(0, 20, "Generating template", OLED_8X16);		// 生成模板
				OLED_Update();
                Delay_ms(5000);		
				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
					OLED_ShowString(0, 20, "Template correct", OLED_8X16);		// 模板成功
					OLED_Update();
					Delay_ms(5000);	
					processnum=4;		// 跳到第五步
				}
				else
				{
					processnum=0;
					OLED_ShowString(5,4,"shengchengcuowu==", OLED_8X16);
					OLED_Update();
				}
				Delay_ms(5200);
				break;
			case 4:
				OLED_Clear();			// 清屏
				Delay_ms(1000);	
				ensure=PS_StoreChar(CharBuffer2,ID);							// 储存模板
				if(ensure==0x00) 
				{
					OLED_Clear();		// 清屏
					OLED_ShowString(0, 20, "Record Success", OLED_8X16);		// 录指纹成功	
					OLED_ShowNum(0,20,ID,3, OLED_8X16);
					OLED_Update();
					ID++;
					Delay_ms(5000);	
					PS_ValidTempleteNum(&ValidN);								// 读库指纹个数
					OLED_ShowString(0,40,"shengyu==", OLED_8X16);
					OLED_Update();
         
					OLED_ShowNum(30,10,AS608Para.PS_max-ValidN,3, OLED_8X16);
					OLED_Update();
					Delay_ms(5000);	
					OLED_Clear();
					return ;
				}
				else 
				{
					processnum=0;
                    OLED_ShowString(0,0,"Error", OLED_8X16);					// 错误
					OLED_Update();
				}
				Delay_ms(3500);
				OLED_Clear();													// 清屏					
				break;				
		}
		Delay_ms(400);
		if(i==10)																// 超过5次没有按手指则退出
		{
			OLED_Clear();
			break;
		}
	}
}


// 刷指纹
int press_FR(void)
{
	SearchResult seach;
	u8 ensure;
	char str[256];
	ensure=PS_GetImage();
	OLED_Clear();
	OLED_ShowString(0,0,"Checking fingerprint", OLED_8X16);				// 检测指纹中
	OLED_Update();
	if(ensure==0x00)													// 获取图像成功 
	{
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00)		// 生成特征成功
		{
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)	// 搜索成功
			{				
				OLED_Clear();
                OLED_ShowString(0,0,"fingerprint Correct, Unlock", OLED_8X16);	// 指纹正确开锁
				OLED_Update();
				sprintf(str,"ID:%d     match",seach.pageID);
				sprintf(str,":%d",seach.mathscore);
				Delay_ms(3800);
				OLED_Clear();
				return 0;
			}
			else {
				OLED_ShowString(0,0,"fingerprint error", OLED_8X16);			// 指纹错误	
				OLED_ShowString(0,20,"fingerprint error", OLED_8X16);			// 错误
				OLED_Update();
				Delay_ms(3000);
				OLED_Clear();
				return -1;
			}				
		}
		else
		{
			OLED_ShowString(0,0,"Error", OLED_8X16);							// 错误
			OLED_Update();
			Delay_ms(2000);
			OLED_Clear();
		}
	}
	return -1;	
}
