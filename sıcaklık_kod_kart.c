#include <ADUC841.h>
#include <math.h>

//////////////// Tanimlamalar ///////////////////////////////////////

void timer_ayar(void);
void kesme_ayar(void);
void ADC_ayar(void);
void DAC_ayar(void);
void hata_hesapla(void);
void timer_baslat(void);

void DAC0_yaz(int deger);


unsigned char ref_h;
unsigned char ref_l;
unsigned char cikis_h;
unsigned char cikis_l;
unsigned  int ref;
unsigned  int cikis;
unsigned int ref_oku(void);
unsigned int cikis_oku(void);
int hata;
int kontrol_P;
///////////////// kesme ayarlari //////////////////////////////////////

void Timer0_kesmesi(void) interrupt 1 
{ 
    static unsigned int dongu_sayisi = 0; 
    dongu_sayisi++;

    if (dongu_sayisi >=80) 
			{ 				
				////////GOREV //////////////////////
			  
//				ref=ref_oku();    // pot1 deki degeri oku 
//				ref_h=(ref >> 8) & 0xFF;
//				ref_l= ref & 0xFF;
//				
//				cikis=cikis_oku();  // pot2 deki degeri oku
//				cikis_h=(cikis >> 8)& 0xFF;
//				cikis_l= cikis & 0xFF;
//				
//				hata=ref-cikis;
//				

				
				DAC0_yaz(cikis); 
				
				
		
				
				/////////////////////////////////////
			  dongu_sayisi = 0;
			 }

    TH0 = 0xD4;        // Timer tekrar baslatilmasi için degerler
    TL0 = 0xCC;
    TF0 = 0;           // Timer0 bayragini temizle
}
unsigned int ref_oku(void) 
{
		unsigned int ref;
	
	  ADCCON2=0x00; //adc 0 kanali seçtik.
	  SCONV=1;
	  while(SCONV==1){};
		
		
		ref=((ADCDATAH & 0x0f)<<8) + ADCDATAL;  
	  return ref;  
}	

unsigned int cikis_oku(void) 
{
		unsigned int out;
	  ADCCON2=0x01; //adc 1 kanali seçtik.
	  SCONV=1;
	  while(SCONV==1){};
		
		out=((ADCDATAH & 0x0f)<<8) + ADCDATAL;   
	  return out;  

}
void DAC0_yaz(int deger)
{     //Dac0=Kontrol_p i yazdiriyoruz
	if(deger<0)
	{
		deger=0;
	}
	if(deger>2048)
	{
		deger=2048;
	}
	//deger=2048;
	DAC0H=((deger >> 8) & 0x000F);
	DAC0L=deger;
}
///////////////////////////////////////////////////////////////////////////
void main(void)
{
		 
	// kp,ki ilk degerleri ver
		timer_ayar();
		kesme_ayar();
		ADC_ayar();  
		DAC_ayar();
		timer_baslat();
	


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
	 DACCON=0x1F; // dac0 (0000-1001 b)(senkron kapali)
}

void timer_baslat(void)
{
	 TR0 = 1;         // Timer0 baslat
}
