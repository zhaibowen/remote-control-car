#include "stm32f10x.h"                  // Device header
#include "usart.h"
#include "Delay.h"
#include "NRF24L01.h"
#include "Servo.h"

uint8_t Buf[32] = {0};
uint16_t Lx, Ly, Rx, Ry;
uint8_t Key0, Key1, Key2, Key3, Key4, Key5; // X, 低速, 方向左调, 方向右调, 高速, X
uint8_t Key1_pre = 2, Key2_pre = 2, Key3_pre = 2, Key4_pre = 2; // 松开设置
uint16_t knt = 0; // 信号检测

void remote_control_run(void);

int main(void)
{
	uint16_t Speed = 200; // 默认低速
	uint16_t Stop = 1500, Back = 1100; // 停止与后退
	uint16_t Mid = 90, Angle = 24;
	uint8_t debug_flag = 0;
	
	NRF24L01_Init();
	uart_init(115200);
	Servo_Init();
	Servo_SetAngle(Mid);
	Motor_SetPWM(Stop);
	Delay_ms(3000);
	
	while (1)
	{ 
		if (knt >= 50) { // 超过1s没收到信号，停车
			Motor_SetPWM(Stop);
			Servo_SetAngle(Mid);
			knt = 0;
		}
		
		if (NRF24L01_Get_Value_Flag() == 0)
		{
			knt = 0;
			remote_control_run();
			
			if (debug_flag) {
				printf("%d, %d, %d, %d, %d, %d\r\n", Ly, Rx, Key1, Key2, Key3, Key4);
			}
			
			if (Key1_pre == 1 && Key1 == 0) { // 低速
				Speed = 200;
				Angle = 24;
			} else if (Key2_pre == 1 && Key2 == 0 && Mid >= 80) { // 微调前行方向向左
					Mid -= 1;
			} else if (Key3_pre == 1 && Key3 == 0 && Mid <= 100) { // 微调前行方向向右
					Mid += 1;
			} else if (Key4_pre == 1 && Key4 == 0) { // 高速
				Speed = 400;
				Angle = 12;
			}
			Key1_pre = Key1;
			Key2_pre = Key2;
			Key3_pre = Key3;
			Key4_pre = Key4;
			
			if (Ly <= 1000) { // 前进
				Motor_SetPWM(Stop + Speed);
			} else if (Ly >= 3000) { //后退
				Motor_SetPWM(Back);
			} else { // 停止
				Motor_SetPWM(Stop);
			}
			
			if (Rx <= 1000) { // 左转
				Servo_SetAngle(Mid - Angle);
			} else if (Rx >= 3000) { // 右转
				Servo_SetAngle(Mid + Angle);
			} else { // 回正
				Servo_SetAngle(Mid);
			}
		}
		Delay_ms(100);
	}
}

void remote_control_run(void) {
	NRF24L01_GetRxBuf(Buf);
	Lx = Buf[1] * 256 + Buf[0];
	Ly = Buf[3] * 256 + Buf[2];
	Rx = Buf[5] * 256 + Buf[4];
	Ry = Buf[7] * 256 + Buf[6];
	Key0 = Buf[8] ^ 1;
	Key1 = Buf[9] ^ 1;
	Key2 = Buf[10] ^ 1;
	Key3 = Buf[11] ^ 1;
	Key4 = Buf[12] ^ 1;
	Key5 = Buf[13] ^ 1;
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)		//判断是否是TIM2的更新事件触发的中断
	{
		knt ++;
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);			//清除TIM2更新事件的中断标志位
	}
}