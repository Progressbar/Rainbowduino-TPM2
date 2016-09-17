/*
 Rainbowduino v3.0 Library examples:

 Sets pixels on 2D plane (8x8 matrix)

*/

#include <Rainbowduino.h>

#define TPM2_STATE_NO_PACKET 0
#define TPM2_STATE_START 1
#define TPM2_STATE_PAYLOAD_SIZEH 2
#define TPM2_STATE_PAYLOAD_SIZEL 3
#define TPM2_STATE_PKT_NUM 6
#define TPM2_STATE_PKT_CNT 7
#define TPM2_STATE_DATA 4
#define TPM2_STATE_COMMAND 14
#define TPM2_STATE_END 5

uint8_t buffer[192];

unsigned char RED[64] = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,238,221,204,188,171,154,137,119,102,85,
68,51,34,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,35,52};

unsigned char GREEN[64] = {0,17,34,51,68,85,102,119,136,153,170,187,204,221,238,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,238,221,204,188,170,154,136,120,102,86,68,52,34,18,0,0,0,0};

unsigned char BLUE[64] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,18,34,52,68,86,102,120,136,154,170,188,
204,221,238,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};

unsigned char plasma[4][4][4];

void updateDisp();

struct tpm2_struct{
  uint16_t payload_size;
  uint16_t recv_cnt;
  uint16_t buff_size;
  uint8_t * buff_p;
  uint8_t state:4;
  uint8_t data_frame:1;
  uint8_t frame_type:1;
};

struct tpm2_struct tpm2;
unsigned char x,y,z,colorshift=0;

void setup()
{
  tpm2.buff_size = 192;
  tpm2.buff_p = buffer;
  
  for(unsigned char x = 0; x < 4; x++)
  {
    for(unsigned char y = 0; y < 4; y++)
    {
      for(unsigned char z = 0; z < 4; z++)
       {
        int color = int(32.0 + (32.0 * sin(x / 1.0))+ 32.0 + (32.0 * sin(y / 1.0)) + 32.0 + (32.0 * sin(z / 1.0))) / 3;
        plasma[x][y][z] = color;      
       }   
    }
  }
  
  Rb.init();
  Serial.begin(115200);
}

void loop()
{
  static uint32_t last_comm = 0;
  static uint32_t standalone_effect_timer = 0;
  uint32_t act_time = millis();
  
  while(Serial.available())
  {
    uint8_t c = Serial.read();
    switch(tpm2.state)
    {
      case TPM2_STATE_START:
        tpm2.state = TPM2_STATE_PAYLOAD_SIZEH;
        if (c == 0xDA){
          tpm2.data_frame = 1;
          break;
        }
        else if (c == 0xC0)
        {
          tpm2.data_frame = 0;
          break;
        }
        tpm2.state = TPM2_STATE_NO_PACKET;
      case TPM2_STATE_NO_PACKET:
        if (c == 0xC9)
        {
          tpm2.state = TPM2_STATE_START;
          tpm2.frame_type = 0;
        }
        else if (c == 0x9C)
        {
          tpm2.state = TPM2_STATE_START;
          tpm2.frame_type = 1;
        }
        break;
      case TPM2_STATE_PAYLOAD_SIZEH:
        tpm2.payload_size = ((uint16_t)c) << 8;
        tpm2.state = TPM2_STATE_PAYLOAD_SIZEL;
        break;
      case TPM2_STATE_PAYLOAD_SIZEL:
        tpm2.payload_size |= c;
        if (tpm2.payload_size)
        {
          
        }
        tpm2.recv_cnt = 0;
        if (tpm2.frame_type)
        {
          tpm2.state = TPM2_STATE_PKT_NUM;
          break;
        }
        
        if (tpm2.data_frame)
          tpm2.state = TPM2_STATE_DATA;
        else
          tpm2.state = TPM2_STATE_COMMAND;
        break;
      case TPM2_STATE_PKT_NUM:
        
        tpm2.state = TPM2_STATE_PKT_CNT;
        break;
      case TPM2_STATE_PKT_CNT:
        
        if (tpm2.data_frame)
          tpm2.state = TPM2_STATE_DATA;
        else
          tpm2.state = TPM2_STATE_COMMAND;
        break;
      case TPM2_STATE_DATA:
        if(tpm2.recv_cnt < tpm2.buff_size)
          tpm2.buff_p[tpm2.recv_cnt] = c;
        if(++tpm2.recv_cnt == tpm2.payload_size)
        tpm2.state = TPM2_STATE_END;
        break;
      case TPM2_STATE_END:
        if (c = 0x36)
        {
          if (tpm2.data_frame)
          {
            updateDisp();
            last_comm = act_time;
          }
        }
        tpm2.state = TPM2_STATE_NO_PACKET;
        break;
      case TPM2_STATE_COMMAND:
        
        if(++tpm2.recv_cnt == tpm2.payload_size)
        tpm2.state = TPM2_STATE_END;
        break;
      default:
        break;
    }
  }
  if (((act_time-5000-last_comm)<0x80000000)&&((act_time-standalone_effect_timer)<0x80000000))
  {
    standalone_effect_timer = act_time + 100;
	last_com = act_time - 10000;
    for(x=0;x<4;x++)  
      for(y=0;y<4;y++)  
        for(z=0;z<4;z++)
          Rb.setPixelZXY(z,x,y,
				(  RED[plasma[x][y][z] + colorshift]) % 256,
				(GREEN[plasma[x][y][z] + colorshift]) % 256,
				( BLUE[plasma[x][y][z] + colorshift]) % 256
			);
    ++colorshift;
  }
}

void updateDisp()
{
  for(uint8_t i = 0; i < 64; i++)
    Rb.setPixelXYsquaref(i>>3,i&0x07,buffer[i*3],buffer[i*3+1],buffer[i*3+2]);
}




