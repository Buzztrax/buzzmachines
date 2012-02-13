#include <math.h>
#include <stdio.h>

//
//	This little program prints
//	the waves2.h header, which
//	contains the waveforms used by
//	the 4fm2f buzz generator. The
//	command line goes:
//
//	4fm2f > waves2.h
//

int main()
{
	int i;
	float t;

	printf("short waveforms[16][4096] = {\n\n\n");

	// SINE WAVE
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Sine\n");
		printf("%d",(int)(sin(2.0 * M_PI /8192.0 * (2*i+1))*32768) );
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");

	// HALF SINE WAVE
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Half-sine\n");
		if (i<2048)
			printf("%d",(int)(sin(2.0 * M_PI /8192.0 * (2*i+1))*32768) );
		else
			printf("0");
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");

	// ABS SINE WAVE
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Abs-sine\n");
		printf("%d",(int)(sin(2.0 * M_PI /16384.0 * (2*i+1))*32768) );
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");

	// ALT SINE WAVE
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Alt-sine\n");
		if (i<2048)
			printf("%d",(int)(sin(2.0 * M_PI /4096.0 * (2*i+1))*32768) );
		else
			printf("0");
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");

	// CAMEL SINE WAVE
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Camel-sine\n");
		if (i<2048)
			printf("%d",(int)(sin(2.0 * M_PI /4096.0 * (2*(i&0x3ff)+1))*32768) );
		else
			printf("0");
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");







	// SAWED SINE WAVE
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Sawed-sine\n");
		if (i<512)
			printf("%d",(int)(sin(2.0 * M_PI /4096.0 * (2*i+1))*32768) );
		else if (i<3584)
			printf("%d",(int)(cos(2.0 * M_PI /12288.0 * (2*i+1-1024))*32768) );
		else
			printf("%d",(int)(sin(2.0 * M_PI /4096.0 * (2*i+1))*32768) );
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");

	// SQUARED SINE WAVE
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Squared-sine\n");
		if (i<512)
			printf("%d",(int)(sin(2.0 * M_PI /4096.0 * (2*i+1))*32768) );
		else if (i<1536)
			printf("32767");
		else if (i<2560)
			printf("%d",(int)(sin(2.0 * M_PI /4096.0 * (2*i+1))*-32768) );
		else if (i<3584)
			printf("-32767");
		else
			printf("%d",(int)(sin(2.0 * M_PI /4096.0 * (2*i+1))*32768) );
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");

	// DUTY SINE WAVE
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Duty-sine\n");
		if (i<512)
			printf("%d",(int)(sin(2.0 * M_PI /4096.0 * (2*i+1))*32768) );
		else if (i<2560)
			printf("32767");
		else
			printf("%d",(int)(sin(2.0 * M_PI /4096.0 * (2*i+1))*32768) );
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");

	// FEEDBACK SINE saw
	t = 0;
	for(i=0;i<4096;i++)
		t = sin(2.0 * M_PI /8192.0 * (2*i+1) - t*0.9);
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Fb-even\n");
		t = sin(2.0 * M_PI /8192.0 * (2*i+1) - t*0.9);
		printf("%d", (int)(t*32768) );
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");

	// FEEDBACK SINE square
	t = 0;
	for(i=0;i<4096;i++)
	{
		if(t<0)
			t *= -1;
		t = sin(2.0 * M_PI /8192.0 * (2*i+1) + t*0.9);
	}
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Fb-odd\n");
		if(t<0)
			t *= -1;
		t = sin(2.0 * M_PI /8192.0 * (2*i+1) + t*0.9);
		printf("%d", (int)(t*32768) );
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");

	// FEEDBACK SINE half
	t = 0;
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Fb-half\n");
		t = sin(2.0 * M_PI /8192.0 * (2*i+1) + t*0.9);
		if(t<0)
			t = 0;
		printf("%d", (int)(t*32768) );
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}

	printf(",\n\n\n");


	// SQUARE WAVE
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Square\n");
		if (i<2048)
			printf("32767");
		else
			printf("-32767");
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");

	// SQUARE WAVE 2
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Square 2\n");
		if (i<1024)
			printf("32767");
		else if (i<3072)
			printf("-32767");
		else
			printf("32767");
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");

	// SAW WAVE, DOWN
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Saw\n");
		printf("%d",32760-i*16);
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");

	// SAW WAVE 2 (UP)
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Saw 2\n");
		if(i<1024)
			printf("%d",i*16+16392);
		else
			printf("%d",i*16-49144);
		if (i!=4095)
			printf(",");
		else
			printf("}");
	}
	printf(",\n\n\n");

	// TRIANGLE WAVE
	for(i=0;i<4096;i++)
	{
		if (i==0)
			printf("{ // Triangle\n");

		if(i<1024)
			printf("%d",i*32+16);
		else if(i<3072)
			printf("%d",65520-i*32);
		else
			printf("%d",i*32-131056);

		if (i!=4095)
			printf(",");
		else
			printf("}");
	}

	printf("};\n\n\n");
}