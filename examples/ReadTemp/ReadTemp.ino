#include <OneWire.h>

// OneWire 可适用于 DS18S20, DS18B20, DS1822, DS2438 等单总线通讯器件

//说明：此库只适用于Atom
//定义引脚
int ow = D0;

OneWire  ds(ow);  // 需要接1 - 4.7K 上拉电阻

void setup(void)
{
    Serial.begin(115200);
    ds.begin();
}

void loop(void)
{
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    byte addr[8];
    float celsius, fahrenheit;

    if ( !ds.search(addr))
    {
        Serial.println("No more addresses.");
        Serial.println();
        ds.reset_search();
        delay(250);
        return;
    }

    Serial.print("ROM =");

    for( i = 0; i < 8; i++)
    {
        Serial.write(' ');
        Serial.print(addr[i], HEX);
    }

    if (OneWire::crc8(addr, 7) != addr[7])
    {
        Serial.println("CRC is not valid!");
        return;
    }

    Serial.println();

    //ROM区的第一个字节表示是何种芯片
    switch (addr[0])
    {
        case 0x10:
            Serial.println("  Chip = DS18S20");  // DS1820也可以
            type_s = 1;
            break;
        case 0x28:
            Serial.println("  Chip = DS18B20");
            type_s = 0;
            break;
        case 0x22:
            Serial.println("  Chip = DS1822");
            type_s = 0;
            break;
        case 0x26:
            Serial.println("  Chip = DS2438");
            type_s = 2;
            break;
        default:
            Serial.println("Device is not a DS18x20/DS1822/DS2438 device. Skipping...");
            return;
    }

    ds.reset();
    ds.select(addr);
    ds.write(0x44);        //开始转换
    delay(900);

    present = ds.reset();
    ds.select(addr);
    ds.write(0xB8,0);
    ds.write(0x00,0);

    present = ds.reset();
    ds.select(addr);
    ds.write(0xBE, 0);
    ds.write(0x00,0);

    Serial.print("  Data = ");
    Serial.print(present, HEX);
    Serial.print(" ");
    for ( i = 0; i < 9; i++)
    {  //共9个字节
        data[i] = ds.read();
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.print(" CRC=");
    Serial.print(OneWire::crc8(data, 8), HEX);
    Serial.println();

    //将数据转换为实际温度
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s)
    {
        if (type_s==1)
        {    // DS18S20
            raw = raw << 3; // 默认9 bit的分辨率
            if (data[7] == 0x10) {
                // 计数保留了12bit的分辨率
                raw = (raw & 0xFFF0) + 12 - data[6];
            }
            celsius = (float)raw / 16.0;
        }
        else
        { // type_s==2 for DS2438
            if (data[2] > 127) data[2]=0;
            data[1] = data[1] >> 3;
            celsius = (float)data[2] + ((float)data[1] * .03125);
        }
    }
    else
    {  // DS18B20 and DS1822
        byte cfg = (data[4] & 0x60);

        if (cfg == 0x00) raw = raw & ~7;  // 9 bit 分辨率, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit 分辨率, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit 分辨率, 375 ms
        ////默认12 bit的分辨率，750ms的转换时间
        celsius = (float)raw / 16.0;
    }
    fahrenheit = celsius * 1.8 + 32.0;

    Serial.print("  Temperature = ");
    Serial.print(celsius);
    Serial.print(" Celsius, ");
    Serial.print(fahrenheit);
    Serial.println(" Fahrenheit");
}
