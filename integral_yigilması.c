#include <ADUC841.h>
#include <math.h>
//////////////// Tanimlamalar ///////////////////////////////////////
sbit LED = P0^1;  // LED P0.1 pinine tanimlandi
sbit led_ayar=P2^3;
sbit buton_2= P2^1;

void timer_ayar(void);
void kesme_ayar(void);
void ADC_ayar(void);
void DAC_ayar(void);
void hata_hesapla(void);
void timer_baslat(void);
void DAC0_yaz(int deger);
int PID_hesapla(int ref, int cikis);

unsigned char ref_h;
unsigned char ref_l;
unsigned char cikis_h;
unsigned char cikis_l;
unsigned char hata_h;
unsigned char hata_l;
unsigned char kontrol_int_h;
unsigned char kontrol_int_l;

unsigned int ref;
unsigned int cikis;
unsigned int ref_oku(void);
unsigned int cikis_oku(void);

int hata, kontrol_int;
float kontrol_P , kontrol_I , kontrol_D , kontrol , kontrol_temp = 0 ;
int hata_eski=0;
int hata_toplam=0;

//***********PID katsayilari****
float Kp = 0.071;
float Ki = 0.0773;
float Kd = -3.1525;

float katsayi;
float farkalma=0;
float anti_windup_sonucu_hata;

//***************** kesme ayarlari ***************************

void Timer0_kesmesi(void) interrupt 1 
{ 
    static unsigned int dongu_sayisi = 0; 
    dongu_sayisi++;

    if (dongu_sayisi >=6400) 
			{ 				
				////////GOREV //////////////////////
				LED = ~LED;  
				
				ref=ref_oku();    // pot1 deki degeri oku 
				cikis=cikis_oku();  // pot2 deki degeri oku
				
				ref_h=(ref >> 8) & 0xFF;
				ref_l= ref & 0xFF;
				cikis_h=(cikis >> 8)& 0xFF;
				cikis_l= cikis & 0xFF;
				
				hata=ref-cikis;
				hata_h=(hata >> 8)& 0xFF;
				hata_l= hata & 0xFF;
				
				kontrol_int = PID_hesapla(ref,cikis);
				
				kontrol_int_h=(kontrol_int >> 8) & 0xFF;
				kontrol_int_l= kontrol_int & 0xFF;
				
				//kontrol_int=ref; //açik çevrim çalisma
				DAC0_yaz(kontrol_int);
				
			  dongu_sayisi = 0;
			 }

    TH0 = 0xD4;        // Timer tekrar baslatilmasi için degerler
    TL0 = 0xCC;
    TF0 = 0;           // Timer0 bayragini temizle
}

//************** ADC okuma fonksiyonlari **********************
unsigned int ref_oku(void) 
{
		unsigned int ref;
	
	  ADCCON2=0x00; // adc0 sicaklik ref, adc7 hiz ref
	  SCONV=1;
	  while(SCONV==1){};
		
		
		ref=((ADCDATAH & 0x0f)<<8) + ADCDATAL;  
	  return ref;  
}	

unsigned int cikis_oku(void) 
{
		unsigned int out;
	  ADCCON2=0x02; // adc2 sicaklik cikis, adc2 hiz ref
	  SCONV=1;
	  while(SCONV==1){};
		
		out=((ADCDATAH & 0x0f)<<8) + ADCDATAL;   
	  return out;  
}

//********************* DAC yazma fonksiyonu ************************ 
void DAC0_yaz(int deger)
{     //Dac0=Kontrol_p i yazdiriyoruz
	if(deger<0)
	{
		deger=0;
	}
	if(deger>4095)
	{
		deger=4095;
	}
	//deger=2048;
	DAC0H=((deger >> 8) & 0x000F);
	DAC0L=deger;
}
//***********************************************
void main(void)
{
		//while(buton_2==1){};
	
		timer_ayar();
		kesme_ayar();
		ADC_ayar();  
		DAC_ayar();
		timer_baslat();
	
		TR1=1;  // tmr baslat

		while (1){};
}
void timer_ayar(void)
{
	 TMOD = 0x01;      // Timer0 mod 1 (16-bit timer)
   TH0 = 0xD4;      // 65535 - 11059 = 54476 -> D4CC (hex)
   TL0 = 0xCC;
}
void kesme_ayar(void)
{
	 ET0 =1;         // Timer0 kesmesini aktif et
   EA = 1;          // Genel kesmeleri aktif et
}
void ADC_ayar(void)
{
	 ADCCON1=0xFC; //1(enerji)0(dahili)00 0011   dahili adc kullandim
}
void DAC_ayar(void)
{
	 DACCON=0x7F; // dac0 (0000-1001 b)(senkron kapali)
}

void timer_baslat(void)
{
	 TR0 = 1;         // Timer0 baslat
}

 //////////////// PID hesap fonksiyonu //////////////
int PID_hesapla(int ref, int cikis)
{
	hata = ref - cikis;

	// P ve D terimleri direkt hesaplanir
	kontrol_P = Kp * hata;
	kontrol_D = Kd * (hata - hata_eski);
	//katsayi =2.5;
	katsayi = sqrt(abs(Ki/Kd))*100; 
	// Kontrol çikisi sinir içinde kaliyorsa integral terimi biriktir
	if (kontrol_temp >= 0 && kontrol_temp <= 4095)
	{
		hata_toplam += hata;
	}
	// Aksi halde integral yigilmasi olabilecegi için toplama yapma
	farkalma = kontrol_int - kontrol;  
	anti_windup_sonucu_hata += hata+ (farkalma * katsayi);
	
	// Önce geçici kontrol sinyali olusturulur (I terimi hariç)
	kontrol_temp = kontrol_P + kontrol_D + Ki * hata_toplam;

	// Integral terimini ayri hesapla
	kontrol_I = Ki * anti_windup_sonucu_hata;

	// Tam kontrol sinyalini hesapla
	kontrol = kontrol_P + kontrol_I + kontrol_D;

	// Çikisi sinirla
	if (kontrol_int < 0)
		kontrol_int = 0;
	if (kontrol_int > 4095)
		kontrol_int = 4095;

	// Eski hata güncelle
	hata_eski = hata;

	kontrol_int = (int)kontrol;
	return kontrol_int;
}
